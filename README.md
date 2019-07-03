# dRealms Token Standard

The dRealms Token Standard is a lightweight cross-game NFT standard for EOSIO software.

## Key Features

* `Lightweight and Scalable`

    Because of dRealms's elegant design, it uses far fewer resources in order to operate. Contract tables are lean and have been optimized to scale naturally by demand.

* `Custom Licensing Models`

    dRealms was designed with cross-game assets in mind. dRealms's unique licensing feature lets developers monetize their game assets further (if desired) by allowing third parties to add custom asset packages to an existing NFT token. Token creators have the following options (ordered from most restrictive to least restrictive) when licensing their game assets:

    **Disabled Licensing**: this setting disables new license additions entirely. This feature is desirable for token creators who want to maintain full control of all NFT assets.

    **Open Licensing**: this setting allows any user to reserve a license slot for an NFT for free (not counting RAM costs). This setting is desirable if asset creators want their NFTs to be heavily modifiable and open for all asset creators to use.

    **Permissioned Licensing**: this setting allows token creators to individually approve additional license slots from a list of requests submitted by prospective asset developers.

    
    

## Application Token Interface (ATI)

dRealms's novel ATI feature makes developing NFT's as familiar as making regular game assets. A token's ATI defines data types and formatting so the dRealms Unity Plugin knows what to expect when querying for NFT data on-chain. 

```
{
    "comment": "This file was generated with dRealms-atigen.",
    "version": "dRealms-atigen/1.0",
    "engine": {
        "name": "unity",
        "version": "2019.6.5"
    },
    "details": {
        "horns": "uint16",
        "head": "uint16",
        "eyes": "uint16",
        "color_pri": "uint32",
        "color_sec": "uint32",
        "level": "uint8",
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

## Roadmap

**dRealms v1.0.0:**

1. Revise license flow and action signatures
2. Implement DEX features
3. Purchasable license model - additional license models

**dRealms v1.1.0**

1. ATI refinement
2. Unity Plugin
