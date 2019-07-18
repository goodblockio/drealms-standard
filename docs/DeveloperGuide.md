# dRealms Developer Guide

dRealms is a lightweight cross-game standard for creating tokenized digital assets for EOSIO blockchains.

## Project Setup

To begin, navigate to the base project folder: `drealms-standard/`

    cd contracts && mkdir build && mkdir build/drealms

    chmod +x build.sh

    chmod +x deploy.sh

### Build

    ./build.sh drealms

### Deploy

    ./deploy.sh drealms { local | test | production }

## Contract Setup

In order to fully utilize the features dRealms provides, a dRealms contract config must first be initialized. This is done by calling the `setconfig()` action.

### ACTION `setconfig()`

- `drealms_version` is a string representation of the dRealms version used by this contract. dRealms follows the SemVer 2.0.0 versioning pattern so its best to follow that where possible.

- `core_sym` is the symbol of the core system token of the blockchain this contract is deployed to.

- `contract_owner` is the account name that owns the whole contract. Only certain actions require this authority.

- `min_license_length` is the minimum length of time, in seconds, that a license may be created for across the whole contract.

- `max_license_length` is the maximum length of time, in seconds, that a license may be created for across the whole contract.

    ```
    cleos push action account setconfig '["v1.0.0", "4,TLOS", "youraccount", 604800, 62899200]' -p account
    ```

## Nonfungible Actions

dRealms nonfungible actions that are used throughout the lifecycle of a nonfungible token. To get started creating a new NFT, simply call the `createnft()` action.

### ACTION `createnft()`

- `token_name` is the name of the new token family.

- `issuer` is the account allowed to issue or retire NFTs.

- `retirable` allows tokens to be retired by the issuer. The issuer must own the tokens to be retired.

- `transferable` allows tokens to be transferred. Note that by virtue of being transferable tokens are able to be traded on an open market.

- `consumable` allows the token to be consumed by the token holder. Often used to trigger effects of one-use tokens in a game or app. Additional information can be supplied in the memo field of the consume() action for customizable token consumption effects.

- `max_supply` is the maximum number of NFTs from this token family allowed to exist at once.

    ```
    cleos push action account createnft '["dragons", "testaccounta", true, true, false, 100]' -p testaccounta
    ```

### ACTION `issuenft()`

- `to` is the account to receive the newly issued NFT.

- `token_name` is the NFT token family from which to issue.

- `memo` is a memo for extra data.

    ```
    cleos push action account issuenft '["testaccountb", "dragons", "test issuenft memo"]' -p testaccounta
    ```

### ACTION `retirenft()`

- `token_name` is the token family of the NFT(s) to retire.

- `serials` is a list of NFT serial numbers to retire.

- `memo` is a memo for extra data.

    ```
    cleos push action youraccount transfernft '["testaccounta", "testaccountb", "dragons", [0, 1], "test transfernft memo"]' -p account
    ```

### ACTION `transfernft()`

- `from` is the name of the account sending the NFT.

- `to` is the account receiving the NFT.

- `token_name` is the token family of the NFT(s) to transfer.

- `serials` is a list of NFT serial numbers to transfer.

- `memo` is a memo for extra data.

    ```
    cleos push action account transfernft '["testaccounta", "testaccountb", "dragons", [0, 1], "test transfernft memo"]' -p testaccounta
    ```

### ACTION `consumenft()`

* `token_name` is the token family of the NFT to consume.

* `serial` is the serial number of the NFT to consume.

* `memo` is a memo for extra data.

    ```
    cleos push action account consumenft '["dragons", 1, "test consumenft memo"]' -p testaccounta
    ```

### ACTION `newchecksum()`

* `token_name` is the token family of the NFT to update.

* `license_owner` is the name of the NFT license owner.

* `serial` is the serial number of the NFT to update.

* `new_checksum` is the new checksum to save to the NFT.

    ```
    cleos push action account newchecksum '["dragons", "testaccounta", 1, "rga59c6"]' -p testaccounta
    ```

## License Actions

The dRealms License interface allows (or disallows) third parties to obtain, modify, and remove licenses from NFT families. After obtaining a license, this interface allows such third parties to save a custom representation of an NFT for use in their game or application.

### ACTION `setlicmodel()`

This action will update the license model set on the token family. Currently, the supported options are: `disabled`, `open`, and `permissioned`. 

Disabled is the default set by all new token families and disallows any additional licenses on the token (other than the initial one created along with the token family). 

Open allows any other user to add a new license, provided the new license expiration falls between the min_license_length and max_license_length defined by calling set_config(). This setting is great for tokens that intend to have large mod communities, as it allows new token representations to be created at will.

Permissioned licensing means only the token issuer may create new licenses, but they may do so on behalf of another party (perhaps after negotiating a licensing deal).

- `token_name` is the token family to update with the new license model.

- `new_license_model` is the new license model being set.

    ```
    cleos push action account setlicmodel '["dragons", "permissioned"]' -p testaccounta
    ```

### ACTION `newlicense()`

- `token_name` is the token family receiving the new license.

- `owner` is the owner of the new license.

- `expiration` is the expiration time of the new license.

    ```
    cleos push action account newlicense '["dragons", "testaccountb", "2019-05-22T21:39:03"]' -p testaccounta
    ```

### ACTION `eraselicense()`

- `token_name` is the token family from which to erase the license.

- `license_owner` is the owner of the license to erase.

    ```
    cleos push action account eraselicense '["dragons", "testaccountb"]' -p testaccountb
    ```

### ACTION `setalgo()`

- `token_name` is the token family of the license.

- `license_owner` is the name of the license owner.

- `new_checksum_algo` is the new checksum algorithm. e.g. "sha256", "md5", etc.

    ```
    cleos push action account setalgo '["dragons", "testaccountb", "sha256"]' -p testaccountb
    ```

### ACTION `setati()`

- `token_name` is the token family of the license.

- `license_owner` is the name of the license owner.

- `new_ati_uri` is the new endpoint storing the license ATI.

    ```
    cleos push action account setati '["dragons", "testaccountb", "http://dragons.io/atis/dragons"]' -p testaccountb
    ```

### ACTION `newuri()`

The newuri() action will create a new uri within the given uri_group and assign it a key equal to uri_name. Currently, the three uri groups are: `full`, `base`, and `relative`. 

A full uri is already complete - in other words, it doesn't need to be combined with another uri to make a complete endpoint.

A base uri is the first part of a complete uri. Concatenate a base uri with the respective relative uri to form a complete endpoint.

A relative uri is the second part of a base uri. When combined with a base uri from the respective license it forms a complete endpoint from which all metadata about that version of the NFT is returned. If adding a new relative uri, you must supply the serial number of the NFT as well.

- `token_name` is the token family to upsert the uri for.

- `license_owner` is the owner of the license being updated.

- `uri_group` is the group of the new uri.

- `uri_name` is the name of the new uri.

- `new_uri` is the uri as a string.

- `optional: serial` if updating a relative uri, the serial number of the NFT to update.

    ```
    cleos push action account newuri '["dragons", "testaccountb", "full", "website", "http://dragons.io", null]' -p testaccountb
    ```

    ```
    cleos push action account newuri '["dragons", "testaccountb", "relative", "testaccountb", "?serial=1", 1]' -p testaccountb
    ```

### ACTION `deleteuri()`

- `token_name` is the token family to delete the uri from.

- `license_owner` is the license owner of the uri being deleted.

- `uri_group` is the group of the new uri ("full", "base", or "relative").

- `uri_name` is the name of the uri to delete.

- `optinal: serial` if deleteing a relative uri, the serial number of the NFT to delete from.

    ```
    cleos push action account deleteuri '["dragons", "testaccountb", "full", "website", null]' -p testaccountb
    ```

    ```
    cleos push action account deleteuri '["dragons", "testaccountb", "relative", "testaccountb", 1]' -p testaccountb
    ```

## Fungible Actions

The dRealms standard supports fungible tokens and allows for more granular customization when compared to the typical eosio.token format.

### ACTION `create()`

- `issuer` 

- `retirable`

- `transferable`

- `consumable`

- `max_supply`

    ```
    cleos push action account create '["testaccounta", true, false, true, "1000.00 TEST"]' -p testaccounta
    ```

### ACTION `issue()`

- `to`

- `quantity`

- `memo`

    ```
    cleos push action account issue '["tetaccounta", "50.00 TEST", "test issue"]' -p testaccounta
    ```

### ACTION `retire()`

- `quantity`

- `memo`

    ```
    cleos push action account retire '["5.00 TEST", "test retire"]' -p testaccountb
    ```

### ACTION `transfer()`

- `from`

- `to`

- `quantity`

- `memo`

    ```
    cleos push action account transfer '["testaccounta", "testaccountb", "25.00 TEST", "test transfer"]' -p testaccounta
    ```

### ACTION `consume()`

- `owner`

- `quantity`

- `memo`

    ```
    cleos push action account consume '["testaccountb", "1.00 TEST", "test consume"]' -p testaccountb
    ```

### ACTION `open()`

- `owner`

- `token_sym`

- `ram_payer`

    ```
    cleos push action account open '["testaccountb", "2,TEST", "testaccountb"]' -p testaccountb
    ```

### ACTION `close()`

- `owner`

- `token_sym`

    ```
    cleos push action account close '["testaccountb", "2,TEST"]' -p testaccountb
    ```

## Application Token Interface (ATI)

dRealms's ATI feature makes developing NFT's as familiar as making regular game assets. Any active license can supply a custom ATI for a token. 

An ATI defines data types and formatting so the dRealms Unity Plugin knows what to expect when querying for NFT data on-chain. This allows for "just in time" creation of NFT assets in-game, since the game application already knows how to build the frame of the asset - it just waits for data to return from the chain to render the complete asset.

#### ATI Example:

```
{
    "comment": "This file was generated with drealms-atigen.",
    "version": "drealms-atigen/0.1.0",
    "engine": {
        "name": "unity",
        "version": "2019.6.5"
    },
    "license": {
        "contract": "drakoskeepio",
        "owner": "goodblocktls",
        "token_family": "dragons"
    },
    "interface": {
        "nft_id": "uint64",
        "owner": "name",
        "level": "uint8",
        "stats": {
            "strength": "uint16_t",
            "dexterity": "uint16_t",
            "constitution": "uint16_t",
            "intelligence": "uint16_t",
            "wisdom": "uint16_t",
            "charisma": "uint16_t"
        },
        "skills": [
            {
                "name": "string",
                "power": "uint8",
                "effect": "string"
            },
            {
                "name": "string",
                "power": "uint8",
                "effect": "string"
            }
            ...
        ]
        ...
    }
    ...
}
```


## Development Example

**Scenario**: GoodBlock Games has launched a new title where in-game dragons are tokenized on the Telos Blockchain, and Bethesda Game Studios wants to build a game where those same dragon tokens are importable and usable in their game. 

Since GoodBlock Games created the original game asset they control the licensing rights to their dragon token, and therefore may allow or disallow Bethesda Game Studios to get a new license slot for the dragon tokens. When GoodBlock Games created their dragon tokens they set the licensing model to **Permissioned Licensing**, meaning new license slots are only obtainable through on-chain approval by GoodBlock Games. GoodBlock Games and Bethesda Game Studios negotiate a deal, and GoodBlock Games agrees to give Bethesda Game Studios a 1-year license slot for the dragon tokens. The deal is accepted by both parties and the license slot is opened on-chain.

Bethesda Game Studios' new game is a Sci-Fi FPS style shoooter (one where a 2D fantasy dragon would almost certainly be out of place), so during license negotiation Bethesda works to ensure GBG approves of the representation their dragon tokens will have within BGS's game - in this case, fantasy inspired laser rifles (EX: a water dragon in Drakos Keep becomes a specialized laser rifle with a dragonscale skin, shoots blue lasers, and has increased weapon handling). The rigidity to which asset creators must adhere to the original token design is decided during license negotioation and is outside the scope of the token contract's responsibility.

Note that having token representation vary by game is intentional, as this decouples game assets from the original game and opens them up for creative use in other games. While some games are designed to be a giant sandbox, and there will no doubt be many games that use dRealms assets this way, it is also important to understand that the many games are hand-crafted experiences where simply "dropping in" would be inappropriate. 

The most common usage of interoperable cross-game digital assets will be games designed with heterogenious dRealms game assets in mind.
