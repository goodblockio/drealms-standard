# Nifty Token Standard

The Nifty Token Standard is a lightweight, scalable, and licensable cross-game NFT standard for EOSIO software.

## Key Features

* `Lightweight`

    Nifty was elegantly designed to allow for fast token lookup - no more extensive drilling through tables to complete your data query.

* `Scalable`

    Because of Nifty's elegant design, it uses far fewer resources in order to operate. Contract tables are lean and have been optimized to scale naturally by demand. 

* `Licensable`

    Nifty was designed with cross-game assets in mind. Nifty's unique licensing feature lets developers monetize their game assets further (if desired) by allowing third parties to add custom asset packages to an existing NFT token. Token creators have the option of allowing third parties to purchase new licenses, allowing open license access, or disabling license addition altogether.

    **Monetary Licensing**: this setting allows token creators to monetize their game assets by allowing certain accounts write access to extra metadata slots on an NFT. The original token creator will always get the first license slot for free regardless of their token settings. More licenses can be purchased for an NFT which allows third parties to write data to an additional metadata slot (and only that slot, overwriting data saved by other games is expressly prohibited).

    **Permissioned Licensing**: this setting allows token creators to individually approve additional license slots from a list of requests submitted by prospective asset developers.

    **Open Licensing**: this setting allows any user to reserve a license slot for an NFT for free (not counting RAM costs). This setting is desirable if asset creators want their NFTs to be heavily modifiable and open for all asset creators to use.

    **Disabled Licensing**: this setting disables new license additions entirely. This feature is desirable for token creators who want to maintain full control of all NFT assets.

* `Creative Control`

    Nifty believes that original asset creators should maintain creative control over the representation of their assets in other games. Whether this means the creator enforces strict asset templating or leaves it wide open for developers to use their imaginations, is entirely up to the original asset creator.

* `Application Token Interfaces`

    Nifty's novel ATI feature makes developing NFT's as familiar as making regular game assets. A token's ATI defines data types and formatting so the Nifty Unity Plugin knows what to expect when querying for NFT data on chain. 

## Application Token Interface (ATI)

```
{
    "comment": "This file was generated with nifty-atigen.",
    "version": "nifty-atigen/1.0",
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

## Manifest Template - maybe...

The Nifty Manifest Template is used to describe the *format* in which NFT instance data will be returned to the client.

```
{
    "version": "1",
    "asset": {
        "engine": {
            "name": "unity",
            "version": "2019.1.5"
        },
        "template": {
            "name": "2dasset",
            "version": "1"
        },
        "extension": ".fbx",
        "language": "en"
        ...
    },
    "instance": {
        "class": "creature",
        "subclass": "dragon",
        "customclass": "drakoskeep"
    }
}
```
