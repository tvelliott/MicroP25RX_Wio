
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

extern TFT_eSPI tft;
extern int current_button_mode;
extern int gen_screencaps;
extern void do_screencap( void );

void clr_screen( void );
void send_cmd( const char *str, int len );
static int FNT = 4;
extern int do_refresh;
extern Keybord mykey; // Cleate a keybord

int handle_button_mode( void )
{
  int ret = get_menu_choice( 4, "MONITOR  MODE", "CONFIG  MODE", "TG  ZONE MODE", "SIGNALS / GAIN  MODE", NULL, NULL, NULL, NULL );
  if( ret == -1 ) return -1;

  current_button_mode = ret;
  char cmd[64];
  snprintf( cmd, 63, "wio_but_mode %u\r\n", ret );

  if( gen_screencaps ) do_screencap();

  send_cmd( cmd, strlen( cmd ) );
  clr_screen();

  return current_button_mode;
}
