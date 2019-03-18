#pragma once
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>

namespace common
{

using ChecksumType = capi_checksum256;

class random
{
public:
    template<class T>
    struct data
    {
        T content;
        int block;
        int prefix;

        data(T t)
        {
            content   = t;
            block  = ::tapos_block_num();
            prefix = ::tapos_block_prefix();
        }
    };

    struct st_seeds
    {
        ChecksumType seed1;
        ChecksumType seed2;
    };

public:
    random();
    ~random();

    template<class T>
    ChecksumType create_sys_seed(T mixed) const;

    void seed(ChecksumType sseed, ChecksumType useed);

    void mixseed(ChecksumType& sseed, ChecksumType& useed, ChecksumType& result) const;

    // generator number ranged [0, max-1]
    uint64_t generator(uint64_t max = 101);

    uint64_t gen(ChecksumType& seed, uint64_t max = 101) const;

    ChecksumType get_sys_seed() const;
    ChecksumType get_user_seed() const;
    ChecksumType get_mixed() const;
    ChecksumType get_seed() const;
private:
    ChecksumType _sseed;
    ChecksumType _useed;
    ChecksumType _mixed;
    ChecksumType _seed;
};

random::random() {}
random::~random() {}

template<class T>
ChecksumType random::create_sys_seed(T mixed) const
{
    ChecksumType result;
    data<T> mixed_block(mixed);
    const char *mixed_char = reinterpret_cast<const char *>(&mixed_block);
    ::sha256((char *)mixed_char, sizeof(mixed_block), &result);
    return result;
}

void random::seed(ChecksumType sseed, ChecksumType useed)
{
    _sseed = sseed;
    _useed = useed;
    mixseed(_sseed, _useed, _mixed);
    _seed  = _mixed;
}

void random::mixseed(ChecksumType& sseed, ChecksumType& useed, ChecksumType& result) const
{
    st_seeds seeds;
    seeds.seed1 = sseed;
    seeds.seed2 = useed;
    ::sha256( (char *)&seeds.seed1, sizeof(seeds.seed1) * 2, &result);
}

uint64_t random::generator(uint64_t max)
{
    mixseed(_mixed, _seed, _seed);
    uint64_t r = gen(_seed, max);
    return r;
}

uint64_t random::gen(ChecksumType& seed, uint64_t max) const
{
    if (max <= 0)
    {
        return 0;
    }
    const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&seed);
    uint64_t r = p64[1] % max;
    return r;
}

ChecksumType random::get_sys_seed() const
{
    return _sseed;
}

ChecksumType random::get_user_seed() const
{
    return _useed;
}

ChecksumType random::get_mixed() const
{
    return _mixed;
}

ChecksumType random::get_seed() const
{
    return _seed;
}

}//namespace common
