# Nifty Token Standard

The Nifty Token Standard is a lightweight, scalable, and licensable game-based NFT standard for EOSIO software.

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

    Nifty believes that original asset creators should maintain creative control over the representation of their assets in other games. Whether this means the creator enforces strict asset templating or leaves it wide open for asset developers to use their imagination is entirely up to the original asset creator.

## Manifest/Metadata Template

```
{
    "version": "1",
    "asset": {
        "template": "2dasset",
        "class": "creature"
    },
    "class": "creature",
    "instance": {
        "strength": "5",
        "dexterity": "3",
        "constitution": "8",
        "intelligence": "7",
        "wisdom": "2",
        "charisma": "9"
    },
    "file": "dragon51.2dasset",
    "created": "2019-06-05T19:20:50+00:00",
    "creator": "craig.tf"
}
```

## Asset Templates

* `2dasset`

```
{
    "imageSmall": "dragon51.png",
    "imageLarge": "dragon51.png",
    ...
}
```

* `3dasset`

```
{
    "render": "dragon51.fbx",
    "scale": "small",
    "class": ""
    ...
}
```

## Class Templates

* `vehicle`

```
{
    "type": "truck",
    "wheels": "4",
    "passengers": "2",
    "color": "blue",
    ...
}
```

* `gun`

```
{
    "type": "rifle",
    "caliber": "5.56mm",
    "designation": "M4 Carbine",
    "skin": "dragonscales.texture",
    ...
}
```

* `creature`

```
{
    "type": "dragon",
    "element": "water"
    ...
}
```
