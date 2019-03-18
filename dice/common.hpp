#pragma once
#include <common/logger.hpp>
#include <common/common.hpp>
#include <common/random.hpp>
#include <common/tables.hpp>
#include <common/referrals.hpp>

namespace dice {

enum TransactionNumber : uint8_t
{
    BET = 1,
    RESOLVED = 2,
    MINT = 3,
    DISTRIBUTE = 4,
    NOTIFY = 5
};

enum RollType : uint8_t
{
    LEFT = 1,
    RIGHT = 2
};

}//namespace dice