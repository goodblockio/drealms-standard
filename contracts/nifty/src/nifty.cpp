#include <nifty.hpp>

nifty::nifty(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

nifty::~nifty() {}

//======================== nonfungible actions ========================

ACTION nifty::createnft(name token_name, name issuer, bool burnable, bool transferable, bool consumable, uint64_t max_supply) {
    //authenticate
    require_auth(issuer);

    //open stats table, search for token_name
    stats_table stats(get_self(), get_self().value);
    auto stat = stats.find(token_name.value);

    //validate
    check(stat == stats.end(), "token name already exists");
    check(max_supply > 0, "max supply must be a positive number");
    
    //emplace new stats
    stats.emplace(issuer, [&](auto& col) {
        col.token_name = token_name;
        col.issuer = issuer;
        col.license_model = name("disabled");
        col.burnable = burnable;
        col.transferable = transferable;
        col.consumable = consumable;
        col.supply = uint64_t(0);
        col.max_supply = max_supply;
    });

    //open licenses table, find license
    licenses_table licenses(get_self(), token_name.value);
    auto lic = licenses.find(issuer.value);

    //validate
    check(lic == licenses.end(), "license already exists for token name");

    //emplace new license slot
    licenses.emplace(issuer, [&](auto& col) {
        col.owner = issuer;
        col.expiration = time_point_sec(current_time_point());
        col.contract_uri = "";
        col.ati_uri = "";
        col.package_uri = "";
        col.asset_bundle_uri= "";
        col.json_uri= "";
    });

}

ACTION nifty::issuenft(name to, name token_name, string immutable_data, string memo) {
    //open stats table, get token data
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //authenticate
    require_auth(stat.issuer);

    //validate
    check(is_account(to), "to account does not exist");
    check(stat.supply + 1 <= stat.max_supply, "token at max supply");
    // check(immutable_data != "", "immutable data cannot be blank");

    //increment nft supply
    stats.modify(stat, same_payer, [&](auto& col) {
        col.supply += uint64_t(1);
    });

    //open nfts table
    nfts_table nfts(get_self(), token_name.value);

    //get new serial
    uint64_t new_serial = nfts.available_primary_key();

    //emplace new NFT
    nfts.emplace(stat.issuer, [&](auto& col) {
        col.serial = new_serial;
        col.owner = to;
        col.immutable_data = immutable_data;
        col.mutable_data = "";
    });

}

ACTION nifty::transfernft(name from, name to, name token_name, vector<uint64_t> serials, string memo) {
    //authenticate
    require_auth(from);

    //opens stats table, get token data
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //validate
    check(stat.transferable, "token is not transferable");

    //loop over each serial and change ownership
    for (uint64_t serial : serials) {
        //open nfts table, get nft
        nfts_table nfts(get_self(), token_name.value);
        auto& nft = nfts.get(serial, "nft not found");

        //validate
        check(from == nft.owner, "only nft owner is allowed to transfer");

        //modify nft ownership to recipient
        nfts.modify(nft, same_payer, [&](auto& col) {
            col.owner = to;
        });

    }

}

ACTION nifty::burnnft(name token_name, vector<uint64_t> serials, string memo) {
    //get stats table
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //authenticate
    require_auth(stat.issuer);

    //validate
    check(stat.burnable, "token is not burnable");
    check(stat.supply >= serials.size(), "cannot burn supply below 0");

    //decrement nft supply
    stats.modify(stat, same_payer, [&](auto& col) {
        col.supply -= serials.size();
    });

    //loop over each serial and erase nft
    for (uint64_t serial : serials) {
        //open nfts table, get nft
        nfts_table nfts(get_self(), token_name.value);
        auto& nft = nfts.get(serial, "nft not found");

        //check that issuer owns each nft before erasing
        check(nft.owner == stat.issuer, "only issuer may burn tokens");

        //erase nft
        nfts.erase(nft);
    }

}

ACTION nifty::consumenft(name token_name, uint64_t serial, string memo) {
    //open stats table, get stat
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //open nfts table, get nft
    nfts_table nfts(get_self(), token_name.value);
    auto& nft = nfts.get(serial, "nft not found");

    //authenticate
    require_auth(nft.owner);

    //validate
    check(stat.consumable, "nft is not consumable");

    //decrement nft supply
    stats.modify(stat, same_payer, [&](auto& col) {
        col.supply -= uint64_t(1);
    });

    //erase nft
    nfts.erase(nft);
}

ACTION nifty::updatenft(name token_name, uint64_t serial, string new_mutable_data) {
    //open stats table, get stats
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //open nft table, get nft
    nfts_table nfts(get_self(), token_name.value);
    auto& nft = nfts.get(serial, "nft not found");

    //authenticate
    require_auth(stat.issuer);

    //modify nft mutable uri
    nfts.modify(nft, same_payer, [&](auto& col) {
        col.mutable_data = new_mutable_data;
    });

}



//======================== licensing actions ========================

ACTION nifty::setlicensing(name token_name, name new_license_model) {
    //open stats table, get token
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //authenticate
    require_auth(stat.issuer);

    //validate
    check(validate_license_model(new_license_model), "invalid license model");

    //modify licensing
    stats.modify(stat, same_payer, [&](auto& col) {
        col.license_model = new_license_model;
    });
    
}

ACTION nifty::newlicense(name token_name, name owner, time_point_sec expiration, string contract_uri) {
    //open stats table, get stats
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //intialize defaults
    name ram_payer = owner;
    time_point_sec new_expiration = time_point_sec(current_time_point()) + DEFAULT_LICENSE_LENGTH;
    time_point_sec min_expiration = time_point_sec(current_time_point()) + MIN_LICENSE_LENGTH;
    time_point_sec max_expiration = time_point_sec(current_time_point()) + MAX_LICENSE_LENGTH;

    //validate
    switch (stat.license_model.value) 
    {
        case name("disabled").value : 
            check(false, "licensing is disabled");
            break;
        case name("open").value : 
            require_auth(owner);
            check(expiration > min_expiration, "expiration is less than minimum expiration");
            check(expiration < max_expiration, "expiration is more than maximum expiration");
            // if (expiration < min_expiration) {
            //     new_expiration = time_point_sec(current_time_point()) + MIN_LICENSE_LENGTH;
            // } else if (expiration > max_expiration) {
            //     new_expiration = time_point_sec(current_time_point()) + MAX_LICENSE_LENGTH;
            // }
            break;
        // case name("purchasable").value : 
        //     require_auth(get_self());
        //     check(false, "in development...");
        //     break;
        case name("permissioned").value : 
            require_auth(stat.issuer);
            ram_payer = stat.issuer;
            new_expiration = expiration;
            break;
        default:
            check(false, "invalid licensing");
    }

    //open license table, search for license
    licenses_table licenses(get_self(), token_name.value);
    auto lic = licenses.find(owner.value);

    if (lic == licenses.end()) {
        //emplace new license
        licenses.emplace(ram_payer, [&](auto& col) {
            col.owner = owner;
            col.expiration = new_expiration;
            col.contract_uri = "";
            col.ati_uri = "";
            col.package_uri = "";
            col.asset_bundle_uri= "";
            col.json_uri= "";
        });
    } else {
        //renew existing license
        licenses.modify(*lic, same_payer, [&](auto& col) {
            col.expiration = new_expiration;
            col.contract_uri = contract_uri;
        });
    }

}

ACTION nifty::editlicense(name token_name, name owner, string new_ati_uri, string new_package_uri, string new_asset_bundle_uri, string new_json_uri) {
    //authenticate
    require_auth(owner);

    //open licenses table, get license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(owner.value, "license not found");

    //validate
    check(owner == lic.owner, "only license owner may update license data");

    //modify license uris
    licenses.modify(lic, same_payer, [&](auto& col) {
        col.ati_uri = new_ati_uri;
        col.package_uri = new_package_uri;
        col.asset_bundle_uri = new_asset_bundle_uri;
        col.json_uri= new_json_uri;
    });

}

ACTION nifty::eraselicense(name token_name, name license_owner) {
    //open stats table, get stats
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //authenticate
    require_auth(stat.issuer);

    //open licenses table, get license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //TODO?: use alternative revocation?
    //update license expiration to now (alternative to erasing license)
    // licenses.modify(lic, same_payer, [&](auto& col){
    //     col.expiration = time_point_sec(current_time_point());
    // });

    //erase license slot
    licenses.erase(lic);

}



//========== helper functions ==========

bool nifty::validate_license_model(name license_model) {
    
    switch (license_model.value) 
    {
        case name("disabled").value :
            break;
        case name("open").value : 
            break;
        case name("purchasable").value :
            break;
        case name("permissioned").value :
            break;
        default:
            return false;
    }

    return true;
}



//========== migration actions ==========

ACTION nifty::delstats(name token_name) {
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");
    stats.erase(stat);
}

ACTION nifty::dellic(name token_name, name license_owner) {
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");
    licenses.erase(lic);
}

ACTION nifty::delnft(name token_name, uint64_t serial) {
    nfts_table nfts(get_self(), token_name.value);
    auto& nft = nfts.get(serial, "nft not found");
    nfts.erase(nft);
}

ACTION nifty::delcurr(symbol sym) {
    currencies_table currencies(get_self(), get_self().value);
    auto& curr = currencies.get(sym.code().raw(), "currency not found");
    currencies.erase(curr);
}

ACTION nifty::delacct(name owner, symbol sym) {
    accounts_table accounts(get_self(), owner.value);
    auto& acct = accounts.get(sym.code().raw(), "account not found");
    accounts.erase(acct);
}



//========== dispatcher ==========

// extern "C"
// {
//     void apply(uint64_t receiver, uint64_t code, uint64_t action)
//     {
//         if (code == receiver)
//         {
//             switch (action)
//             {
//                 EOSIO_DISPATCH_HELPER(marketplace, 
//                     (listforsale)(buyitem)(redeem) //marketplace
//                     (setkey)(newvoucher)(regcurrency)(regdeveloper)(regcategory) //admin
//                     (delvoucher)); //migration tools
//             }
//         } else if (code == name("eosio.token").value && action == name("transfer").value) {
//             execute_action<marketplace>(eosio::name(receiver), eosio::name(code), &marketplace::catch_transfer);
//         } else if (code == name("dgoods").value && action == name("transfernft").value) {
//             execute_action<marketplace>(eosio::name(receiver), eosio::name(code), &marketplace::catch_dgoods_transfernft);
//         }
//     }
// }

// extern "C" bool pre_dispatch(name self, name original_receiver, name action) {
//    print_f("pre_dispatch : % % %\n", self, original_receiver, action);
//    name nm;
//    read_action_data((char*)&nm, sizeof(nm));
//    if (nm == "quit"_n) {
//       return false;
//    }
//    return true;
// }

// extern "C" void post_dispatch(name self, name original_receiver, name action) {
//    print_f("post_dispatch : % % %\n", self, original_receiver, action);
//    std::set<name> valid_actions = {"test1"_n, "test2"_n, "test4"_n, "test5"_n};
//    check(valid_actions.count(action) == 0, "valid action should have dispatched");
//    check(self == "eosio"_n, "should only be eosio for action failures");
// }                                                                                                                  