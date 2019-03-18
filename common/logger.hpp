#pragma once
#include <eosiolib/types.h>
#include <eosiolib/print.h>
#include <eosiolib/name.hpp>
#include <vector>
#include <string>

namespace eosio
{
    inline void print(uint16_t num)
    {
        printui(num);
    }
    inline void print(uint8_t num)
    {
        printui(num);
    }

    inline void print(std::vector<eosio::name> vec)
    {
        std::string msg = "[";
        for(auto acc : vec)
        {
            msg += acc.to_string();
            msg +="|";
        }
        msg +="]";
        prints(msg.c_str());
    }
}


