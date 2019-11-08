/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 idkwhoiam322 <idkwhoiam322@raphielgang.org>
 */

/* In-kernel powerHAL to replicate some behaviours of the pixel powerHAL */

#ifndef _POWER_HAL_H
#define _POWER_HAL_H

#ifdef CONFIG_CPU_INPUT_BOOST
void powerhal_boost_kick(void);
void powerhal_boost_kick_max(unsigned int duration_ms);
#else
static inline void powerhal_boost_kick(void) { }
static inline void powerhal_boost_kick_max(unsigned int duration_ms) { }
#endif

#endif /* _POWER_HAL_H */
