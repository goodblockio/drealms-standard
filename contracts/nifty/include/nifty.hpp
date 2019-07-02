/**
 * The Nifty Token Standard is a lightweight cross-game NFT standard for EOSIO software.
 * 
 * @author Craig Branscom
 * @contract nifty
 * @version v0.1.0
 * @copyright defined in LICENSE.txt
 */

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/transaction.hpp>
#include <eosio/ignore.hpp>

using namespace std;
using namespace eosio;

//TODO?: bulk transfernfts action
//TODO?: remove burnable feature
//TODO?: set immutable data after issue? im_data = set once, m_data = set whenever

CONTRACT nifty : public contract {

    public:

    nifty(name self, name code, datastream<const char*> ds);

    ~nifty();

    //constants
    const symbol CORE_SYM = symbol("TLOS", 4);
    const uint32_t DEFAULT_LICENSE_LENGTH = 31536000; //1 year in seconds
    const uint32_t MIN_LICENSE_LENGTH = 604800; //1 week in seconds
    const uint32_t MAX_LICENSE_LENGTH = 62899200; //2 years in seconds


    //======================== nonfungible actions ========================

    //creates a new token stat, initially sets licensing to disabled
    ACTION createnft(name token_name, name issuer, bool burnable, bool transferable, bool consumable, uint64_t max_supply);

    //issues a new NFT token
    ACTION issuenft(name to, name token_name, string immutable_data, string memo);

    //transfers nft(s) of a single token name to recipient account, if transferable
    ACTION transfernft(name from, name to, name token_name, vector<uint64_t> serials, string memo);

    //burns nft of a single token name, if burnable
    ACTION burnnft(name token_name, vector<uint64_t> serials, string memo);

    //consumes an nft, if consumable
    ACTION consumenft(name token_name, uint64_t serial, string memo);

    //edits nft uris
    ACTION updatenft(name token_name, uint64_t serial, string new_mutable_data);



    //======================== licensing actions ========================

    //sets new license model on existing token stats
    ACTION setlicensing(name token_name, name new_license_model);

    //adds a new license
    ACTION newlicense(name token_name, name owner, time_point_sec expiration);

    //revokes a license
    ACTION eraselicense(name token_name, name license_owner);

    //updates a uri if found, inserts if not found
    ACTION upserturi(name token_name, name license_owner, name uri_type, name uri_name, string new_uri);

    //removes a uri
    ACTION removeuri(name token_name, name license_owner, name uri_type, name uri_name);



    //======================== fungible actions ========================

    //creates a fungible token
    // ACTION create();

    //issues a fungible token
    // ACTION issue();

    //transfers fungible tokens
    // ACTION transfer();

    //burns fungible tokens
    // ACTION burn();

    //opens a zero balance wallet
    // ACTION open();

    //closes a zero balance wallet
    // ACTION close();

    //withdraws balance
    // ACTION withdraw();

    //deposits transfer
    // [[eosio::on_notify("eosio.token::transfer")]]
    // void deposit(name from, name to, asset quantity, string memo);

    

    //======================== admin actions ========================

    //upserts config singleton
    // ACTION setconfig(string nifty_version, symbol core_sym, name contract_owner, 
        // uint32_t default_license_length, uint32_t min_license_length, uint32_t max_license_length);
    


    //========== helper functions ==========

    bool validate_license_model(name license_model);



    //========== migration actions ==========

    ACTION delstats(name token_name);

    ACTION dellic(name token_name, name license_owner);

    ACTION delnft(name token_name, uint64_t serial);

    ACTION delcurr(symbol sym);

    ACTION delacct(name owner, symbol sym);



    //======================== tables ========================

    //@scope singleton
    //@ram 
    // TABLE config {
    //     string nifty_version;
    //     symbol core_sym;
    //     name contract_owner;
    //     uint32_t default_license_length;
    //     uint32_t min_license_length;
    //     uint32_t max_license_length;
    // };
    // typedef singleton<name("configs"), config> config_singleton;


    //@scope get_self().value
    //@ram ~507 bytes
    TABLE stats {
        name token_name;
        name issuer;
        name license_model;
        bool burnable;
        bool transferable;
        bool consumable;
        uint64_t supply;
        uint64_t issued_supply;
        uint64_t max_supply;

        uint64_t primary_key() const { return token_name.value; }
        EOSLIB_SERIALIZE(stats, 
            (token_name)(issuer)
            (license_model)
            (burnable)(transferable)(consumable)
            (supply)(issued_supply)(max_supply))
    };
    typedef multi_index<name("stats"), stats> stats_table;
    

    //@scope token_name.value
    //@ram ~303 bytes
    TABLE license {
        name owner;
        time_point_sec expiration;
        map<name, string> full_uris;
        map<name, string> base_uris;
        
        uint64_t primary_key() const { return owner.value; }
        EOSLIB_SERIALIZE(license, (owner)(expiration)(full_uris)(base_uris))
    };
    typedef multi_index<name("licenses"), license> licenses_table;


    //@scope token_name.value
    //@ram ~198 bytes
    TABLE nonfungible {
        uint64_t serial;
        name owner;
        string immutable_data; //immutable so updates don't break composite uri
        string mutable_data; //TODO?: make optional - std::optional<string> mutable_data;

        uint64_t primary_key() const { return serial; }
        EOSLIB_SERIALIZE(nonfungible, 
            (serial)(owner)(immutable_data)(mutable_data))
    };
    typedef multi_index<name("nfts"), nonfungible> nfts_table;


    //@scope get_self().value
    //@ram ~354 bytes
    TABLE currency {
        name issuer;
        bool burnable;
        bool transferable;
        asset supply;
        asset max_supply;
        
        uint64_t primary_key() const { return supply.symbol.code().raw(); }
        EOSLIB_SERIALIZE(currency, (issuer)(burnable)(transferable)(supply)(max_supply))
    };
    typedef multi_index<name("currencies"), currency> currencies_table;


    //@scope owner.value
    //@ram ~144 bytes
    TABLE account {
        asset balance;
        
        uint64_t primary_key() const { return balance.symbol.code().raw(); }
        EOSLIB_SERIALIZE(account, (balance))
    };
    typedef multi_index<name("accounts"), account> accounts_table;

};