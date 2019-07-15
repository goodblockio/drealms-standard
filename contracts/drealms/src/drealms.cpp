#include <drealms.hpp>

drealms::drealms(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

drealms::~drealms() {}

//======================== admin actions ========================

ACTION drealms::setconfig(string drealms_version, symbol core_sym, name contract_owner, 
    uint32_t default_license_length, uint32_t min_license_length, uint32_t max_license_length) {
    
    //authenticate
    require_auth(get_self());

    //open configs singleton
    configs_singleton configs(get_self(), get_self().value);

    //build new config
    config new_config = {
        drealms_version, //drealms_version
        core_sym, //core_sym
        contract_owner, //contract_owner
        default_license_length, //default_license_length
        min_license_length, //min_license_length
        max_license_length, //max_license_length
    };

    //set new config
    configs.set(new_config, get_self());
}

//======================== nonfungible actions ========================

ACTION drealms::createnft(name token_name, name issuer, bool retirable, bool transferable, bool consumable, uint64_t max_supply) {
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
        col.retirable = retirable;
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

ACTION drealms::retirenft(name token_name, vector<uint64_t> serials, string memo) {
    //get stats table
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token stats not found");

    //authenticate
    require_auth(stat.issuer);

    //validate
    check(stat.retirable, "token is not retirable");
    check(stat.supply >= serials.size(), "cannot retire supply below 0");

    //reduce nft supply
    stats.modify(stat, same_payer, [&](auto& col) {
        col.supply -= serials.size();
    });

    //loop over each serial and erase nft
    for (uint64_t serial : serials) {
        //open nfts table, get nft
        nfts_table nfts(get_self(), token_name.value);
        auto& nft = nfts.get(serial, "nft not found");

        //check that issuer owns each nft before retiring
        check(nft.owner == stat.issuer, "only issuer may retire tokens");

        //retire nft
        nfts.erase(nft);
    }
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

    //open configs singleton, get configs
    configs_singleton configs(get_self(), get_self().value);
    auto current_configs = configs.get();

    //intialize defaults
    name ram_payer = owner;
    time_point_sec new_expiration = time_point_sec(current_time_point()) + current_configs.default_license_length;
    time_point_sec min_expiration = time_point_sec(current_time_point()) + current_configs.min_license_length;
    time_point_sec max_expiration = time_point_sec(current_time_point()) + current_configs.max_license_length;

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

//======================== fungible actions ========================

ACTION drealms::create(name issuer, bool retirable, bool transferable, bool consumable, asset max_supply) {
    //authenticate
    require_auth(issuer);

    //validate
    check(max_supply.symbol.is_valid(), "invalid symbol name");
    check(max_supply.is_valid(), "invalid supply");
    check(max_supply.amount > 0, "max supply must be positive");

    //open currencies table, search for currency
    currencies_table currencies(get_self(), get_self().value);
    auto existing = currencies.find(max_supply.symbol.code().raw());

    //validate
    check(existing == currencies.end(), "token with symbol already exists" );

    //empalce new currency
    currencies.emplace(get_self(), [&]( auto& col) {
       col.issuer = issuer;
       col.retirable = retirable;
       col.transferable = transferable;
       col.consumable = consumable;
       col.supply = asset(0, max_supply.symbol);
       col.max_supply = max_supply;
    });
}

ACTION drealms::issue(name to, asset quantity, string memo) {
    //validate
    check(is_account(to), "to account doesn't exist");
    check(quantity.symbol.is_valid(), "invalid symbol name");
    check(memo.size() <= 256, "memo has more than 256 bytes");
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must issue positive quantity");

    //open currencies table, get currency
    currencies_table currencies(get_self(), get_self().value);
    auto& curr = currencies.get(quantity.symbol.code().raw(), "currency not found");

    //authenticate
    require_auth(curr.issuer);

    //validate
    check(quantity.symbol == curr.supply.symbol, "symbol precision mismatch");
    check(quantity.amount <= curr.max_supply.amount - curr.supply.amount, "issuing quantity would exceed max supply");

    //update currency supply
    currencies.modify(curr, same_payer, [&](auto& col) {
       col.supply += quantity;
    });

    //update recipient balance
    add_balance(to, quantity, curr.issuer);

    //notify recipient account
    require_recipient(to);
}

ACTION drealms::retire(asset quantity, string memo) {
    //validate
    check(quantity.symbol.is_valid(), "invalid symbol name");
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must retire positive quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //open currencies table, 
    currencies_table currencies(get_self(), get_self().value);
    auto& curr = currencies.get(quantity.symbol.code().raw(), "currency not found");

    //authenticate
    require_auth(curr.issuer);

    //validate
    check(curr.retirable, "currency is not retirable");
    check(quantity.symbol == curr.supply.symbol, "symbol precision mismatch");
    check(quantity <= curr.supply, "cannot retire supply below zero");

    //update currencies table
    currencies.modify(curr, same_payer, [&](auto& col) {
       col.supply -= quantity;
    });

    //remove quantity from issuer
    sub_balance(curr.issuer, quantity);
}

ACTION drealms::transfer(name from, name to, asset quantity, string memo) {
    //validate
    check(from != to, "cannot transfer to self");
    check(is_account(to), "recipient account does not exist");
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must transfer positive quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //authenticate
    require_auth(from);

    //open currencies table
    currencies_table currencies(get_self(), get_self().value);
    auto& curr = currencies.get(quantity.symbol.code().raw(), "currency not found");

    //validate
    check(quantity.symbol == curr.supply.symbol, "symbol precision mismatch");

    //determine ram payer
    auto payer = has_auth(to) ? to : from;

    sub_balance(from, quantity);
    add_balance(to, quantity, payer);

    //notify from and to accounts
    require_recipient(from);
    require_recipient(to);
}

ACTION drealms::open(name owner, symbol token_sym, name ram_payer) {
    //authenticate
    require_auth(ram_payer);

    //validate
    check(is_account(owner), "owner account does not exist");

    //open currencies table, get currency
    currencies_table currencies(get_self(), get_self().value);
    auto& curr = currencies.get(token_sym.code().raw(), "currency not found");

    //open accounts table, search for account
    accounts_table accounts(get_self(), owner.value);
    auto acct = accounts.find(token_sym.code().raw());

    //validate
    check(curr.supply.symbol == token_sym, "symbol precision mismatch" );
    check(acct == accounts.end(), "account already exists");

    //emplace new account
    accounts.emplace(ram_payer, [&](auto& col){
        col.balance = asset(0, token_sym);
    });
}

ACTION drealms::close(name owner, symbol token_sym) {
    //authenticate
    require_auth(owner);

    //open accounts table, get account
    accounts_table accounts(get_self(), owner.value);
    auto& acct = accounts.get(token_sym.code().raw(), "account not found");

    //validate
    check(acct.balance.amount == 0, "cannot close account unless balance is zero" );

    //close account
    accounts.erase(acct);
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

void drealms::add_balance(name to, asset quantity, name ram_payer) {
    //open accounts table, search for account
    accounts_table to_accts(get_self(), to.value);
    auto to_acct = to_accts.find(quantity.symbol.code().raw());

    //if new account pay ram, update balance if not
    if(to_acct == to_accts.end()) {
        to_accts.emplace(ram_payer, [&](auto& col){
            col.balance = quantity;
        });
    } else {
        to_accts.modify(to_acct, same_payer, [&](auto& col) {
            col.balance += quantity;
        });
    }
}

void drealms::sub_balance(name from, asset quantity) {
    //open accounts table, get account
    accounts_table from_accts(get_self(), from.value);
    auto& from_acct = from_accts.get(quantity.symbol.code().raw(), "account not found");

    //validate
    check(from_acct.balance.amount >= quantity.amount, "overdrawn balance" );

    //subtract quantity from balance
    from_accts.modify(from_acct, from, [&](auto& col) {
        col.balance -= quantity;
    });
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
                                                                                                            