# Nifty Token Standard

The Nifty Token Standard is a lightweight, scalable, and manageable game-based NFT standard for EOSIO software.

## Key Features

* `Lightweight`

    Nifty was elegantly designed to allow for fast token lookup - no more drilling through tables to complete your data query.

* `Scalable`

    Because of Nifty's elegant design, it uses far fewer resources in order to operate. Contract tables have been optimized to scale naturally by demand. 

* `Manageable`

    Nifty was designed with cross-game assets in mind. Nifty's unique licensing feature lets developers monetize their game assets further (if desired) by allowing third parties to add custom asset packages to an existing NFT token. Token creators have the option of allowing third parties to purchase new licenses, allowing open license access, or disabling license addition altogether.

    **Monetary Licensing**: this setting allows token creators to monetize their game assets by allowing certain accounts write access to extra metadata slots on an NFT. The original token creator will always get the first license slot for free regardless of their token settings. More licenses can be purchased for an NFT which allows third parties to write data to an additional metadata slot (and only that slot, overwriting data saved by other games is expressly prohibited).

   **Open Licensing**: this setting allows any user to reserve a license slot for an NFT for free (not counting RAM costs). This setting is desirable if asset creators want their NFTs to be heavily modifiable and open for all asset creators to use.

   **Disabled Licensing**: this setting disables new license additions entirely. This feature is desirable for token creators who want to maintain full control of all NFT assets.

## Token Templates


