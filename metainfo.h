
//MIT License
//
//Copyright (c) 2022 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.



#ifndef __metainof_header
#define __metainof_header

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HW_TYPE_NANO 0
#define HW_TYPE_MICRO 1
#define HW_TYPE_EAGLE_HH 2
#define HW_TYPE_EAGLE_HH_ANT_DIV 3

#define META_DATA_TYPE_CONST 1
#define META_DATA_TYPE_ZONEINFO 2
#define META_DATA_TYPE_META_CONFIG_INFO 3
#define META_DATA_TYPE_FFT 4
#define META_DATA_TYPE_IQ8 5

#define WIO_BUTTON_MODE_MONITOR 0
#define WIO_BUTTON_MODE_CONFIG 1
#define WIO_BUTTON_MODE_TG_ZONE 2
#define WIO_BUTTON_MODE_RF_GAIN 3

#define P25_STATE_SYNC 1
#define P25_STATE_TSBK 2
#define P25_STATE_VOICE 3
#define P25_STATE_TDU 4
#define P25_STATE_TDULC 5
#define P25_STATE_VGRANT 6
#define P25_STATE_TSBK_ERR 7
#define P25_STATE_VERR1 8
#define P25_STATE_VERR2 9

void update_metainfo( void );
void init_metainfo( void );
void init_metainfo( void );
void meta_set_wioline1( char *str );
void meta_set_wioline2( char *str );

void p25_add_state( uint8_t state );
uint8_t p25_get_state( void );

extern int8_t sym_const[];
extern uint32_t RID;

typedef struct {

  uint32_t magic;
  uint16_t port;
  uint16_t len;

  double current_freq;

  uint32_t wacn_id;
  uint16_t sys_id;
  uint16_t site_id;
  uint16_t rf_id;
  uint16_t p25_sys_nac;
  uint8_t alpha[13];
  uint8_t desc[33];
  uint8_t mac[6];


  int lna_gain;
  int mixer_gain;
  int vga_gain;

  float err_hz_f;
  float est_hz_f;
  uint8_t synced;
  float nco_offset;
  float loop_freq;
  float evm_db;
  float evm_p;
  int use_demod;
  int did_freq_cc_lock;

  int rssi_i;
  int32_t tg_s;
  float erate;

  uint8_t status_led;
  uint8_t tg_led;

  uint8_t on_control_b;
  int32_t follow;
  uint8_t sys_name[32];
  uint8_t is_control_b;

  uint8_t phase2;

  float rssi_f;

  float squelch;
  int agcmode;
  volatile float max_freq_hz;
  volatile uint16_t max_freq_cnt;
  uint8_t mode_b;
  uint8_t slot_b;
  uint32_t send_const_count;
  uint8_t roam;
  float bat_volt_f;
  uint32_t global_sync_count;
  uint16_t tgzone_s;
  uint32_t lsm_std;
  uint8_t cc_pause;

  uint8_t wio_packet_id;

  uint32_t RID;
  uint8_t  alias[32];

  uint8_t do_wio_lines;
  uint8_t wio_line1[32];
  uint8_t wio_line2[64];
  uint32_t wio_timeout;

  uint8_t tsbk_sec;
  uint32_t total_session_time;

  uint32_t ref_freq_cal;

  uint8_t site_name[32];
  uint8_t data_type;
  uint8_t data[128];

  uint8_t wio_button_mode;
  uint8_t audio_mute;
  uint8_t learn_mode;

  uint16_t tgzone;

  int voice_tg_timeout;
  uint8_t roaming;
  int roaming_timeout;

  uint32_t roam_wacn;
  uint16_t roam_sysid;
  uint8_t inc_in_scan;
  uint8_t lock_nac;

#if 1
  uint16_t col1;
  uint16_t col2;
  uint16_t col3;
  uint16_t col4;
  uint16_t col5;
  uint16_t col6;
  uint16_t col7;
  uint16_t col8;
  uint16_t col9;
  uint16_t col10;
  uint16_t col11;
  uint16_t col12;
  uint16_t line0_bg;
  uint16_t line0_fg;
  uint16_t col_menu_fg;
  uint16_t col_menu_bg;
  uint16_t col_def_bg;
  uint16_t col_def_fg;
  uint16_t col_def_led1_on;
  uint16_t col_def_led2_on;
  uint16_t col_def_led1_off;
  uint16_t col_def_led2_off;
  uint16_t col_def_indicator;
  uint16_t col_def_const;
#endif

  uint8_t draw_const_circles;

  float audio_volume_f;
  uint32_t speaker_en;

  uint32_t decoder;
  int32_t  squelch_dbm;

  uint32_t peak_det;
  uint32_t gain_controller;
  uint32_t p25_state;
  uint32_t p25_state_pid;
  uint32_t antenna;
  uint32_t hw_type;

  uint32_t crc_val;
}
metainfo;

extern volatile int metainfo_ready;
extern metainfo minfo;

extern uint8_t WIO_STATUS[];
extern metainfo *mptr;

#ifdef __cplusplus
}
#endif
#endif
