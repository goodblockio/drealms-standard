/**
 * The Nifty Token Standard is a lightweight cross-game NFT standard for EOSIO software.
 * 
 * @author Craig Branscom
 * @contract nifty
 * @version v0.1
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


    //======================== tables ========================

    //@scope get_self().value
    //@ram 
    TABLE stats {
        name token_name;
        name creator;
        name licensing;
        bool burnable;
        bool transferable;
        uint64_t supply;
        uint64_t max_supply;

        uint64_t primary_key() const { return token_name.value; }
        EOSLIB_SERIALIZE(stats, 
            (token_name)(creator)
            (licensing)
            (burnable)(transferable)
            (supply)(max_supply))
    };
    typedef multi_index<name("statistics"), stats> stats_table;

    //@scope token_name.value
    //@ram 
    TABLE license {
        name owner;
        //TODO?: add name category; //TODO?: rename to token_theme?
        //TODO?: add time_point_sec expiration;
        //TODO?: add asset license_price; //set to zero if not applicable

        string manifest_uri; //details contents of unity package //TODO?: rename to ati_uri 
        string package_uri; //contains object.unitypackage at endpoint

        string asset_bundle_uri_head; //uri tail is on nft
        string json_uri_head; //uri tail is on nft
        
        uint64_t primary_key() const { return owner.value; }
        EOSLIB_SERIALIZE(license, 
            (owner)
            (manifest_uri)(package_uri)
            (asset_bundle_uri_head)(json_uri_head))
    };
    typedef multi_index<name("licenses"), license> licenses_table;


    //@scope token_name.value
    //@ram 
    TABLE nonfungible {
        uint64_t serial;
        name owner;

        string asset_bundle_uri_tail; //contains asset bundle at endpoint
        string json_uri_tail; //contains raw token meta in json format at endpoint //EX: tokenname=dragons&serial=5

        uint64_t primary_key() const { return serial; }
        EOSLIB_SERIALIZE(nonfungible, 
            (serial)(owner)
            (asset_bundle_uri_tail)(json_uri_tail))
    };
    typedef multi_index<name("nonfungibles"), nonfungible> nonfungibles_table;


    //@scope symbol.code().raw()
    //@ram 
    // TABLE fungible {
    //     asset balance;
    //     uint64_t primary_key() const { return balance.symbol.code().raw(); }
    //     EOSLIB_SERIALIZE(fungible, (balance))
    // };
    // typedef multi_index<name("fungibles"), fungible> fungibles_table;

    //@scope singleton
    //@ram 
    // TABLE config {
    //     string nifty_version;
    //     symbol core_sym;
    //     asset default_license_length;
    // };
    // typedef singleton<name("configs"), config> config_singleton;



    //======================== nonfungible actions ========================

    //creates a new token stats row, initially sets licensing to disabled
    ACTION create(name token_name, name creator, bool burnable, bool transferable, uint64_t max_supply);

    //issues a new NFT token
    ACTION issue(name recipient, name token_name, string query_string, string memo);

    //transfers nft(s) of a single token name to recipient account
    ACTION transfer(name recipient, name sender, name token_name, vector<uint64_t> serials, string memo);

    //burns nft(s) of a single token name if they are burnable
    ACTION burn(name creator, name token_name, vector<uint64_t> serials, string memo);

    //edits nft uris
    ACTION editnfturis(name token_name, uint64_t serial, name owner, string new_json_uri, string new_asset_bundle_uri); //TODO?: rename to editnftdata()



    //======================== licensing actions ========================

    //sets new licensing status for a token, and license price if applicable
    ACTION setlicensing(name token_name, name new_licensing, asset license_price); //TODO: make license_price optional param

    //adds a new license
    ACTION newlicense(name token_name, name owner, string ati_uri, string base_uri);

    //edits a current license uri
    ACTION edituris(name token_name, name owner, string new_ati_uri, string new_base_uri); //TODO?: rename to editlicuris

    //buys a new license for a token, triggered from the eosio.token::transfer action
    // [[eosio::on_notify("eosio.token::transfer")]]
    // void buylicense(name token_name, name owner);

    //renews a token license
    // ACTION renewlicense(name token_name, name owner, );



    //========== helper functions ==========

    bool validate_licensing(name licensing, asset license_price);



    //========== migration actions ==========

    //TODO: void delnft();

    //TODO: void delft();

    //TODO: void dellicense();

    //TODO: void delmeta();



};