/**
 * The Nifty Token Standard is a lightweight, scalable, and licensable cross-game NFT standard for EOSIO software.
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
        asset license_price; //set to zero if not applicable
        bool burnable;
        bool transferable;
        uint64_t supply;
        uint64_t max_supply;

        uint64_t primary_key() const { return token_name.value; }
        EOSLIB_SERIALIZE(stats, (token_name)(creator)
            (licensing)(license_price)
            (burnable)(transferable)
            (supply)(max_supply))
    };
    typedef multi_index<name("statistics"), stats> stats_table;

    //@scope token_name.value
    //@ram 
    TABLE license {
        name owner;
        string ati_uri;
        string base_uri;

        //TODO?: add time_point expiration;

        uint64_t primary_key() const { return owner.value; }
        EOSLIB_SERIALIZE(license, (owner)(ati_uri)(base_uri))
    };
    typedef multi_index<name("licenses"), license> licenses_table;


    //@scope token_name.value
    //@ram 
    TABLE nonfungible {
        uint64_t serial;
        name owner;
        string query_string; //EX: tokenname=dragons&serial=5

        uint64_t primary_key() const { return serial; }
        EOSLIB_SERIALIZE(nonfungible, (serial)(owner)(query_string))
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



    //======================== nonfungible actions ========================

    //creates a new token stats row, initially sets licensing to disabled
    ACTION create(name token_name, name creator, bool burnable, bool transferable, uint64_t max_supply);

    //issues a new NFT token
    ACTION issue(name recipient, name token_name, string query_string, string memo);

    //transfers nft(s) of a single token name to recipient account
    ACTION transfer(name recipient, name sender, name token_name, vector<uint64_t> serials, string memo);

    //burns nft(s) of a single token name if they are burnable
    ACTION burn(name creator, name token_name, vector<uint64_t> serials, string memo);



    //======================== licensing actions ========================

    //sets new licensing status for a token, and license price if applicable
    ACTION setlicensing(name token_name, name new_licensing, asset license_price); //TODO: make license_price optional param

    //adds a new license to a token with open licensing
    ACTION newlicense(name token_name, name owner, string ati_uri, string base_uri);

    //edits a current license uri
    ACTION edituris(name token_name, name owner, string new_ati_uri, string new_base_uri);

    //buys a new license for a token, triggered from the eosio.token::transfer action
    // [[eosio::on_notify("eosio.token::transfer")]]
    // void buylicense(name token_name, name owner);



    //========== helper functions ==========

    bool validate_licensing(name licensing, asset license_price);



    //========== migration actions ==========

    //TODO: void delnft();

    //TODO: void delft();

    //TODO: void dellicense();

    //TODO: void delmeta();



};