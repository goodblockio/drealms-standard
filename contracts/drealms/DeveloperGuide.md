# dRealms Developer Guide

dRealms is a lightweight cross-game NFT standard for EOSIO software.

## Project Setup

To begin, navigate to the base project folder: `drealms-standard/`

    cd contracts && mkdir build

    cd build && mkdir drealms

    cd ..

    chmod +x build.sh

    chmod +x deploy.sh


#### Build

    ./build.sh drealms


#### Deploy

    ./deploy.sh drealms { local | test | production }


## NFT Interface

The following is a description of the dRealms NFT interface:



### `createnft()`

Creates a new NFT family

* `token_name` is the name of the new token family.

* `issuer` is the account allowed to issue (mint) new NFTs.

* `burnable` allows tokens to be burnt by the issuer.

* `transferable` allows tokens to be transferred.

* `consumable` allows the token to be consumed by the owner. Often used to trigger consumption of one-use tokens.

* `max_supply` is the maximum number of NFTs allowed to be created.



### `issuenft()`

Issues a new NFT

* `to` is the account to receive the issued NFT.

* `token_name` is the token family of the NFT to issue.

* `immutable_data` is the immutable data for the NFT.

* `memo` is a memo for extra data.



### `transfernft()`

Transfers an NFT

* `from` is the name of the account sending the NFT.

* `to` is the account receiving the NFT.

* `token_name` is the token family of the NFT(s) to transfer.

* `serials` is a list of NFT serial numbers to transfer.

* `memo` is a memo for extra data.



### `burnnft()`

Burns NFT(s)

* `token_name` is the token family of the NFT(s) to burn.

* `serials` is a list of NFT serial numbers to burn.

* `memo` is a memo for extra data.

### `consumenft()`

Consumes an NFT

* `token_name` is the token family of the NFT to consume.

* `serial` is the serial number of the NFT to consume.

* `memo` is a memo for extra data.

### `updatenft()`

Updates the mutable data on an NFT

* `token_name` is the token family of the NFT to update.

* `serial` is the serial number of the NFT to update.

* `new_mutable_data` is the new mutable data to assign to the NFT.



## License Interface

The following is a description of the dRealms License interface:



### `setlicensing()`

* `token_name` is the token family to accept the new license model.

* `new_license_model` is the new license model being set.



### `newlicense()`

* `token_name` is the token family receiving the new license.

* `owner` is the owner of the new license.

* `expiration` is the expiration time of the license.



### `eraselicense()`

* `token_name` is the token family to erase the license from.

* `license_owner` is the owner of the license to erase.



### `upserturi()`

* `token_name` is the token family to upsert the uri for.

* `license_owner` is the owner of the license being updated.

* `uri_type` is the type of the new uri ("full" or "base").

* `uri_name` is the name of the new uri.

* `new_uri` is the uri as a string.



### `removeuri()`

* `token_name` is the token family to remove the uri from.

* `license_owner` is the owner of the license being removed.

* `uri_type` is the type of the uri to remove ("full" or "base").

* `uri_name` is the name of the uri to remove.



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

**Scenario**: GoodBlock Games has launched a new title where in-game dragons are tokenized on the Telos Blockchain, and Bethesda Game Studios wants to build a game where those same dragon tokens are imported and usable in their game. 

Since GoodBlock Games created the original game asset they control the licensing rights to their dragon token, and therefore may allow or disallow Bethesda Game Studios to get a new license slot for the dragon tokens. When GoodBlock Games created their dragon tokens they set the licensing model to **Permissioned Licensing**, meaning new license slots are obtainable through approval by GoodBlock Games. GoodBlock Games and Bethesda Game Studios negotiate a deal, and GoodBlock Games agrees to give Bethesda Game Studios a 1-year license slot for GBG's dragon assets. The deal is accepted by both parties and the license slot is opened on-chain.

For our example, Bethesda Game Studios is making a Sci-Fi FPS style game (one where a 2D fantasy dragon would almost certainly be out of place), so during license negotiation Bethesda works to ensure GBG is okay with allowing their dragon tokens to be represented as a visibly different asset - in this case, fantasy inspired laser rifles (EX: a water dragon in Drakos Keep becomes a specialized laser rifle with a dragonscale skin, shoots blue lasers, and has increased weapon handling). The rigidity to which asset creators must adhere to the original token design is decided during license purchase and is outside the scope of the token contract's responsibility.

Note that having token representation vary by game is intentional, as this decouples game assets from the original game and opens them up for creative use in other games. While some games are designed to be a giant sandbox, and there will no doubt be many games that use dRealms assets this way, it is also important to understand that the majority of games are very hand-crafted experiences where simply "dropping in" an asset developed for another game would break the immersion and look out of place in the game world.
