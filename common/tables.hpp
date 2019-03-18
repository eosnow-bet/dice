#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/name.hpp>
#include <string>

namespace common {
namespace tables {


/*
 * Table with referral relations
*/
struct [[eosio::table("referrals")]] Referral
{
    eosio::name player;             // player
    eosio::name referrer;           // another player who invited this user
    eosio::asset ref_balance;       // shadow balance of this player

    uint64_t primary_key() const
    {
        return player.value;
    };

    EOSLIB_SERIALIZE(Referral,
        (player)(referrer)(ref_balance)
    );
};


/*
 * struct to catch "eosio.token".transfer(...) action
 * */
struct TokenTransfer {
    eosio::name from;
    eosio::name to;
    eosio::asset quantity;
    std::string memo;
};

}//tables
}//common