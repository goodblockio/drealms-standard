/**
 * Nifty is an NFT standard for game-based tokens.
 * 
 * @author Craig Branscom
 * @contract nifty
 * @copyright defined in LICENSE.txt
 */

#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/permission.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/ignore.hpp>

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

        uint64_t primary_key() const { return token_name.value; }
        EOSLIB_SERIALIZE(stats, (token_name)(creator))
    };
    typedef multi_index<name("statistics"), stats> stats_table;


    //@scope token_name.value
    //@ram 
    TABLE nonfungible {
        uint64_t serial;
        string uri;

        uint64_t primary_key() const { return serial; }
        EOSLIB_SERIALIZE(nonfungible, (serial))
    };
    typedef multi_index<name("nonfungibles"), nonfungible> nonfungibles_table;


    //@scope token_name.value
    //@ram 
    TABLE license {
        name owner;

        uint64_t primary_key() const { return owner; }
        EOSLIB_SERIALIZE(license, (owner))
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

    //creates a new token stats row, creator always gets first license row for free
    ACTION create(name token_name, name creator); //TODO: add settings to allow/disallow license purchases

    //adds a new license to a token
    ACTION newlicense(name token_name, name owner);

    //mints a new NFT token
    ACTION mint();

    //========== functions ==========

    

    //========== reactions ==========



    //========== migration actions ==========

    //TODO: void delnft();

    //TODO: void delft();

    //TODO: void dellicense();

    //TODO: void delmeta();

};