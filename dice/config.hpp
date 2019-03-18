#pragma once
#include <dice/common.hpp>
#include <dice/tables.hpp>

namespace dice {
namespace config {

    constexpr auto one_hour_in_seconds = 60 * 60;
    constexpr auto one_day_in_seconds = 24 * 60 * 60;
    constexpr auto one_week_in_seconds = one_day_in_seconds * 7;
    constexpr auto one_month_in_seconds = one_day_in_seconds * 30;
    constexpr auto distribution_expire_in_seconds = 60 * 60;
    constexpr auto one_year_in_seconds = one_day_in_seconds * 365;

    tables::Config init_main_config(eosio::name owner)
    {
        return {
                owner,                                  /* owner */
                owner,                                  /* admin */
                owner,                                  /* ante_token */
                false,                                  /* enabled_betting */
                true,                                   /* enabled_minting */
                true,                                   /* enabled_payout */
                eosio::asset(0, common::EOS_SYMBOL),    /* eos_balance */
                {
                        0,                              /* first */
                        0,                              /* last */
                        100,                            /* max */
                },
                {
                        0,                              /* first */
                        0,                              /* last */
                        100,                            /* max */
                },
                {
                        0,                              /* first */
                        0,                              /* last */
                        100,                            /* max */
                },
                eosio::asset(10, common::EOS_SYMBOL),   /* high_bet_bound */
                5,                                      /* rare_bet_bound */
                2.0,                                    /* ante_in_eos */
                0.1,                                    /* referral_multiplier */
                0.01,                                   /* jackpot percent */
                eosio::asset(0, common::EOS_SYMBOL),    /* jackpot balance */
                eosio::asset(0, common::EOS_SYMBOL),    /* total payout */
                eosio::asset(0, common::EOS_SYMBOL),    /* total bet amount */
                {                                       /* day_leader_board */
                        10,                                     /* size */
                        0.01,                                   /* bonus_percent */
                        0,                                      /* distribution_id */
                        eosio::time_point{eosio::seconds(0)},   /* distribution_start */
                        eosio::time_point{eosio::seconds(0)},   /* period_start */
                        one_day_in_seconds,                     /* period_length */
                },
                {                                       /* month_leader_board */
                        10,                                     /* size */
                        0.02,                                   /* bonus_percent */
                        0,                                      /* distribution_id */
                        eosio::time_point{eosio::seconds(0)},   /* distribution_start */
                        eosio::time_point{eosio::seconds(0)},   /* period_start */
                        one_month_in_seconds,                   /* period_length */
                },
                1,
        };
    }

    tables::DiceLimit init_dice_limits()
    {
        return {
                5,      /*min_value*/
                94,     /*max_value*/
                0.10,   /*max_bet_percent*/
                100,    /*max_bet_num*/
                eosio::asset(1 * common::EOS_MULTIPLIER, common::EOS_SYMBOL),         /*min_bet*/
                eosio::asset(10000 * common::EOS_MULTIPLIER, common::EOS_SYMBOL),     /*balance_protect*/
                0.05,   /*platform_fee*/
        };
    }

    tables::BetToken init_bet_token()
    {
        return {
                common::EOS_SYMBOL,
                0,
                0,
                0,
                0,
        };
    }

    tables::AnteBonus ante_bonuses[] = {
            {1, 10, 1.01},
            {11, 20, 1.02},
            {21, 30, 1.03},
            {31, 40, 1.04},
            {41, 50, 1.05},
            {51, 100, 1.07},
            {101, 200, 1.10},
            {201, 500, 1.15},
            {501, (uint16_t(1) << 16) - 1, 1.20},
    };

    void init_ante_bonuses(eosio::name pay_for_ram, tables::AnteBonusesConfig& bonuses)
    {
        for(const auto& item: ante_bonuses)
        {
            bonuses.emplace(pay_for_ram, [&](auto& cfg)
            {
                cfg.begin = item.begin;
                cfg.end = item.end;
                cfg.multiplier = item.multiplier;
            });
        }
    }
}//namespace config
}//namespace dice

