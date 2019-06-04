/**
 * Nifty is an NFT standard for game-based tokens.
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


    const symbol CORE_SYM = symbol("TLOS", 4);
    const asset LICENSE_PRICE = asset(5000, CORE_SYM);


    //======================== tables ========================

    //@scope get_self().value
    //@ram 
    TABLE stats {
        name token_name;
        name creator;
        name licensing;

        //TODO: add license_price?
        //TODO: add supply?
        //TODO: add max_supply?
        //TODO: add bool burnable;
        //TODO: add bool transferable;

        uint64_t primary_key() const { return token_name.value; }
        EOSLIB_SERIALIZE(stats, (token_name)(creator)(licensing))
    };
    typedef multi_index<name("statistics"), stats> stats_table;


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


    //@scope token_name.value
    //@ram 
    TABLE license {
        name owner;
        string base_uri;

        uint64_t primary_key() const { return owner.value; }
        EOSLIB_SERIALIZE(license, (owner)(base_uri))
    };
    typedef multi_index<name("licenses"), license> licenses_table;


    //@scope symbol.code().raw()
    //@ram 
    // TABLE fungible {
    //     asset balance;
    //     uint64_t primary_key() const { return balance.symbol.code().raw(); }
    //     EOSLIB_SERIALIZE(fungible, (balance))
    // };
    // typedef multi_index<name("fungibles"), fungible> fungibles_table;



    //======================== nonfungible actions ========================

    //creates a new token stats row, creator always gets licenses for free
    ACTION create(name token_name, name creator, name licensing);

    //issues a new NFT token
    ACTION issue(name recipient, name token_name, string query_string, string memo);

    //transfers nft(s) of a single token name to recipient account
    ACTION transfer(name recipient, name sender, name token_name, vector<uint64_t> serials, string memo);

    //burns nft(s) of a single token name if they are burnable
    ACTION burn(name creator, name token_name, vector<uint64_t> serials, string memo);



    //======================== licensing actions ========================

    //sets new licensing status for a token
    ACTION setlicensing(name token_name, name new_licensing);

    //adds a new license to a token with open licensing
    ACTION addlicense(name token_name, name owner, string base_uri); //TODO: add price, make price and base_uri optional params?

    //edits a current license uri
    ACTION editlicense(name token_name, name owner, string new_base_uri); //TODO: add new_price, make new_price and new_base_uri optional params?

    //buys a new license for a token, triggered from the eosio.token::transfer action
    // [[eosio::on_notify("eosio.token::transfer")]]
    // void buylicense(name token_name, name owner);



    //========== helper functions ==========

    bool is_valid_licensing(name licensing);



    //========== migration actions ==========

    //TODO: void delnft();

    //TODO: void delft();

    //TODO: void dellicense();

    //TODO: void delmeta();



};