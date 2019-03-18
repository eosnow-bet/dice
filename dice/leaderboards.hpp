#pragma once

#include <dice/common.hpp>
#include <dice/tables.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/name.hpp>
#include <string>
#include <math.h>

namespace dice {

enum LeaderBoardType: uint8_t
{
    DAY = 1,
    MONTH = 2,
};

class LeaderBoards
{
    using LeaderBoardConfig = dice::tables::LeaderBoardConfig;
    using Config = dice::tables::Config;

    eosio::name _owner;
    Config& _config;
    tables::BetsCountDayTop _dayTop;
    tables::BetsCountMonthTop _monthTop;

    template<uint8_t TYPE> LeaderBoardConfig& get_config_ref();
    template<uint8_t TYPE> auto& get_leader_board();
    template<uint8_t TYPE> auto get_leader_board_index();

    template<>LeaderBoardConfig& get_config_ref<LeaderBoardType::DAY>(){return _config.day_leader_board;};
    template<>LeaderBoardConfig& get_config_ref<LeaderBoardType::MONTH>(){return _config.month_leader_board;};
    template<> auto& get_leader_board<LeaderBoardType::DAY>(){return _dayTop;}
    template<> auto& get_leader_board<LeaderBoardType::MONTH>(){return _monthTop;}

    template<> auto get_leader_board_index<LeaderBoardType::DAY>(){return _dayTop.get_index<"bybetsamount"_n>();}
    template<> auto get_leader_board_index<LeaderBoardType::MONTH>(){return _monthTop.get_index<"bybetsamount"_n>();}

    template<class T>
    std::vector<eosio::name> get_leaders(T& table, const LeaderBoardConfig& cfg)
    {
        std::vector<eosio::name> out;
        out.reserve(cfg.size);
        for(auto it = table.rbegin();it != table.rend();++it)
        {
            out.push_back(it->account);
            if(out.size() >= cfg.size)
                break;
        }
        return out;
    }

    template<class T>
    void clean_board(T& board)
    {
        while(board.begin() != board.end())
        {//clean leader board
            board.erase(board.begin());
        }
    }

    void check_expired_distribution(LeaderBoardConfig& cfg)
    {
        if(cfg.is_distribution_expired(config::distribution_expire_in_seconds))
        {
            log("DEBUG: expired distribution detected\n");
            std::string msg = "Expired leader board distribution detected. period_length=";
            msg += std::to_string(cfg.period_length);
            eosio::transaction deferred;
            deferred.actions.emplace_back(
                    eosio::permission_level{_owner, "active"_n},
                    _owner, "notify"_n,
                    std::make_tuple(msg)
            );
            deferred.delay_sec = 1;
            auto distribution_id = _config.next_deferred_id(TransactionNumber::NOTIFY);
            deferred.send(distribution_id, _owner);
            cfg.stop_distribution();
        }
    }

    template<uint8_t TYPE>
    void check_leader_board()
    {
        log("check leader board % \n", TYPE);
        auto& cfg = get_config_ref<TYPE>();
        check_expired_distribution(cfg);
        if(!cfg.is_period_ended() || cfg.is_distribution_active())
        {
            /*
                skip this if
                - current period not ended
                - new distribution is not finished yet
             */
            return;
        }
        log("DEBUG: it is necessary to distribute bonuses\n");
        auto bonus = eosio::asset{
                int64_t(_config.eos_balance.amount * cfg.bonus_percent),
                common::EOS_SYMBOL
        };
        if(bonus.amount <= 0)
        {//not enough balance to do distribution
            return;
        }
        log("DEBUG: enough balance to distribute\n");
        auto board = get_leader_board_index<TYPE>();
        auto leaders = get_leaders(board, cfg);
        cfg.update_period_start();
        if(leaders.size() < 1)
        {//not enough leaders to do distribution
            return;
        }
        log("DEBUG: enough users to distribute\n");
        eosio::transaction deferred;
        auto distribution_id = _config.next_deferred_id(TransactionNumber::DISTRIBUTE);
        cfg.start_distribution(distribution_id);
        deferred.actions.emplace_back(
                eosio::permission_level{_owner, "active"_n},
                _owner, "distribute"_n,
                std::make_tuple(
                        _owner,
                        TYPE,
                        leaders,
                        bonus
                )
        );
        deferred.delay_sec = 1;
        log("Before call distribute(%, %, %, %) \n", _owner, TYPE, leaders, bonus);
        deferred.send(distribution_id, _owner);
        clean_board(board);
    }

    template<uint8_t TYPE>
    void remove_extra_records()
    {
        auto& cfg = get_config_ref<TYPE>();
        auto index = get_leader_board_index<TYPE>();
        std::vector<eosio::name> to_delete;
        auto count = 0;
        for (auto it = index.rbegin();it != index.rend();++it)
        {
            ++count;
            if(count <= cfg.size)
                continue;
            //one or more if size was changed on previous action
            to_delete.push_back(it->account);
        }
        if(to_delete.size()>0)
        {
            auto& table = get_leader_board<TYPE>();
            for (auto account : to_delete)
            {
                table.erase(table.find(account.value));
            }
        }
    }

    template<uint8_t TYPE>
    void update_leader_board(eosio::name player, const tables::PlayerBetsStatistics& stats)
    {
        auto& board = get_leader_board<TYPE>();
        auto it = board.find(player.value);
        if(board.end() == it)
        {
            board.emplace(_owner, [&](auto& b)
            {
                b.account = player;
                b.stats = stats;
            });
            remove_extra_records<TYPE>();
        }
        else
        {
            board.modify(it, _owner, [&](auto& b)
            {
                b.stats = stats;
            });
        }
    }

public:
    LeaderBoards(eosio::name owner, tables::Config& config):
        _owner(owner), _config(config), _dayTop(owner, owner.value), _monthTop(owner, owner.value)
    {
    }

    void update_player_stats(const tables::Player& player)
    {
        update_leader_board<LeaderBoardType::DAY>(player.account, player.day);
        update_leader_board<LeaderBoardType::MONTH>(player.account, player.month);
    }

    void refresh()
    {
        check_leader_board<LeaderBoardType::DAY>();
        check_leader_board<LeaderBoardType::MONTH>();
    }

    inline void stop_distribution(uint8_t type)
    {
        if (LeaderBoardType::DAY == type)
        {
            _config.day_leader_board.stop_distribution();
        }
        else if (LeaderBoardType::MONTH == type)
        {
            _config.month_leader_board.stop_distribution();
        }
    }

    inline void on_distribution_failed(eosio::action& action)
    {
        using Data = std::tuple<eosio::name, uint8_t, std::vector<eosio::name>, eosio::asset>;
        auto [caller, type, leaders, bonus] = action.data_as<Data>();
        require_recipient(caller);
        stop_distribution(type);
        log("ERROR: distribute(%, %, %, %)\n", caller, type, common::as_string(leaders).c_str(), bonus);
    }

    void distributeLeadersBonuses(eosio::name caller, uint8_t type, const std::vector<eosio::name>& leaders, eosio::asset bonus)
    {
        log("distribute(%, %, %, %)\n", caller, type, common::as_string(leaders).c_str(), bonus);
        require_auth(caller);
        eosio_assert(_config.admin == caller || _config.owner == caller, "You cannot call this function.");
        eosio_assert(bonus.is_valid(), "Wrong bonus value.");
        eosio_assert(bonus.symbol == common::EOS_SYMBOL , "Wrong bonus symbol.");
        eosio_assert(bonus.amount > 0, "Wrong quantity.");
        eosio_assert(_config.eos_balance.amount - bonus.amount >= 0, "Not enough balance to distribute bonuses.");
        if(_config.enabled_payout)
        {
            int i = 1;
            double multiplier = 0.5;
            for(auto& player : leaders)
            {
                auto quantity = eosio::asset{int64_t(bonus.amount * multiplier), common::EOS_SYMBOL};
                std::string msg = "Receive your bonus ";
                msg.append(quantity.to_string());
                msg.append(" for ");
                msg.append(std::to_string(i));
                msg.append(" place in leader board.");

                eosio::action(
                        eosio::permission_level{_owner, "active"_n},
                        "eosio.token"_n,
                        "transfer"_n,
                        std::make_tuple(
                                _owner,
                                player,
                                quantity,
                                msg
                        )
                ).send();
                multiplier /= 2;
                ++i;
            }
            _config.eos_balance -= bonus;
        }
        stop_distribution(type);
    }
};

}//dice