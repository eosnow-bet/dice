#pragma once
#include <common/logger.hpp>
#include <dice/tables.hpp>

#ifdef DEBUG_CONTRACT

template<class T, class ... TArgs>
void log(const T& var, const TArgs& ... args)
{
    eosio::print_f(var, args ...);
}

#else
template<class T, class ... TArgs>
void log(const T& var, const TArgs& ... args)
{
    //empty
}
#endif
