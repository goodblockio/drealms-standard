#include <nifty.hpp>

nifty::nifty(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

nifty::~nifty() {}

//======================== nonfungible actions ========================

ACTION create(name token_name, name creator) {
    //authenticate
    require_auth(creator);

    //get stats table, search for token_name
    stats_table stats(get_self(), get_self().value);
    auto st = stats.find(token_name.value);

    //validate
    check(st == stats.end(), "token name already exists");

    //get licenses table
    licenses_table licenses(get_self(), get_self().value);
    auto li = licenses.find(token_name);
    
    //emplace new stats
    stats.emplace(get_self(), [&](auto& row) {
        row.token_name = token_name;
        row.creator = creator;
    });

    //emplace new license slot
    licenses.emplace(get_self(), [&](auto& row) {
        row.owner = creator;
    });
}

//========== functions ==========



//========== reactions ==========



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