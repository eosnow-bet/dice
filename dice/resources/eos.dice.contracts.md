<h1 class="contract"> 
    admin.set
</h1>

## ```admin.set(name caller, name admin)```
* `caller` - User who want to call this action
* `admin` - New admin account

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> admin.set '["<owner>", "<admin>"]' -p <owner>@active
```

<h1 class="contract"> 
    betting.set
</h1>

## ```betting.set(eosio::name caller, bool enabled)```
* `caller` - User who want to call this action
* `enabled` - True | False

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> betting.set '["<owner_or_admin>", 0]' -p <owner_or_admin>@active
cleos push action <owner> betting.set '["<owner_or_admin>", 1]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    minting.set
</h1>

## ```minting.set(eosio::name caller, bool enabled)```
* `caller` - User who want to call this action
* `enabled` - True | False

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> minting.set '["<owner_or_admin>", 0]' -p <owner_or_admin>@active
cleos push action <owner> minting.set '["<owner_or_admin>", 1]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    payout.set
</h1>

## ```payout.set(eosio::name caller, bool enabled)```
* `caller` - User who want to call this action
* `enabled` - True | False

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> payout.set '["<owner_or_admin>", 0]' -p <owner_or_admin>@active
cleos push action <owner> payout.set '["<owner_or_admin>", 1]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    ante.set
</h1>

## ```ante.set(eosio::name caller, eosio::name name)```
* `caller` - User who want to call this action
* `name` - Name of eos.ante owner account

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> ante.set '["<owner_or_admin>", "ante_contract_owner"]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    params.set
</h1>

## ```params.set(eosio::name caller, uint16_t min, uint16_t max, uint16_t max_bet_num)```
* `caller` - User who want to call this action
* `min` - lower bound for dice game
* `max` - upper bound for dice game
* `max_bet_num` - additional parameter for reward calculation

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> params.set '["<owner_or_admin>", 1, 100, 100]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    protect.set
</h1>

## ```protect.set(eosio::name caller, eosio::asset balance_protect)```
* `caller` - User who want to call this action
* `balance_protect` - Minimum value of eos balance to allow betting for players 

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> protect.set '["<owner_or_admin>", "1000.0000 EOS"]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    fee.set
</h1>

## ```fee.set(eosio::name caller, double platform_fee)```
* `caller` - User who want to call this action
* `platform_fee` - multiplier as percent i.e.: 0.01 == 1% 

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> fee.set '["<owner_or_admin>", 0.01]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    minbet.set    
</h1>

## ```minbet.set(eosio::name caller, eosio::asset min_bet)```
* `caller` - User who want to call this action
* `min_bet` - Minimum allowed bet amount 

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> minbet.set '["<owner_or_admin>", "1.0001 EOS"]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    rate.set
</h1>

## ```rate.set(eosio::name caller, double rate)```
* `caller` - User who want to call this action
* `rate` - How many ante tokens in 1 EOS (conversion rate) 

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> rate.set '["<owner_or_admin>", 1.2]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    balance.set
</h1>

## ```balance.set(eosio::name caller, eosio::asset balance)```
* `caller` - User who want to call this action
* `balance` - Set current eos balance of contract (It is necessary to add existing EOS after contract deploy)
 
Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> balance.set '["<owner_or_admin>", "1000.0000 EOS"]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    mbp.set
</h1>

## ```mbp.set(eosio::name caller, double max_bet_percent)```
* `caller` - User who want to call this action
* `max_bet_percent` - It's a percent from eos balance. Can be used as limit for maximum bet and maximum reward. 

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> mbp.set '["<owner_or_admin>", 0.10]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    bets.setl
</h1>

Change size of `bets.all` table

## ```bets.setl(eosio::name caller, uint64_t size)```
* `caller` - User who want to call this action
* `size` - Length of bets history

```
Example:
cleos push action <owner> bets.setl '["<owner_or_admin>", 10]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    high.setl
</h1>

Change size of `bets.high` table

## ```high.setl(eosio::name caller, uint64_t size)```
* `caller` - User who want to call this action
* `size` - Length of bets history

```
Example:
cleos push action <owner> high.setl '["<owner_or_admin>", 10]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    rare.setl
</h1>

Change size of `bets.rare` table

## ```rare.setl(eosio::name caller, uint64_t size)```
* `caller` - User who want to call this action
* `size` - Length of bets history

```
Example:
cleos push action <owner> rare.setl '["<owner_or_admin>", 10]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    rare.bet.set
</h1>

## ```rare.bet.set(eosio::name caller, uint16_t rare_bet_bound)```
* `caller` - User who want to call this action
* `rare_bet_bound` - Minimum probability in percent to add record to `bets.rare`  

```
Example:
cleos push action <owner> rare.bet.set '["<owner_or_admin>", "1.0000 EOS"]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    high.bet.set
</h1>

## ```high.bet.set(eosio::name caller, eosio::asset high_bet_bound))```
* `caller` - User who want to call this action
* `high_bet_bound` - Minimum value for `bets.high`

```
Example:
cleos push action <owner> high.bet.set '["<owner_or_admin>", "1.0000 EOS"]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    bet
</h1>

## ```bet(eosio::name player, eosio::name inviter, eosio::asset quantity, uint8_t roll_type, uint16_t roll_border)```
* `player` - User who made bet
* `inviter` - User who invited this player to play the game
* `quantity` - Bet amount ("1.0000 EOS")
* `roll_type` - Bet type (1 | 2)
* `roll_border` - Border parameter for betting.  

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> bet'["<player_who_bets>", "<player_who_invited>", "1.0001 EOS", 1, 42]' -p <owner_or_admin>@active
cleos push action <owner> bet'["<player_who_bets>", "<player_who_invited>", "1.0001 EOS", 2, 21]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    resolved
</h1>

## ```resolved(eosio::name player, eosio::name inviter, eosio::asset quantity, uint8_t roll_type, uint16_t roll_border)```
* `player` - User who made bet
* `inviter` - User who invited this player to play the game
* `quantity` - Bet amount ("1.0000 EOS")
* `roll_type` - Bet type (1 | 2)
* `roll_border` - Border parameter for betting.  

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> resolved'["<player_who_bets>", "<player_who_invited>", "1.0001 EOS", 1, 42]' -p <owner_or_admin>@active
cleos push action <owner> resolved'["<player_who_bets>", "<player_who_invited>", "1.0001 EOS", 2, 21]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    distribute
</h1>

## ```distribute(eosio::name caller, uint8_t type, const std::vector\<eosio::name\>& leaders, eosio::asset bonus)```
* `caller` - Account who call this action
* `type` - Type of distribution, day or month (1 | 2)
* `leaders` - List of leaders from 1st to last
* `bonus` - Balance in eos to distribute 

Expected permissions: `owner` or `admin`


<h1 class="contract"> 
    dlp.set    
</h1>

Change day_leader_board.bonus_percent

## ```dlp.set(eosio::name caller, double percent)```
* `caller` - User who want to call this action
* `percent` - percent from current eos_balance to distribute 

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> dlp.set '["<owner_or_admin>", 0.11]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    mlp.set    
</h1>

Change month_leader_board.bonus_percent

## ```mlp.set(eosio::name caller, double percent)```
* `caller` - User who want to call this action
* `percent` - percent from current eos_balance to distribute 

Expected permissions: `owner` or `admin`

```
Example:
cleos push action <owner> mlp.set '["<owner_or_admin>", 0.11]' -p <owner_or_admin>@active
```

<h1 class="contract"> 
    notify    
</h1>

Fake action with string parameter to notify admin\owner about something. For internal usage only.

## ```notify(std::string msg)```

Expected permissions: `owner` or `admin`