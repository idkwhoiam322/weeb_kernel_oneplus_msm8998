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
Based on Weeb Kernel v3.00 Release - Codename: Ricardo
```
- Compiled using GCC 9.1.0 bare metal compiled by kdrag0n
- Support for Custom ROMs on android 9 and 10 only
- Linux 4.4.196
- Cleaned up OnePlus changes as much as possible and brought bringup as close to CAF as I could
- Use latest [ at time of release ] public CAF tag for 8x98 - LA.UM.8.4.r1-04500-8x98.0
- Use qcacld-3.0 from wahoo android10-release [ CAF Q qcacld-3.0 is bugged ]
- PELT for EAS
- Replicate pixel's powerHAL behaviour to disable EAS behaviour on app launch
- Disable schedtune boost and bias on screen off
- disable debugfs
- Latest f2fs merge from kernel/common
- Latest fixes for clang support
- Used latest GCC and clang to fix several code issues detected
- KCAL support
- KLAPSE support
- Sound control
- Hight Brightness Mode ( HBM )
- CFQ upstreamed
- increased kgsl priority
- vdso32 support
- ULPS mode for display
- Disabled audit
- Lower touch latency
- kerneltoast's ( aka sultanxda ) devfreq boost driver
- Removed QoS code treewide added by OnePlus
- Several EAS backports from higher kernel versions ( 4.9, 4.14, 4.19, some from mainline too )
- schedutil updated with several patches from higher kernel versions
- cpufreq backports by celtare21
- Several improvements from p2, p3/a, p4 Q tag
- Disabled a ton of unnecessary logging
- top-app schedtune boost default at 5, configurable by the user
- Cpu Input Boost by kerneltoast
- UFS ricing from wahoo
- cpuidle, kgsl, mdss, QoS and UFS power efficiency improvements by kerneltoast
- Fake sched_boost proc to fool userspace since I'm not using dynamic stune boost
- Removed excessive debug bloat from qcacld
- Fixed several memory leaks
- Sultan's binder rewrite
- Sultan's ion rewrite
- Sultan's iommu rewrite
- Adreno improvements
- improvements to sdcardfs, qseecom
- use rcu normal
- Some changes from pixel and google guidelines to boot faster
- tons of more small changes throughout
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