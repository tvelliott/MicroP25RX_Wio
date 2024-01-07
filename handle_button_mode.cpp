
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



#include <Arduino.h>
#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include "list_menu.h"
#include "handle_menus.h"
#include "Free_Keybord.h"
#include "metainfo.h"

#include <FlashAsEEPROM.h>
#include "lcd_backlight.hpp"

extern TFT_eSPI tft;
extern int current_button_mode;
extern int gen_screencaps;
extern void do_screencap( void );

extern LCDBackLight backLight;
extern uint8_t brightness;


void clr_screen( void );
void send_cmd( const char *str, int len );
static int FNT = 4;
extern int do_refresh;
extern Keybord mykey; // Cleate a keybord

int handle_button_mode( void )
{
  int ret = -1;

  if( mptr->speaker_en == 0 ) {
    ret = get_menu_choice( 8, "MONITOR  MODE", "CONFIG  MODE", "TG  ZONE MODE", "SIGNALS / GAIN  MODE", "ENABLE SPEAKER", "GAIN CONTROLLER", "SET WIO BRIGHTNESS", "SET LAYOUT" );
  } else {
    ret = get_menu_choice( 8, "MONITOR  MODE", "CONFIG  MODE", "TG  ZONE MODE", "SIGNALS / GAIN  MODE", "DISABLE SPEAKER", "GAIN CONTROLLER", "SET WIO BRIGHTNESS", "SET LAYOUT" );
  }

  if( ret == -1 ) return -1;

  if( ret == 5 ) {
    int ret3 = get_menu_choice( 6, "GC OFF", "GC-1 >6dB (SENSITIVE)", "GC-2 >6dB (LINEAR)", "PEAK DETECTOR 1", "PEAK DETECTOR 2", "PEAK DETECTOR 3", NULL, NULL );

    if( ret3 == -1 ) return -1;

    if( ret3 >= 0 && ret3 < 3 ) {
      char cmd[64];
      snprintf( cmd, 63, "hw_gains %u\r\n", ret3 );
      send_cmd( cmd, strlen( cmd ) );
    }
    if( ret3 >= 3 ) {
      char cmd[64];
      if( ret3 == 3 ) snprintf( cmd, 63, "peak_det 0\r\n" );
      if( ret3 == 4 ) snprintf( cmd, 63, "peak_det 1\r\n" );
      if( ret3 == 5 ) snprintf( cmd, 63, "peak_det 2\r\n" );
      send_cmd( cmd, strlen( cmd ) );
    }

    ret = current_button_mode;
  }

  if( ret == 7 ) {
    int ret3 = get_menu_choice( 8, "TG HIST (default)", "DIAGNOSTIC", "LAYOUT 3", "LAYOUT 4", "LAYOUT 5", "LAYOUT 6", "LAYOUT 7", "LAYOUT 8" );
    //if( ret3 == -1 ) return -1;

    char cmd[64];
    snprintf( cmd, 63, "layout %u\r\n", ret3+1 ); //layout 1-8
    send_cmd( cmd, strlen( cmd ) );
    ret = 0;
    clr_screen();
  }

  if( ret == 4 ) {
    if( mptr->speaker_en == 1 ) {
      send_cmd( "speaker_en 0", 12 );
    } else {
      send_cmd( "speaker_en 1", 12 );
    }
    ret = 0;
  }

// brightness
  if( ret == 6 ) {
    int ret3 = get_menu_choice( 8, "5 %", "10 %", "20 %", "30 %", "40 %", "50 %", "75 %", "100 %" );
    if( ret3 == -1 ) return -1;
    if( ret3 == 0 ) {
      brightness = 5;
      backLight.setBrightness( brightness );
      EEPROM.write( 0, brightness );
    }
    if( ret3 == 1 ) {
      brightness = 10;
      backLight.setBrightness( brightness );
      EEPROM.write( 0, brightness );
    }
    if( ret3 == 2 ) {
      brightness = 20;
      backLight.setBrightness( brightness );
      EEPROM.write( 0, brightness );
    }
    if( ret3 == 3 ) {
      brightness = 30;
      backLight.setBrightness( brightness );
      EEPROM.write( 0, brightness );
    }
    if( ret3 == 4 ) {
      brightness = 40;
      backLight.setBrightness( brightness );
      EEPROM.write( 0, brightness );
    }
    if( ret3 == 5 ) {
      brightness = 50;
      backLight.setBrightness( brightness );
      EEPROM.write( 0, brightness );
    }
    if( ret3 == 6 ) {
      brightness = 75;
      backLight.setBrightness( brightness );
      EEPROM.write( 0, brightness );
    }
    if( ret3 == 7 ) {
      brightness = 100;
      backLight.setBrightness( brightness );
      EEPROM.write( 0, brightness );
    }

    EEPROM.commit();  //save brightness
    ret = 0;
  }

// }

  current_button_mode = ret;
  char cmd[64];
  snprintf( cmd, 63, "wio_but_mode %u\r\n", ret );

  if( gen_screencaps ) do_screencap();

  send_cmd( cmd, strlen( cmd ) );
  clr_screen();
  return current_button_mode;
}
