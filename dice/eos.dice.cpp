#include <dice/eos.dice.hpp>
#include <eosiolib/time.hpp>

using namespace eosio;

namespace
{
    using namespace dice::tables;
    using namespace dice::config;

    bool filter_bet_transactions(eosio::name owner, const dice::tables::Config& cfg, const common::tables::TokenTransfer& transfer)
    {
        if(transfer.from == owner ||
            transfer.from == cfg.admin)
        {
            return false;
        }
        if(transfer.to != owner)
        {
            return false;
        }
        bool is_bet = common::startsWith(transfer.memo, "bet");
        if(!is_bet)
        {
            return false;
        }
        auto& ref = transfer.quantity;
        if(!ref.is_valid() || ref.symbol != common::EOS_SYMBOL || ref.amount <= 0)
        {
            return false;
        }
        return true;
    }

    bool filter_replenishment_transactions(eosio::name owner, const dice::tables::Config& cfg,
            const common::tables::TokenTransfer& transfer)
    {
        if(transfer.from == owner ||
            transfer.from == cfg.owner)
        {
            return false;
        }
        if(transfer.to != owner)
        {
            return false;
        }
        auto& ref = transfer.quantity;
        if(!ref.is_valid() || ref.symbol != common::EOS_SYMBOL || ref.amount <= 0)
        {
            return false;
        }
        return true;
    }

    template<class T>
    void add_bet_record(T& table, const eosio::name& payer, TableId& tbl_id, const eosio::name& player,
            const eosio::asset& bet, const eosio::asset& reward, uint8_t roll_type, uint16_t roll_border,
            uint16_t roll_value, const capi_checksum256& seed, const eosio::name& inviter)
    {
        log("add_bet_record\n");
        auto id = tbl_id.next();
        auto it = table.find(id);
        if(it == table.end())
        {
            log("before emplace bet history\n");
            table.emplace(payer, [&](auto& record)
            {
                record.id = id;
                record.player = player;
                record.roll_type = roll_type;
                record.roll_border = roll_border;
                record.roll_value = roll_value;
                record.bet = bet;
                record.payout.push_back(reward);
                record.seed = seed;
                record.inviter = inviter;
                record.time = eosio::time_point(eosio::seconds(now()));
            });
        }
        else
        {
            log("before modify bet history\n");
            table.modify(it, payer, [&](auto& record)
            {
                record.id = id;
                record.player = player;
                record.roll_type = roll_type;
                record.roll_border = roll_border;
                record.roll_value = roll_value;
                record.bet = bet;
                record.payout.clear();
                record.payout.push_back(reward);
                record.seed = seed;
                record.inviter = inviter;
                record.time = eosio::time_point(eosio::seconds(now()));
            });
        }
        // remove start record if current frame size > max
        if(tbl_id.last - tbl_id.first + 1 > tbl_id.max)
        {
            //just remove first by primary key
            table.erase(table.begin());
            ++tbl_id.first;
        }
    }

    void update_player_bets_statistics(dice::tables::PlayerBetsStatistics& stats, const eosio::asset& bet,
            const eosio::asset& reward, eosio::time_point last_bet_time = eosio::time_point(eosio::seconds(0)),
            uint32_t period_in_sec = 0)
    {
        log("update_player_bets_statistics(%, %)\n", bet, reward);
        if(0 != period_in_sec)
        {
            // 0 means non resetable statistics
            auto time_now = eosio::time_point(eosio::seconds(now()));
            auto current_period_number = time_now.sec_since_epoch() / period_in_sec;
            auto start = eosio::time_point(eosio::seconds(current_period_number * period_in_sec));
            if(start > last_bet_time)
            {
                stats.reset();
            }
        }

        ++stats.bets;
        stats.total_bet_amount += bet.amount;
        if (reward.amount > 0)
        {
            ++stats.wons;
            stats.total_payout += reward.amount;
        }
    }

    auto update_player_statistics(dice::tables::Config& config, dice::tables::Players& table, const eosio::name& player,
            const eosio::asset& bet, const eosio::asset& reward)
    {
        // reset obsolete
        auto it = table.find(player.value);
        if(table.end() == it)
        {
            log("before emplace player statistics\n");
            it = table.emplace(config.owner, [&](auto& p)
            {
                p.account = player;
                p.total.reset();
                p.day.reset();
                p.week.reset();
                p.month.reset();
                p.jackpot_sequence = -1;
                p.jackpot_sequence_values = "";
                update_player_bets_statistics(p.total, bet, reward);
                update_player_bets_statistics(p.day, bet, reward);
                update_player_bets_statistics(p.week, bet, reward);
                update_player_bets_statistics(p.month, bet, reward);
                p.last_bet_time = eosio::time_point(eosio::seconds(now()));
                p.last_bet = bet;
                p.last_payout = reward;
            });
        }
        else
        {
            log("before modify player statistics\n");
            auto previous_bet_time = it->last_bet_time;
            table.modify(it, config.owner, [&](auto& p)
            {
                //update statistics
                update_player_bets_statistics(p.total, bet, reward);
                update_player_bets_statistics(p.day, bet, reward, previous_bet_time, one_day_in_seconds);
                update_player_bets_statistics(p.week, bet, reward, previous_bet_time, one_week_in_seconds);
                update_player_bets_statistics(p.month, bet, reward, previous_bet_time, one_month_in_seconds);
                p.last_bet_time = eosio::time_point(eosio::seconds(now()));
                p.last_bet = bet;
                p.last_payout = reward;
            });
        }
        return it;
    }

    double get_bonus_multiplier(const dice::tables::AnteBonusesConfig& table,
            const dice::tables::PlayerBetsStatistics& day_stats)
    {
        auto bets_count = day_stats.bets;
        for (auto& row: table )
        {
            if(row.begin <= bets_count && bets_count <= row.end)
            {
                return row.multiplier;
            }
        }
        return 1.0;
    }

}

namespace dice {

Dice::Dice(eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds)
        : contract(receiver, code, ds),
          _globalConfig(_self, _self.value),
          _diceLimits(_self, _self.value),
          _betTokens(_self, common::EOS_SYMBOL.raw()),
          _bonusesConfig(_self, _self.value),
          _bets(_self, _self.value),
          _highBets(_self, _self.value),
          _rareBets(_self, _self.value),
          _players(_self, _self.value),
          _jackpots(_self, _self.value),
          _referrals(_self),
          _leaderBoards(_self, _stateConfig)
{
    log("Dice Constructor started\n");
    if (!_globalConfig.exists())
    {//on first call
        _stateConfig = config::init_main_config(_self);
        _stateLimits = config::init_dice_limits();
        _stateEosToken = config::init_bet_token();
        config::init_ante_bonuses(_self, _bonusesConfig);
    }
    else
    {
        _stateConfig = _globalConfig.get();
        _stateLimits = _diceLimits.get();
        _stateEosToken = _betTokens.get();
    }
    _referrals.setBonusMultiplier(_stateConfig.referral_multiplier);
    _leaderBoards.refresh();

    log("Dice Constructor finished\n");
}

Dice::~Dice()
{
    log("Dice destructor started\n");
    _globalConfig.set(_stateConfig, _self);
    _diceLimits.set(_stateLimits, _self);
    _betTokens.set(_stateEosToken, _self);
    log("Dice destructor finished\n");
}

void Dice::setAdmin(eosio::name caller, eosio::name admin)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot change admin.");
    eosio_assert(is_account(admin), "Unregistered 'admin' account.");
    _stateConfig.admin = admin;
}

void Dice::setBettingEnabled(eosio::name caller, bool enabled)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    _stateConfig.enabled_betting = enabled;
}

void Dice::setMintingEnabled(eosio::name caller, bool enabled)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    _stateConfig.enabled_minting = enabled;
}

void Dice::setPayoutEnabled(eosio::name caller, bool enabled)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    _stateConfig.enabled_payout = enabled;
}

void Dice::setAnteTokenAccount(eosio::name caller, eosio::name name)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    _stateConfig.ante_token = name;
}

void Dice::setGameParams(eosio::name caller, uint16_t min, uint16_t max, uint16_t max_bet_num)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    _stateLimits.min_value = min;
    _stateLimits.max_value= max;
    _stateLimits.max_bet_num = max_bet_num;
}

void Dice::setMinBet(eosio::name caller, eosio::asset min_bet)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(min_bet.is_valid(), "Wrong protect value.");
    eosio_assert(min_bet.symbol == common::EOS_SYMBOL , "Wrong protect value.");
    eosio_assert(min_bet.amount > 0 , "Wrong protect value.");
    _stateLimits.min_bet = min_bet;
}

void Dice::setExchangeRate(eosio::name caller, double rate)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(rate > 0, "Wrong exchange rate");
    _stateConfig.ante_in_eos = rate;
}

void Dice::setPlatformFee(eosio::name caller, double platform_fee)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(platform_fee > 0, "Wrong exchange rate");
    _stateLimits.platform_fee = platform_fee;
}

void Dice::setBalanceValue(eosio::name caller, eosio::asset balance)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(balance.is_valid(), "Wrong protect value.");
    eosio_assert(balance.symbol == common::EOS_SYMBOL , "Wrong protect value.");
    eosio_assert(balance.amount > 0 , "Wrong protect value.");
    _stateConfig.eos_balance = balance;
}

void Dice::setBalanceProtect(eosio::name caller, eosio::asset balance_protect)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(balance_protect.is_valid(), "Wrong protect value.");
    eosio_assert(balance_protect.symbol == common::EOS_SYMBOL , "Wrong protect value.");
    eosio_assert(balance_protect.amount > 0 , "Wrong protect value.");
    _stateLimits.balance_protect = balance_protect;
}

void Dice::setMaxBetPercent(eosio::name caller, double max_bet_percent)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(max_bet_percent > 0, "Wrong max_bet_percent");
    _stateLimits.max_bet_percent = max_bet_percent;
}

void Dice::setBetsHistoryLength(eosio::name caller, uint64_t size)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(size >= 1, "Bet history length must be greater than 0.");
    _stateConfig.bets_id.max = size;
}

void Dice::setRareBetsHistoryLength(eosio::name caller, uint64_t size)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(size >= 1, "Bet history length must be greater than 0.");
    _stateConfig.rare_bets_id.max = size;
}

void Dice::setHighBetsHistoryLength(eosio::name caller, uint64_t size)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(size >= 1, "Bet history length must be greater than 0.");
    _stateConfig.high_bets_id.max = size;
}

uint8_t Dice::get_winners(uint8_t roll_type, uint16_t roll_border)
{
    uint8_t num;
    if (roll_type == RollType::LEFT)
    {
        num = roll_border;
    }
    else if (roll_type == RollType::RIGHT)
    {
        num = _stateLimits.max_bet_num - 1 - roll_border;
    }
    return num;
}

eosio::asset Dice::get_bet_reward(uint8_t roll_type, uint16_t roll_border, const eosio::asset& quantity)
{
    auto num = get_winners(roll_type, roll_border);
    eosio_assert(num != 0, "Wrong configuration: _stateLimits.max_bet_num = 1 + roll_border");
    auto reward = eosio::asset{
        int64_t(quantity.amount * (1 - _stateLimits.platform_fee) * (100.0 / num)),
        common::EOS_SYMBOL
    };
    log("DEBUG: quantity=%, num=%, %, %, %, %\n", quantity, num,
            (1 - _stateLimits.platform_fee),
            (100 / num),
            (1 - _stateLimits.platform_fee) * (100.0 / num),
            reward);
    return reward;
}

void Dice::on_replenishment(const common::tables::TokenTransfer& data)
{
    _stateConfig.eos_balance += data.quantity;
}

void Dice::notify(std::string)
{
    require_auth(_self);
    require_recipient(_stateConfig.admin);
    if(_stateConfig.owner != _stateConfig.admin)
    {
        require_recipient(_stateConfig.owner);
    }
}

void Dice::on_bet(const common::tables::TokenTransfer& data)
{
    log("on_bet\n");
    _stateConfig.eos_balance += data.quantity;
    eosio_assert(_stateConfig.eos_balance >= _stateLimits.balance_protect, "Game under maintain, stay tuned.");
    eosio_assert(data.quantity.amount <= _stateConfig.eos_balance.amount * _stateLimits.max_bet_percent,
                 "Bet amount exceeds max amount.");

    std::vector <std::string> pieces;
    common::split(data.memo, pieces, ",");
    eosio_assert(pieces.size() >= 3, "Wrong memo parameter.");
    eosio_assert(!pieces[1].empty(), "Roll type cannot be empty!");
    eosio_assert(!pieces[2].empty(), "Roll prediction cannot be empty!");

    uint8_t roll_type = atoi(pieces[1].c_str());
    eosio_assert(roll_type == RollType::LEFT || roll_type == RollType::RIGHT, "Unsupported roll type.");

    uint16_t roll_border = atoi(pieces[2].c_str());
    if (roll_type == RollType::LEFT) {
        eosio_assert(roll_border <= _stateLimits.max_value, "Bet border must be <= MAX value.");
    } else if (roll_type == RollType::RIGHT) {
        eosio_assert(roll_border >= _stateLimits.min_value, "Bet border must >= MIN value.");
    }

    eosio::name inviter;
    if ((pieces.size() == 2) || pieces[3].empty())
    {
        //use the same account as better
        inviter = data.from;
    }
    else
    {
        inviter = eosio::name(pieces[3]);
    }

    eosio_assert(data.quantity >= _stateLimits.min_bet, "Bet must be >= min_bet.");
    auto max_possible_reward = get_bet_reward(roll_type, roll_border, data.quantity);

    std::string msg = "Bet reward must be between ";
    msg += _stateLimits.min_bet.to_string();
    msg += " and ";
    msg += max_possible_reward.to_string();
    eosio_assert(max_possible_reward.amount <= (_stateConfig.eos_balance.amount * _stateLimits.max_bet_percent), msg.c_str());
    log("DEBUG: before call bet(%,%,%,%,%)\n", data.from, inviter, data.quantity, roll_type, roll_border);
    eosio::transaction deferred;
    deferred.actions.emplace_back(
            permission_level{_self, "active"_n},
            _self, "bet"_n,
            std::make_tuple(
                    data.from,
                    inviter,
                    data.quantity,
                    roll_type,
                    roll_border
            )
    );
    uint128_t deferred_id = _stateConfig.next_deferred_id(TransactionNumber::BET);
    deferred.delay_sec = 1;
    deferred.send(deferred_id, _self);
}

void Dice::on_error(eosio::onerror& error)
{
    log("on_error(eosio::onerror& error)\n");
    auto error_trx = error.unpack_sent_trx();
    std::string msg = "Action(s) failed: ";
    for(auto& action : error_trx.actions)
    {
        msg += action.name.to_string();
        msg += "|";

        if(action.name == "distribute"_n)
        {
            _leaderBoards.on_distribution_failed(action);
        }
        else if(action.name == "bet"_n)
        {
            log("ERROR: `bet` action failed\n");
        }
        else if(action.name == "resolved"_n)
        {
            log("ERROR: `resolved` action failed\n");
        }
        else if(action.name == "mint"_n)
        {
            log("ERROR: `mint` action failed\n");
        }
    }

    eosio::action(
            eosio::permission_level{_self, "active"_n},
            _self,
            "notify"_n,
            std::make_tuple(msg)
    ).send();
}

void Dice::on_transfer()
{
    log("on_transfer catched\n");
    /*
     possible memo format:
     1. starts from bet
        bet,roll_type,roll_value,inviter   - bet with inviter
        bet,roll_type,roll_value           - bet without inviter

     2. any other memo means "balance replenishment"
        just increase eos_balance
     */
    auto data = unpack_action_data<common::tables::TokenTransfer>();
    if (filter_bet_transactions(_self, _stateConfig, data))
    {
        log("filtered bet transaction\n");
        on_bet(data);
    }
    else if (filter_replenishment_transactions(_self, _stateConfig, data))
    {
        log("filtered incoming transaction\n");
        on_replenishment(data);
    }
}

void Dice::makeBet(eosio::name player, eosio::name inviter, eosio::asset quantity, uint8_t roll_type,
        uint16_t roll_border)
{
    log("makeBet(%,%,%,%,%)\n", player, inviter, quantity, roll_type, roll_border);
    require_auth(_self);
    if(_stateConfig.enabled_betting)
    {
        eosio::transaction deferred;
        deferred.actions.emplace_back(
                permission_level{_self, "active"_n},
                _self, "resolved"_n,
                std::make_tuple(
                        player,
                        inviter,
                        quantity,
                        roll_type,
                        roll_border
                )
        );
        deferred.delay_sec = 2;
        uint128_t deferred_id =  _stateConfig.next_deferred_id(TransactionNumber::RESOLVED);
        deferred.send(deferred_id, _self);
    }
}

void Dice::pay_for_win(const eosio::name& player, const eosio::asset& quantity, std::string& message)
{
    log("pay_for_win(%, %)\n", player, quantity);
    log("DEBUG: win -> %\n", message.c_str());
    if(_stateConfig.enabled_payout)
    {
        action(
                permission_level{_self, "active"_n},
                "eosio.token"_n,
                "transfer"_n,
                std::make_tuple(
                        _stateConfig.owner,
                        player,
                        quantity,
                        message
                )
        ).send();
    }
    _stateConfig.eos_balance -= quantity;
    _stateConfig.total_payout += quantity;
    _stateEosToken.out += quantity.amount;
}

void Dice::register_bet(const eosio::name& player, const eosio::asset& bet, const eosio::asset& reward,
        uint8_t roll_type, uint16_t roll_border, uint16_t roll_value, const eosio::name& inviter)
{
    log("register_bet(%, %, %, %, %, %, %)\n",
            player, bet, reward, roll_type, roll_border, roll_value, inviter);

    log("DEBUG: store record to bets.all\n");
    add_bet_record(_bets, _self, _stateConfig.bets_id, player, bet, reward, roll_type, roll_border, roll_value,
            _seed, inviter);

    log("DEBUG: store record to bets.high\n");
    if(bet >= _stateConfig.high_bet_bound)
    {
        add_bet_record(_highBets, _self, _stateConfig.high_bets_id, player, bet, reward, roll_type, roll_border,
                       roll_value, _seed, inviter);
    }
    log("DEBUG: store record to bets.rare\n");
    auto num = get_winners(roll_type, roll_border);
    if(reward.amount > 0 && num <= _stateConfig.rare_bet_bound)
    {
        add_bet_record(_rareBets, _self, _stateConfig.rare_bets_id, player, bet, reward, roll_type, roll_border,
                roll_value, _seed, inviter);
    }
    log("DEBUG: update record in 'players' table \n");
    auto playerIt = update_player_statistics(_stateConfig, _players, player, bet, reward);
    _stateConfig.total_bet_amount += bet;
    _leaderBoards.update_player_stats(*playerIt);
}

void Dice::setHighBetBound(eosio::name caller, eosio::asset high_bet_bound)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(high_bet_bound.is_valid(), "Wrong protect value.");
    eosio_assert(high_bet_bound.symbol == common::EOS_SYMBOL , "Wrong protect value.");
    eosio_assert(high_bet_bound.amount > 0 , "Wrong protect value.");
    _stateConfig.high_bet_bound = high_bet_bound;
}

void Dice::setRareBetBound(eosio::name caller, uint16_t rare_bet_bound)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(rare_bet_bound > 0 && rare_bet_bound < 100, "Wrong rare_bet_bound parameter.");
    _stateConfig.rare_bet_bound = rare_bet_bound;
}

void Dice::send_to_jackpot_game(const eosio::name& player, const eosio::asset& quantity, uint64_t roll_value)
{
    log("send_to_jackpot_game(%, %)\n", player, roll_value);
    require_auth(_self);

    _stateConfig.jackpot_balance.amount += quantity.amount*_stateConfig.jackpot_percent;
    log("DEBUG: Jackpot %\n", _stateConfig.jackpot_balance.amount);

    auto it = _players.find(player.value);

    if (_players.end() == it) { return; }

    if ((*it).jackpot_sequence == 5) {
        _players.modify(it, _stateConfig.owner, [&](auto& record)
        {
            record.jackpot_sequence = -1;
            record.jackpot_sequence_values = "";
        });
    }

    uint8_t sequence = roll_value/10;
    //uint8_t sequence = (*it).jackpot_sequence + 1;
    int player_sequence = (*it).jackpot_sequence;

    log("DEBUG: jackpot sequence %, roll %\n", sequence, roll_value);
    log("DEBUG: jackpot player sequence %\n", player_sequence);

    if (player_sequence + 1 == sequence) {
        _players.modify(it, _stateConfig.owner, [&](auto& record)
        {
            record.jackpot_sequence = sequence;
            record.jackpot_sequence_values.append(std::to_string(roll_value));
            record.jackpot_sequence_values.append(";");
        });
        if (sequence == 5) {
            log("JACKPOT\n");

            _jackpots.emplace(_stateConfig.owner, [&](auto& record)
            {
                record.id = _jackpots.available_primary_key();
                record.player = player;
                record.time = eosio::time_point(eosio::seconds(now()));
                record.amount = _stateConfig.jackpot_balance;
            });

            std::string msg = "Congradulations! You're JACKPOT winner! Receive your ";
            msg.append(std::to_string(_stateConfig.jackpot_balance.amount));
            msg.append(" EOS prize");

            pay_for_win(player, _stateConfig.jackpot_balance, msg);
            _stateConfig.jackpot_balance = eosio::asset(0, common::EOS_SYMBOL);
            _stateConfig.jackpot_balance.amount = 0;
        }
    } 
    else 
    {
        _players.modify(it, _stateConfig.owner, [&](auto& record)
        {
            record.jackpot_sequence = -1;
            record.jackpot_sequence_values = "";
        });
    }
}

void Dice::mint_tokens(const eosio::name& player, const eosio::asset& bet, const eosio::asset& reward,
        const eosio::name& inviter)
{
    log("mint_tokens(%, %, %, %)\n", player, bet, reward, inviter);
    require_auth(_self);
    auto it = _players.find(player.value);
    eosio_assert(it != _players.end(), "Logic error.");
    log("DEBUG: day.bets=%\n", it->day.bets);
    auto multiplier = get_bonus_multiplier(_bonusesConfig, it->day);
    int64_t ante_count = ((double)bet.amount / _stateConfig.ante_in_eos) * multiplier;
    auto mint_amount = eosio::asset{ante_count, common::ANTE_SYMBOL};
    log("DEBUG: mint_amount=% bonus_multiplier=% bet=%\n", mint_amount, multiplier, bet);
    if(_stateConfig.enabled_minting && _stateConfig.ante_token != _self)
    {
        eosio::transaction deferred;
        deferred.actions.emplace_back(
                permission_level{_self, "active"_n},
                _stateConfig.ante_token, "mint"_n,
                std::make_tuple(
                        _stateConfig.admin,
                        mint_amount,
                        player,
                        inviter.value
                )
        );
        deferred.delay_sec = 1;
        uint128_t deferred_id =  _stateConfig.next_deferred_id(TransactionNumber::MINT);
        deferred.send(deferred_id, _self);
    }
}

void Dice::resolveBet(eosio::name player, eosio::name inviter, eosio::asset quantity, uint8_t roll_type,
                   uint16_t roll_border)
{
    log("resolveBet(%, %, %, %, %)\n", player, inviter, quantity, roll_type, roll_border);
    require_auth(_self);
    eosio_assert(quantity.amount > 0, "Wrong quantity.");
    uint64_t roll_value = get_random(_stateLimits.max_value);

    bool is_win = (roll_type == RollType::LEFT && roll_value < roll_border) ||
            (roll_type == RollType::RIGHT && roll_value > roll_border);
    eosio::asset reward{0, common::EOS_SYMBOL};
    _stateEosToken.in += quantity.amount;
    ++_stateEosToken.bets;
    if (is_win)
    {
        reward = get_bet_reward(roll_type, roll_border, quantity);

        log("win detected. reward=%\n", reward);
        std::string msg = "You win! Your bet seed was: ";
        msg.append((const char*)_seed.hash);
        pay_for_win(player, reward, msg);

        _stateEosToken.out += reward.amount;
        _stateEosToken.wons += 1;
    }
    register_bet(player, quantity, reward, roll_type, roll_border, roll_value, inviter);
    _referrals.on_player_bet(player, inviter, quantity, reward);
    send_to_jackpot_game(player, quantity, roll_value);
    mint_tokens(player, quantity, reward, inviter);
}

uint64_t Dice::get_random(uint64_t max)
{
    auto sseed = _random.create_sys_seed(0);
    auto size = transaction_size();
    char buffer[size];
    read_transaction(&buffer[0], size);
    capi_checksum256 checksum;
    sha256(buffer, size, &checksum);
    printhex(&checksum, sizeof(checksum));
    print("\n");
    _random.seed(sseed, checksum);
    uint64_t result = _random.generator(max);
    log("get_random(%)->%\n", max, result);
    _seed = _random.get_seed();
    return result;
}

void Dice::distributeLeadersBonuses(eosio::name caller, uint8_t type, const std::vector<eosio::name>& leaders, eosio::asset bonus)
{
    _leaderBoards.distributeLeadersBonuses(caller, type, leaders, bonus);
}

void Dice::setDayLeaderPercent(eosio::name caller, double percent)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(percent > 0, "percent > 0 expected");
    _stateConfig.day_leader_board.bonus_percent = percent;
}

void Dice::setMonthLeaderPercent(eosio::name caller, double percent)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(percent > 0, "percent > 0 expected");
    _stateConfig.month_leader_board.bonus_percent = percent;
}

void Dice::setJackpotPercent(eosio::name caller, double percent)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(percent > 0, "percent > 0 expected");
    _stateConfig.jackpot_percent = percent;
}

void Dice::setRefferalMultiplier(eosio::name caller, double multiplier)
{
    require_auth(caller);
    eosio_assert(_stateConfig.admin == caller || _stateConfig.owner == caller, "You cannot call this function.");
    eosio_assert(multiplier > 0, "multiplier > 0 expected");
    _stateConfig.referral_multiplier = multiplier;
}

}//dice

