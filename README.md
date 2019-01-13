# Weeb Kernel for OnePlus 5/T msm8998 Repository

![logo](https://img.xda-cdn.com/6suyxKjsTSz7Oba53XGoKRgEagg=/https%3A%2F%2Fi.imgur.com%2Fha29jHc.png)

> This repository is for all the keks I do on msm8998, specifically cheeseburger and dumpling so feel free to spank me for all the times I fatten up your foods and overcook them

## Semaphore CI Build Status
[![Build Status](https://semaphoreci.com/api/v1/whoknowswhoiam/weebmsm8998-pie/branches/9-0/badge.svg)](https://semaphoreci.com/whoknowswhoiam/weebmsm8998-pie)
## Branching, mechanics, and stuffs
```
9.0: EXPERIMENTAL Branch, feel free to kang and use for whatever but do know that `I love force pushing` and I really mean `love` so like you should know what to do to deal with it because I often drop commits even though I usually end up doing it on another branch.
pie: "Stable" Branch, My final branch which I will use to build my stable releases. No force pushes.
pie_als: OnePlus Pie Beta OSS + upstreamed to linux latest tag.
pie_caf: OnePlus Pie Beta OSS + upstreamed to caf latest tag.
pie_clang: OnePlus Pie Beta OSS + Nathan's latest clang stuffs merged for 4.4-pie.
pie_eas: OnePlus Pie Beta OSS + EAS base stuff from Joshuous's Oreo source.
oneplus/QC8998_P_9.0_Beta: OnePlus Pie Beta OSS which everything is based on. This branch will only pull from OnePlus's own update.
Anything that is not mentioned above: Experimenetal and you should probably just leave it alone. Feel free to peek but don't blame me thanks!
```
## About Phone
![phones](https://telegra.ph/file/00a5eb3b0b5dd14e4c065.png)

`									(Left) OnePlus 5T - OnePlus 5 (Right)									`	
###### OnePlus 5
```
Device Codename - Cheeseburger
Release Date - 22nd June 2017
Current Android Version - 9/Pie
```
*Full Specifications - [GSM Arena](https://www.gsmarena.com/oneplus_5-8647.php)*

###### OnePlus 5T
```
Device Codename - Dumpling
Release Date - 16 November 2017
Current Android Version - 9/Pie
```
*Full Specifications - [GSM Arena](https://www.gsmarena.com/oneplus_5t-8912.php)*


## Features
Based on Weeb Kernel v2.01 Odyssey Release
```
- Latest Linux Upstream - 4.4.170
- Latest CAF Upstream - LA.UM.7.4.r1-04600-8x98.0
- Up to date with latest OnePlus changes
- EAS stuff from Josh's Oreo source as base for EAS side changes
- HMP Stuff removed
- EAS stuff upstreamed from various sources (caf, nathan, etc.)
- Dynamic Stune Boost
- Disalbed Qualcomm Download Mode
- Fixed dmesg spam (reverted faulty commits)
- Fixed several warnings and improved code using GCC 9.0 and Clang from Mainline
- Upstreamed z3fold and used as default for zswap (thanks to celtare21)
- qcacld-3.0 workqueues relaxed (thanks to raphielscape)
- Enabled YAMA LSM Security
- SELinux Enforcing (Unless your ROM says otherwise)
- Removed OnePlus QoS code (thanks to Francisco Franco)
- CFQ Upstreamed
- ZRAM Upstreamed
- ZSTD Upstreamed
- LZ4 Upstreamed
- LZ4 as default ZRAM Compression Algorithm
- VFS Cache Pressure dropped to 50
- Swappiness dropped to 8
- Westwood as default TCP Congestion Algorithm
- Sound Control
- some arm64 related optimizations
- Compiled using Clang 8.0.7: With O3 optimizations wherever possible
- KCAL Control [Note: This doesn't seem to work on Pie yet, will investigate at some point]
- Wireguard Support
- Sweep2wake, double tap to wake gestures by flar2
- Support for Custom ROMs + Omni
- HZ 300
- Backlight dimmer
- CPU Governors cut down to just schedutil and the fallback performance governor
- Disables dm-verity
```

## Features you will never see
```
- Overclock/Underclock
- Undervolting
- Anything that compromises performance or causes any kind of janks, if I've added any such changes myself, I'll revert them before they go into stable
- Any schedulers other than cfq (maybe maple since Flash Kernel had it, but for now even that's a no)
- No fsync toggle (stupid to disable it)
- No disabling CRC check (Not worth it, we are not using EMMC weak ass storage)
- Any governors other than schedutil
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