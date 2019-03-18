# Description
eos.dice contract description

# Build 
## Debug
* cd scripts/
* `./build_debug.sh` to build with [debug_config.hpp](debug_config.hpp) and tests.
* `build/output/debug/` will contains compiled debug contract version and abi
* `cd build` and `make test ARGS='-V'` to run tests (if all contracts are placed in default locations)
   otherwise use direct call
   ```bash
   python3 eos.dice.test.py --etoken_dir ="{path to eosio.token compiled contract}/eosio.token/" --contract_dir = "{path to this repo}/scripts/build/output/debug/"
   ``` 
   
## Release
* cd scripts/
* `./build_release.sh` to build with [config.hpp](config.hpp) and without tests
* `build/output/release/` will contains compiled release contract version and abi

# Test 

# Tables

Before you start:
* `dice` used as eos.dice contract owner
* used output for debug version of contract  

## ```cfg.main```

```bash
./cleos.sh get table dice dice cfg.main
```
### Fields
* eosio::name `owner`          - contract owner
* eosio::name `admin`          - contract admin
* eosio::name `ante_token`     - ante.token contract owner
* bool `enabled_betting`       - set false to disable betting
* bool `enabled_minting`       - set false to disable token minting
* bool `enabled_payout`        - set false to disable payouts
* eosio::asset `eos_balance`   - active eos_balance of this contract
* TableId `bets_id`            - id for table bets.all
* TableId `high_bets_id`       - id for table bets.high
* TableId `rare_bets_id`       - id for table bets.rare
* eosio::asset `high_bet_bound`- lower bound of High Bets
* uint16_t `rare_bet_bound`    - upper bound for possible wins (for Rare Bets)
* double `ante_in_eos`         - how many ante tokens in 1 EOS
* double `referral_multiplier` - i.e. 10% of loosing bets will be added to referrer shadow balance
* LeaderBoardConfig `day_leader_board` - configuration for day leader board
* LeaderBoardConfig `month_leader_board` - configuration for month leader board
* uint64_t `base_deferred_id` - base part of deferred id
    

### struct TableId
* uint64_t `max`               - max `visible` elements
* uint64_t `first`             - id of first row stored in table (0 - on start)
* uint64_t `last`              - id of last row stored in table

### struct LeaderBoardConfig
* uint8_t `size` - maximum amount of leaders for month bonuses
* double `bonus_percent` - which part of contract balance will be distributed on the end of period
* uint128_t `distribution_id` - id of deferred transaction with distribute action
* eosio::time_point `period_start` - start time of current period
* uint32_t `period_length` - length of current period
 
Example:
```json5
{
  "rows": [{
      "owner": "dice",
      "admin": "dice",
      "ante_token": "ante",
      "enabled_betting": 1,
      "enabled_minting": 1,
      "enabled_payout": 1,
      "eos_balance": "10022457.6173 EOS",
      "bets_id": {
        "first": 11,
        "last": 15,
        "max": 5
      },
      "high_bets_id": {
        "first": 1,
        "last": 15,
        "max": 100
      },
      "rare_bets_id": {
        "first": 0,
        "last": 0,
        "max": 100
      },
      "high_bet_bound": "0.0010 EOS",
      "rare_bet_bound": 3,
      "ante_in_eos": "2.00000000000000000",
      "referral_multiplier": "0.10000000000000001",
      "day_leader_board": {
        "size": 10,
        "bonus_percent": "0.10000000000000001",
        "distribution_id": "0x00000000000000000000000000000000",
        "period_start": "2019-02-15T15:09:00.000",
        "period_length": 180
      },
      "month_leader_board": {
        "size": 10,
        "bonus_percent": "0.20000000000000001",
        "distribution_id": "0x00000000000000000000000000000000",
        "period_start": "2019-02-15T15:10:00.000",
        "period_length": 300
      },
      "base_deferred_id": 48
    }
  ],
  "more": false
}

```


## ```dice.limits```

```bash
./cleos.sh get table dice dice dice.limits
```

### Fields 
* uint16_t `min_value`           - min dice value i.e.: 0
* uint16_t `max_value`           - max dice value i.e.: 100
* double `max_bet_percent`       - max percent from balance of possible bet (or reward)
* uint16_t `max_bet_num`         - param to calculate reward, by default = 100
* eosio::asset `min_bet`         - min bet amount i.e.: 1.0000 EOS
* eosio::asset `balance_protect` - min balance amount when betting is stopped
* double `platform_fee`          - platform fee

Example:
```json5
{
  "rows": [{
      "min_value": 0,
      "max_value": 100,
      "max_bet_percent": "1.00000000000000000",
      "max_bet_num": 100,
      "min_bet": "0.0001 EOS",
      "balance_protect": "0.0000 EOS",
      "platform_fee": "0.00000000000000000"
    }
  ],
  "more": false
}
```

## ```bets.all```

```bash
./cleos.sh get table dice dice bets.all
./cleos.sh get table dice dice bets.all --index 2 --key-type i64 --limit 100
```

### Fields
* uint64_t `id`                        - bet number in history
* eosio::name `player`                 - account who placed bet
* uint8_t `roll_type`                  - 1 || 2
* uint64_t `roll_border`               - roll border ie: 50
* uint64_t `roll_value`                - roll value ie: 42
* eosio::asset `bet`                   - bet amount "1.0001 EOS"
* std::vector\<eosio::asset\> `payout` - payouts use vector for future usage ie: ["1.0001 EOS", "1.0001 EOS", ..]
* eosio::name `inviter`                - another player who gave referral id to this player
* capi_checksum256 `seed`              - seed which was used to generate random value
* eosio::time_point_sec `time`         - time point in seconds

###Indexes
* primary index - by id (unique)
* `byplayer` - by player name (non unique)

Example:
```json5
{
  "rows": [{
      "id": 2,
      "player": "user1",
      "roll_type": 1,
      "roll_border": 42,
      "roll_value": 88,
      "bet": "1.0001 EOS",
      "payout": [
        "0.0000 EOS"
      ],
      "inviter": "user1",
      "seed": "0501be3f6eed5c36480f2c592c78fba370297d798efc2bf41a83aa5608f5b526",
      "time": "2019-02-04T14:24:51"
    }
  ],
  "more": false
}
```

## ```players```

```bash
./cleos.sh get table dice dice players
./cleos.sh get table dice dice players --index 2 --key-type i64 --limit 100
```

### Fields
* eosio::name `account`                - account who played
* eosio::time_point `last_bet_time`    - time of last bet
* eosio::asset `last_bet`              - amount and symbol of last bet
* eosio::asset `last_payout`           - amount and symbol of last payout
* PlayerBetsStatistics `total`         - total statistics
* PlayerBetsStatistics `day`           - day statistics
* PlayerBetsStatistics `week`          - week statistics
* PlayerBetsStatistics `month`         - month statistics

#### PlayerBetsStatistics
* eosio::symbol `symbol`               - symbol code ie: EOS
* uint64_t `total_bet_amount`          - amount and symbol of all bets
* uint64_t `total_payout`              - amount and symbol pf all payouts
* uint64_t `bets`                      - how many bets
* uint64_t `wons`                      - how many wons 

###Indexes
* primary index - by player name (unique)
* `bydayb` - by day bets amount 
* `bydaybc` - by day bets count 
* `byweekb` - by week bets amount 
* `byweekbc` - by week bets count 
* `bymonthb` - by month bets amount 
* `bymonthbc` - by month bets count 
        
Example:
```json5
{
  "rows": [{
      "account": "user1",
      "last_bet_time": "2019-02-04T14:24:51.000",
      "last_bet": "1.0001 EOS",
      "last_payout": "0.0000 EOS",
      "total": {
        "symbol": "4,EOS",
        "total_bet_amount": 10001,
        "total_payout": 0,
        "bets": 1,
        "wons": 0
      },
      "day": {
        "symbol": "4,EOS",
        "total_bet_amount": 10001,
        "total_payout": 0,
        "bets": 1,
        "wons": 0
      },
      "week": {
        "symbol": "4,EOS",
        "total_bet_amount": 10001,
        "total_payout": 0,
        "bets": 1,
        "wons": 0
      },
      "month": {
        "symbol": "4,EOS",
        "total_bet_amount": 10001,
        "total_payout": 0,
        "bets": 1,
        "wons": 0
      }
    }
  ],
  "more": false
}
```

## ```ante.bonuses```

```bash
./cleos.sh get table dice dice ante.bonuses
```

### Fields
* uint16_t `begin`         - begin of period (count of bets)
* uint16_t `end`           - end of period (count of bets)
* double `multiplier`      - bonus multiplier

Example:
```json5
{
  "rows": [{
      "begin": 1,
      "end": 10,
      "multiplier": "1.01000000000000000"
    },{
      "begin": 11,
      "end": 20,
      "multiplier": "1.02000000000000000"
    },{
      "begin": 21,
      "end": 30,
      "multiplier": "1.03000000000000000"
    },{
      "begin": 31,
      "end": 40,
      "multiplier": "1.04000000000000000"
    },{
      "begin": 41,
      "end": 50,
      "multiplier": "1.05000000000000000"
    },{
      "begin": 51,
      "end": 100,
      "multiplier": "1.07000000000000001"
    },{
      "begin": 101,
      "end": 200,
      "multiplier": "1.10000000000000001"
    },{
      "begin": 201,
      "end": 500,
      "multiplier": "1.14999999999999999"
    },{
      "begin": 501,
      "end": 65535,
      "multiplier": "1.20000000000000001"
    }
  ],
  "more": false
}
```

## ```bet.tokens```

```bash
./cleos.sh get table dice 1397703940 bet.tokens
```
### Fields
* eosio::symbol `name` - descriptor of bet symbol  
* int64_t `in`         - total amount of income  
* int64_t `out`        - total amount of payout
* uint64_t `bets`      - total count of bets 
* uint64_t `wons`      - total count of wons

IMPORTANT:
1397703940 it is a number equivalent of symbol '4, EOS'

Example:
```json5
{
  "rows": [{
      "name": "4,EOS",
      "in": 0,
      "out": 0,
      "bets": 0,
      "wons": 0
    }
  ],
  "more": false
}
```

## ```top.day and top.month```

Leader boards by bets count from first to last.

```bash
./cleos.sh get table dice dice top.day
```
### Fields
* eosio::name account - account
* PlayerBetsStatistics stats - statistics

### Indexes
* primary key - index by account.value 
* `bybetscount` - index by bets count  

Example:
```json5
{
  "rows": [{
      "account": "user1",
      "stats": {
        "symbol": "4,EOS",
        "total_bet_amount": 400000004,
        "total_payout": 308771932,
        "bets": 4,
        "wons": 2
      }
    }
  ],
  "more": false
}
```

# Links

[Tables](tables.hpp)

[Dice actions description](resources/eos.dice.contracts.md)

[Debug config file](debug_config.hpp)

[Release config file](config.hpp)