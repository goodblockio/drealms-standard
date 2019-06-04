#include <nifty.hpp>

nifty::nifty(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

nifty::~nifty() {}

//======================== nonfungible actions ========================

ACTION nifty::create(name token_name, name creator, name licensing) {
    //authenticate
    require_auth(creator);

    //get stats table, search for token_name
    stats_table stats(get_self(), get_self().value);
    auto st = stats.find(token_name.value);

    //validate
    check(st == stats.end(), "token name already exists");
    check(is_valid_licensing(licensing), "invalid licensing type");
    
    //emplace new stats
    stats.emplace(creator, [&](auto& row) {
        row.token_name = token_name;
        row.creator = creator;
        row.licensing = licensing;
    });

    //get licenses table
    licenses_table licenses(get_self(), token_name.value);
    auto li = licenses.find(creator.value);

    //emplace new license slot
    licenses.emplace(creator, [&](auto& row) {
        row.owner = creator;
        row.base_uri = "";
    });
}

ACTION nifty::issue(name recipient, name token_name, string query_string, string memo) {
    //get stats table
    stats_table stats(get_self(), get_self().value);
    auto& st = stats.get(token_name.value, "token name not found");

    //authenticate
    require_auth(st.creator);

    //validate
    check(is_account(recipient), "recipient account does not exist");

    //get nonfungibles table
    nonfungibles_table nfts(get_self(), token_name.value);

    //emplace new NFT
    nfts.emplace(st.creator, [&](auto& row) {
        row.serial = nfts.available_primary_key();
        row.owner = recipient;
        row.query_string = query_string;
    });

    //TODO: issue to creator, then inline transfer to recipient

}

ACTION nifty::transfer(name recipient, name sender, name token_name, vector<uint64_t> serials, string memo) {
    //authenticate
    require_auth(sender);

    //loop over each serial and change ownership
    for (uint64_t serial : serials) {
        //get nonfungibles table
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
    // check(st.burnable == true, "token name disallows burning");

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

ACTION nifty::setlicensing(name token_name, name new_licensing) {
    //get stats table
    stats_table stats(get_self(), get_self().value);
    auto& st = stats.get(token_name.value, "token name not found");

    //authenticate
    require_auth(st.creator);

    //validate
    check(is_valid_licensing(new_licensing), "invalid licensing type");

    //TODO: modify licensing info
    
}

ACTION nifty::addlicense(name token_name, name owner, string base_uri) {
    //get stats table
    stats_table stats(get_self(), get_self().value);
    auto& st = stats.get(token_name.value, "token name not found");

    //validate
    check(st.licensing == name("open"), "licensing is not open");

    //TODO: add licensing row

}

ACTION nifty::editlicense(name token_name, name owner, string new_base_uri) {
    //authenticate
    require_auth(owner);

    //get licenses table
    licenses_table licenses(get_self(), token_name.value);
    auto& lic = licenses.get(owner.value, "license for owner not found");

    //TODO: modify base uri

}



//========== helper functions ==========

bool nifty::is_valid_licensing(name licensing) {
    switch (licensing.value) 
    {
    case name("monetary").value :
        return true;
    case name("open").value : 
        return true;
    case name("disabled").value :
        return true;
    default:
        return false;
    }
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