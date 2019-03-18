#include <dice/common.hpp>

#ifdef DEBUG_CONTRACT
    #include <dice/debug_config.hpp>
#else
    #include <dice/config.hpp>
#endif

#include <dice/logger.hpp>
#include <dice/tables.hpp>
#include <dice/leaderboards.hpp>

namespace dice {

/*
 * contract name should be equal to file name
 * otherwise you will receive empty abi file
 * */
class [[eosio::contract("eos.dice")]] Dice: public eosio::contract
{
protected:
    // currents states of different types of configs
    tables::Config _stateConfig;
    tables::BetToken _stateEosToken;
    tables::DiceLimit _stateLimits;
    // singletons
    tables::ContractConfig _globalConfig;
    tables::DiceLimits _diceLimits;
    tables::BetTokens _betTokens;
    // tables
    tables::AnteBonusesConfig _bonusesConfig;
    tables::Bets _bets;
    tables::HighBets _highBets;
    tables::RareBets _rareBets;
    tables::Players _players;
    tables::Jackpots _jackpots;

    common::random _random;
    capi_checksum256 _seed;

    common::Referrals _referrals;
    LeaderBoards _leaderBoards;

    void on_replenishment(const common::tables::TokenTransfer& transfer);
    void on_bet(const common::tables::TokenTransfer& transfer);
    eosio::asset get_bet_reward(uint8_t roll_type, uint16_t roll_border, const eosio::asset& quantity);
    uint64_t get_random(uint64_t max);
    uint8_t get_winners(uint8_t roll_type, uint16_t roll_border);
    void pay_for_win(const eosio::name& player, const eosio::asset& quantity, std::string& message);
    void register_bet(const eosio::name& player, const eosio::asset& bet, const eosio::asset& reward,
                            uint8_t roll_type, uint16_t roll_border, uint16_t roll_value, const eosio::name& inviter);
    void send_to_jackpot_game(const eosio::name& player, const eosio::asset& quantity, uint64_t roll_value);
    void mint_tokens(const eosio::name& player, const eosio::asset& bet, const eosio::asset& reward,
            const eosio::name& inviter);
public:
    Dice(eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds);
    ~Dice();

    [[eosio::action("admin.set")]] void setAdmin(eosio::name caller, eosio::name admin);
    [[eosio::action("betting.set")]] void setBettingEnabled(eosio::name caller, bool enabled);
    [[eosio::action("minting.set")]] void setMintingEnabled(eosio::name caller, bool enabled);
    [[eosio::action("payout.set")]] void setPayoutEnabled(eosio::name caller, bool enabled);
    [[eosio::action("ante.set")]] void setAnteTokenAccount(eosio::name caller, eosio::name name);
    [[eosio::action("params.set")]] void setGameParams(eosio::name caller, uint16_t min, uint16_t max, uint16_t max_bet_num);
    [[eosio::action("protect.set")]] void setBalanceProtect(eosio::name caller, eosio::asset balance_protect);
    [[eosio::action("fee.set")]] void setPlatformFee(eosio::name caller, double platform_fee);
    [[eosio::action("minbet.set")]] void setMinBet(eosio::name caller, eosio::asset min_bet);
    [[eosio::action("rate.set")]] void setExchangeRate(eosio::name caller, double rate);
    [[eosio::action("balance.set")]] void setBalanceValue(eosio::name caller, eosio::asset balance);
    [[eosio::action("mbp.set")]] void setMaxBetPercent(eosio::name caller, double max_bet_percent);
    [[eosio::action("bets.setl")]] void setBetsHistoryLength(eosio::name caller, uint64_t size);
    [[eosio::action("high.setl")]] void setHighBetsHistoryLength(eosio::name caller, uint64_t size);
    [[eosio::action("rare.setl")]] void setRareBetsHistoryLength(eosio::name caller, uint64_t size);
    [[eosio::action("high.bet.set")]] void setHighBetBound(eosio::name caller, eosio::asset high_bet_bound);
    [[eosio::action("rare.bet.set")]] void setRareBetBound(eosio::name caller, uint16_t rare_bet_bound);
    [[eosio::action("dlp.set")]] void setDayLeaderPercent(eosio::name caller, double percent);
    [[eosio::action("mlp.set")]] void setMonthLeaderPercent(eosio::name caller, double percent);
    [[eosio::action("jackpot.set")]] void setJackpotPercent(eosio::name caller, double percent);
    [[eosio::action("referral.set")]] void setRefferalMultiplier(eosio::name caller, double multiplier);
    [[eosio::action("notify")]] void notify(std::string);

    [[eosio::action("distribute")]] void distributeLeadersBonuses(eosio::name caller, uint8_t type,
            const std::vector<eosio::name>& leaders, eosio::asset bonus);

    [[eosio::action("bet")]] void makeBet(eosio::name player, eosio::name inviter, eosio::asset quantity,
                                          uint8_t roll_type, uint16_t roll_border);

    [[eosio::action("resolved")]] void resolveBet(eosio::name player, eosio::name inviter, eosio::asset quantity,
                                                  uint8_t roll_type, uint16_t roll_border);

    //catched events
    void on_transfer();
    //events
    void on_error(eosio::onerror& error);
};

} /// namespace dice



extern "C"
void apply(uint64_t receiver, uint64_t code, uint64_t action)
{
    DISPATCH_ME(dice::Dice::setAdmin, admin.set)
    DISPATCH_ME(dice::Dice::setBettingEnabled, betting.set)
    DISPATCH_ME(dice::Dice::setMintingEnabled, minting.set)
    DISPATCH_ME(dice::Dice::setPayoutEnabled, payout.set)
    DISPATCH_ME(dice::Dice::setAnteTokenAccount, ante.set)
    DISPATCH_ME(dice::Dice::setGameParams, params.set)
    DISPATCH_ME(dice::Dice::setPlatformFee, fee.set)
    DISPATCH_ME(dice::Dice::setMinBet, minbet.set)
    DISPATCH_ME(dice::Dice::setExchangeRate, rate.set)
    DISPATCH_ME(dice::Dice::setBalanceValue, balance.set)
    DISPATCH_ME(dice::Dice::setBalanceProtect, protect.set)
    DISPATCH_ME(dice::Dice::setMaxBetPercent, mbp.set)
    DISPATCH_ME(dice::Dice::makeBet, bet)
    DISPATCH_ME(dice::Dice::resolveBet, resolved)
    DISPATCH_ME(dice::Dice::setBetsHistoryLength, bets.setl)
    DISPATCH_ME(dice::Dice::setHighBetsHistoryLength, high.setl)
    DISPATCH_ME(dice::Dice::setRareBetsHistoryLength, rare.setl)
    DISPATCH_ME(dice::Dice::setHighBetBound, high.bet.set)
    DISPATCH_ME(dice::Dice::setRareBetBound, rare.bet.set)
    DISPATCH_ME(dice::Dice::distributeLeadersBonuses, distribute)
    DISPATCH_ME(dice::Dice::notify, notify)
    DISPATCH_ME(dice::Dice::setDayLeaderPercent, dlp.set)
    DISPATCH_ME(dice::Dice::setMonthLeaderPercent, mlp.set)
    DISPATCH_ME(dice::Dice::setJackpotPercent, jackpot.set)
    DISPATCH_ME(dice::Dice::setRefferalMultiplier, referral.set)

    DISPATCH_EXTERNAL(eosio, onerror, dice::Dice::on_error)
    DISPATCH_EXTERNAL(eosio.token, transfer, dice::Dice::on_transfer)
}
