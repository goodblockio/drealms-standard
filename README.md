# dRealms™ Token Standard

dRealms™ is a cross-game standard for creating tokenized game assets for EOSIO blockchains.

## Features

* `Robust Token Ownership and Authorship`

    dRealms offers a flexible ownership/authorship model for asset creation and true ownership token economics. The current true ownership paradigm assumes 'complete ownership', whether that design works for every use case or not. dRealms offers a suite of tokenomics designs that developers can choose between depending on the needs of their game and business model.

* `Nonfungible and Fungible Tokens`

    The dRealms standard offers highly customizable fungible and nonfungible tokens. In addition to the immortal tokens common to most true ownership tokens, dRealms Token Schemas can be retirable, transferable, or consumable, activatable, or any combination thereof.

* `Common Stat Blocks`

    dRealms handles game asset progression across multiple games, meaning a player's progress in one game doesn't conflict with progress made in another game - they are complimentary. Games award experience directly to NFTs through legitimate gameplay, which can then be spent by the NFT owner to level up the asset or held onto for later. Experience belongs to the NFT, so if a player decides to sell their NFT on a dRealms Marketplace all experience will travel with the game asset.

* `Lightweight and Scalable Design`

    Because of dRealms' elegant contract design, it uses far fewer resources to operate compared to other NFT contracts. Contract tables are lean and have been optimized to scale naturally by demand, without hogging up unecessary resources.

* `Custom Licensing Models`

    dRealms offers a variety of licensing models, allowing creators to decide how their assets will work with others' games:

    **Disabled Licensing**: this setting disables new license additions entirely. This feature is desirable for token creators who want to maintain full control of all NFT assets.

    **Open Licensing**: this setting allows any user to reserve a license slot for an NFT for free (not counting RAM costs). This setting is desirable if asset creators want their NFTs to be heavily modifiable and open for all asset creators to use.

    **Permissioned Licensing**: this setting allows token creators to individually approve additional license slots from a list of requests submitted by prospective asset developers.

* `dRealms Application Token Interface (ATI)`

    dRealms' unique ATI feature makes creating NFT assets as easy as regular game objects, meaning developers can build games in a familiar way but easily flag certain game objects as having an NFT counterpart logged on-chain. This allows game engines to build "default" game objects from the ATI and then render the object after being returned the actual values from the chain.

## Documentation

[Developer Guide](docs/DeveloperGuide.md)

## Roadmap

**dRealms v0.1.0**

1. Refined ATI Documentation
2. Unity Plugin MVP
3. Additional License Models

**dRealms v0.2.0**

Coming Soon...
