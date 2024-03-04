
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
#include "metainfo.h"

extern TFT_eSPI tft;
extern int current_button_mode;
extern TFT_eSprite spr;
void init_sprites( void );
void draw_tg_zones( void );
void get_tg_zones( void );
void handle_button_mode( void );
void send_cmd( const char *cmd, int len );

extern uint16_t tgzone;

static int x = 0;
static int y = 0;

extern uint8_t tg_zones[16][7];

static uint8_t str1[8];
static uint8_t str2[8];
static uint8_t str3[8];
static uint8_t str4[8];
static uint8_t str5[8];
static uint8_t str6[8];
static uint8_t str7[8];
static uint8_t str8[8];
static uint8_t str9[8];
static uint8_t str10[8];
static uint8_t str11[8];
static uint8_t str12[8];
static uint8_t str13[8];
static uint8_t str14[8];
static uint8_t str15[8];
static uint8_t str16[8];

//////////////////////////////
//////////////////////////////
void tgz_inc_x()
{
  x++;
  if( x > 1 ) x = 0;
}
//////////////////////////////
//////////////////////////////
void tgz_dec_x()
{
  x--;
  if( x < 0 ) x = 1;
}
//////////////////////////////
//////////////////////////////
void tgz_inc_y()
{
  y++;
  if( y > 7 ) {
    y = 0;
    tgz_inc_x();
  }
}
//////////////////////////////
//////////////////////////////
void tgz_dec_y()
{
  y--;
  if( y < 0 ) {
    y = 7;
    tgz_dec_x();
  }
}

//////////////////////////////
//////////////////////////////
int get_sel_zone()
{
  int sel_zone = ( y * 2 ) + x; //(0-15)
  return sel_zone;
}

//////////////////////////////
//////////////////////////////
void tg_toggle_select()
{
  int sel_zone = ( y * 2 ) + x; //(0-15)

  uint16_t z = ( tgzone ^ ( 1 << sel_zone ) );
  uint8_t cmd[64];
  sprintf( ( char * )cmd, "tgzone %u\r\n", z );
  send_cmd( ( const char * )cmd, 14 );
}
//////////////////////////////
//////////////////////////////
void draw_tg_zones()
{

#if 1

  uint8_t FNT = 4;
  init_sprites();

#if 0 //testing
  y++;
  y = y % 8;
  if( y == 0 ) x++;
  x = x % 2;
#endif

  str1[7] = 0;
  str2[7] = 0;
  str3[7] = 0;
  str4[7] = 0;
  str5[7] = 0;
  str6[7] = 0;
  str7[7] = 0;
  str8[7] = 0;
  str9[7] = 0;
  str10[7] = 0;
  str11[7] = 0;
  str12[7] = 0;
  str13[7] = 0;
  str14[7] = 0;
  str15[7] = 0;
  str16[7] = 0;

  memcpy( str1, &tg_zones[0][0], 7 );
  memcpy( str2, &tg_zones[1][0], 7 );
  memcpy( str3, &tg_zones[2][0], 7 );
  memcpy( str4, &tg_zones[3][0], 7 );
  memcpy( str5, &tg_zones[4][0], 7 );
  memcpy( str6, &tg_zones[5][0], 7 );
  memcpy( str7, &tg_zones[6][0], 7 );
  memcpy( str8, &tg_zones[7][0], 7 );
  memcpy( str9, &tg_zones[8][0], 7 );
  memcpy( str10, &tg_zones[9][0], 7 );
  memcpy( str11, &tg_zones[10][0], 7 );
  memcpy( str12, &tg_zones[11][0], 7 );
  memcpy( str13, &tg_zones[12][0], 7 );
  memcpy( str14, &tg_zones[13][0], 7 );
  memcpy( str15, &tg_zones[14][0], 7 );
  memcpy( str16, &tg_zones[15][0], 7 );

  int y_offset = 15;

  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground

  if( x == 0 && y == 0 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 0 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str1, 27, 5, FNT );

  spr.pushSprite( 5, 0 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif


#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 1 && y == 0 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 1 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str2, 27, 5, FNT );

  spr.pushSprite( 165, 0 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 0 && y == 1 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 2 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str3, 27, 5, FNT );

  spr.pushSprite( 5, 28 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 1 && y == 1 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }


  if( ( tgzone & ( 1 << 3 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str4, 27, 5, FNT );

  spr.pushSprite( 165, 28 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 0 && y == 2 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }


  if( ( tgzone & ( 1 << 4 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str5, 27, 5, FNT );

  spr.pushSprite( 5, 56 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 1 && y == 2 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 5 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str6, 27, 5, FNT );

  spr.pushSprite( 165, 56 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 0 && y == 3 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 6 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str7, 27, 5, FNT );

  spr.pushSprite( 5, 84 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 1 && y == 3 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 7 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str8, 27, 5, FNT );

  spr.pushSprite( 165, 84 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 0 && y == 4 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 8 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str9, 27, 5, FNT );

  spr.pushSprite( 5, 112 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 1 && y == 4 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 9 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str10, 27, 5, FNT );

  spr.pushSprite( 165, 112 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 0 && y == 5 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 10 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str11, 27, 5, FNT );

  spr.pushSprite( 5, 140 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 1 && y == 5 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 11 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str12, 27, 5, FNT );

  spr.pushSprite( 165, 140 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 0 && y == 6 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 12 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str13, 27, 5, FNT );

  spr.pushSprite( 5, 168 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 1 && y == 6 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 13 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str14, 27, 5, FNT );

  spr.pushSprite( 165, 168 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 0 && y == 7 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 14 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str15, 27, 5, FNT );

  spr.pushSprite( 5, 196 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

#if 1
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  spr.createSprite( 160, 29 ); //allocate sprite memory
  spr.fillSprite( TFT_BLACK ); //clear to black bground
  if( x == 1 && y == 7 ) {
    spr.setTextColor( TFT_BLACK, TFT_WHITE );
  } else {
    spr.setTextColor( TFT_WHITE, TFT_BLACK );
  }

  if( ( tgzone & ( 1 << 15 ) ) ) {
    spr.fillCircle( 15, 15, 8, TFT_GREEN );
  } else {
    spr.fillCircle( 15, 15, 8, TFT_DARKGREY );
  }
  spr.drawString( ( const char * )str16, 27, 5, FNT );

  spr.pushSprite( 165, 196 + y_offset ); //transfer to lcd, x,y = 240,210
  spr.deleteSprite(); //free memory
#endif

}
