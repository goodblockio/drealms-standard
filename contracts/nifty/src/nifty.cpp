#include <nifty.hpp>

nifty::nifty(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

nifty::~nifty() {}

//======================== nonfungible actions ========================

ACTION nifty::create(name token_name, name creator, bool burnable, bool transferable, uint64_t max_supply) {
    //authenticate
    require_auth(creator);

    //get stats table, search for token_name
    stats_table stats(get_self(), get_self().value);
    auto st = stats.find(token_name.value);

    //validate
    check(st == stats.end(), "token name already exists");
    //TODO?: add checks for max_supply
    
    //emplace new stats
    stats.emplace(creator, [&](auto& row) {
        row.token_name = token_name;
        row.creator = creator;
        row.licensing = name("disabled");
        row.license_price = asset(0, CORE_SYM);
        row.burnable = burnable;
        row.transferable = transferable;
        row.supply = uint64_t(0);
        row.max_supply = max_supply;
    });

    //open licenses table, find license
    licenses_table licenses(get_self(), token_name.value);
    auto lic = licenses.find(creator.value);

    //validate
    check(lic == licenses.end(), "license already exists for token name");

    //emplace new license slot
    licenses.emplace(creator, [&](auto& row) {
        row.owner = creator;
        row.ati_uri = "";
        row.base_uri = "";
    });
}

ACTION nifty::issue(name recipient, name token_name, string query_string, string memo) {
    //open stats table, get token data
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token name not found");

    //authenticate
    require_auth(stat.creator);

    //validate
    check(is_account(recipient), "recipient account does not exist");
    check(stat.supply + 1 <= stat.max_supply, "token name at max supply");

    //get nonfungibles table
    nonfungibles_table nfts(get_self(), token_name.value);

    //calc new serial
    uint64_t new_serial = nfts.available_primary_key();

    //emplace new NFT
    nfts.emplace(stat.creator, [&](auto& row) {
        row.serial = new_serial;
        row.owner = stat.creator;
        row.query_string = query_string;
    });

    //add new serial to vector
    vector<uint64_t> new_serials = {new_serial};

    //send inline transfer action to recipient, requires eosio.code on active permission
    action(permission_level{get_self(), name("active")}, get_self(), name("transfer"), make_tuple(
        recipient, //recipient
        get_self(), //sender
        token_name, //token_name
        new_serials, //serials
        std::string("issue") //memo
    )).send();

}

ACTION nifty::transfer(name recipient, name sender, name token_name, vector<uint64_t> serials, string memo) {
    //authenticate
    require_auth(sender);

    //opens stats table, get token data
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token name not found");

    //validate
    check(stat.transferable, "token disallows transfers");

    //loop over each serial and change ownership
    for (uint64_t serial : serials) {
        //open nonfungibles table, get token data
        nonfungibles_table nfts(get_self(), token_name.value);
        auto& nft = nfts.get(serial, "nft not found");

        //validate
        check(sender == nft.owner, "only nft owner is allowed to transfer");

        //modify nft ownership to recipient
        nfts.modify(nft, same_payer, [&](auto& row) {
            row.owner = recipient;
        });

    }

}

ACTION nifty::burn(name creator, name token_name, vector<uint64_t> serials, string memo) {
    //authenticate
    require_auth(creator);

    //get stats table
    stats_table stats(get_self(), get_self().value);
    auto& st = stats.get(token_name.value, "token name not found");

    //validate
    check(st.creator == creator, "only creator can burn tokens");
    check(st.burnable, "token disallows burning");

    //loop over each serial and erase nft
    for (uint64_t serial : serials) {
        //get nonfungibles table
        nonfungibles_table nfts(get_self(), token_name.value);
        auto& nft = nfts.get(serial, "nft not found");

        //erase nft
        nfts.erase(nft);
    }
}



//======================== licensing actions ========================

ACTION nifty::setlicensing(name token_name, name new_licensing, asset license_price) {
    //open stats table, get token
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token name not found");

    //authenticate
    require_auth(stat.creator);

    //validate
    check(validate_licensing(new_licensing, license_price), "invalid licensing");

    asset new_license_price = asset(0, CORE_SYM);

    //apply license price, if applicable
    if (new_licensing == name("monetary")) {
        new_license_price = license_price;
    }

    //modify licensing
    stats.modify(stat, same_payer, [&](auto& row) {
        row.licensing = new_licensing;
        row.license_price = new_license_price;
    });
    
}

ACTION nifty::newlicense(name token_name, name owner, string ati_uri, string base_uri) {
    //open stats table, get stats
    stats_table stats(get_self(), get_self().value);
    auto& stat = stats.get(token_name.value, "token name not found");

    //validate
    switch (stat.licensing.value) 
    {
        case name("monetary").value :
            check(false, "monetary licensing requires a purchase with the buylicense action");
        case name("permissioned").value :
            require_auth(stat.creator);
        case name("open").value : 
            break;
        case name("disabled").value :
            check(false, "disabled licensing prohibits new license creation");
        default:
            check(false, "invalid licensing");
    }

    //open license table, find license
    licenses_table licenses(get_self(), token_name.value);
    auto lic = licenses.find(owner.value);

    //validate
    check(lic == licenses.end(), "owner already has a license");

    //emplace new license
    licenses.emplace(owner, [&](auto& row) {
        row.owner = owner;
        row.ati_uri = ati_uri;
        row.base_uri = base_uri;
    });

}

ACTION nifty::edituris(name token_name, name owner, string new_ati_uri, string new_base_uri) {
    //authenticate
    require_auth(owner);

    //open licenses table, get license
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(owner.value, "license for owner not found");

    //validate
    check(owner == lic.owner, "only license owner may edit uris");

    //modify license uris
    licenses.modify(lic, same_payer, [&](auto& row) {
        row.ati_uri = new_ati_uri;
        row.base_uri = new_base_uri;
    });

}



//========== helper functions ==========

bool nifty::validate_licensing(name licensing, asset license_price) {
    
    switch (licensing.value) 
    {
        case name("monetary").value :
            check(license_price.symbol.is_valid(), "invalid symbol name");
            check(license_price.is_valid(), "invalid price");
            check(license_price.amount > 0, "price must be positive");
            break;
        case name("open").value : 
            break;
        case name("permissioned").value :
            break;
        case name("disabled").value :
            break;
        default:
            return false;
    }

    return true;
}



//========== migration tools ==========



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