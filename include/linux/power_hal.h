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

/* CPUBW management */
#ifdef CONFIG_DEVFREQ_GOV_QCOM_BW_HWMON
void set_hyst_trigger_count_val(int val);
void set_hist_memory_val(int val);
void set_hyst_length_val(int val);
#else
static inline void set_hyst_trigger_count_val(int val) { }
static inline void set_hist_memory_val(int val) { }
static inline void set_hyst_length_val(int val) { }
#endif

/* UFS Boosting */
extern struct Scsi_Host *ph_host;

void set_ufshcd_clkgate_enable_status(u32 value);

/* Video Playback detection */
extern bool video_streaming;
void video_streaming_disable_schedtune(void);

#endif /* _POWER_HAL_H */
