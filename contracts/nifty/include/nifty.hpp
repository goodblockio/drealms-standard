/**
 * The Nifty Token Standard is a lightweight cross-game NFT standard for EOSIO software.
 * 
 * @author Craig Branscom
 * @contract nifty
 * @version v0.1.1
 * @copyright defined in LICENSE.txt
 */

#pragma once

#include <eosio/eosio.hpp>
// #include <eosio/permission.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/transaction.hpp>
#include <eosio/ignore.hpp>

using namespace std;
using namespace eosio;

CONTRACT nifty : public contract {

    public:

    nifty(name self, name code, datastream<const char*> ds);

    ~nifty();

    //constants
    const symbol CORE_SYM = symbol("TLOS", 4);
    const uint32_t DEFAULT_LICENSE_LENGTH = uint32_t(31536000); //1 year in seconds



    //======================== nonfungible actions ========================

    //creates a new token stats row, initially sets licensing to disabled
    ACTION createnft(name token_name, name issuer, bool burnable, bool transferable, uint64_t max_supply);

    //issues a new NFT token
    ACTION issuenft(name to, name token_name, string immutable_uri_tail, string memo);

    //transfers nft(s) of a single token name to recipient account
    ACTION transfernft(name from, name to, name token_name, vector<uint64_t> serials, string memo);

    //burns nft of a single token name, if burnable
    ACTION burnnft(name issuer, name token_name, vector<uint64_t> serials, string memo);

    //updates token stats table
    // ACTION updatestats(name token_name, bool burnable, bool transferable);

    //edits nft uris
    ACTION updatenft(name token_name, uint64_t serial, string new_mutable_uri_tail);



    //======================== licensing actions ========================

    //sets new license model on existing token stats
    ACTION setlicensing(name token_name, name new_license_model); //TODO?: rename to setlicmodel()

    //adds a new license
    ACTION newlicense(name token_name, name owner, time_point_sec expiration, string contract_uri);

    //renews an existing license
    ACTION renewlicense(name token_name, name owner, time_point_sec expiration, string contract_uri); //TODO: make exp and uri optional params

    //updates license uris
    ACTION updatelic(name token_name, name owner, 
        string new_ati_uri, string new_package_uri, string new_asset_bundle_uri_head, string new_json_uri_head);

    //revokes a license
    ACTION revokelic(name token_name, name license_owner);

    //buys a new license for a token, triggered from the eosio.token::transfer action
    // [[eosio::on_notify("eosio.token::transfer")]]
    // void buylicense(name from, name to, asset quantity, string memo);



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
    //     asset default_license_length;
    // };
    // typedef singleton<name("configs"), config> config_singleton;


    //@scope get_self().value
    //@ram ~322 bytes
    TABLE stats {
        name token_name;
        name issuer;
        name license_model;
        bool burnable;
        bool transferable;
        uint64_t supply;
        uint64_t max_supply;

        uint64_t primary_key() const { return token_name.value; }
        EOSLIB_SERIALIZE(stats, 
            (token_name)(issuer)
            (license_model)
            (burnable)(transferable)
            (supply)(max_supply))
    };
    typedef multi_index<name("statistics"), stats> stats_table; //TODO?: rename table
    

    //@scope token_name.value
    //@ram ~303 bytes
    TABLE license {
        name owner;
        time_point_sec expiration;
        string contract_uri; //blank if not applicable //TODO?: remove
        string ati_uri; //defines ATI for tokens using created under this license
        string package_uri; //contains object.unitypackage at endpoint
        string asset_bundle_uri_head;
        string json_uri_head;
        
        uint64_t primary_key() const { return owner.value; }
        EOSLIB_SERIALIZE(license, 
            (owner)(expiration)
            (contract_uri)(ati_uri)(package_uri)
            (asset_bundle_uri_head)(json_uri_head))
    };
    typedef multi_index<name("licenses"), license> licenses_table;


    //@scope token_name.value
    //@ram ~198 bytes
    TABLE nonfungible {
        uint64_t serial;
        name owner;
        string immutable_uri_tail; //immutable so updates don't break composite uri
        string mutable_uri_tail; //TODO?: make optional - std::optional<string> mutable_uri;

        uint64_t primary_key() const { return serial; }
        EOSLIB_SERIALIZE(nonfungible, 
            (serial)(owner)(immutable_uri_tail)(mutable_uri_tail))
    };
    typedef multi_index<name("nonfungibles"), nonfungible> nonfungibles_table;


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