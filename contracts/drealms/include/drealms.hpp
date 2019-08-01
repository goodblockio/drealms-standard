// dRealms is a cross-game token standard for EOSIO software.
// 
// @author Craig Branscom
// @contract drealms
// @version v0.2.0
// @copyright defined in LICENSE.txt

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/transaction.hpp>
#include <eosio/singleton.hpp>
#include <eosio/ignore.hpp>

// #include <map>

using namespace std;
using namespace eosio;

//TODO: update fungibles and nonfungibles vectors when creating new schema or fts
//TODO: design next_level formulae
//TODO: refactor table opens
//TODO: remove next_level from nft, instead set level-up formula in schema and calculate next_level from current level

//TODO?: rename issuenft() to mintnft()
//TODO?: move relatives and checksums to separate table
//TODO?: change license key to license_name

CONTRACT drealms : public contract {

public:

    drealms(name self, name code, datastream<const char*> ds);

    ~drealms();

    //======================== realm actions ========================

    //sets realmdata singleton
    ACTION setrealmdata(string drealms_version, name realm_name);

    //======================== schema actions ========================

    //creates a new nft schema, initially sets licensing to disabled
    ACTION newnftschema(name new_schema_name, name issuer, uint64_t max_supply, symbol exp_symbol,
        bool retirable, bool transferable, bool consumable, bool activatable);

    //toggles a schema setting
    ACTION toggle(name schema_name, name setting_name);

    //adds a stat to defaults
    ACTION addstat(name schema_name, name stat_name, uint32_t default_value);

    //syncs a token with the map of stats in schema
    ACTION syncstats(name schema_name, uint64_t serial);

    //TODO: make action to set levelling formula

    //called to award an nft with experience
    ACTION awardexp(name schema_name, name license_owner, uint64_t serial, asset experience);

    //======================== licensing actions ========================

    //sets new license model on existing token stats
    ACTION setlicmodel(name schema_name, name new_license_model);

    //adds a new license
    ACTION newlicense(name schema_name, name owner, time_point_sec expiration);

    //erases a license
    ACTION eraselicense(name schema_name, name license_owner);

    //sets minimum and maximum license lengths
    ACTION setlicminmax(name schema_name, uint32_t min_license_length, uint32_t max_license_length);

    //sets a license's checksum algorithm
    ACTION setalgo(name schema_name, name license_owner, string new_checksum_algo);

    //updates a license's ATI
    ACTION setati(name schema_name, name license_owner, string new_ati_uri);

    //updates a uri if found, inserts if not found
    ACTION newuri(name schema_name, name license_owner, name uri_group, name uri_name, string new_uri, optional<uint64_t> serial);

    //deletes a uri
    ACTION deleteuri(name schema_name, name license_owner, name uri_group, name uri_name, optional<uint64_t> serial);

    //======================== nonfungible actions ========================

    //issues a new nft
    ACTION issuenft(name to, name schema_name, string memo, bool log);

    //retires nft(s) of a single token name, if retirable
    ACTION retirenft(name schema_name, vector<uint64_t> serials, string memo);

    //transfers nft(s) of a single token name to recipient account, if transferable
    ACTION transfernft(name from, name to, name schema_name, vector<uint64_t> serials, string memo);

    //consumes an nft, if consumable
    ACTION consumenft(name schema_name, uint64_t serial, string memo);

    //activate and nft, if activatable
    ACTION activatenft(name schema_name, uint64_t serial, string memo);

    //updates a checksum if found, inserts if not found
    ACTION newchecksum(name schema_name, name license_owner, uint64_t serial, string new_checksum);

    //logs an nfts data
    ACTION lognft(name to, name schema_name, uint64_t serial);

    //spends earned experience to level up
    ACTION levelup(name schema_name, uint64_t serial);

    //spends an available point on a stat to upgrade it
    ACTION spendpoint(name schema_name, uint64_t serial, name stat_name);

    //======================== fungible actions ========================

    //creates a fungible token
    ACTION create(name issuer, bool retirable, bool transferable, bool consumable, asset max_supply);

    //issues a fungible token
    ACTION issue(name to, asset quantity, string memo);

    //retires fungible tokens
    ACTION retire(asset quantity, string memo);

    //transfers fungible tokens
    ACTION transfer(name from, name to, asset quantity, string memo);

    //consumes a fungible token
    ACTION consume(name owner, asset quantity, string memo);

    //opens a zero balance wallet
    ACTION open(name owner, symbol currency_symbol, name ram_payer);

    //closes a zero balance wallet
    ACTION close(name owner, symbol currency_symbol);

    //withdraws balance
    // ACTION withdraw();

    //deposits transfer
    // [[eosio::on_notify("eosio.token::transfer")]]
    // void deposit(name from, name to, asset quantity, string memo);

    //========== helper functions ==========

    bool validate_license_model(name license_model);

    bool validate_uri_group(name uri_group);

    void add_balance(name to, asset quantity, name ram_payer);

    void sub_balance(name from, asset quantity);

    //======================== tables ========================

    //scope: singleton
    //ram: ~40 bytes
    TABLE realmdata {
        string drealms_version;
        name realm_name;
        vector<name> nonfungibles; //TODO?: rename to schemas
        vector<symbol> fungibles; //TODO?: rename to currencies
    };
    typedef singleton<name("realmdata"), realmdata> realmdata_singleton;

    //scope: get_self().value
    //ram: ~507 bytes
    TABLE schema {
        name schema_name;
        name issuer;
        
        uint64_t supply;
        uint64_t issued_supply;
        uint64_t max_supply;

        name license_model;
        uint32_t min_license_length;
        uint32_t max_license_length;

        map<name, bool> settings; //retirable, transferable, consumable, activatable
        map<name, uint32_t> default_stats; //defaults used when minting a new nft
        symbol exp_symbol;

        uint64_t primary_key() const { return schema_name.value; }
        EOSLIB_SERIALIZE(schema, 
            (schema_name)(issuer)
            (supply)(issued_supply)(max_supply)
            (license_model)(min_license_length)(max_license_length)
            (settings)(default_stats)(exp_symbol))
    };
    typedef multi_index<name("schemas"), schema> schemas_table;

    //scope: schema_name.value
    //ram: ~303 bytes
    TABLE license {
        name owner;
        time_point_sec expiration;
        string checksum_algo;
        map<name, string> full_uris;
        map<name, string> base_uris;
        
        uint64_t primary_key() const { return owner.value; }
        EOSLIB_SERIALIZE(license, (owner)(expiration)(checksum_algo)(full_uris)(base_uris))
    };
    typedef multi_index<name("licenses"), license> licenses_table;

    //scope: schema_name.value
    //ram: ~198 bytes
    TABLE nonfungible {
        uint64_t serial;
        name owner;

        uint16_t level; //starts at level 1
        asset experience; //EXP, SCALES, etc
        asset next_level; //same formula for all members of schema (linear, quadratic, etc)
        uint8_t unspent; //points to spend on upgrading stats
        map<name, uint32_t> stats; //strength => 1, dexterity => 1, etc.

        map<name, string> relative_uris;
        map<name, string> checksums;

        uint64_t primary_key() const { return serial; }
        EOSLIB_SERIALIZE(nonfungible, 
            (serial)(owner)
            (level)(experience)(next_level)(unspent)(stats)
            (relative_uris)(checksums))
    };
    typedef multi_index<name("nfts"), nonfungible> nfts_table;

    //scope: get_self().value
    //ram: ~354 bytes
    TABLE currency {
        name issuer;
        bool retirable;
        bool transferable;
        bool consumable;
        asset supply;
        asset max_supply;
        
        uint64_t primary_key() const { return supply.symbol.code().raw(); }
        EOSLIB_SERIALIZE(currency, (issuer)(retirable)(transferable)(consumable)(supply)(max_supply))
    };
    typedef multi_index<name("currencies"), currency> currencies_table;

    //scope: owner.value
    //ram: ~144 bytes
    TABLE account {
        asset balance;
        
        uint64_t primary_key() const { return balance.symbol.code().raw(); }
        EOSLIB_SERIALIZE(account, (balance))
    };
    typedef multi_index<name("accounts"), account> accounts_table;

};