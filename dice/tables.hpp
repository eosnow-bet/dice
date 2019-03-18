#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>
#include <limits>

namespace dice {
namespace tables {


/*
 * simple description of table id
 * */
struct TableId
{
    uint64_t first; // id of first row stored in table
    uint64_t last;  // id of last row stored in table
    uint64_t max;   // visible frame size

    uint64_t next()
    {
        if(0 == first)
        {
            ++first;
        }
        if (last == std::numeric_limits<uint64_t>::max())
        {
            last = 1;
        }
        else
        {
            ++last;
        }
        return last;
    }

    void print() const
    {
        eosio::print_f("[first=% last=% max=%]", first, last, max);
    }

    EOSLIB_SERIALIZE(TableId,
        (first)(last)(max)
    );
};

/*
 * leader board config
 * */
struct LeaderBoardConfig
{
    uint8_t size;                   // maximum amount of leaders for month bonuses
    double bonus_percent;           // which part of contract balance will be distributed on the end of period
    uint128_t distribution_id;      // id of deferred transaction with distribute action
    eosio::time_point distribution_start; // start time of current distribution
    eosio::time_point period_start; // start time of current period
    uint32_t period_length;         // length of current period


    bool is_distribution_active()const
    {
        return distribution_id != 0;
    }

    bool is_distribution_expired(int64_t period)const
    {
        auto time_now = eosio::time_point(eosio::seconds(now()));
        return is_distribution_active() && (time_now - distribution_start).to_seconds() > period;
    }

    bool is_period_ended()const
    {
        auto time_now = eosio::time_point(eosio::seconds(now()));
        auto current_period_number = time_now.sec_since_epoch() / period_length;
        auto start = eosio::time_point(eosio::seconds(current_period_number * period_length));
        return start > period_start;
    }

    void update_period_start()
    {
        auto time_now = eosio::time_point(eosio::seconds(now()));
        auto current_period_number = time_now.sec_since_epoch() / period_length;
        period_start = eosio::time_point(eosio::seconds(current_period_number * period_length));
    }

    inline void start_distribution(uint128_t id)
    {
        distribution_id = id;
        distribution_start = eosio::time_point(eosio::seconds(now()));;
    }

    inline void stop_distribution()
    {
        distribution_id = 0;
        distribution_start = eosio::time_point{eosio::seconds(0)};
    }
    
    void print() const
    {
        eosio::print_f("[% ; % ; % ; % ; % ]", (int)size, bonus_percent, distribution_id,
                period_start.sec_since_epoch(), period_length);
    }

    EOSLIB_SERIALIZE(LeaderBoardConfig,
        (size)
        (bonus_percent)
        (distribution_id)
        (distribution_start)
        (period_start)
        (period_length)
    );
};


/*
 * Table to store main config.
 */
struct [[eosio::table("cfg.main"), eosio::contract("eos.dice")]] Config
{
    eosio::name owner;                          // contract owner
    eosio::name admin;                          // contract admin
    eosio::name ante_token;                     // ante.token contract owner
    bool enabled_betting;                       // set false to disable betting
    bool enabled_minting;                       // set false to disable token minting
    bool enabled_payout;                        // set false to disable payout on win
    eosio::asset eos_balance;                   // eos_balance
    TableId bets_id;                            // id for table bets.all
    TableId high_bets_id;                       // id for table bets.high
    TableId rare_bets_id;                       // id for table bets.rare
    eosio::asset high_bet_bound;                // lower bound of High Bets
    uint16_t rare_bet_bound;                    // upper bound for possible wins (for Rare Bets)
    double ante_in_eos;                         // how many ante tokens in 1 EOS
    double referral_multiplier;                 // i.e. 10% of loosing bets will be added to referrer shadow balance
    double jackpot_percent;                     // jackpot percent for each bet
    eosio::asset jackpot_balance;               // jackpot balance
    eosio::asset total_payout;                  // total payout
    eosio::asset total_bet_amount;              // total bet amount

    LeaderBoardConfig day_leader_board;         // configuration for day leader board
    LeaderBoardConfig month_leader_board;       // configuration for month leader board
    uint64_t base_deferred_id;

    void print() const
    {
        eosio::print_f("Config['%';'%';'%';'%';'%';'%';'%';'%';'%';'%';'%';'%';'%';'%';]\n",
                owner, admin, ante_token, enabled_betting, enabled_minting, enabled_payout, eos_balance,
                       bets_id, high_bets_id, rare_bets_id, high_bet_bound, rare_bet_bound, ante_in_eos,
                       referral_multiplier, jackpot_percent, day_leader_board,
                       month_leader_board);
    }

    inline uint128_t next_deferred_id(uint8_t nested_action_number)
    {
        ++base_deferred_id;
        uint128_t deferred_id = (uint128_t(nested_action_number) << 64) | uint128_t(base_deferred_id);
        return deferred_id;
    }

    EOSLIB_SERIALIZE(Config,
        (owner)(admin)(ante_token)
        (enabled_betting)(enabled_minting)(enabled_payout)
        (eos_balance)
        (bets_id)
        (high_bets_id)
        (rare_bets_id)
        (high_bet_bound)
        (rare_bet_bound)
        (ante_in_eos)
        (referral_multiplier)
        (jackpot_percent)
        (jackpot_balance)
        (total_payout)
        (total_bet_amount)
        (day_leader_board)
        (month_leader_board)
        (base_deferred_id)
    )
};
typedef eosio::singleton<"cfg.main"_n, Config> ContractConfig;

/*
 * Table to store ante minting bonuses.
 */
struct [[eosio::table("ante.bonuses"), eosio::contract("eos.dice")]] AnteBonus
{
    uint16_t begin;                         // start amount of bets
    uint16_t end;                           // stop amount of bets
    double multiplier;                      // bonus multiplier

    uint64_t primary_key() const
    {
        return begin;
    };

    void print() const
    {
        eosio::print_f("AnteBonus['%';'%';'%']\n", (int)begin, (int)end, multiplier);
    }
    EOSLIB_SERIALIZE(AnteBonus, (begin)(end)(multiplier))
};
typedef eosio::multi_index<"ante.bonuses"_n, AnteBonus> AnteBonusesConfig;

/*
 * Table for dice game limits
*/
struct [[eosio::table("dice.limits"), eosio::contract("eos.dice")]] DiceLimit
{
    uint16_t min_value;                 // min dice value i.e.: 0
    uint16_t max_value;                 // max dice value i.e.: 100
    double max_bet_percent;             // max percent from balance of possible bet
    uint16_t max_bet_num;               // param to calculate reward, by default = 100
    eosio::asset min_bet;               // min bet amount i.e.: 1.0000 EOS
    eosio::asset balance_protect;       // min balance amount when betting is stopped
    double platform_fee;                // platform fee

    void print() const
    {
        eosio::print_f("Config[min_value='%';max_value='%';max_bet_percent='%';max_bet_num='%';min_bet='%';"
                       "balance_protect='%';platform_fee='%']\n",
                (int)min_value, (int)max_value, max_bet_percent, (int)max_bet_num, min_bet,
                balance_protect, platform_fee);
    }
    EOSLIB_SERIALIZE(DiceLimit,
            (min_value)(max_value)(max_bet_percent)(max_bet_num)(min_bet)(balance_protect)(platform_fee))
};
typedef eosio::singleton<"dice.limits"_n, DiceLimit> DiceLimits;

/*
 * Table for income token statistics
*/
struct [[eosio::table("bet.tokens"), eosio::contract("eos.dice")]] BetToken
{
    eosio::symbol name;             // name of bet currency i.e.: EOS
    int64_t in;                     // all amount of spent tokens
    int64_t out;                    // all amount of won tokens
    uint64_t bets;                  // how many bets
    uint64_t wons;                  // how many wons

    uint64_t primary_key() const
    {
        return name.raw();
    };

    EOSLIB_SERIALIZE(BetToken,
            (name)(in)(out)(bets)(wons)
    );
};
typedef eosio::singleton<"bet.tokens"_n, BetToken> BetTokens;

/*
 * Table with history of bets
*/
struct [[eosio::table("bet"), eosio::contract("eos.dice")]] Bet
{
    uint64_t id;                        // bet number in history
    eosio::name player;                 // account who placed bet
    uint8_t roll_type;                  // 1 || 2
    uint64_t roll_border;               // roll border ie: 50
    uint64_t roll_value;                // roll value ie: 42
    eosio::asset bet;                   // bet amount "1.0001 EOS"
    std::vector<eosio::asset> payout;   // payouts use vector for future usage ie: ["1.0001 EOS", "1.0001 EOS", ..]
    eosio::name inviter;                // another player who gave referral id to this player
    capi_checksum256 seed;              // seed which was used to generate random value
    eosio::time_point_sec time;         // time point in seconds

    uint64_t primary_key() const
    {
        return id;
    };

    uint64_t by_player() const
    {
        return player.value;
    };

    EOSLIB_SERIALIZE(Bet,
        (id)(player)(roll_type)(roll_border)(roll_value)(bet)(payout)(inviter)(seed)(time)
    );

};

typedef eosio::multi_index<"bets.all"_n, Bet,
        eosio::indexed_by<"byplayer"_n, eosio::const_mem_fun<Bet, uint64_t, &Bet::by_player>>> Bets;

typedef eosio::multi_index<"bets.high"_n, Bet,
        eosio::indexed_by<"byplayer"_n, eosio::const_mem_fun<Bet, uint64_t, &Bet::by_player>>> HighBets;

typedef eosio::multi_index<"bets.rare"_n, Bet,
        eosio::indexed_by<"byplayer"_n, eosio::const_mem_fun<Bet, uint64_t, &Bet::by_player>>> RareBets;


/*
 * Table with history of jackpots
*/
struct [[eosio::table("jackpot"), eosio::contract("eos.dice")]] Jackpot
{
    uint64_t id;                        // id
    eosio::name player;                 // account who won
    eosio::time_point time;             // time of jackpot
    eosio::asset amount;                // amount of jackpot

    uint64_t primary_key() const
    {
        return id;
    };

    uint64_t by_player() const
    {
        return player.value;
    };

    EOSLIB_SERIALIZE(Jackpot,
        (id)(player)(time)(amount)
    );

};

typedef eosio::multi_index<"jackpots"_n, Jackpot,
        eosio::indexed_by<"byplayer"_n, eosio::const_mem_fun<Jackpot, uint64_t, &Jackpot::by_player>>> Jackpots;

/*
 * Structure store player bet statistics
*/
struct PlayerBetsStatistics
{
    eosio::symbol symbol;               // symbol code ie: EOS
    uint64_t total_bet_amount;          // amount and symbol of all bets
    uint64_t total_payout;              // amount and symbol pf all payouts
    uint64_t bets;                      // how many bets
    uint64_t wons;                      // how many wons

    // reset all counters
    void reset()
    {
        symbol = common::EOS_SYMBOL;
        total_bet_amount = 0;
        total_payout = 0;
        bets = 0;
        wons = 0;
    }

    EOSLIB_SERIALIZE(PlayerBetsStatistics,
        (symbol)(total_bet_amount)(total_payout)(bets)(wons)
    );
};

/*
 * Statistics for each player.
*/
struct [[eosio::table("player"), eosio::contract("eos.dice")]] Player
{
    eosio::name account;                // account who played
    eosio::time_point last_bet_time;    // time of last bet
    eosio::asset last_bet;              // amount and symbol of last bet
    eosio::asset last_payout;           // amount and symbol of last payout
    int jackpot_sequence;               // player jackpot sequence 0..5
    std::string jackpot_sequence_values;// roll values for jackpot sequence

    PlayerBetsStatistics total;         // total statistics
    PlayerBetsStatistics day;           // day statistics
    PlayerBetsStatistics week;          // week statistics
    PlayerBetsStatistics month;         // month statistics

    uint64_t primary_key() const
    {
        return account.value;
    };

    uint64_t by_day_bets()const
    {
        return day.total_bet_amount;
    }
    uint64_t by_day_bets_count()const
    {
        return day.bets;
    }
    uint64_t by_week_bets()const
    {
        return week.total_bet_amount;
    }
    uint64_t by_week_bets_count()const
    {
        return week.bets;
    }
    uint64_t by_month_bets()const
    {
        return month.total_bet_amount;
    }

    uint64_t by_month_bets_count()const
    {
        return month.bets;
    }

    EOSLIB_SERIALIZE(Player,
            (account)(last_bet_time)(last_bet)(last_payout)(jackpot_sequence)(jackpot_sequence_values)(total)(day)(week)(month)
    );
};

typedef eosio::multi_index<"players"_n, Player,
        eosio::indexed_by<"bydayb"_n, eosio::const_mem_fun<Player, uint64_t, &Player::by_day_bets>>,
        eosio::indexed_by<"bydaybc"_n, eosio::const_mem_fun<Player, uint64_t, &Player::by_day_bets_count>>,
        eosio::indexed_by<"byweekb"_n, eosio::const_mem_fun<Player, uint64_t, &Player::by_week_bets>>,
        eosio::indexed_by<"byweekbc"_n, eosio::const_mem_fun<Player, uint64_t, &Player::by_week_bets_count>>,
        eosio::indexed_by<"bymonthb"_n, eosio::const_mem_fun<Player, uint64_t, &Player::by_month_bets>>,
        eosio::indexed_by<"bymonthbc"_n, eosio::const_mem_fun<Player, uint64_t, &Player::by_month_bets_count>>
    > Players;

/*
 * Top stats for period (day\month)
*/
struct [[eosio::table("top"), eosio::contract("eos.dice")]] Top
{
    eosio::name account;                // account
    PlayerBetsStatistics stats;         // statistics

    uint64_t primary_key() const
    {
        return account.value;
    };

    uint64_t by_bets()const
    {
        return stats.total_bet_amount;
    }

    EOSLIB_SERIALIZE(Top, (account)(stats));
};

typedef eosio::multi_index<"top.day"_n, Top,
            eosio::indexed_by<"bybetsamount"_n, eosio::const_mem_fun<Top, uint64_t, &Top::by_bets>>
        > BetsCountDayTop;

typedef eosio::multi_index<"top.month"_n, Top,
            eosio::indexed_by<"bybetsamount"_n, eosio::const_mem_fun<Top, uint64_t, &Top::by_bets>>
        > BetsCountMonthTop;

/*
 * Alias to generate abi
*/
struct [[eosio::table("referrals"), eosio::contract("eos.dice")]] Referral: public common::tables::Referral
{
};

}//namespace tables
}//namespace dice