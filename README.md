# Weeb Kernel for OnePlus 5/T msm8998 Repository

![logo](https://img.xda-cdn.com/6suyxKjsTSz7Oba53XGoKRgEagg=/https%3A%2F%2Fi.imgur.com%2Fha29jHc.png)

> This repository is for all the keks I do on msm8998, specifically cheeseburger and dumpling so feel free to spank me for all the times I fatten up your foods and overcook them

## Semaphore CI Build Status
[![Build Status](https://semaphoreci.com/api/v1/whoknowswhoiam/weebmsm8998-pie/branches/9-0/badge.svg)](https://semaphoreci.com/whoknowswhoiam/weebmsm8998-pie)
## Branching, mechanics, and stuffs
```
9.0: Staging Branch, feel free to kang and use for whatever but do know that `I love force pushing` and I really mean `love` so like you should know what to do to deal with it because I often drop commits even though I usually end up doing it on another branch.
pie: "Stable" Branch, My final branch which I will use to build my stable releases. No force pushes.
pie_als: OnePlus Pie Beta OSS + upstreamed to linux latest tag.
pie_caf: OnePlus Pie Beta OSS + upstreamed to caf latest tag.
pie_clang: OnePlus Pie Beta OSS + Nathan's latest clang stuffs merged for 4.4-pie.
pie_eas: OnePlus Pie Beta OSS + EAS base stuff from Joshuous's Oreo source + a ton of picks from 4.9 backported by Kuran and (very few) by me and some others too.
pie_f2fs: OnePlus Pie BETA OSS + F2FS merged from aosp kernel/common and patchese from arter97.
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
Based on Weeb Kernel v2.1x Release - Codename: AURA
```
- Compiled using Latest AOSP Clang
- Latest Linux Upstream - 4.4.176
- Latest CAF Upstream - LA.UM.7.4.r1-04700-8x98.0
- Up to date with latest OnePlus changes
- Cleaned up some OnePlus code
- EAS stuff from Josh's Oreo source as base for EAS side changes
- HMP Stuff removed
- Removed unnecessary code
- Dynamic Stune Boost introduction and improvements (thanks to joshuous and RenderBroken)
- Improvements to devfreq
- Disabled Qualcomm Download Mode
- F2FS upstreamed to kernel/common [February] along with some improvements from arter97
- Fixed several warnings and improved code using GCC 9.0 and Clang from Mainline
- Upstreamed z3fold and used as default for zswap (thanks to celtare21)
- qcacld-3.0 workqueues relaxed (thanks to raphielscape)
- Several Kernel Hardening patches thanks to CopperHeadOS and others
- Enabled YAMA LSM Security
- SELinux Enforcing (Unless your ROM says otherwise)
- SELINUX: Disable auditing
- NTFS R/W support
- Removed OnePlus QoS code (thanks to Francisco Franco)
- CFQ Upstreamed
- ZRAM Upstreamed
- ZSTD Upstreamed
- LZ4 Upstreamed
- LZ4 as default ZRAM Compression Algorithm
- Swappiness dropped to 8
- Dirty Ratio = 5
- Dirty Background Ratio = 2
- Several updates to kernel/sched and some backported from the future (Thanks to nathanchance, google, kuran kaname, etc. etc.)
- kgsl backports (thanks to celtare21)
- Reset LMK to android 4.4 state and improved on that (Thanks to Franciso Franco and Kuran Kaname (Celtare21))
- Westwood as default TCP Congestion Algorithm
- Sound Control
- some arm64 related optimizations
- KCAL Control
- Wireguard Support
- Sweep2sleep, double tap to wake gestures by flar2
- HZ 100
- Backlight dimmer
- CPU Governors cut down to just schedutil and performance(for quicker boot only) governors
- CAKE as default net qdisc (thanks to kdrag0n)
- Upstreamed kthread
- RCU Upstream
- vdso32 support
- BFQ backported from 4.9
- USB Fast Charging (DISABLED by default)
- High Brightness Mode
- Disables dm-verity
- Variants:
	OxygenOS
	Custom ROMs that aren't omni
	Omni based custom ROMs
- Disable KALLSYMS since we have useless pstore, and this allows us to have a significantly smaller kernel image
- Performance cluster underclocked to 1958400 kHz - Does not affect performance significantly but has improvements in device temperature and some decent improvements in battery
```

## Features you will never see
```
- Overclock [Nope, I blindly trust Qualcomm here]
- Undervolting
- Anything that compromises performance or causes any kind of janks, if I've added any such changes myself, I'll revert them before they go into stable
- Any schedulers other than cfq (maybe maple since Flash Kernel had it, but for now even that's a no) [Exception to the BFQ backport I did cuz it seems to be more consistent than cfq in my experience]
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