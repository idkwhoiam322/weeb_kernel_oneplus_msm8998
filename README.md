# Weeb Kernel for OnePlus 5/T msm8998 Repository

![logo](https://img.xda-cdn.com/6suyxKjsTSz7Oba53XGoKRgEagg=/https%3A%2F%2Fi.imgur.com%2Fha29jHc.png)

> This repository is for all the keks I do on msm8998, specifically cheeseburger and dumpling so feel free to spank me for all the times I fatten up your foods and overcook them


## Branching, mechanics, and stuffs
```
9.0: "Stable" Branch, feel free to kang and use for whatever but do know that `I love force pushing` and I really mean `love` so like you should know what to do to deal with it because I often drop commits even though I usually end up doing it on another branch.
pie_als: OnePlus Pie Beta OSS + upstreamed to linux latest tag.
pie_caf: OnePlus Pie Beta OSS + upstreamed to caf latest tag.
pie_clang: OnePlus Pie Beta OSS + Nathan's latest clang stuffs merged for 4.4-pie.
pie_eas: OnePlus Pie Beta OSS + EAS base stuff from Joshuous's Oreo source.
oneplus/QC8998_P_9.0_Beta: OnePlus Pie Beta OSS which everything is based on. This branch will only pull from OnePlus's own update.
Anything that is not mentioned above: Experimenetal and you should probably just leave it alone. Feel free to peek but don't blame me thanks!
```
## Features
![phones](https://telegra.ph/file/00a5eb3b0b5dd14e4c065.png)

```
- EAS stuff from Josh's Oreo source as base
- HMP Stuff removed
- EAS stuff upstreamed from various sources (caf, nathan, etc.)
- Latest Linux Upstream
- Latest CAF Upstream
- Dynamic Stune Boost
- CFQ Upstreamed
- ZRAM Upstreamed
- ZSTD as default ZRAM Compression Algorithm
- Westwood as default TCP Congestion Algorithm
- Sound Control
- Flat Memory model
- some arm64 related optimizations
- Compiled using Clang 8.0.6: With O3 optimizations wherever possible
- KCAL Control [Note: This doesn't seem to work on Pie yet, will investigate at some point]
- Boeffla Wakelock Blocker
```
> I don't have much to say about Boeffla's Wakelock Blocker, just read what I think of it [HERE](https://github.com/whoknowswhoiam/weebmsm8998-pie/commit/210374f687bc11d06800d2881a1bc1a92d97b3af).
```
- Wireguard Support
- Sweep2wake, double tap to wake, vibration control by flar2
- Support for Custom ROMs
- HZ 500
- Backlight dimmer
- CPU Governors cut down to just schedutil and the fallback performance governor
- Disables dm-verity
- Kernel Samepage Merging
```

## LICENSE
```
 Copyright (c), The Linux Foundation. All rights reserved.
 
 This software is licensed under the terms of the GNU General Public
 License version 2, as published by the Free Software Foundation, and
 may be copied, distributed, and modified under those terms.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
```
> See the accompanying [COPYING](https://github.com/whoknowswhoiam/weebmsm8998-pie/blob/9.0/COPYING) file for more details.

> Also this is kanged from [HERE](https://github.com/RaphielGang/bash_kernel_sdm845/tree/README).