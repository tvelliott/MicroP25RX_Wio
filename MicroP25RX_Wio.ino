
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

//#include <FlashAsEEPROM.h> // temp disabled for hardcoded brightness
#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include "Free_Keybord.h"
#include "list_menu.h"
#include "handle_menus.h"
#include "handle_button_mode.h"
#include "meta_config_info.h"
#include "two_tone.h"

#include "lcd_backlight.hpp"

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite( &tft );
Keybord mykey; // Cleate a keybord


#include "buttons.h"

#include <SPI.h>
#include <wiring_private.h>

#include <stdint.h>
#include "crc32.h"
#include "metainfo.h"

#include <Seeed_FS.h>
#include <SD/Seeed_SD.h>

static LCDBackLight backLight;
//static uint8_t brightness = 5;
uint8_t brightness = 5;
static uint8_t maxBrightness = 100;

static int two_tone_A;
static int two_tone_B;
static int prev_max_freq_cnt;

meta_config_info minfo_config;

static uint32_t roam_time;

extern volatile int cmd_acked;

#define FFT_M 128
uint8_t fft_data[FFT_M];
int8_t iq8_data[512];
uint8_t iq8_idx;

static int force_scan;
static int gain_select;
static int _lna_gain;
static int _mixer_gain;
static int _vga_gain;
static int is_vga_auto;

void do_edit( metainfo *m );
uint8_t tg_zones[16][7];
void draw_tg_zones( void );
void tg_toggle_select( void );
int get_sel_zone( void );

volatile uint8_t buf[1500];
metainfo minfo;
metainfo minfo_verified;
metainfo minfo_copy;
static int clean_wio_line2;

metainfo *mptr;

extern uint32_t power_button_press_time;
extern uint32_t b_button_press_time;
extern uint32_t c_button_press_time;
static int did_save;
static int did_edit;
int current_button_mode = -1;
static int prev_button_mode = -1;

volatile uint32_t rx_count;
volatile int spi_state;
volatile int do_draw_rx;
volatile int buf_idx;

static double freq_val;

volatile uint16_t meta_port;
volatile uint16_t meta_len;
volatile uint32_t sclk_count;
/////////////////////////////////////////////////////
//////////////////////////////////////////////////TG LOG////////////////
bool TGLogScreen = true; //<< set true for default
char TGlog1[35];
char TGlog2[35];
char TGlog3[35];
char TGlog4[35];
char TGlog5[35];
char TGlog6[35];
char TGlog7[35];
char TGlog8[35];
char tglog_buf[35];

char ToneAlias[25];
///////////////////////////////////////////////////////////////////////////////
//

void reset_info( void );
void change_freq( int freq, int is_inc );
int is_valid_freq( double freq );
char disp_buf[256];
char disp_buf2[256];
char line0_str[64];
char line1_str[64];
char line2_str[64];
char line3_str[64];
char line4_str[64];
char line5_str[64];
char line6_str[64];
char line7_str[64];
char line8_str[64];
double current_freq;
double next_freq;
static int freq_idx;
static uint32_t status_timer;

char rid_alias[64];   // <<<<<<<<<<<<<<<< for rid on line 2


static uint32_t init_wdog1; //const
static uint32_t init_wdog2; //sysinfo
static double freq_table[8];


static int FNT = 4;

static uint8_t demod;
static uint8_t inc_in_scan;
static uint8_t learn;
int32_t follow;
static int32_t mute;
uint16_t tgzone;

static double prev_freq;
static int32_t prev_tgs;
static int freq_changed;

void tgz_inc_x();
void tgz_dec_x();
void tgz_inc_y();
void tgz_dec_y();

static int scap_count;
static int did_sd_init;


#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char *sbrk( int incr );
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

static int freeMemory()
{
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char *>( sbrk( 0 ) );
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}


#define DO_SCREENCAPS 1
int gen_screencaps = 0;
////////////////////////////////////////////////////////////////////////////////////
// when DO_SCREENCAPS==1, then generate screencaps throughout the menu system
//
// easy way to dump the LCD to the SD card, each subsequent call
//
// creates a new screencap file with .001, .002, ...  suffix
//
// The format is RGB 8-bit. Use ImageMagick convert to convert the files to png
//
// e.g. convert -depth 8 -size 320x240+0 rgb:screencap.1 screencap1.png
//
////////////////////////////////////////////////////////////////////////////////////
void do_screencap()
{

#ifdef DO_SCREENCAPS==1
  File scap_file;
  uint8_t screencap_data[3 * 320 * 30];

  char filename[64];

  sprintf( filename, "/screencap.%d", ++scap_count );


  if( !did_sd_init && !SD.begin( SDCARD_SS_PIN, SDCARD_SPI ) ) {
    return;
  }
  did_sd_init = 1;

  if( SD.exists( filename ) ) {
    SD.remove( filename );
  }

  scap_file = SD.open( filename, FILE_WRITE );
  if( scap_file == NULL ) return;

  //do 30 lines at a time so we don't take up too much ram
  tft.readRectRGB( 0, 0, 320, 30, screencap_data );
  scap_file.write( screencap_data, 320 * 30 * 3 );
  tft.readRectRGB( 0, 30, 320, 30, screencap_data );
  scap_file.write( screencap_data, 320 * 30 * 3 );
  tft.readRectRGB( 0, 60, 320, 30, screencap_data );
  scap_file.write( screencap_data, 320 * 30 * 3 );
  tft.readRectRGB( 0, 90, 320, 30, screencap_data );
  scap_file.write( screencap_data, 320 * 30 * 3 );
  tft.readRectRGB( 0, 120, 320, 30, screencap_data );
  scap_file.write( screencap_data, 320 * 30 * 3 );
  tft.readRectRGB( 0, 150, 320, 30, screencap_data );
  scap_file.write( screencap_data, 320 * 30 * 3 );
  tft.readRectRGB( 0, 180, 320, 30, screencap_data );
  scap_file.write( screencap_data, 320 * 30 * 3 );
  tft.readRectRGB( 0, 210, 320, 30, screencap_data );
  scap_file.write( screencap_data, 320 * 30 * 3 );

  scap_file.close();
#endif
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void init_sprites()
{
  spr = TFT_eSprite( &tft );
  spr.setColorDepth( 4 );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clr_screen()
{
  tft.fillScreen( mptr->col_def_bg ); //background
  memset( line0_str, 0x00, 64 );
  memset( line1_str, 0x00, 64 );
  memset( line2_str, 0x00, 32 );
  memset( line3_str, 0x00, 32 );
  memset( line4_str, 0x00, 32 );
  memset( line5_str, 0x00, 32 );
  memset( line6_str, 0x00, 32 );
  memset( line7_str, 0x00, 32 );
  memset( line8_str, 0x00, 32 );
}

#if 0
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
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line0()
{
  tft.fillRect( 0, 0, 320, 15, mptr->line0_bg );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line1()
{
  tft.fillRect( 0, 15, 320, 23, mptr->col_def_bg ); // 'height' changed to reduce flicker
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line2()
{
  tft.fillRect( 0, 38, 320, 30, mptr->col_def_bg ); // 'y' changed to reduce flicker
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line3()
{
  tft.fillRect( 0, 60, 325, 40, mptr->col_def_bg ); // height 40 - width to 325
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line4()
{
  tft.fillRect( 0, 90, 320, 30, mptr->col_def_bg );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line5()
{
  tft.fillRect( 0, 120, 320, 30, mptr->col_def_bg );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line6()
{
  tft.fillRect( 0, 150, 320, 30, mptr->col_def_bg );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line7()
{
  tft.fillRect( 0, 180, 320, 30, mptr->col_def_bg );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line8()
{
  tft.fillRect( 0, 207, 235, 35, mptr->col_def_bg );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void setup()
{

  pinMode( WIO_BUZZER, OUTPUT ); // <<<<<play buzzer tone

  mptr = &minfo_verified;

  tft.begin();
  tft.setRotation( 3 );
  tft.fillScreen( mptr->col_def_bg ); //background
  tft.setFreeFont( FS18 );
  tft.fillScreen( mptr->col_def_bg ); //background


// brightness = EEPROM.read(0); //disabled eeprom
  brightness = 50; //hardcoded brightness startup value
  if( brightness < 5 ) brightness = 5;
  if( brightness > 100 ) brightness = 100;
  backLight.initialize();
  backLight.setBrightness( brightness );

  init_sprites();

  //joystick
  pinMode( WIO_5S_UP, INPUT_PULLUP );
  pinMode( WIO_5S_DOWN, INPUT_PULLUP );
  pinMode( WIO_5S_LEFT, INPUT_PULLUP );
  pinMode( WIO_5S_RIGHT, INPUT_PULLUP );
  pinMode( WIO_5S_PRESS, INPUT_PULLUP );

  //3-buttons
  pinMode( WIO_KEY_A, INPUT_PULLUP );
  pinMode( WIO_KEY_B, INPUT_PULLUP );
  pinMode( WIO_KEY_C, INPUT_PULLUP );


  tft.setFreeFont( NULL );
  tft.setTextColor( mptr->col_def_fg, mptr->col_def_bg );

  sprintf( disp_buf, "%s", "BlueTail    MicroP25RX" );
  FNT = 4;
  tft.drawString( disp_buf, 5, 10, FNT );

  //SPI.begin();
  initSPI();

  //we have to control this manually due to silicon bug?? in SAMD51
  pinMode( PIN_SPI_MISO, OUTPUT );
  digitalWrite( PIN_SPI_MISO, HIGH );

  pinMode( PIN_SPI_SS, INPUT_PULLUP );
  //interrupt handler for Wio -> to -> Teensy
  attachInterrupt( digitalPinToInterrupt( PIN_SPI_SS ), spi_miso_int, FALLING );

  delay( 500 ); //let button-inputs settle after power-up

  clr_buttons();
  did_save = 0;
  b_button_press_time = 0;
  c_button_press_time = 0;
  power_button_press_time = 0;

}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void draw_button_modes()
{
  FNT = 2;

  tft.setTextColor( mptr->line0_fg, mptr->line0_bg );

  if( strncmp( ( char * )line0_str, ( char * )disp_buf, 63 ) != 0 ) {
    // clear_line0();
    strncpy( line0_str, disp_buf, 63 );

    switch( current_button_mode ) {
    case    WIO_BUTTON_MODE_MONITOR :
      snprintf( disp_buf, 63,   "TG HOLD        AUDIO MUTE        TG SKIP " );
      tft.drawString( disp_buf, 5, 0, FNT );
      break;

    case    WIO_BUTTON_MODE_CONFIG :
      if( mptr->roaming == 5 ) {
        snprintf( disp_buf, 63, "INC SCAN     CONFIG MENU     EXC SCAN    " );
      } else {
        snprintf( disp_buf, 63, "LEARN ON/OFF CONFIG MENU   DEMOD MODE    " );
      }
      tft.drawString( disp_buf, 5, 0, FNT );
      break;

    case    WIO_BUTTON_MODE_TG_ZONE :
      snprintf( disp_buf, 63, "TOGGLE EN    EDIT Z-NAME   MONITOR MODE    " );
      tft.drawString( disp_buf, 5, 0, FNT );
      break;

    case    WIO_BUTTON_MODE_RF_GAIN :
      snprintf( disp_buf, 63, "TOGGLE SCAN  SAVE GAINS    DEMOD MODE     " );
      tft.drawString( disp_buf, 5, 0, FNT );
      break;
    }
  }


}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void loop()
{

  check_buttons();

  //POWER OFF
  if( press_but_pressed && ( millis() - power_button_press_time > 0 ) && press_but == 0xff ) {
    if( ( millis() - power_button_press_time > 2000 ) ) {
      send_cmd( "power_off", 9 ); //power down
      delay(100);
      send_cmd( "power_off", 9 ); //power down
      delay(100);
      send_cmd( "power_off", 9 ); //power down
    }
    else if( ( millis() - power_button_press_time > 500 ) ) {
      if( strncmp( ( char * )line1_str, ( char * )"POWER OFF", 63 ) != 0 ) clear_line1();
      strncpy( line1_str, "POWER OFF", 63 );
      tft.drawString( line1_str, 20, 20, FNT );
    }
  }
  if( press_but_pressed && press_but == 0x00 ) { //select button mode menu
    power_button_press_time=0;
  }

  //check for edit menu shortcut. must be in monitor mode
  if( current_button_mode == WIO_BUTTON_MODE_MONITOR || current_button_mode == WIO_BUTTON_MODE_CONFIG ) {
    //middle-top button held for more than 3 seconds?
    if( !did_edit && C_but_pressed && ( millis() - c_button_press_time > 1000 ) && C_but == 0xff ) {
      //do edit here

      memcpy( ( void * )&minfo_copy, ( void * )&minfo_verified, sizeof( metainfo ) );
      metainfo *m = &minfo_copy;
      do_edit( m );
      clr_screen();
      did_edit = 1;
    } else if( did_edit && C_but == 0x00 ) {
      did_edit = 0;
      c_button_press_time = 0;
      C_but_pressed = 0;
    }
  }


  //middle-top button held for more than 3 seconds?
  if( !did_save && B_but_pressed && ( millis() - b_button_press_time > 3000 ) && B_but == 0xff ) {
    send_cmd( "save", 4 ); //save config
    did_save = 1;
//   EEPROM.commit();  //save brightness // disabled with hardcoded brightness
  } else if( did_save && B_but == 0x00 ) {
    did_save = 0;
    b_button_press_time = 0;
    B_but_pressed = 0;
  }

  /////////////////////////////////////////////////////////////////////////
  // CONFIG MODE
  /////////////////////////////////////////////////////////////////////////
  if( current_button_mode == WIO_BUTTON_MODE_CONFIG ) {

    //pressed and released
    if( press_but_pressed && press_but == 0x00 ) { //select button mode menu
      press_but_pressed = 0;
      memcpy( ( void * )&minfo_copy, ( void * )&minfo_verified, sizeof( metainfo ) );
      metainfo *m = &minfo_copy;

      if( gen_screencaps ) do_screencap();

      int ret = handle_button_mode();
      if( ret >= 0 ) {
        current_button_mode = ret;
      } else {
        B_but = 0;
        B_but_pressed = 0;
        clr_screen();
      }
    }
    //pressed and released
    if( right_but_pressed && right_but == 0x00 ) {
      right_but_pressed = 0;
      send_cmd( "nfreq_force", 11 ); //next primary/active frequency
    }
    //pressed and released
    if( left_but_pressed && left_but == 0x00 ) {
      left_but_pressed = 0;
      send_cmd( "pfreq", 5 ); //previous primary/active frequency
    }
    //pressed and released
    if( up_but_pressed && up_but == 0x00 ) {
      up_but_pressed = 0;
      char cmd_str[32];
      sprintf( cmd_str, "nsfreq %05X %03X\r\n", mptr->wacn_id, mptr->sys_id ); //roam mode=3 needs wacn and sysid arguments
      int len = strlen( cmd_str );
      send_cmd( cmd_str, len ); //next primary/active frequency
    }
    //pressed and released
    if( down_but_pressed && down_but == 0x00 ) {
      down_but_pressed = 0;
      char cmd_str[32];
      sprintf( cmd_str, "psfreq %05X %03X\r\n", mptr->wacn_id, mptr->sys_id ); //roam mode=3 needs wacn and sysid arguments
      int len = strlen( cmd_str );
      send_cmd( cmd_str, len ); //next primary/active frequency
    }


    //pressed, released
    //left-most button
    if( C_but_pressed && C_but == 0x00 ) {
      C_but_pressed = 0;
      c_button_press_time = 0;

      char cmd[32];
      if( mptr->roaming == 5 ) {
        snprintf( cmd, 31, "inc_scan %05X %03X %u %u 1\r\n", mptr->wacn_id, mptr->sys_id, mptr->site_id, mptr->rf_id );
        send_cmd( ( const char * ) cmd, strlen( cmd ) );
      } else {
        snprintf( cmd, 31, "learn %d\r\n", ( learn ^ 0x01 ) ); //learn mode on/off
        send_cmd( ( const char * ) cmd, strlen( cmd ) );
      }
    }
    //middle-most button
    if( B_but_pressed && B_but == 0x00 ) { //configuration menu
      B_but_pressed = 0;
      b_button_press_time = 0;


      memcpy( ( void * )&minfo_copy, ( void * )&minfo_verified, sizeof( metainfo ) );
      metainfo *m = &minfo_copy;
      handle_main_menu( m );

      clr_screen();
    }
    //right-most button
    if( A_but_pressed && A_but == 0x00 ) { //demod mode LSM/FM
      A_but_pressed = 0;
      char cmd[32];
      if( mptr->roaming == 5 ) {
        snprintf( cmd, 31, "inc_scan %05X %03X %u %u 0\r\n", mptr->wacn_id, mptr->sys_id, mptr->site_id, mptr->rf_id );
        send_cmd( ( const char * ) cmd, strlen( cmd ) );
      } else {
        snprintf( cmd, 31, "demod %d\r\n", ( mptr->use_demod ^ 0x01 ) );
        send_cmd( ( const char * ) cmd, strlen( cmd ) );
      }
    }
  }
  /////////////////////////////////////////////////////////////////////////
  // TG ZONE MODE
  /////////////////////////////////////////////////////////////////////////
  else if( current_button_mode == WIO_BUTTON_MODE_TG_ZONE ) {
    //pressed and released
    if( press_but_pressed && press_but == 0x00 ) { //select button mode menu
      press_but_pressed = 0;

      if( gen_screencaps ) do_screencap();

      int ret = handle_button_mode();
      if( ret >= 0 ) {
        current_button_mode = ret;
      } else {
        B_but = 0;
        B_but_pressed = 0;
        clr_screen();
      }
    }
    //pressed and released
    if( right_but_pressed && right_but == 0x00 ) {
      right_but_pressed = 0;
      tgz_inc_x();
    }
    //pressed and released
    if( left_but_pressed && left_but == 0x00 ) {
      left_but_pressed = 0;
      tgz_dec_x();
    }
    //pressed and released
    if( up_but_pressed && up_but == 0x00 ) {
      up_but_pressed = 0;
      tgz_dec_y();
    }
    //pressed and released
    if( down_but_pressed && down_but == 0x00 ) {
      down_but_pressed = 0;
      tgz_inc_y();
    }
    if( A_but_pressed && A_but == 0x00 ) {
      A_but_pressed = 0;
      char cmd[64];
      snprintf( cmd, 63, "wio_but_mode 0\r\n" );
      send_cmd( cmd, 15 );
    }
    if( C_but_pressed && C_but == 0x00 ) {
      C_but_pressed = 0;
      tg_toggle_select();
    }
    if( B_but_pressed && B_but == 0x00 ) {
      B_but_pressed = 0;
      int zone = get_sel_zone();

      tft.setTextColor( mptr->col_def_fg, mptr->col_def_bg );
      clr_screen();
      FNT = 4;
      tft.drawString( "Zone Name", 5, 10, FNT );
      mykey.set_flag( 2 ); //start with alphanum
      mykey.set_cur( 28 ); //start key
      draw_keybord( mykey, 0, 110, 320, 120, 32, 1 );
      zone &= 0x0f;

      char zone_name[8];
      zone_name[7] = 0;
      memcpy( zone_name, &tg_zones[zone][0], 7 );
      String zone_str = text_input_5waySwitch( mykey, 35, 60, String( ( char * ) zone_name ) );

      zone_str.replace( ' ', '_' );

      memcpy( zone_name, zone_str.c_str(), 7 );

      char cmd[32];
      snprintf( cmd, 31, "zonename %u %s\r\n", zone, zone_name );
      send_cmd( ( const char * ) cmd, strlen( cmd ) );
    }
  }
  /////////////////////////////////////////////////////////////////////////
  // MONITOR MODE
  /////////////////////////////////////////////////////////////////////////
  else if( current_button_mode == WIO_BUTTON_MODE_MONITOR ) {

    if( up_but_pressed && up_but == 0x00 ) {
      up_but_pressed = 0; // Joystick Up
      brightness += 5; // step up by 5 (range 5 to 100)
      if( brightness >= maxBrightness ) brightness = 5; // At Max cycle back to 5
      backLight.setBrightness( brightness );
      //   EEPROM.write(0,brightness); // disabled with hardcoded brightness
    } //change backlight
    if( down_but_pressed && down_but == 0x00 ) {
      down_but_pressed = 0; // Joystick Down
      brightness -= 5; // step down by 5 (range 5 to 100)
      if( brightness >= maxBrightness ) brightness = 5; // low set at 5
      if( brightness <= 0 ) brightness = maxBrightness; // At 0 cycle back to Max
      backLight.setBrightness( brightness );
      // EEPROM.write(0,brightness);// disabled with hardcoded brightness
    } //change backlight

    //pressed and released
    if( press_but_pressed && press_but == 0x00 ) { //select button mode menu
      press_but_pressed = 0;
      memcpy( ( void * )&minfo_copy, ( void * )&minfo_verified, sizeof( metainfo ) );
      metainfo *m = &minfo_copy;

      if( gen_screencaps ) do_screencap();

      int ret = handle_button_mode();
      if( ret >= 0 ) {
        current_button_mode = ret;
      } else {
        B_but = 0;
        B_but_pressed = 0;
        clr_screen();
      }
    }
    //pressed and released
    if( right_but_pressed && right_but == 0x00 ) {
      right_but_pressed = 0;
      send_cmd( "nfreq_force", 11 ); //next primary/active frequency
    }
    //pressed and released
    if( left_but_pressed && left_but == 0x00 ) {
      left_but_pressed = 0;
      send_cmd( "pfreq", 5 ); //previous primary/active frequency
    }
    //pressed, released
    //left-most button
    if( C_but_pressed && C_but == 0x00 ) { //TG HOLD
      C_but_pressed = 0;
      c_button_press_time = 0;
      char cmd[32];
      if( follow == 0 ) {
        snprintf( cmd, 31, "f %d\r\n", prev_tgs );
        send_cmd( ( const char * ) cmd, strlen( cmd ) );
      } else {
        snprintf( cmd, 31, "f\r\n" );
        send_cmd( ( const char * ) cmd, strlen( cmd ) );
      }
    }
    //middle-most button
    if( B_but_pressed && B_but == 0x00 ) { //MUTE
      B_but_pressed = 0;
      b_button_press_time = 0;

      char cmd[32];
      snprintf( cmd, 31, "audio_mute %u\r\n", ( mute ^ 0x01 ) );
      send_cmd( ( const char * ) cmd, strlen( cmd ) );
    }
    //right-most button
    if( A_but_pressed && A_but == 0x00 ) { //SKIP
      A_but_pressed = 0;

      char cmd[32];
      if( prev_tgs > 0 ) {
        snprintf( cmd, 31, "s %d\r\n", prev_tgs );
        send_cmd( ( const char * ) cmd, strlen( cmd ) );
      } else {
        snprintf( cmd, 31, "s\r\n", prev_tgs );
        send_cmd( ( const char * ) cmd, strlen( cmd ) );
      }
    }
  }
  /////////////////////////////////////////////////////////////////////////
  // RF GAIN MODE
  /////////////////////////////////////////////////////////////////////////
  else if( current_button_mode == WIO_BUTTON_MODE_RF_GAIN ) {
    //pressed and released
    if( press_but_pressed && press_but == 0x00 ) { //select button mode menu
      press_but_pressed = 0;
      memcpy( ( void * )&minfo_copy, ( void * )&minfo_verified, sizeof( metainfo ) );
      metainfo *m = &minfo_copy;

      if( gen_screencaps ) do_screencap();

      int ret = handle_button_mode();
      if( ret >= 0 ) {
        current_button_mode = ret;
      } else {
        B_but = 0;
        B_but_pressed = 0;
        clr_screen();
      }
    }

    //pressed and released
    if( up_but_pressed && up_but == 0x00 ) {
      up_but_pressed = 0;
      gain_select--;
      if( gain_select < 0 ) gain_select = 2;
    }
    //pressed and released
    if( down_but_pressed && down_but == 0x00 ) {
      down_but_pressed = 0;
      gain_select++;
      if( gain_select > 2 ) gain_select = 0;
    }


    //pressed and released
    //if (right_but_pressed && right_but == 0x00) {
    if( right_but_pressed ) {
      right_but_pressed = 0;

      char cmd[32];
      if( gain_select == 0 ) snprintf( cmd, 31, "lna_gain up\r\n" );
      if( gain_select == 1 ) snprintf( cmd, 31, "mix_gain up\r\n" );
      if( gain_select == 2 ) snprintf( cmd, 31, "vga_gain up\r\n" );
      send_cmd( ( const char * ) cmd, strlen( cmd ) );

      delay( 25 );
    }
    //pressed and released
    //if (left_but_pressed && left_but == 0x00) {
    if( left_but_pressed ) {
      left_but_pressed = 0;

      char cmd[32];
      if( gain_select == 0 ) snprintf( cmd, 31, "lna_gain down\r\n" );
      if( gain_select == 1 ) snprintf( cmd, 31, "mix_gain down\r\n" );
      if( gain_select == 2 ) snprintf( cmd, 31, "vga_gain down\r\n" );
      send_cmd( ( const char * ) cmd, strlen( cmd ) );

      delay( 25 );
    }
    //pressed, released
    //left-most button
    if( C_but_pressed && C_but == 0x00 ) { //lna_gain
      C_but_pressed = 0;
      c_button_press_time = 0;

      FNT = 4;
      force_scan ^= 0x01;
      if( force_scan ) sprintf( disp_buf, "SCANNING ENABLE    " );
      else sprintf( disp_buf, "SCANNING DISABLE    " );
      tft.drawString( disp_buf, 5, 210, FNT );
      delay( 250 );
    }
    //middle-most button
    if( B_but_pressed && B_but == 0x00 ) { //mix gain
      B_but_pressed = 0;
      b_button_press_time = 0;

      char cmd_str[32];

      is_vga_auto = ( ( mptr->vga_gain & 0x80 ) != 0 );
      if( is_vga_auto ) {
        sprintf( cmd_str, "update_gains %05X %03X %03u %03u %u %u %u\r\n", mptr->wacn_id, mptr->sys_id, mptr->site_id, mptr->rf_id, _lna_gain, _mixer_gain, _vga_gain );

      } else {
        sprintf( cmd_str, "update_gains %05X %03X %03u %03u %u %u %u\r\n", mptr->wacn_id, mptr->sys_id, mptr->site_id, mptr->rf_id, _lna_gain, _mixer_gain, _vga_gain );
      }

      FNT = 4;
      tft.setFreeFont( NULL );
      tft.setTextColor( mptr->col_def_fg, mptr->col_def_bg );


      int len = strlen( cmd_str );
      send_cmd( cmd_str, len ); //next primary/active frequency

      sprintf( disp_buf, "CONFIGURATION SAVED           " );
      tft.drawString( disp_buf, 5, 210, FNT );
      delay( 250 );

    }
    //right-most button
    if( A_but_pressed && A_but == 0x00 ) { //vga gain
      A_but_pressed = 0;
      char cmd[32];
      snprintf( cmd, 31, "demod %d\r\n", ( mptr->use_demod ^ 0x01 ) );
      send_cmd( ( const char * ) cmd, strlen( cmd ) );
    }
  }

  ///////////////////////////////////////////////////////////////////////
  // We received a new metainfo structure. Time to redraw the screen
  ///////////////////////////////////////////////////////////////////////
  if( do_draw_rx ) {

    __disable_irq();
    do_draw_rx = 0;
    memcpy( ( uint8_t * ) &minfo, ( uint8_t * ) buf, sizeof( metainfo ) );
    __enable_irq();

    metainfo *mptr = ( metainfo * ) &minfo;
    crc32_val = ~0L;


    uint32_t mi_crc = crc32_range( ( unsigned char * ) &minfo, sizeof( metainfo ) - 8 );

    //validate the structure with 32-bit crc
    if( mptr->crc_val == mi_crc && mptr->port == 8893 ) { //8893=metainfo structure
      rx_count++;

      tgzone = mptr->tgzone;

      if( mptr->data_type == META_DATA_TYPE_ZONEINFO ) {
        memcpy( &tg_zones[0][0], &mptr->data[0], 112 ); //tg zones 7 x 16
      }
      if( mptr->data_type == META_DATA_TYPE_META_CONFIG_INFO ) {
        memcpy( ( char * ) &minfo_config, ( char * ) &mptr->data[0], sizeof( meta_config_info ) );
      }

      memcpy( ( void * )&minfo_verified, ( void * )&minfo, sizeof( metainfo ) );

      demod = mptr->use_demod;
      follow = mptr->follow;
      mute = mptr->audio_mute;
      learn = mptr->learn_mode;
      inc_in_scan = mptr->inc_in_scan;


      extern uint8_t packet_id;
      if( packet_id == mptr->wio_packet_id ) {
        //the teensy digital board received the last command
        //sent with send_cmd( *str, int len)

        __disable_irq();
        cmd_acked = 0;
        clr_cmd(); //clear the command buffer
        __enable_irq();
      }



      if( current_button_mode != WIO_BUTTON_MODE_RF_GAIN ) {
        force_scan = 0;
      }


      //////////////////////////////////////////////////////////////////////////////////////////////
      // SCANNING/ROAMING CONTROL via the 'nfreq' command.
      // only send nfreq command if the following:
      // - current button mode is WIO_BUTTON_MODE_MONITOR
      // - TG HOLD / follow is not active
      // - pending commands have been acked
      // - roaming is enabled
      // - voice conversation timeout has expired
      // - time since last channel change is >250ms
      //////////////////////////////////////////////////////////////////////////////////////////////
      //ROAMING MODE 1
      if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 &&
          mptr->roaming == 1 && mptr->voice_tg_timeout == 0 && ( millis() - roam_time > mptr->roaming_timeout ) ) { //ROAMING=1 // scanning

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      }
      //ROAMING MODE 2
      else if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 && ( millis() - roam_time > mptr->roaming_timeout ) &&
               mptr->roaming == 2 && ( mptr->evm_p > 10 || mptr->rssi_f < -115 ) ) { //ROAMING=2 // auto-switch-over-on-lost-sig, P+S ALL systems

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      }
      //ROAMING MODE 3
      else if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 && ( millis() - roam_time > mptr->roaming_timeout ) &&
               mptr->roaming == 3 && ( mptr->evm_p > 10 || mptr->rssi_f < -115 ) ) { //ROAMING=3 // P+S auto-switch-over-on-lost-sig, SINGLE SYSTEM

        roam_time = millis();
        char cmd_str[32];
        sprintf( cmd_str, "nfreq %05X %03X\r\n", mptr->roam_wacn, mptr->roam_sysid ); //roam mode=3 needs wacn and sysid arguments
        send_cmd( cmd_str, 15 );
      }
      //ROAMING MODE 4
      else if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 && ( millis() - roam_time > mptr->roaming_timeout ) &&
               mptr->roaming == 4 && ( mptr->evm_p > 10 || mptr->rssi_f < -115 ) ) { //ROAMING=4 // auto-switch-over-on-lost-sig, P+S+A ALL systems

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      }
      //ROAMING MODE 5
      else if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 &&
               mptr->roaming == 5 && mptr->voice_tg_timeout == 0 && ( millis() - roam_time > mptr->roaming_timeout ) ) { //ROAMING=5 // scanning, INC_IN_SCAN=1

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      } else if( mptr->voice_tg_timeout ) {
        roam_time = millis(); //hang around ater the conversation too
      }



      //check if the button / display mode changed
      current_button_mode = mptr->wio_button_mode;

      if( current_button_mode != prev_button_mode ) {
        clr_screen();
        clr_buttons();

        did_save = 0;
        b_button_press_time = 0;
        c_button_press_time = 0;
      }

      prev_button_mode = current_button_mode;




      if( current_button_mode == WIO_BUTTON_MODE_TG_ZONE ) {
        __disable_irq();
        do_draw_rx = 0;
        memcpy( ( uint8_t * ) &minfo, ( uint8_t * ) buf, sizeof( metainfo ) );
        __enable_irq();

        draw_button_modes();
        draw_tg_zones();
        goto draw_end;  //it really is ok to use goto. don't worry about it.
      } else if( current_button_mode == WIO_BUTTON_MODE_RF_GAIN ) {

        int y_offset = 15;

        draw_button_modes();

        __disable_irq();
        do_draw_rx = 0;
        memcpy( ( uint8_t * ) fft_data, ( uint8_t * ) mptr->data, FFT_M );
        __enable_irq();

        if( mptr->data_type == META_DATA_TYPE_FFT ) {
          //draw_fft();
          init_sprites(); //best to re-init
          spr.createSprite( 128, 95 ); //allocate sprite memory
          spr.fillSprite( mptr->col_def_bg ); //clear to black bground

          spr.fillSprite( mptr->col_def_bg );
          for( int i = 0; i < 128; i++ ) {
            if( i > 0 ) {
              spr.drawLine( i, ( 110 - ( int )fft_data[i] ) + 90, i + 1, ( 110 - ( int )fft_data[i + 1] ) + 90, TFT_GREEN );
            }
          }
          spr.pushSprite( 5, 90 + y_offset ); //send to lcd. upper left corner of sprite
          spr.deleteSprite();  //free memory
        }
#if 1
        //////////////////////////////////////////////////////////
        //Draw IQ Constellation
        //////////////////////////////////////////////////////////
        int idx = 0;
        int ii;
        int qq;
        int8_t *ptr;

        if( mptr->data_type == META_DATA_TYPE_CONST ) { //incoming data is symbol constellation?

          ptr = ( int8_t * ) &mptr->data[0]; //pointer to incoming data 128 bytes
          spr.createSprite( 80, 80 ); //allocate memory for 80 x 80 sprite
          spr.fillSprite( mptr->col_def_bg );
          //grid lines
          //   spr.drawLine( 40, 0, 40, 80, TFT_DARKGREY ); //remove axis lines
          //   spr.drawLine( 0, 40, 80, 40, TFT_DARKGREY ); // remove axis lines
          //draw 64-symbol constellation
          idx = 0;
          for( int i = 0; i < 64; i++ ) {
            ii = *ptr++ / 2; //scale to +/- 32 range
            qq = *ptr++ / 2;

            if( mptr->draw_const_circles) {
              spr.fillCircle( 40 + ii, 40 + qq, 2, mptr->col_def_const ); //symbols
            }
            else {
              spr.drawPixel( 40 + ii, 40 + qq, mptr->col_def_const ); // use Pixel rather than Circle
            }
          }
          spr.pushSprite( 220, 115 + y_offset ); //send to lcd. upper left corner of sprite
          spr.deleteSprite();  //free memory
        }
#endif

#if 1
        //////////////////////////////////////////////////////////
        //Draw IQ / XY
        //////////////////////////////////////////////////////////

        if( mptr->data_type == META_DATA_TYPE_IQ8 ) { //incoming data is symbol constellation?

          iq8_idx++;
          iq8_idx &= 0x03;

          __disable_irq();
          do_draw_rx = 0;
          //if(iq8_idx==0) {
          memcpy( ( uint8_t * ) &iq8_data[0], ( uint8_t * ) mptr->data, FFT_M );
          //}
#if 0
          else if( iq8_idx == 1 ) {
            memcpy( ( uint8_t * ) &iq8_data[128], ( uint8_t * ) mptr->data, FFT_M );
          } else if( iq8_idx == 2 ) {
            memcpy( ( uint8_t * ) &iq8_data[256], ( uint8_t * ) mptr->data, FFT_M );
          } else if( iq8_idx == 3 ) {
            memcpy( ( uint8_t * ) &iq8_data[384], ( uint8_t * ) mptr->data, FFT_M );
          }
#endif
          __enable_irq();

          //if(iq8_idx==3) {
          spr.createSprite( 80, 80 ); //allocate memory for 80 x 80 sprite
          spr.fillSprite( TFT_BLACK );

          for( int i = 0; i < 128 - 2; i += 2 ) {
            spr.drawLine( 40 + iq8_data[i], 40 + iq8_data[i + 1], 40 + iq8_data[i + 2], 40 + iq8_data[i + 3], TFT_CYAN );
          }

          spr.pushSprite( 30, 8 + y_offset ); //send to lcd. upper left corner of sprite
          //}
          spr.deleteSprite();  //free memory
        }
#endif

        FNT = 2;
        spr.createSprite( 170, 20 ); //allocate memory for 80 x 80 sprite
        spr.fillSprite( TFT_BLACK );
        _lna_gain = mptr->lna_gain;
        if( _lna_gain < 0 ) _lna_gain = 0;
        if( _lna_gain > 16 ) _lna_gain = 16;

        if( _lna_gain < 16 ) {
          spr.setTextColor( TFT_WHITE, TFT_BLACK );
          if( gain_select == 0 ) sprintf( disp_buf, "> %02d LNA", _lna_gain );
          else sprintf( disp_buf, "%02d LNA", _lna_gain );
          spr.drawString( disp_buf, 0, 0, FNT );
          spr.fillRect( 60, 3, 7 * _lna_gain, 7, TFT_WHITE );
        } else {
          spr.setTextColor( TFT_GREEN, TFT_BLACK );
          if( gain_select == 0 ) sprintf( disp_buf, "> LNA AUTO" );
          else sprintf( disp_buf, "LNA AUTO" );
          spr.drawString( disp_buf, 0, 0, FNT );
        }
        spr.pushSprite( 140, 50 + y_offset ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory

        spr.createSprite( 170, 20 ); //allocate memory for 80 x 80 sprite
        _mixer_gain = mptr->mixer_gain;
        if( _mixer_gain < 0 ) _mixer_gain = 0;
        if( _mixer_gain > 16 ) _mixer_gain = 16;
        if( _mixer_gain < 16 ) {
          spr.setTextColor( TFT_WHITE, TFT_BLACK );
          if( gain_select == 1 ) sprintf( disp_buf, "> %02d MIX", _mixer_gain );
          else sprintf( disp_buf, "%02d MIX", _mixer_gain );
          spr.drawString( disp_buf, 0, 0, FNT );
          spr.fillRect( 60, 3, 7 * _mixer_gain, 7, TFT_WHITE );
        } else {
          spr.setTextColor( TFT_GREEN, TFT_BLACK );
          if( gain_select == 1 ) sprintf( disp_buf, "> MIX AUTO" );
          else sprintf( disp_buf, "MIX AUTO" );
          spr.drawString( disp_buf, 0, 0, FNT );
        }
        spr.pushSprite( 140, 70 + y_offset ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory

        spr.createSprite( 170, 20 ); //allocate memory for 80 x 80 sprite
        _vga_gain = mptr->vga_gain;
        if( _vga_gain < 0 ) _vga_gain = 0;

        is_vga_auto = ( ( _vga_gain & 0x80 ) != 0 );
        _vga_gain &= 0x0f;

        if( !is_vga_auto ) {
          spr.setTextColor( TFT_WHITE, TFT_BLACK );
          if( gain_select == 2 ) sprintf( disp_buf, "> %02d VGA", _vga_gain );
          else sprintf( disp_buf, "%02d VGA", _vga_gain );
          spr.drawString( disp_buf, 0, 0, FNT );
          spr.fillRect( 60, 3, 7 * _vga_gain, 7, TFT_WHITE );
        } else {
          spr.setTextColor( TFT_GREEN, TFT_BLACK );
          //if(gain_select==2) sprintf(disp_buf,"> VGA AUTO %d dB", _vga_gain*3);
          //else sprintf(disp_buf,"VGA AUTO %d dB", _vga_gain*3);
          if( gain_select == 2 ) sprintf( disp_buf, "> VGA AUTO %d  ", _vga_gain );
          else sprintf( disp_buf, "VGA AUTO %d  ", _vga_gain );
          spr.drawString( disp_buf, 0, 0, FNT );
        }
        spr.pushSprite( 140, 90 + y_offset ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory



#if 0
        spr.createSprite( 90, 20 ); //allocate memory for 80 x 80 sprite
        if( gain_select == 3 ) sprintf( disp_buf, "> SAVE GAINS" );
        else sprintf( disp_buf, "SAVE GAINS" );
        spr.drawString( disp_buf, 0, 0, FNT );
        spr.pushSprite( 140, 110 + y_offset ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory
#endif

        FNT = 2;
        tft.setTextColor( TFT_LIGHTGREY, TFT_BLACK );
        char demod_str[8];
        demod_str[0] = 0;
        if( mptr->use_demod == 0 ) strcpy( demod_str, "LSM" );
        if( mptr->use_demod == 1 ) strcpy( demod_str, "FM" );

        sprintf( disp_buf, "DEMOD %s   ", demod_str );
        tft.drawString( disp_buf, 140, 110 + y_offset, FNT );


        FNT = 4;
        tft.setFreeFont( NULL );
        tft.setTextColor( TFT_WHITE, TFT_BLACK );

        sprintf( disp_buf, "EVM %3.1f  RSSI %3.1f dBm    ", mptr->evm_p, mptr->rssi_f );
        tft.drawString( disp_buf, 5, 210, FNT );

        FNT = 2;
        snprintf( disp_buf, 32, "FREQ: %3.6f MHz", mptr->current_freq );
        tft.drawString( disp_buf, 140, 0 + y_offset, FNT );

        if( mptr->on_control_b && mptr->total_session_time > 1500 ) {
          sprintf( disp_buf, "TSBK/SEC %u      ", mptr->tsbk_sec );
        } else {
          sprintf( disp_buf, "ERATE %1.3f      ", mptr->erate );
        }
        tft.drawString( disp_buf, 140, 15 + y_offset, FNT );

        sprintf( disp_buf, "SITE %d, RFSS %d     ", mptr->site_id, mptr->rf_id );
        tft.drawString( disp_buf, 140, 30 + y_offset, FNT );

        goto draw_end;  //it really is ok to use goto. don't worry about it.
      }


      //FONT SIZE 1,2,4,8
      FNT = 4;

      tft.setFreeFont( NULL );
      tft.setTextColor( mptr->col_def_fg, mptr->col_def_bg );

      if( prev_freq != mptr->current_freq ) {
        freq_changed = 1;

      }
      prev_freq = mptr->current_freq;


      tft.setTextColor( mptr->col2, mptr->col_def_bg );

      //if( !mptr->do_wio_lines ) {
      if( mptr->tg_s != prev_tgs ) {
        //clear_line2();
      }
      prev_tgs = mptr->tg_s;


      if( mptr->tg_s <= 0 ) {
        snprintf( disp_buf2, 32, " %3.6f MHz", mptr->current_freq );   // changes for rid on line 2 - remove "FREQ:" text

        if( strncmp( ( char * )disp_buf2, ( char * )line3_str, 31 ) != 0 ) clear_line3();
        strncpy( line3_str, disp_buf2, 31 );

        if( !mptr->do_wio_lines && follow > 0 ) {
          snprintf( disp_buf, 32, "HOLDING ON TG %d", follow );
          if( strncmp( ( char * )disp_buf, ( char * )line2_str, 31 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 31 );
          tft.drawString( disp_buf, 5, 40, FNT );
        }
        if( !mptr->do_wio_lines && follow == 0 ) {
          snprintf( disp_buf, 32, "   " );
          mptr->desc[19] = 0;
          if( strncmp( ( char * )disp_buf, ( char * )line2_str, 31 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 31 );
        }
      } else {

        if( !mptr->do_wio_lines ) {
          snprintf( disp_buf, 32, "TG: %d, %s", mptr->tg_s, mptr->alpha );
          mptr->desc[19] = 0;
          //   if( strncmp( ( char * )disp_buf, ( char * )line2_str, 31 ) != 0 ) clear_line2();   // changes for rid on line 2
          //   strncpy( line2_str, disp_buf, 31 );
          //   tft.drawString( disp_buf, 5, 40, FNT );
        }

        snprintf( disp_buf2, 24, "%s", mptr->desc );
        if( strncmp( ( char * )disp_buf2, ( char * )line3_str, 31 ) != 0 )

        {
          clear_line3();


          snprintf( tglog_buf, 32, "%s  %d  ", disp_buf2, mptr->tg_s ); // send desc and tg number to buf

          // move the TG log down
          strncpy( TGlog8, TGlog7, 32 );
          strncpy( TGlog7, TGlog6, 32 );
          strncpy( TGlog6, TGlog5, 32 );
          strncpy( TGlog5, TGlog4, 32 );
          strncpy( TGlog4, TGlog3, 32 );
          strncpy( TGlog3, TGlog2, 32 );
          strncpy( TGlog2, TGlog1, 32 );
          strncpy( TGlog1, tglog_buf, 32 );

          sprintf( tglog_buf, " " ); // clear buf


          if( TGLogScreen == true ) {
            tft.setTextColor( mptr->col5, mptr->col_def_bg ); //
            tft.fillRect( 0, 115, 240, 75,  mptr->col_def_bg ); //
            // Highlight FTOs in red in TG log by looking for ">>".
            if( strstr( TGlog1, ">>" ) ) {
              tft.setTextColor( ILI9341_RED, mptr->col_def_bg );
            } else tft.setTextColor( mptr->col5, mptr->col_def_bg );
            tft.drawString( TGlog1, 5, 117, 1 ); //
            if( strstr( TGlog2, ">>" ) ) {
              tft.setTextColor( ILI9341_RED, mptr->col_def_bg );
            } else tft.setTextColor( mptr->col5, mptr->col_def_bg );
            tft.drawString( TGlog2, 5, 126, 1 ); // 130
            if( strstr( TGlog3, ">>" ) ) {
              tft.setTextColor( ILI9341_RED, mptr->col_def_bg );
            } else tft.setTextColor( mptr->col5, mptr->col_def_bg );
            tft.drawString( TGlog3, 5, 135, 1 ); // 144
            if( strstr( TGlog4, ">>" ) ) {
              tft.setTextColor( ILI9341_RED, mptr->col_def_bg );
            } else tft.setTextColor( mptr->col5, mptr->col_def_bg );
            tft.drawString( TGlog4, 5, 144, 1 ); // 158
            if( strstr( TGlog5, ">>" ) ) {
              tft.setTextColor( ILI9341_RED, mptr->col_def_bg );
            } else tft.setTextColor( mptr->col5, mptr->col_def_bg );
            tft.drawString( TGlog5, 5, 153, 1 ); // 172
            if( strstr( TGlog6, ">>" ) ) {
              tft.setTextColor( ILI9341_RED, mptr->col_def_bg );
            } else tft.setTextColor( mptr->col5, mptr->col_def_bg );
            tft.drawString( TGlog6, 5, 162, 1 ); //
            if( strstr( TGlog7, ">>" ) ) {
              tft.setTextColor( ILI9341_RED, mptr->col_def_bg );
            } else tft.setTextColor( mptr->col5, mptr->col_def_bg );
            tft.drawString( TGlog7, 5, 171, 1 ); //
            if( strstr( TGlog8, ">>" ) ) {
              tft.setTextColor( ILI9341_RED, mptr->col_def_bg );
            } else tft.setTextColor( mptr->col5, mptr->col_def_bg );
            tft.drawString( TGlog8, 5, 180, 1 ); //
          } //

        } //


        strncpy( line3_str, disp_buf2, 31 );

      }
      if( mptr->tg_s <= 0 ) {
        tft.setTextColor( 0xF643, mptr->col_def_bg ); // set color to a yellow (0xF643) for on CCH freq
      } else {
        tft.setTextColor( mptr->col3, mptr->col_def_bg ); // set to line 3 color
      }

      tft.drawString( disp_buf2, 5, 70, FNT );


      //}

      FNT = 2;
      tft.setTextColor( mptr->col4, mptr->col_def_bg );

      uint8_t nac_lock_str = 'U';
      if( mptr->lock_nac ) nac_lock_str = 'L';

      if( mptr->on_control_b ) {
        tft.setTextColor( 0xF643, mptr->col_def_bg ); // set color to a yellow for on CCH freq

        sprintf( disp_buf, " %3.0fdBm CCH TSBK/SEC %u                    ", mptr->rssi_f, mptr->tsbk_sec ); // removed cal freq //

      } else {
        tft.setTextColor( mptr->col4, mptr->col_def_bg );

        sprintf( disp_buf, " %3.0fdBm TCH %3.6f MHz                     ", mptr->rssi_f, mptr->current_freq ); // removed erate

      }

      if( strcmp( disp_buf, line4_str ) != 0 ) {
        //clear_line4(); //space at the end of this line is better solution
        tft.drawString( disp_buf, 5, 99, FNT ); // move up from 100 to 99
      }
      strcpy( line4_str, disp_buf );

      FNT = 2;
      tft.setTextColor( mptr->col5, mptr->col_def_bg );
      char demod_str[8];
      demod_str[0] = 0;
      if( mptr->use_demod == 0 ) strcpy( demod_str, "LSM" );
      if( mptr->use_demod == 1 ) strcpy( demod_str, "FM" );

      sprintf( disp_buf, "SITE %d, RFSS %d  DEMOD %s", mptr->site_id, mptr->rf_id, demod_str );
      if( strcmp( disp_buf, line5_str ) != 0 ) {
        //   clear_line5();
        if( TGLogScreen != true ) {
          clear_line5();  // < tg log screen <<<<<<<<<
          tft.drawString( disp_buf, 5, 130, FNT );
        }
        // tft.drawString(disp_buf, 5, 130, FNT);
      }
      strcpy( line5_str, disp_buf );

      tft.setTextColor( mptr->col6, mptr->col_def_bg );
      sprintf( disp_buf, "NCO1 %3.3f, NCO2 %3.2f                  ", mptr->nco_offset, mptr->loop_freq ); //remove evm
      if( strcmp( disp_buf, line6_str ) != 0 ) {
        //clear_line6(); //space at the end of this line is better solution
        if( TGLogScreen != true ) {
          tft.drawString( disp_buf, 5, 160, FNT ); // < tg log screen <<<<<<<<<<<<<<<<
        }
        // tft.drawString(disp_buf, 5, 160, FNT);
      }
      strcpy( line6_str, disp_buf );

      tft.setTextColor( mptr->col7, mptr->col_def_bg );
      sprintf( disp_buf, " %s EV %3.1f ER %1.3f        ", demod_str, mptr->evm_p, mptr->erate ); // <<<<<<<< removed wacn plus added evm and erate here


      if( strcmp( disp_buf, line7_str ) != 0 ) {
        //clear_line7(); //space at the end of this line is better solution
        tft.drawString( disp_buf, 5, 190, FNT );
      }
      strcpy( line7_str, disp_buf );

      FNT = 2;
      memset( disp_buf, 0x00, sizeof( disp_buf ) );
      tft.setTextColor( mptr->col8, mptr->col_def_bg );
      sprintf( disp_buf, "NCO1 %3.3f, NCO2 %3.2f, EVM %3.1f    ", mptr->nco_offset, mptr->loop_freq, mptr->evm_p );
      if( mptr->on_control_b == 0 && mptr->RID != 0 && mptr->wio_line2[0] == 0 ) {


        sprintf( disp_buf, "%s", mptr->sys_name );

        sprintf( rid_alias, " %s %u", mptr->alias, mptr->RID );
        if( strcmp( rid_alias, line2_str ) != 0 ) {
          tft.setTextColor( mptr->col2, mptr->col_def_bg );

          if( strncmp( ( char * )rid_alias, ( char * )line2_str, 64 ) != 0 ) {
            clear_line2();
            tft.drawString( rid_alias, 5, 40, 4 );
            strncpy( line2_str, rid_alias, 64 );
          }

        }
        //sprintf( disp_buf, "                      " );
      } else {
        if( current_button_mode == WIO_BUTTON_MODE_CONFIG ) {
          draw_button_modes(); //<<< added
          if( mptr->roaming ) {
            sprintf( disp_buf, "CONFIG  ROAM-PAUSE" );
          } else {
            sprintf( disp_buf, "MODE: CONFIG          " );
          }
        } else if( current_button_mode == WIO_BUTTON_MODE_MONITOR ) {
          draw_button_modes(); //<<< added


          #if 0
          if( mptr->roaming ) {
            sprintf( disp_buf, "MONITOR ROAM-ON-%u Free %u", mptr->roaming, freeMemory() );
          } else {
            // sprintf( disp_buf, "MONITOR ROAM-OFF" );
            sprintf( disp_buf, "MONITOR - MEM %u         ", freeMemory() ); // <<<<<<<<<<<<
          }
          #else
            if( mptr->roaming ) {
              sprintf( disp_buf, "MONITOR ROAM-ON-%u BAT %1.2fv", mptr->roaming, mptr->bat_volt_f );
            } else {
              // sprintf( disp_buf, "MONITOR ROAM-OFF" );
              sprintf( disp_buf, "MONITOR - BAT %1.2fv        ", mptr->bat_volt_f ); // <<<<<<<<<<<<
            }
          #endif



        } else {
          sprintf( disp_buf, " " );
        }
      }
      if( strcmp( disp_buf, line8_str ) != 0 ) {
        tft.setTextColor( mptr->col8, mptr->col_def_bg );
        strcpy( line8_str, disp_buf );
        clear_line8();
        tft.drawString( disp_buf, 5, 220, 1 ); // FNT 1 from 2
      }

      //start of sprites
      init_sprites(); //best to re-init
#if 1
      //////////////////////////////////////////////////////////
      //Draw STATUS INDICATORS HOLD, MUTE,
      //////////////////////////////////////////////////////////
      spr.createSprite( 60, 30 ); //allocate sprite memory
      spr.fillSprite( mptr->col_def_bg ); //clear to black bground

      spr.setTextColor( mptr->col9, mptr->col10 );

      memset( disp_buf, 0x00, sizeof( disp_buf ) );
      if( follow || mute || inc_in_scan ) strcat( disp_buf, " " );

      if( follow > 0 ) strcat( disp_buf, "H " );
      if( mute > 0 ) strcat( disp_buf, "M " );
      if( inc_in_scan > 0 ) strcat( disp_buf, "S " );

      spr.drawString( disp_buf, 0, 0, FNT );


      spr.pushSprite( 265, 185 ); //transfer to lcd, x,y = 240,210
      spr.deleteSprite(); //free memory
#endif

#if 1
      //////////////////////////////////////////////////////////
      //Draw Status LEDS  / P1-P2 indicator
      //////////////////////////////////////////////////////////
      spr.createSprite( 80, 40 ); //allocate sprite memory
      spr.fillSprite( mptr->col_def_bg ); //clear to black bground

      spr.setTextColor( mptr->col_def_indicator, mptr->col_def_bg );
      if( mptr->phase2 ) {
        spr.drawString( "P2", 0, 8, FNT ); //p25 p2
      } else {
        spr.drawString( "P1", 0, 8, FNT ); //p25 p1
      }

      //status led
      if( mptr->status_led ) {
        spr.fillCircle( 35, 12, 10, mptr->col_def_led1_on );
      } else {
        spr.fillCircle( 35, 12, 10, mptr->col_def_led1_off );
      }

      //TG led
      if( mptr->tg_led ) {
        spr.fillCircle( 60, 12, 10, mptr->col_def_led2_on );
      } else {
        spr.fillCircle( 60, 12, 10, mptr->col_def_led2_off );
      }
      spr.pushSprite( 240, 212 ); //transfer to lcd, x,y = 240,210
      spr.deleteSprite(); //free memory
#endif

#if 1
      //////////////////////////////////////////////////////////
      //Draw IQ Constellation
      //////////////////////////////////////////////////////////
      int idx = 0;
      int ii;
      int qq;
      int8_t *ptr;

      if( mptr->data_type == META_DATA_TYPE_CONST ) { //incoming data is symbol constellation?

        ptr = ( int8_t * ) &mptr->data[0]; //pointer to incoming data 128 bytes
        spr.createSprite( 80, 80 ); //allocate memory for 80 x 80 sprite
        spr.fillSprite( TFT_BLACK );
        //grid lines
        //    spr.drawLine( 40, 0, 40, 80, TFT_DARKGREY );
        //    spr.drawLine( 0, 40, 80, 40, TFT_DARKGREY );
        //draw 64-symbol constellation
        idx = 0;
        for( int i = 0; i < 64; i++ ) {
          ii = *ptr++ / 2; //scale to +/- 32 range
          qq = *ptr++ / 2;

          if( mptr->draw_const_circles) {
            spr.fillCircle( 40 + ii, 40 + qq, 2, mptr->col_def_const ); //symbols
          }
          else {
            spr.drawPixel( 40 + ii, 40 + qq, mptr->col_def_const ); // use Pixel rather than Circle
          }
        }
        spr.pushSprite( 233, 98 ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory
      }
#endif
      draw_button_modes();

      tft.setTextColor( mptr->col1, mptr->col_def_bg );


      if( mptr->do_wio_lines ) {


        if( mptr->wio_line1[0] != 0 ) {
          snprintf( disp_buf, 25, "%s", mptr->wio_line1 );
          tonelookup(); //<<<<<<<<<<<<<<<<<<< TONE lookup for alias
          FNT = 2;

          if( strncmp( ( char * )line1_str, ( char * )disp_buf, 63 ) != 0 ) clear_line1();
          strncpy( line1_str, disp_buf, 63 );
          if( ( strstr( disp_buf, "TONE_A:" ) ) != NULL ) {
            tft.setTextColor( ILI9341_ORANGE, mptr->col_def_bg );
            tft.drawString( disp_buf, 9, 18, 4 );
          } else tft.drawString( disp_buf, 20, 20, FNT );
        } else {
          FNT = 2;
          mptr->sys_name[18] = 0;
          snprintf( disp_buf, 50, "%s" );

          if( strncmp( ( char * )line1_str, ( char * )disp_buf, 63 ) != 0 ) clear_line1();
          strncpy( line1_str, disp_buf, 63 );

          tft.drawString( disp_buf, 20, 20, FNT );
        }


        if( mptr->wio_line2[0] != 0 ) {
          FNT = 2;
          snprintf( disp_buf, 63,  "%s", mptr->wio_line2 );
          if( strncmp( ( char * )line2_str, ( char * )disp_buf, 63 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 63 );

          if( ( strstr( disp_buf, "FREQ_A:" ) ) != NULL ) { //do not display tone freq info wio line2 (tone lookup will display alias)
            tft.setTextColor( ILI9341_RED, mptr->col_def_bg ); // show line2 in RED during tone out
            tft.drawString( ToneAlias, 9, 44, 4 ); //
            strcpy( line2_str, ToneAlias ); // show tone alias
          }  else if( ( strstr( disp_buf, "FREQ_A:" ) ) != NULL ) {;} //do not show freq info
          else  tft.drawString( disp_buf, 5, 40, FNT );
        } else {
          FNT = 2;
          //mptr->sys_name[18]=0;
          //mptr->site_name[18]=0;
          //snprintf(disp_buf, 63,"%s / %s",mptr->sys_name,mptr->site_name);
          snprintf( disp_buf, 63, "   " );

          if( strncmp( ( char * )line2_str, ( char * )disp_buf, 63 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 63 );

          tft.drawString( disp_buf, 5, 40, FNT );
        }
      } else {
        FNT = 2;
        mptr->sys_name[18] = 0;
        mptr->site_name[18] = 0;

        //clear values
        if( freq_changed ) {
          freq_changed = 0; //keep this at the end within valid crc check
          two_tone_A = 0;
          two_tone_B = 0;
        }

#if 0
        int freq_idx = -1;

        float fm = -1.0f;
        //we wait for a value update before processing
        if( mptr->max_freq_cnt > 8 ) {
          //freq_idx = two_tone_get_idx((int) mptr->max_freq_hz);
          fm = mptr->max_freq_hz;

        }
#if 0
        if( mptr->max_freq_cnt == 0 ) {
          two_tone_A = 0;
          two_tone_B = 0;
        }
#endif

#if 1
        if( fm > 0 ) {  //valid frequency from table?
          if( two_tone_A == 0 ) {
            two_tone_A = ( int ) fm;
          } else if( two_tone_B == 0 ) {
            if( ( int ) fm != two_tone_A ) {
              two_tone_B = ( int ) fm;
            }
          }
        }

        if( two_tone_A > 0 || two_tone_B > 0 ) {
          if( two_tone_A && two_tone_B ) snprintf( disp_buf, 50, "TWO-TONE A:%u  B:%u", two_tone_A, two_tone_B )
            else snprintf( disp_buf, 50, "TWO-TONE A:%u", two_tone_A );
        } else {
          snprintf( disp_buf, 50, "%s / %s", mptr->sys_name, mptr->site_name );
        }
        if( strncmp( ( char * )line1_str, ( char * )disp_buf, 31 ) != 0 ) clear_line1();
        strncpy( line1_str, disp_buf, 31 );
        tft.drawString( disp_buf, 5, 10, FNT );
#else
        //if(freq_idx>0) {
        snprintf( disp_buf, 50, "Freq: %3.1f Hz, CNT: %u", fm, mptr->max_freq_cnt );
        if( strncmp( ( char * )line1_str, ( char * )disp_buf, 31 ) != 0 ) clear_line1();
        strncpy( line1_str, disp_buf, 31 );
        tft.drawString( disp_buf, 5, 10, FNT );
        //}
#endif
#else
        if( demod == 0 || demod == 1 ) {
          snprintf( disp_buf, 50, "%s / %s   %05X-%03X-%03X%c  ", mptr->sys_name, mptr->site_name, mptr->wacn_id, mptr->sys_id, mptr->p25_sys_nac, nac_lock_str ); // <<<< moved wacn to line 1

        } else if( demod == 2 ) {
          snprintf( disp_buf, 50, "FMNB     " );
        }
        if( strncmp( ( char * )line1_str, ( char * )disp_buf, 63 ) != 0 ) clear_line1();
        strncpy( line1_str, disp_buf, 63 );
        tft.drawString( disp_buf, 5, 20, FNT );
#endif
      }

    }

draw_end:
    status_timer = millis();

  }

  if( status_timer > 0 && millis() - status_timer > 1000 ) {
    status_timer = 0;

    __disable_irq();
    spi_state = 0;
    __enable_irq();

  }

}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void change_freq( int freq, int is_inc )
{

  clr_screen();

  int i;

  for( i = 0; i < 8; i++ ) { //try all 8 until we find a valid one
    freq_idx &= 0x07; //limit from 0-7
    double next_freq = freq_table[freq_idx];


    if( is_valid_freq( next_freq ) ) { //only send new frequency if it is in a valid ranage
      //give it a chance to catch up after a freq change
      init_wdog1 = millis(); //voice
      init_wdog2 = millis(); //const

      sprintf( disp_buf, "freq %3.6f\r\n", next_freq );
      tft.setFreeFont( FS18 );
      tft.setTextColor( TFT_WHITE, TFT_BLACK );

      //send frequency command


      memcpy( disp_buf, "FREQ", 4 ); //keep this after the command. P25RX wants lower case for the command.
      tft.drawString( disp_buf, 10, 10 );
      return;
    }

    if( is_inc ) freq_idx++; //last one wasn't valid, so try the next one
    else freq_idx--;
  }
}

//////////////////////////////////////////////////////////
#define precision 7
int test_doubles_equal( double d1, double d2 )
{
  return fabs( d1 - d2 ) < pow( 10, -precision );
}

////////////////////////////////////////////////////////////////////////////////
//p25 freq ranges
//25-512 MHz
//758-824 MHz
//849-869 MHz  <- support this one
//894-960 MHz
//1240-1300 MHz
////////////////////////////////////////////////////////////////////////////////
int is_valid_freq( double freq )
{

  int band = 0;

  if( freq >= 25.0 && freq <= 512.0 ) band = 1;  //band 1
  if( freq >= 758.0 && freq <= 824.0 ) band = 2;  //band 2
  if( freq >= 849.0 && freq <= 869.0 ) band = 3;  //band 3
  if( freq >= 894.0 && freq <= 960.0 ) band = 4;  //band 4
  if( freq >= 1240.0 && freq <= 1300.0 ) band = 5;  //band 5


  return band;
  //return 1;
}

void tonelookup()  // <<<<<<<< tone lookup
{
  tft.setTextColor( ILI9341_RED, mptr->col_def_bg );   // set line 1 to red
  strncpy( ToneAlias, " FTO ALIAS HERE  ", 23 );
//example of tone lookup lines
// if ((strstr(disp_buf,"TONE_A:64,  TONE_B:16"))) {strncpy( TGlog1, " >> FIRE DEPT NAME  <<   ", 33 );  tft.drawString( TGlog1, 5, 220, 2 ); strncpy(ToneAlias, "FIRE DEPT NAME  ",23);  playTone(400, 100);}

} //end tonelookup


void playTone( int tone, int duration )
{
  for( long i = 0; i < duration * 1000L; i += tone * 2 ) {
    digitalWrite( WIO_BUZZER, HIGH );
    delayMicroseconds( tone );
    digitalWrite( WIO_BUZZER, LOW );
    delayMicroseconds( tone );
  }
}
