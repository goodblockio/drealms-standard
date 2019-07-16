# dRealms Token Standard

dRealms is a cross-game token standard for EOSIO software.

## Features

* `Robust Token Ownership and Authorship`

    dRealms offers the ideal ownership/authorship model for token economics. The current tokenomics paradigm pushes for 'complete ownership', whether that design works for every use case or not. dRealms offers a suite of tokenomics designs and developers can choose which one best suits their unique business model.

* `Nonfungible and Fungible Tokens`

    The dRealms standard offers highly customizable fungible and nonfungible tokens. Token Families can be retirable, transferable, or consumable, or any combination of the three.

* `Lightweight and Scalable Design`

    Because of dRealms' elegant contract design, it uses far fewer resources to operate compared to other NFT contracts. Contract tables are lean and have been optimized to scale naturally by demand, without hogging up unecessary resources.

* `Custom Licensing Models`

    **Disabled Licensing**: this setting disables new license additions entirely. This feature is desirable for token creators who want to maintain full control of all NFT assets.

    **Open Licensing**: this setting allows any user to reserve a license slot for an NFT for free (not counting RAM costs). This setting is desirable if asset creators want their NFTs to be heavily modifiable and open for all asset creators to use.

    **Permissioned Licensing**: this setting allows token creators to individually approve additional license slots from a list of requests submitted by prospective asset developers.

* `dRealms Application Token Interface (ATI)`

    dRealms' unique ATI feature makes creating NFT assets as easy as regular game objects, meaning developers can build games in a familiar way but easily flag certain game objects as having an NFT counterpart logged on-chain. This allows game engines to build "default" game objects from the ATI and then render the object after being returned the actual values from the chain.

## Documentation

[Developer Guide](docs/DeveloperGuide.md)

## Roadmap

**dRealms v1.1.0**

1. Refined ATI Documentation
2. Unity Plugin MVP
3. Additional License Models

**dRealms v1.2.0**

Coming Soon...
