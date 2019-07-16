// dRealms is a lightweight cross-game NFT standard for EOSIO software.
// 
// @author Craig Branscom
// @contract drealms
// @version v0.1.0
// @copyright defined in LICENSE.txt

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/transaction.hpp>
#include <eosio/singleton.hpp>
#include <eosio/ignore.hpp>

using namespace std;
using namespace eosio;

//TODO?: bulk transfernfts action
//TODO?: use alternative license erasing? could just expire that way it keeps the table data

//TODO: add revokelic() that revokes an active license (only by issuer)
//TODO: rename all instances of token_name to token_family - easier to understand diff between tokens and token stats

CONTRACT drealms : public contract {

public:

    drealms(name self, name code, datastream<const char*> ds);

    ~drealms();

    //constants
    // const symbol CORE_SYM = symbol("TLOS", 4);
    // const uint32_t DEFAULT_LICENSE_LENGTH = 31536000; //1 year in seconds
    // const uint32_t MIN_LICENSE_LENGTH = 604800; //1 week in seconds
    // const uint32_t MAX_LICENSE_LENGTH = 62899200; //2 years in seconds

    //======================== admin actions ========================

    //sets config singleton
    ACTION setconfig(string drealms_version, symbol core_sym, name contract_owner, 
        uint32_t min_license_length, uint32_t max_license_length);

    //======================== nonfungible actions ========================

    //creates a new token stat, initially sets licensing to disabled
    ACTION createnft(name token_name, name issuer, bool retirable, bool transferable, bool consumable, uint64_t max_supply);

    //issues a new NFT token
    ACTION issuenft(name to, name token_name, string memo);

    //retires nft(s) of a single token name, if retirable
    ACTION retirenft(name token_name, vector<uint64_t> serials, string memo);

    //transfers nft(s) of a single token name to recipient account, if transferable
    ACTION transfernft(name from, name to, name token_name, vector<uint64_t> serials, string memo);

    //consumes an nft, if consumable
    ACTION consumenft(name token_name, uint64_t serial, string memo);

    //updates a checksum if found, inserts if not found
    ACTION newchecksum(name token_name, name license_owner, uint64_t serial, string new_checksum);

    //======================== licensing actions ========================

    //sets new license model on existing token stats
    ACTION setlicmodel(name token_name, name new_license_model);

    //adds a new license
    ACTION newlicense(name token_name, name owner, time_point_sec expiration);

    //erases a license
    ACTION eraselicense(name token_name, name license_owner);

    //sets a license's checksum algorithm
    ACTION setalgo(name token_name, name license_owner, string new_checksum_algo);

    //updates a license's ATI
    ACTION setati(name token_name, name license_owner, string new_ati_uri);

    //updates a uri if found, inserts if not found
    ACTION newuri(name token_name, name license_owner, name uri_group, name uri_name, string new_uri, optional<uint64_t> serial);

    //deletes a uri
    ACTION deleteuri(name token_name, name license_owner, name uri_group, name uri_name, optional<uint64_t> serial);

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
    ACTION open(name owner, symbol token_sym, name ram_payer);

    //closes a zero balance wallet
    ACTION close(name owner, symbol token_sym);

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
    //ram: 
    TABLE config {
        string drealms_version;
        symbol core_sym;
        name contract_owner;
        uint32_t default_license_length;
        uint32_t min_license_length;
        uint32_t max_license_length;
    };
    typedef singleton<name("config"), config> configs_singleton;

    //scope: get_self().value
    //ram: ~507 bytes
    TABLE stats {
        name token_name;
        name issuer;
        name license_model;
        bool retirable;
        bool transferable;
        bool consumable;
        uint64_t supply;
        uint64_t issued_supply;
        uint64_t max_supply;

        uint64_t primary_key() const { return token_name.value; }
        EOSLIB_SERIALIZE(stats, 
            (token_name)(issuer)(license_model)
            (retirable)(transferable)(consumable)
            (supply)(issued_supply)(max_supply))
    };
    typedef multi_index<name("stats"), stats> stats_table;

    //scope: token_name.value
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

    //scope: token_name.value
    //ram: ~198 bytes
    TABLE nonfungible {
        uint64_t serial;
        name owner;
        map<name, string> relative_uris;
        map<name, string> checksums;

        uint64_t primary_key() const { return serial; }
        EOSLIB_SERIALIZE(nonfungible, 
            (serial)(owner)(relative_uris)(checksums))
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