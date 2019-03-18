#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/fixed_bytes.hpp>

namespace common
{
    constexpr eosio::symbol EOS_SYMBOL = eosio::symbol("EOS", 4);
    constexpr eosio::symbol ANTE_SYMBOL = eosio::symbol("ANTE", 8);
    constexpr int64_t ANTE_MAX_SUPPLY = eosio::asset::max_amount;
    constexpr uint32_t EOS_MULTIPLIER = 10000;

    template <class Container>
    void split(const std::string& str, Container& cont, const std::string& delims = " ")
    {
        std::size_t current, previous = 0;
        current = str.find_first_of(delims);
        while (current != std::string::npos)
        {
            cont.push_back(str.substr(previous, current - previous));
            previous = current + 1;
            current = str.find_first_of(delims, previous);
        }
        cont.push_back(str.substr(previous, current - previous));
    }

    inline bool startsWith(const std::string& mainStr, const std::string& toMatch)
    {
        if(mainStr.find(toMatch) == 0)
            return true;
        else
            return false;
    }

    inline eosio::time_point shift_current_time(uint32_t seconds)
    {
        auto now_time = eosio::time_point(eosio::seconds(now()));
        return  eosio::time_point(eosio::seconds(seconds));
    }

    std::string as_string(const std::vector<eosio::name>& vec)
    {
        std::string str = "[";
        for(auto& player : vec)
        {
            str += player.to_string();
            str += " ";
        }
        str += "]";
        return str;
    }
}//namespace common

#define DISPATCH_ME(MEMBER, NAME) \
if(code == receiver) \
{ \
    if (eosio::name(BOOST_PP_STRINGIZE(NAME)).value == action) \
    { \
        eosio::execute_action(eosio::name(receiver), eosio::name(code), &MEMBER); \
        return; \
    } \
}

#define DISPATCH_EXTERNAL(OTHER_CONTRACT, OTHER_ACTION, MEMBER) \
    if((code == eosio::name(BOOST_PP_STRINGIZE(OTHER_CONTRACT)).value) && (action == eosio::name(BOOST_PP_STRINGIZE(OTHER_ACTION)).value)) \
    { \
        eosio::execute_action(eosio::name(receiver), eosio::name(code), &MEMBER); \
        return; \
    }
