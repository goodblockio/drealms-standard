#include <drealms.hpp>

drealms::drealms(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

drealms::~drealms() {}

//======================== realm actions ========================

ACTION drealms::setrealmdata(string drealms_version, name realm_name) {
    //authenticate
    require_auth(get_self());

    //open realmdata singleton
    realmdata_singleton realmd(get_self(), get_self().value);

    //build nft and ft vectors
    vector<name> nfts = {};
    vector<symbol> fts = {};

    //build new realmdata
    auto new_realmdata = realmdata{
        drealms_version, //drealms_version
        realm_name, //realm_name
        nfts, //nonfungibles
        fts //fungibles
    };

    //set new config
    realmd.set(new_realmdata, get_self());
}

//======================== schema actions ========================

ACTION drealms::newnftschema(name new_schema_name, name issuer, uint64_t max_supply, symbol exp_symbol,
    bool retirable, bool transferable, bool consumable, bool activatable) {
    //authenticate
    require_auth(issuer);

    //open schemas table, search for schema
    schemas_table schemas(get_self(), get_self().value);
    auto sch = schemas.find(new_schema_name.value);

    //validate
    check(sch == schemas.end(), "schema name already exists");
    check(max_supply > 0, "max supply must be a positive number");

    //TODO: validate exp_symbol

    //build initial settings
    map<name, bool> initial_settings;
    initial_settings[name("retirable")] = retirable;
    initial_settings[name("transferable")] = transferable;
    initial_settings[name("consumable")] = consumable;
    initial_settings[name("activatable")] = activatable;

    //build initial default stats
    map<name, uint32_t> initial_default_stats;
    
    //emplace new schema
    schemas.emplace(issuer, [&](auto& col) {
        col.schema_name = new_schema_name;
        col.issuer = issuer;
        col.supply = uint64_t(0);
        col.issued_supply = uint64_t(0);
        col.max_supply = max_supply;
        col.license_model = name("disabled");
        col.min_license_length = 604800;
        col.max_license_length = 31449600;
        col.settings = initial_settings;
        col.default_stats = initial_default_stats;
        col.exp_symbol = exp_symbol;
    });

    //open licenses table, find license
    licenses_table licenses(get_self(), new_schema_name.value);
    auto lic = licenses.find(issuer.value);

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

    //TODO: add schema_name to realmdata.nonfungibles[]

}

ACTION drealms::toggle(name schema_name, name setting_name) {
    //open schemas table, search for schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //authenticate
    require_auth(sch.issuer);

    //validate
    auto set_itr = sch.settings.find(setting_name);
    check(set_itr != sch.settings.end(), "setting not found");

    map<name, bool> new_settings = sch.settings;
    bool toggled_setting = !new_settings[setting_name];

    //toggle settings
    schemas.modify(sch, same_payer, [&](auto& col) {
        col.settings[setting_name] = toggled_setting;
    });
}

ACTION drealms::addstat(name schema_name, name stat_name, uint32_t default_value) {
    //open schemas table, search for schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //authenticate
    require_auth(sch.issuer);

    //validate
    auto stat_itr = sch.default_stats.find(stat_name);
    check(stat_itr == sch.default_stats.end(), "stat already exists in schema");

    //add stat
    schemas.modify(sch, same_payer, [&](auto& col) {
        col.default_stats[stat_name] = default_value;
    });
}

ACTION drealms::syncstats(name schema_name, uint64_t serial) {
    //open schemas table, search for schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //TODO: map stats to nft stats (new stats start at default value, existing stats are untouched)
}

ACTION drealms::awardexp(name schema_name, name license_owner, uint64_t serial, asset experience) {
    //open schemas table, search for schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //open license table, search for license
    licenses_table licenses(get_self(), schema_name.value);
    auto lic = licenses.find(license_owner.value);

    //authenticate
    require_auth(lic->owner);

    //TODO: award experience points
}

ACTION drealms::spendpoint(name schema_name, uint64_t serial, name stat_name) {
    //open schemas table, search for schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //open nfts table, get nft
    nfts_table nfts(get_self(), schema_name.value);
    auto& nft = nfts.get(serial, "nft not found");

    //authenticate
    require_auth(nft.owner);

    //validate
    auto stat_itr = nft.stats.find(stat_name);
    check(stat_itr != nft.stats.end(), "stat name not found");
    check(nft.unspent >= 1, "nft has no points to spend");

    //spend stat point
    nfts.modify(nft, same_payer, [&](auto& col) {
        col.stats[stat_name] += uint32_t(1);
        col.unspent -= 1;
    });
}

//======================== licensing actions ========================

ACTION drealms::setlicmodel(name schema_name, name new_license_model) {
    //open schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //authenticate
    require_auth(sch.issuer);

    //validate
    check(validate_license_model(new_license_model), "invalid license model");

    //modify licensing
    schemas.modify(sch, same_payer, [&](auto& col) {
        col.license_model = new_license_model;
    });
}

ACTION drealms::newlicense(name schema_name, name owner, time_point_sec expiration) {
    //open schemas table, get schmea
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schemas not found");

    //intialize defaults
    name ram_payer = owner;
    time_point_sec new_expiration;
    time_point_sec min_expiration = time_point_sec(current_time_point()) + sch.min_license_length;
    time_point_sec max_expiration = time_point_sec(current_time_point()) + sch.max_license_length;

    //validate
    switch (sch.license_model.value) 
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
            require_auth(sch.issuer);
            ram_payer = sch.issuer;
            new_expiration = expiration;
            break;
        default:
            check(false, "invalid licensing");
    }

    //open license table, search for license
    licenses_table licenses(get_self(), schema_name.value);
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

ACTION drealms::eraselicense(name schema_name, name license_owner) {
    //open schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //open licenses table, get license
    licenses_table licenses(get_self(), schema_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //determine if expired
    bool expired = time_point_sec(current_time_point()) > lic.expiration;

    //authenticate based on expired
    if (expired) {
        check(has_auth(lic.owner) || has_auth(sch.issuer), "only token issuer or license owner may erase after expiration");
    } else {
        require_auth(lic.owner);
    }
    
    //validate
    check(expired, "license has not expired");

    //erase license slot
    licenses.erase(lic);
}

ACTION drealms::setlicminmax(name schema_name, uint32_t min_license_length, uint32_t max_license_length) {
    //open schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //authenticate
    require_auth(sch.issuer);

    //validate
    check(min_license_length > 604800, "min license length must be greater than 1 week");
    check(max_license_length < 31449600, "max liensce length must be less than 1 year");

    //modify licensing
    schemas.modify(sch, same_payer, [&](auto& col) {
        col.min_license_length = min_license_length;
        col.max_license_length = max_license_length;
    });
}

ACTION drealms::setalgo(name schema_name, name license_owner, string new_checksum_algo) {
    //open license table, search for license
    licenses_table licenses(get_self(), schema_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //set new checksum algorithm
    licenses.modify(lic, same_payer, [&](auto& col) {
        col.checksum_algo = new_checksum_algo;
    });
}

ACTION drealms::setati(name schema_name, name license_owner, string new_ati_uri) {
    //open license table, search for license
    licenses_table licenses(get_self(), schema_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //set new ati uri
    licenses.modify(lic, same_payer, [&](auto& col) {
        col.full_uris[name("ati")] = new_ati_uri;
    });
}

ACTION drealms::newuri(name schema_name, name license_owner, name uri_group, name uri_name, string new_uri, optional<uint64_t> serial) {
    //open licenses table, get license
    licenses_table licenses(get_self(), schema_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //TODO: change to switch statement
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
        nfts_table nfts(get_self(), schema_name.value);
        auto& nft = nfts.get(*serial, "nft not found");

        //update relative uri
        nfts.modify(nft, same_payer, [&](auto& col) {
            col.relative_uris[license_owner] = new_uri;
        });

    } else { //invalid uri group
        check(false, "invalid uri group");
    }
}

ACTION drealms::deleteuri(name schema_name, name license_owner, name uri_group, name uri_name, optional<uint64_t> serial) {
    //open licenses table, get license
    licenses_table licenses(get_self(), schema_name.value);
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
        nfts_table nfts(get_self(), schema_name.value);
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

//======================== nonfungible actions ========================

ACTION drealms::issuenft(name to, name schema_name, string memo, bool log) {
    //open schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //authenticate
    require_auth(sch.issuer);

    //validate
    check(is_account(to), "to account does not exist");
    check(sch.supply + 1 <= sch.max_supply, "issuing would breach max supply");

    //open nfts table, get new serial
    nfts_table nfts(get_self(), schema_name.value);
    uint64_t new_serial = sch.issued_supply + 1;

    //increment nft supply and issued supply
    schemas.modify(sch, same_payer, [&](auto& col) {
        col.issued_supply += uint64_t(1);
        col.supply += uint64_t(1);
    });

    //build initial uri and checksum maps
    map<name, string> new_relative_uris;
    map<name, string> new_checksums;

    //TODO: calculate next_level formula
    
    //build default stats
    map<name, uint32_t> intitial_stats = sch.default_stats;

    //emplace new NFT
    nfts.emplace(sch.issuer, [&](auto& col) {
        col.serial = new_serial;
        col.owner = to;
        col.level = uint16_t(1);
        col.experience = asset(0, sch.exp_symbol);
        col.next_level = asset(1000, sch.exp_symbol);
        col.unspent = uint8_t(0);
        col.stats = intitial_stats;
        col.relative_uris = new_relative_uris;
        col.checksums = new_checksums;
    });

    //inline to lognft if true
    if (log) {
        //requires drealms@eosio.code on active perm
        action(permission_level{get_self(), name("active")}, get_self(), name("lognft"), make_tuple(
            to, //to
            schema_name, //schema_name
            new_serial //serial
        )).send();
    }

    //notify recipient account
    require_recipient(to);
}

ACTION drealms::retirenft(name schema_name, vector<uint64_t> serials, string memo) {
    //open schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //authenticate
    require_auth(sch.issuer);

    //validate
    check(sch.settings.at(name("retirable")), "nft is not retirable");
    check(sch.supply >= serials.size(), "cannot retire supply below 0");

    //reduce nft supply
    schemas.modify(sch, same_payer, [&](auto& col) {
        col.supply -= serials.size();
    });

    //loop over each serial and erase nft
    for (uint64_t serial : serials) {
        //open nfts table, get nft
        nfts_table nfts(get_self(), schema_name.value);
        auto& nft = nfts.get(serial, "nft not found");

        //check that issuer owns each nft before retiring
        check(nft.owner == sch.issuer, "only issuer may retire tokens");

        //retire nft
        nfts.erase(nft);
    }
}

ACTION drealms::transfernft(name from, name to, name schema_name, vector<uint64_t> serials, string memo) {
    //authenticate
    require_auth(from);

    //opens schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //validate
    check(sch.settings.at(name("transferable")), "nft is not transferable");
    check(is_account(to), "recipient account does not exist");

    //loop over each serial and change ownership
    for (uint64_t serial : serials) {
        //open nfts table, get nft
        nfts_table nfts(get_self(), schema_name.value);
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

ACTION drealms::consumenft(name schema_name, uint64_t serial, string memo) {
    //open schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //open nfts table, get nft
    nfts_table nfts(get_self(), schema_name.value);
    auto& nft = nfts.get(serial, "nft not found");

    //authenticate
    require_auth(nft.owner);

    //validate
    check(sch.settings.at(name("consumable")), "nft is not consumable");

    //decrement nft supply
    schemas.modify(sch, same_payer, [&](auto& col) {
        col.supply -= uint64_t(1);
    });

    //consume nft
    nfts.erase(nft);
}

ACTION drealms::activatenft(name schema_name, uint64_t serial, string memo) {
    //open schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //open nfts table, get nft
    nfts_table nfts(get_self(), schema_name.value);
    auto& nft = nfts.get(serial, "nft not found");

    //authenticate
    require_auth(nft.owner);

    //validate
    check(sch.settings.at(name("activatable")), "nft is not activatable");
}

ACTION drealms::newchecksum(name schema_name, name license_owner, uint64_t serial, string new_checksum) {
    //open schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //open licenses table, get license
    licenses_table licenses(get_self(), schema_name.value);
    auto& lic = licenses.get(license_owner.value, "license not found");

    //authenticate
    require_auth(lic.owner);

    //open nft table, get nft
    nfts_table nfts(get_self(), schema_name.value);
    auto& nft = nfts.get(serial, "nft not found");

    //modify nft relative uri
    nfts.modify(nft, same_payer, [&](auto& col) {
        col.checksums[license_owner] = new_checksum;
    });
}

ACTION drealms::lognft(name to, name schema_name, uint64_t serial) {
    //authenticate
    require_recipient(get_self());
}

ACTION drealms::levelup(name schema_name, uint64_t serial) {
    //opens schemas table, get schema
    schemas_table schemas(get_self(), get_self().value);
    auto& sch = schemas.get(schema_name.value, "schema not found");

    //open nfts table, get nft
    nfts_table nfts(get_self(), schema_name.value);
    auto& nft = nfts.get(serial, "nft not found");

    //authenticate
    require_auth(nft.owner);

    //TODO: calculate next level cost
    // asset level_up_cost = ...

    //validate
    // check(); //check nft has enought points to level up

    //level up nft
    nfts.modify(nft, same_payer, [&](auto& col) {
        col.level += 1;
        // col.experience -= level_up_cost;
        col.unspent += 1;
    });
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

    //TODO: add symbol to realmdata.fungibles[]

    //emplace new currency
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
    check(curr.transferable, "currency is not transferable");
    check(quantity.symbol == curr.supply.symbol, "symbol precision mismatch");

    //determine ram payer
    auto payer = has_auth(to) ? to : from;

    sub_balance(from, quantity);
    add_balance(to, quantity, payer);

    //notify from and to accounts
    require_recipient(from);
    require_recipient(to);
}

ACTION drealms::consume(name owner, asset quantity, string memo) {
    //authenticate
    require_auth(owner);

    //open currencies table, get currency
    currencies_table currencies(get_self(), get_self().value);
    auto& curr = currencies.get(quantity.symbol.code().raw(), "currency not found");

    //validate
    check(curr.consumable, "currency is not consumable");
    check(quantity.symbol == curr.supply.symbol, "symbol precision mismatch");
    check(quantity <= curr.supply, "cannot consume supply below zero");
    check(quantity.symbol.is_valid(), "invalid symbol name");
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must consume positive quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //update currencies table
    currencies.modify(curr, same_payer, [&](auto& col) {
       col.supply -= quantity;
    });

    //remove quantity from owner
    sub_balance(owner, quantity);
}

ACTION drealms::open(name owner, symbol currency_symbol, name ram_payer) {
    //authenticate
    require_auth(ram_payer);

    //validate
    check(is_account(owner), "owner account does not exist");

    //open currencies table, get currency
    currencies_table currencies(get_self(), get_self().value);
    auto& curr = currencies.get(currency_symbol.code().raw(), "open: currency not found");

    //open accounts table, search for account
    accounts_table accounts(get_self(), owner.value);
    auto acct = accounts.find(currency_symbol.code().raw());

    //validate
    check(curr.supply.symbol == currency_symbol, "open: symbol precision mismatch" );
    check(acct == accounts.end(), "open: account already exists");

    //emplace new account
    accounts.emplace(ram_payer, [&](auto& col){
        col.balance = asset(0, currency_symbol);
    });
}

ACTION drealms::close(name owner, symbol currency_symbol) {
    //authenticate
    require_auth(owner);

    //open accounts table, get account
    accounts_table accounts(get_self(), owner.value);
    auto& acct = accounts.get(currency_symbol.code().raw(), "close: account not found");

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
    auto& from_acct = from_accts.get(quantity.symbol.code().raw(), "sub_balance: account not found");

    //validate
    check(from_acct.balance.amount >= quantity.amount, "sub_balance: overdrawn balance" );

    //subtract quantity from balance
    from_accts.modify(from_acct, from, [&](auto& col) {
        col.balance -= quantity;
    });
}
                                                                                                            