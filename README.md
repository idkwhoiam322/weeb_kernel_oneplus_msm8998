# Weeb Kernel for OnePlus 5/T msm8998 Repository

![logo](https://img.xda-cdn.com/6suyxKjsTSz7Oba53XGoKRgEagg=/https%3A%2F%2Fi.imgur.com%2Fha29jHc.png)

> This repository is for all the keks I do on msm8998, specifically cheeseburger and dumpling so feel free to spank me for all the times I fatten up your foods and overcook them

## Drone CI Build Status
[![Build Status](https://cloud.drone.io/api/badges/whoknowswhoiam/weebmsm8998-pie/status.svg)](https://cloud.drone.io/whoknowswhoiam/weebmsm8998-pie)

## Semaphore CI Build Status
[![Build Status](https://semaphoreci.com/api/v1/whoknowswhoiam/weebmsm8998-pie/branches/9-0/badge.svg)](https://semaphoreci.com/whoknowswhoiam/weebmsm8998-pie)

## Branching, mechanics, and stuffs
```
kernel-4.4: Staging Branch, feel free to kang and use for whatever but do know that `I love force pushing` and I really mean `love` so like you should know what to do to deal with it because I often drop commits even though I usually end up doing it on another branch.
pie: "Stable" Branch, My final branch which I will use to build my stable releases.
	This is finally merged to my stable repo: https://github.com/RaphielGang/weebmsm8998-pie/
pie_als: OnePlus Pie Beta OSS + upstreamed to linux latest tag.
pie_caf: OnePlus Pie Beta OSS + upstreamed to caf latest tag.
pie_clang: OnePlus Pie Beta OSS + Nathan's latest clang stuffs merged for 4.4-pie.
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
Based on Weeb Kernel v2.53 Release - Codename: Caesar
```
- Compiled using GCC 9.1.0 bare metal compiled by kdrag0n
- Latest LTS merged from kernel.org - 4.4.186
- Latest CAF tag - "LA.UM.7.4.r1-05400-8x98.0"
- Latest QCACLD tag - "LA.UM.7.3.r1-07900-sdm845.0"
- Latest f2fs merge from kernel/common
- Latest f2fs related commits ( like rapid gc ) from arter97 as per May 15 2019
- Latest fixes for clang support
- Up to date with latest OnePlus changes
- Used latest GCC and clang to fix several code issues detected
- KCAL support
- Sound control
- Sweep2sleep
- Hight Brightness Mode ( HBM )
- Source unified for oos and custom ROMs
- Redone EAS implementation - seems to work much better now as reported by users and seen by myself
- Disabled CAF CPU_BOOST
- Wireguard support
- CFQ upstreamed
- ZRAM disabled
- increased kgsl priority
- vdso32 support
- ULPS mode for display
- Disabled audit
- HZ 100
- Lower touch latency
- kerneltoast's ( aka sultanxda ) devfreq boost driver
- Removed qos code added by oneplus
- Several EAS backports from higher kernel versions ( 4.9, 4.14, 4.19, some from mainline too )
- schedutil updated with several patches from higher kernel versions
- Some softirq backports from mainline that improved jitter
- CRC32 backports by arter97
- RCU and cpufreq backports by celtare21
- Several improvements from p2 and p3/a Q tag
- Disabled a ton of unnecessary logging
- top-app schedtune.boost locked at 1
- Removed unused frequencies ( All frequencies below 518400 and 806400 have been removed )
- Stune_assist by YaroST12
- cpuset assist
- Cpu Input Boost by kerneltoast
- ufs ricing from wahoo and essential phone
- cpuidle, kgsl, mdss, qos and ufs power efficiency improvements by kerneltoast
- Fake sched_boost proc to fool userspace since I'm not using dynamic stune boost
- Removed excessive debug bloat from qcacld
- Dependency on Magisk removed. Kernel modifies a file in vendor to adjust cpusets ( defaults for most things hardcoded in kernel, users free to modify them )
- Fixed several memory leaks
- Added some build.prop tweaks and switched to the opengl renderer
- Disabled DEBUG_FS
- Disabled FTRACE
- Disabled DEBUG_KERNEL
- KLapse 5.0
- Sultan's binder rewrite
- adreno improvements
- PELT for EAS
- ext4,  genirq, glink, sdcardfs, qseecom ricing
- POCKET_JUDGE ( custom only )
- Wahoo irq balancer
- Variants:
	oos - OxygenOS
	custom - Custom ROMs that aren't omni
	omni - Omni based custom ROMs
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