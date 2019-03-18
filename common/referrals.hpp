#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/name.hpp>
#include <string>
#include <math.h>
#include <common/common.hpp>
#include <common/tables.hpp>


namespace common {

typedef eosio::multi_index<"referrals"_n, tables::Referral> ReferralsTable;

class Referrals final
{
    eosio::name _owner;
    double _multiplier;
protected:
    ReferralsTable _table;

    void send_bonus(eosio::name referrer, eosio::asset balance)
    {
        std::string msg = "Your referral generated bonus for you ";
        msg.append(balance.to_string());
        eosio::print_f("Attempt to transfer % from % to %.\n", balance, _owner, referrer);
        eosio::action(
                eosio::permission_level{_owner, "active"_n},
                "eosio.token"_n,
                "transfer"_n,
                std::make_tuple(
                        _owner,
                        referrer,
                        balance,
                        msg
                )
        ).send();
    }

    auto get_or_create(eosio::name player, eosio::name inviter)
    {
        auto it = _table.find(player.value);
        if(_table.end() == it)
        {
            bool is_valid_inviter = is_account(inviter) &&
                                    inviter != player &&
                                    (_table.end() != _table.find(inviter.value));
            it = _table.emplace(_owner, [&](auto& r)
            {
                r.player = player;
                if(is_valid_inviter)
                {
                    r.referrer = inviter;
                }
                else
                {
                    r.referrer = eosio::name("");
                }
                r.ref_balance = eosio::asset(0, common::EOS_SYMBOL);
            });
        }
        return it;
    }

public:
    Referrals(eosio::name contract_owner):
        _owner(contract_owner),
        _multiplier(0.0),
        _table(contract_owner, contract_owner.value)
    {
        eosio::print("Started referrals constructor.\n");

        eosio::print("Finished referrals constructor.\n");
    };

    ~Referrals()
    {
        eosio::print("Started referrals destructor.\n");

        eosio::print("Finished referrals destructor.\n");
    };

    void setBonusMultiplier(double multiplier)
    {
        _multiplier = abs(multiplier);
    }

    void on_player_bet(eosio::name player, eosio::name inviter, eosio::asset bet, eosio::asset reward)
    {
        eosio::print_f("on_player_bet(%, %, %)\n", player, bet, reward);
        auto it = get_or_create(player, inviter);
        it = _table.find(it->referrer.value);

        if(_table.end() != it)
        {
            _table.modify(it, _owner, [&](auto& f)
            {
                eosio_assert(bet.symbol == f.ref_balance.symbol, "Referrals: Wrong bet symbol.");
                eosio_assert(reward.symbol == f.ref_balance.symbol, "Referrals: Wrong reward symbol.");
                if(reward.amount > 0)
                {
                    f.ref_balance -= (reward - bet);
                }
                else
                {
                    f.ref_balance += eosio::asset{
                            int64_t(bet.amount * _multiplier),
                            common::EOS_SYMBOL
                    };
                }

                if(f.ref_balance.amount > 0)
                {
                    send_bonus(f.player, f.ref_balance);
                    f.ref_balance.amount = 0;
                }
            });
        }
    }
};

}//common