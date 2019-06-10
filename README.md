# Nifty Token Standard

The Nifty Token Standard is a lightweight cross-game NFT standard for EOSIO software.

## Key Features

* `Lightweight and Scalable`

    Because of Nifty's elegant design, it uses far fewer resources in order to operate. Contract tables are lean and have been optimized to scale naturally by demand.

* `Custom Licensing`

    Nifty was designed with cross-game assets in mind. Nifty's unique licensing feature lets developers monetize their game assets further (if desired) by allowing third parties to add custom asset packages to an existing NFT token. Token creators have the option of allowing third parties to purchase new licenses, allowing open license access, or disabling license addition altogether.

    **Monetary Licensing**: this setting allows token creators to monetize their game assets by allowing certain accounts write access to extra metadata slots on an NFT. The original token creator will always get the first license slot for free regardless of their token settings. More licenses can be purchased for an NFT which allows third parties to write data to an additional metadata slot (and only that slot, overwriting data saved by other games is expressly prohibited).

    **Permissioned Licensing**: this setting allows token creators to individually approve additional license slots from a list of requests submitted by prospective asset developers.

    **Open Licensing**: this setting allows any user to reserve a license slot for an NFT for free (not counting RAM costs). This setting is desirable if asset creators want their NFTs to be heavily modifiable and open for all asset creators to use.

    **Disabled Licensing**: this setting disables new license additions entirely. This feature is desirable for token creators who want to maintain full control of all NFT assets.

* `Maintain Creative Control`

    Nifty believes that original asset creators should maintain creative control over the representation of their assets in other games. Whether this means the creator enforces strict asset templating or leaves it wide open for developers to use their imaginations, is entirely up to the original asset creator.

    **Open Control**: Open control allows content creators the freedom to make NFT assets with no creative oversight. This setting means as long as a developer owns a license they may create any kind of asset they wish.

    **Approved Control**: Approved control allows token creators to approve a new asset before letting third party developers emplace assets in a new license slot.

## Application Token Interface (ATI)

Nifty's novel ATI feature makes developing NFT's as familiar as making regular game assets. A token's ATI defines data types and formatting so the Nifty Unity Plugin knows what to expect when querying for NFT data on chain. 

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
## Development Example

**Scenario**: GoodBlock Games has launched a new title where in-game dragons are tokenized on the Telos Blockchain, and Bethesda Game Studios wants to build a game where those same dragon tokens are imported and usable in their game. 

Since GoodBlock Games created the original game asset they control the licensing rights to their dragon token, and therefore may allow or disallow Bethesda Game Studios to get a new license slot for the dragon tokens. When GoodBlock Games created their dragon tokens they set the licensing model to **Monetary Licensing**, meaning new license slots are obtainable through payment to and approval by GoodBlock Games. GoodBlock Games and Bethesda Game Studios negotiate a deal between each other, and GoodBlock Games agrees to give Bethesda Game Studios a 1-year license slot for 300 TLOS. The deal is accepted by both parties and the license slot is opened and purchased on-chain - solidifying the transaction.

Note that GoodBlock Games had several licensing models to choose from when they created their dragon tokens, including permissioned, open, or disabled licensing models. Permissioned is identical to the monetary model, minus the contract-enabled payment. Open licensing signifies that additional license slots are open to any prospective developer and doesn't require approval or permission from the token creator. Disabled licensing means additional license slots are not obtainable.

Now that Bethesda Game Studios has acquired a license slot they may now begin creating their tokenized game assets. Their purchased license slot allows them to save a representation of the dragon asset, whether that is close to the original game asset or not is agreed upon during the licensing purchase.

Note that having token representation vary by game is intentional, as this decouples game assets from their original game and opens them up for use in other games while not appearing out of place in the foreign game world. 
