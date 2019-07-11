# dRealms Token Standard

The dRealms Token Standard is a lightweight cross-game NFT standard for EOSIO software.

## Features

* `Maintain Token Ownership and Authorship`

    dRealms offers the ideal ownership/authorship model for token economics. The current tokenomics paradigm pushes for 'complete ownership', whether that design works for every use case or not. dRealms offers a suite of tokenomics designs and developers can choose which one best suits their unique business model.

* `Custom Licensing Models`

    **Disabled Licensing**: this setting disables new license additions entirely. This feature is desirable for token creators who want to maintain full control of all NFT assets.

    **Open Licensing**: this setting allows any user to reserve a license slot for an NFT for free (not counting RAM costs). This setting is desirable if asset creators want their NFTs to be heavily modifiable and open for all asset creators to use.

    **Permissioned Licensing**: this setting allows token creators to individually approve additional license slots from a list of requests submitted by prospective asset developers.

* `Lightweight and Scalable`

    Because of dRealms's elegant design, it uses far fewer resources to operate. Contract tables are lean and have been optimized to scale naturally by demand.

## Documentation

[Developer Guide](docs/DeveloperGuide.md)

## Roadmap

**dRealms v1.0.0:**

1. Revise license flow and action signatures
2. Implement DEX features
3. Purchasable license model - additional license models
4. Add map of uris to NFT table

**dRealms v1.1.0**

1. ATI refinement
2. Unity Plugin



### URI Actions

//updates a uri if found, inserts if not found
ACTION newfulluri(name token_name, name license_owner, name uri_type, name uri_name, string new_uri);

//removes a uri
ACTION rmvfulluri(name token_name, name license_owner, name uri_type, name uri_name);



//updates a relative uri if found, inserts if not found
ACTION newreluri(name token_name, uint64_t serial, name license_owner, string new_uri, optional<string> new_checksum);

//removes a uri
ACTION rmvreluri(name token_name, uint64_t serial, name license_owner);

//updates a checksum if found, inserts if not found
ACTION newchecksum(name token_name, uint64_t serial, name license_owner, string new_checksum);

//removes checksum
ACTION rmvchecksum(name token_name, uint64_t serial, name license_owner);


All: token_name, license_owner

Lic: token_name, license_owner, uri_group, uri_name

NFT: token_name, license_owner, serial

Single: new_uri, new_checksum


uri groups: full, base, relative, checksum





### CURRENT

LICENSE:
    map<name, string> full_uris;
    map<name, string> base_uris;

NFT:
    string immutable_data;
    string mutable_data;



### OPTION 1

LICENSE:
    string ati_uri;
    string full_uri;
    string base_uri;

NFT:
    map<name, string> relative_uris;
    map<name, string> checksums;



### OPTION 2         <------------

LICENSE:
    map<name, string> full_uris;
    map<name, string> base_uris;

NFT:
    map<name, string> relative_uris;
    map<name, string> checksums;

1. newuri(token_name, license_owner, uri_group, uri_name, new_uri, optional<serial>, optional<string> new_checksum)

2. deleteuri(token_name, license_owner, uri_group, uri_name)

3. newchecksum(token_name, license_owner, serial, new_checksum)

// 4. delchecksum(token_name, license_owner, serial)

// 5. newati(token_name, license_owner, new_ati_uri) //adds or updates a "full_uri" named "ati"_n with the new_ati_uri


new, add, create, 

edit, update

rmv, del, sub, 



### OPTION 3

LICENSE:
    map<name, string> license_uris;

NFT:
    map<name, string> immutable_data;
    map<name, string> mutable_data;





LICENSE:
    map<name, string> license_uris;
    map<name, string> endpoints;
    map<name, string> full_uris; <---
    map<name, string> base_uris; <---

NFT:
    map<name, string> immutable_data;
    map<name, string> mutable_data;
    map<name, string> nft_uris;
    map<name, string> endpoints;
    map<name, string> relative_uris; <---
    map<name, string> checksums; <---
