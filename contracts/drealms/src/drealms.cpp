#include <drealms.hpp>

drealms::drealms(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

drealms::~drealms() {}

//======================== nonfungible actions ========================

ACTION drealms::createnft(name token_name, name issuer, bool burnable, bool transferable, bool consumable, uint64_t max_supply) {
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
        col.issued_supply = uint64_t(0);
        col.max_supply = max_supply;
    });

    //open licenses table, find license
    licenses_table licenses(get_self(), token_name.value);
    auto lic = licenses.find(issuer.value);

    //validate
    check(lic == licenses.end(), "license already exists for token name");

    //build initial uri maps
    map<name, string> new_full_uris;
    map<name, string> new_base_uris;

    //emplace new license slot
    licenses.emplace(issuer, [&](auto& col) {
        col.owner = issuer;
        col.expiration = time_point_sec(current_time_point());
        col.checksum_algo = "";
        col.full_uris = new_full_uris;
        col.base_uris = new_base_uris;
    });
}

ACTION drealms::issuenft(name to, name token_name, string memo) {
    //open stats table, get token stats
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //authenticate
    require_auth(stat.issuer);

    //validate
    check(is_account(to), "to account does not exist");
    check(stat.supply + 1 <= stat.max_supply, "token at max supply");

    //open nfts table, get new serial
    nfts_table nfts(get_self(), token_name.value);
    uint64_t new_serial = stat.issued_supply + 1;

    //increment nft supply and issued supply
    stats.modify(stat, same_payer, [&](auto& col) {
        col.issued_supply += uint64_t(1);
        col.supply += uint64_t(1);
    });

    //build initial uri and checksum maps
    map<name, string> new_relative_uris;
    map<name, string> new_checksums;

    //emplace new NFT
    nfts.emplace(stat.issuer, [&](auto& col) {
        col.serial = new_serial;
        col.owner = to;
        col.relative_uris = new_relative_uris;
        col.checksums = new_checksums;
    });

    //notify recipient account
    require_recipient(to);
}

ACTION drealms::transfernft(name from, name to, name token_name, vector<uint64_t> serials, string memo) {
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

    //notify accounts
    require_recipient(from);
    require_recipient(to);
}

ACTION drealms::burnnft(name token_name, vector<uint64_t> serials, string memo) {
    //get stats table
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //authenticate
    require_auth(stat.issuer);

    //validate
    check(stat.burnable, "token is not burnable");
    check(stat.supply >= serials.size(), "cannot burn supply below 0");

    //reduce nft supply
    stats.modify(stat, same_payer, [&](auto& col) {
        col.supply -= serials.size();
    });

    //loop over each serial and erase nft
    for (uint64_t serial : serials) {
        //open nfts table, get nft
        nfts_table nfts(get_self(), token_name.value);
        auto& nft = nfts.get(serial, "nft not found");

        //check that issuer owns each nft before burning
        check(nft.owner == stat.issuer, "only issuer may burn tokens");

        //burn nft
        nfts.erase(nft);
    }
}

ACTION drealms::consumenft(name token_name, uint64_t serial, string memo) {
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

    //consume nft
    nfts.erase(nft);
}

ACTION drealms::newchecksum(name token_name, name license_owner, uint64_t serial, string new_checksum) {
    //open stats table, get stats
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //open licenses table, get license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //open nft table, get nft
    nfts_table nfts(get_self(), token_name.value);
    auto& nft = nfts.get(serial, "nft not found");

    //validate
    check(nft.relative_uris.find(license_owner) != nft.relative_uris.end(), "checksums must have an associated relative uri");

    //modify nft mutable uri
    nfts.modify(nft, same_payer, [&](auto& col) {
        col.checksums[license_owner] = new_checksum;
    });
}



//======================== licensing actions ========================

ACTION drealms::setlicmodel(name token_name, name new_license_model) {
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

ACTION drealms::newlicense(name token_name, name owner, time_point_sec expiration) {
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
            break;
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
        //build initial uri maps
        map<name, string> new_full_uris;
        map<name, string> new_base_uri;

        //emplace new license
        licenses.emplace(ram_payer, [&](auto& col) {
            col.owner = owner;
            col.expiration = new_expiration;
            col.checksum_algo = "";
            col.full_uris = new_full_uris;
            col.base_uris = new_base_uri;
        });
    } else {
        //renew existing license
        licenses.modify(*lic, same_payer, [&](auto& col) {
            col.expiration = new_expiration;
        });
    }
}

ACTION drealms::eraselicense(name token_name, name license_owner) {
    //open stats table, get stats
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //open licenses table, get license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //determine if expired
    bool expired = time_point_sec(current_time_point()) > lic.expiration;

    //authenticate based on expired
    if (expired) {
        check(has_auth(lic.owner) || has_auth(stat.issuer), "only token issuer or license owner may erase after expiration");
    } else {
        require_auth(lic.owner);
    }
    
    //validate
    check(expired, "license has not expired");

    //erase license slot
    licenses.erase(lic);
}

ACTION drealms::setalgo(name token_name, name license_owner, string new_checksum_algo) {
    //open license table, search for license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //set new checksum algorithm
    licenses.modify(lic, same_payer, [&](auto& col) {
        col.checksum_algo = new_checksum_algo;
    });
}

ACTION drealms::setati(name token_name, name license_owner, string new_ati_uri) {
    //open license table, search for license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //set new ati uri
    licenses.modify(lic, same_payer, [&](auto& col) {
        col.full_uris[name("ati")] = new_ati_uri;
    });
}

ACTION drealms::newuri(name token_name, name license_owner, name uri_group, name uri_name, string new_uri, optional<uint64_t> serial) {
    //open licenses table, get license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //modify appropriate uri list
    if (uri_group == name("full")) { //update full uri
        
        //update full uri
        licenses.modify(lic, same_payer, [&](auto& col) {
            col.full_uris[uri_name] = new_uri;
        });

    } else if (uri_group == name("base")) { //update base uri

        //update base uri
        licenses.modify(lic, same_payer, [&](auto& col) {
            col.base_uris[uri_name] = new_uri;
        });

    } else if (uri_group == name("relative")) { //update relative uri

        //check for serial
        if (!serial) {
            check(false, "must provide serial to update relative uri");
        }

        //open nfts table, get nft
        nfts_table nfts(get_self(), token_name.value);
        auto& nft = nfts.get(*serial, "nft not found");

        //update relative uri
        nfts.modify(nft, same_payer, [&](auto& col) {
            col.relative_uris[license_owner] = new_uri;
        });

    } else { //invalid uri group
        check(false, "invalid uri group");
    }

}

ACTION drealms::deleteuri(name token_name, name license_owner, name uri_group, name uri_name, optional<uint64_t> serial) {
    //open licenses table, get license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //delete uri from appropriate list
    if (uri_group == name("full")) { //delete full uri

        //find uri in uris list
        auto full_itr = lic.full_uris.find(uri_name);

        //validate
        check(full_itr != lic.full_uris.end(), "uri name not found in full uris");
        
        //delete full uri
        licenses.modify(lic, same_payer, [&](auto& col) {
            col.full_uris.erase(full_itr);
        });

    } else if (uri_group == name("base")) { //delete base uri

        //find uri in uris list
        auto base_itr = lic.base_uris.find(uri_name);

        //validate
        check(base_itr != lic.base_uris.end(), "uri name not found in base uris");
        
        //delete base uri
        licenses.modify(lic, same_payer, [&](auto& col) {
            col.base_uris.erase(base_itr);
        });

    } else if (uri_group == name("relative")) { //delete relative uri

        //check for serial
        if (!serial) {
            check(false, "must provide serial to delete relative uri");
        }

        //open nfts table, get nft
        nfts_table nfts(get_self(), token_name.value);
        auto& nft = nfts.get(*serial, "nft not found");

        //find uri in uris list
        auto rel_itr = nft.relative_uris.find(license_owner);

        //validate
        check(rel_itr != nft.relative_uris.end(), "uri name not found in relative uris");

        //update relative uri
        nfts.modify(nft, same_payer, [&](auto& col) {
            col.relative_uris.erase(rel_itr);
        });

    } else { //invalid uri group
        check(false, "invalid uri group");
    }

}



//========== helper functions ==========

bool drealms::validate_license_model(name license_model) {
    
    switch (license_model.value) 
    {
        case name("disabled").value :
            break;
        case name("open").value : 
            break;
        case name("permissioned").value :
            break;
        default:
            return false;
    }

    return true;
}

bool drealms::validate_uri_group(name uri_group) {
    
    switch (uri_group.value) 
    {
        case name("full").value :
            break;
        case name("base").value : 
            break;
        case name("relative").value :
            break;
        default:
            return false;
    }

    return true;
}



//========== migration actions ==========

ACTION drealms::delstats(name token_name) {
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");
    stats.erase(stat);
}

ACTION drealms::dellic(name token_name, name license_owner) {
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");
    licenses.erase(lic);
}

ACTION drealms::delnft(name token_name, uint64_t serial) {
    nfts_table nfts(get_self(), token_name.value);
    auto& nft = nfts.get(serial, "nft not found");
    nfts.erase(nft);
}

ACTION drealms::delcurr(symbol sym) {
    currencies_table currencies(get_self(), get_self().value);
    auto& curr = currencies.get(sym.code().raw(), "currency not found");
    currencies.erase(curr);
}

ACTION drealms::delacct(name owner, symbol sym) {
    accounts_table accounts(get_self(), owner.value);
    auto& acct = accounts.get(sym.code().raw(), "account not found");
    accounts.erase(acct);
}
                                                                                                            