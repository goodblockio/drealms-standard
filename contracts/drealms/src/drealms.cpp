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
        col.full_uris = new_full_uris;
        col.base_uris = new_base_uris;
    });
}

ACTION drealms::issuenft(name to, name token_name, string immutable_data, string memo) {
    //open stats table, get token data
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //authenticate
    require_auth(stat.issuer);

    //validate
    check(is_account(to), "to account does not exist");
    check(stat.supply + 1 <= stat.max_supply, "token at max supply");
    check(immutable_data != "", "immutable data cannot be blank");

    //open nfts table, get new serial
    nfts_table nfts(get_self(), token_name.value);
    uint64_t new_serial = stat.issued_supply + 1;

    //increment nft supply and issued supply
    stats.modify(stat, same_payer, [&](auto& col) {
        col.issued_supply += uint64_t(1);
        col.supply += uint64_t(1);
    });

    //emplace new NFT
    nfts.emplace(stat.issuer, [&](auto& col) {
        col.serial = new_serial;
        col.owner = to;
        col.immutable_data = immutable_data;
        col.mutable_data = "";
    });
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

    //erase nft
    nfts.erase(nft);
}

ACTION drealms::updatenft(name token_name, uint64_t serial, string new_mutable_data) {
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

ACTION drealms::setlicensing(name token_name, name new_license_model) {
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

ACTION drealms::upserturi(name token_name, name license_owner, name uri_type, name uri_name, string new_uri) {
    //open licenses table, get license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //validate
    check(uri_type == name("full") || uri_type == name("base"), "invalid uri type");

    //initialize uri map
    map<name, string> uri_list;

    //branch based on uri_type
    if (uri_type == name("full")) {
        uri_list = lic.full_uris;
    } else if (uri_type == name("base")) {
        uri_list = lic.base_uris;
    }

    //find fee in fees list
    auto itr = uri_list.find(uri_name);

    //update uri if found, insert if not found
    if (itr != uri_list.end()) {
        //update uri
        uri_list[uri_name] = new_uri;

        //update correct uri list
        if (uri_type == name("full")) {
            //update uri list
            licenses.modify(lic, same_payer, [&](auto& col) {
                col.full_uris = uri_list;
            });
        } else if (uri_type == name("base")) {
            //update uri list
            licenses.modify(lic, same_payer, [&](auto& col) {
                col.base_uris = uri_list;
            });
        }
    } else {
        //update correct uri list
        if (uri_type == name("full")) {
            //insert new uri in list
            licenses.modify(lic, same_payer, [&](auto& col) {
                col.full_uris.insert(pair<name, string>(name(uri_name), new_uri));
            });
        } else if (uri_type == name("base")) {
            //insert new uri in list
            licenses.modify(lic, same_payer, [&](auto& col) {
                col.base_uris.insert(pair<name, string>(name(uri_name), new_uri));
            });
        }
    }
}

ACTION drealms::removeuri(name token_name, name license_owner, name uri_type, name uri_name) {
    //open licenses table, get license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //validate
    check(uri_type == name("full") || uri_type == name("base"), "invalid uri type");

    //initialize uri map
    map<name, string> uri_list;

    //branch based on uri_type
    if (uri_type == name("full")) {
        uri_list = lic.full_uris;
    } else if (uri_type == name("base")) {
        uri_list = lic.base_uris;
    }

    //find uri in uris list
    auto itr = uri_list.find(uri_name);

    //valdiate
    check(itr != uri_list.end(), "uri name not found");

    //remove uri from list
    uri_list.erase(itr);

    //update correct uri list
    if (uri_type == name("full")) {
        //update uri list
        licenses.modify(lic, same_payer, [&](auto& col) {
            col.full_uris = uri_list;
        });
    } else if (uri_type == name("base")) {
        //update uri list
        licenses.modify(lic, same_payer, [&](auto& col) {
            col.base_uris = uri_list;
        });
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
                                                                                                            