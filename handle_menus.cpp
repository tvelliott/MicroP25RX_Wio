
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
extern int gen_screencaps;
extern int follow;

void clr_cmd( void );
void clr_screen( void );
void clr_buttons( void );
void draw_config_summary( void );
void send_cmd( const char *str, int len );
static int FNT = 4;
extern Keybord mykey; // Cleate a keybord

void menu_set_font_size( int size );


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void do_edit( metainfo *m )
{
  int ret2 = get_menu_choice( 8,
                              "EDIT  ALIAS",  //0
                              "EDIT  SYSTEM  NAME",//1
                              "EDIT  SITE  NAME", //2
                              "EDIT  TG  NAME",//3
                              "TG  HOLD  TIME",//4
                              "ALLOW  UNKNOWN",//5
                              "DISABLE  TG",//6
                              "TG  PRIORITY  INT"//7
                            );


  if( ret2 == -1 ) {
    return;
  }

  if( ret2 == 1 ) { //edit system name
    tft.setTextColor( TFT_GREEN, TFT_BLACK );
    clr_screen();
    FNT = 4;
    tft.drawString( "System Name", 5, 10, FNT );
    mykey.set_flag( 2 ); //start with alpha
    mykey.set_cur( 28 ); //start key
    draw_keybord( mykey, 0, 110, 320, 120, 32, 1 );
    String system_name = text_input_5waySwitch( mykey, 35, 60, String( ( char * ) m->sys_name ) );
    system_name.replace( " ", "_" );
    system_name.trim();
    char cmd[64];
    snprintf( cmd, 63, "sys_name 0x%05X 0x%03X %s\r\n", m->wacn_id, m->sys_id, system_name.c_str() );
    send_cmd( cmd, strlen( cmd ) );
  }

  if( ret2 == 4 ) { //tgtimeout
    int ret3 = get_menu_choice( 8, "100 ms", "500 ms", "1 sec", "2 sec (default)", "5 sec", "10 sec", "30 sec", "60 sec" );
    if( ret3 == -1 ) return;

    int to = 2000;
    if( ret3 == 0 ) to = 100;
    if( ret3 == 1 ) to = 500;
    if( ret3 == 2 ) to = 1000;
    if( ret3 == 3 ) to = 2000;
    if( ret3 == 4 ) to = 5000;
    if( ret3 == 5 ) to = 10000;
    if( ret3 == 6 ) to = 30000;
    if( ret3 == 7 ) to = 60000;

    char cmd[64];
    snprintf( cmd, 63, "tgtimeout %u\r\n", to );
    send_cmd( cmd, strlen( cmd ) );
  }

  if( ret2 == 5 ) { //allow unknown
    int ret3 = get_menu_choice( 2, "DISABLE", "ENABLE", NULL, NULL, NULL, NULL, NULL, NULL );
    if( ret3 == -1 ) return;

    char cmd[64];
    snprintf( cmd, 63, "allow_unknown %u\r\n", ret3 );
    send_cmd( cmd, strlen( cmd ) );
  }

  if( ret2 == 3 ) { //edit tg name
    tft.setTextColor( TFT_GREEN, TFT_BLACK );
    clr_screen();
    FNT = 4;
    tft.drawString( "TG AlphaTag", 5, 10, FNT );
    mykey.set_flag( 2 ); //start with alpha
    mykey.set_cur( 28 ); //start key
    draw_keybord( mykey, 0, 110, 320, 120, 32, 1 );
    String alpha_tag = text_input_5waySwitch( mykey, 35, 60, String( ( char * ) m->alpha ) );
    alpha_tag.replace( " ", "_" );
    alpha_tag.trim();
    char cmd[64];
    snprintf( cmd, 63, "tgalpha 0x%05X 0x%03X %u %s\r\n", m->wacn_id, m->sys_id, m->tg_s, alpha_tag.c_str() );
    send_cmd( cmd, strlen( cmd ) );
  }

  if( ret2 == 0 ) { //edit alias
    tft.setTextColor( TFT_GREEN, TFT_BLACK );
    clr_screen();

    if( m->RID == 0 ) {
      //handle this
    } else {
      FNT = 4;
      tft.drawString( "RID ALIAS", 5, 10, FNT );
      mykey.set_flag( 2 ); //start with alpha
      mykey.set_cur( 28 ); //start key
      draw_keybord( mykey, 0, 110, 320, 120, 32, 1 );
      String alias = text_input_5waySwitch( mykey, 35, 60, String( ( char * ) m->alias ) );
      alias.replace( " ", "_" );
      alias.trim();
      char cmd[64];
      snprintf( cmd, 63, "ridalias 0x%05X 0x%03X %u %s\r\n", m->wacn_id, m->sys_id, m->RID, alias.c_str() );
      send_cmd( cmd, strlen( cmd ) );
    }
  }

  if( ret2 == 2 ) { //edit site name
    tft.setTextColor( TFT_GREEN, TFT_BLACK );
    clr_screen();
    FNT = 4;
    tft.drawString( "SITE NAME", 5, 10, FNT );
    mykey.set_flag( 2 ); //start with alpha
    mykey.set_cur( 28 ); //start key
    draw_keybord( mykey, 0, 110, 320, 120, 32, 1 );
    String sitename = text_input_5waySwitch( mykey, 35, 60, String( ( char * ) m->site_name ) );
    sitename.replace( " ", "_" );
    sitename.trim();
    char cmd[64];
    snprintf( cmd, 63, "site_name 0x%05X 0x%03X %u %u %s\r\n", m->wacn_id, m->sys_id, m->site_id, m->rf_id, sitename.c_str() );
    send_cmd( cmd, strlen( cmd ) );
  }

  if( ret2 == 6 ) { //disable talkgroup
    char cmd[64];
    if( follow != 0 ) {
      snprintf( cmd, 63, "tgen %u 0\r\n", follow );
    } else {
      snprintf( cmd, 63, "tgen %u 0\r\n", m->tg_s );
    }
    send_cmd( cmd, strlen( cmd ) );
  }

  if( ret2 == 7 ) { //audio agc
    int ret3 = get_menu_choice( 3, "DISABLE", "ENABLE NO TONE", "ENABLE WITH TONE", NULL, NULL, NULL, NULL, NULL );
    if( ret3 == -1 ) return;


    char cmd[64];
    snprintf( cmd, 63, "en_tg_pri_int %u\r\n", ret3 );
    send_cmd( cmd, strlen( cmd ) );
  }


  clr_buttons();

  clr_screen();
}



void handle_main_menu( metainfo *m )
{
  int ret = get_menu_choice( 8, "ZIP  CODE  SEARCH", "ADD  P25  SYSTEM", "EDIT", "SD  CARD", "FLASH  CONFIG", "AUDIO", "ROAMING CFG", "ROAM  TIMEOUT" );
  if( ret == -1 ) return;

  menu_set_font_size( 4 );

  if( ret == 7 ) { //roam timeout
    int ret3 = get_menu_choice( 8, "200 ms", "250 ms", "275 ms", "300 ms", "400 ms", "500 ms (default)", "1 sec", "2 sec" );
    if( ret3 == -1 ) return;

    int to = 500;
    if( ret3 == 0 ) to = 200;
    if( ret3 == 1 ) to = 250;
    if( ret3 == 2 ) to = 275;
    if( ret3 == 3 ) to = 300;
    if( ret3 == 4 ) to = 400;
    if( ret3 == 5 ) to = 500;
    if( ret3 == 6 ) to = 1000;
    if( ret3 == 7 ) to = 2000;

    char cmd[64];
    snprintf( cmd, 63, "roam_to %u\r\n", to );
    send_cmd( cmd, strlen( cmd ) );
  }

  if( ret == 6 ) { //roaming cfg
    menu_set_font_size( 2 );

    int ret2 = get_menu_choice( 6, "ROAMING OFF", "MODE 1 - SCAN ALL KNOWN PRIMARY", "MODE 2 - AUTO-SW ON SIG LOSS. P+S ALL SYS",
                                "MODE 3 - AUTO-SW ON SIG LOSS, SINGLE SYS", "MODE 4 - AUTO-SW ON SIG LOSS, P+S+A ALL SYS", "MODE 5 - SCAN USER SPEC INC_IN_SCAN=1", NULL, NULL );
    if( ret2 == -1 ) {
      menu_set_font_size( 4 );
      return;
    }

    if( ret2 >= 0 && ret2 <= 5 ) { //roaming selection
      char cmd[64];
      if( ret2 == 3 ) {
        snprintf( cmd, 63, "roam3_cfg 0x%05X 0x%03X\r\n", m->wacn_id, m->sys_id );
      } else {
        snprintf( cmd, 63, "roaming %u\r\n", ret2 );
      }
      send_cmd( cmd, strlen( cmd ) );
    }

    menu_set_font_size( 4 );
  }

  if( ret == 0 ) {
    //GET ZIP CODE
    tft.setTextColor( TFT_GREEN, TFT_BLACK );
    clr_screen();
    FNT = 4;
    tft.drawString( "Enter Zip Code", 5, 10, FNT );
    mykey.set_flag( 3 ); //start with numerical
    mykey.set_cur( 8 ); //start key
    draw_keybord( mykey, 0, 110, 320, 120, 32, 1 );
    String zipcode = text_input_5waySwitch( mykey );

    if( !zipcode.equals( "" ) ) { //user can press BUT_C aggain to cancel / ret ""
      //GET RANGE IN MILES
      clr_screen();
      FNT = 4;
      tft.drawString( "Search Range Miles", 5, 10, FNT );
      mykey.set_flag( 3 ); //start with numerical
      mykey.set_cur( 0 ); //start key
      draw_keybord( mykey, 0, 110, 320, 120, 32, 1 );
      String range = text_input_5waySwitch( mykey );

      if( !range.equals( "" ) ) { //user can press BUT_C aggain to cancel / ret ""

        //SEND CMD TO START ZIP SEARCH
        char cmd[64];
        snprintf( cmd, 63, "zip %s %s\r\n", zipcode.c_str(), range.c_str() );
        send_cmd( cmd, strlen( cmd ) );
      }
    }

    clr_buttons();
    clr_screen();
  } else if( ret == 1 ) {
    //ADD P25 SYSTEM
    tft.setTextColor( TFT_GREEN, TFT_BLACK );
    clr_screen();
    FNT = 4;
    tft.drawString( "Frequency  MHz", 5, 10, FNT );
    mykey.set_flag( 3 ); //start with numerical
    mykey.set_cur( 7 ); //start key
    draw_keybord( mykey, 0, 110, 320, 120, 32, 1 );
    String freq_mhz = text_input_5waySwitch( mykey, 35, 60, "", 1 );

    if( !freq_mhz.equals( "" ) ) { //user can press BUT_C aggain to cancel / ret ""
      //freq command
      char cmd[64];
      snprintf( cmd, 63, "freq %3.6f\r\n", freq_mhz.toDouble() );
      send_cmd( cmd, strlen( cmd ) );
      delay( 1500 ); //give the freq command some time to take place
      snprintf( cmd, 63, "sys_name NEW\r\n" ); //change the system name to NEW
      send_cmd( cmd, strlen( cmd ) );
    }

    clr_buttons();
    clr_screen();
  } else if( ret == 2 ) { //EDIT SYSTEM

    do_edit( m );

  } else if( ret == 3 ) { //SD CARD
    int ret2 = get_menu_choice( 8, "SD  FORMAT  CARD", "BACKUP", "RESTORE", "GEN  SUMMARY  REPORT", "IMPORT  TALK  GROUPS", "IMPORT  SITES", "IMPORT  ALIASES", "DO  SCREEN  CAPS" );
    if( ret2 == -1 ) return;

    if( ret2 == 0 ) { //format card
      int ret3 = get_menu_choice( 2, "NO / CANCEL", "YES  FORMAT  SD", NULL, NULL, NULL, NULL, NULL, NULL );
      if( ret3 == -1 ) return;

      if( ret3 == 1 ) {
        char cmd[64];
        snprintf( cmd, 63, "sd_fmt\r\n" );
        send_cmd( cmd, strlen( cmd ) );
        delay( 1000 );

        clr_cmd();
      }
    }

    if( ret2 == 4 ) { //import tg
      char cmd[64];
      snprintf( cmd, 63, "tg_import\r\n" );
      send_cmd( cmd, strlen( cmd ) );
    }
    if( ret2 == 5 ) { //import sites
      char cmd[64];
      snprintf( cmd, 63, "site_import\r\n" );
      send_cmd( cmd, strlen( cmd ) );
    }
    if( ret2 == 6 ) { //import alias
      char cmd[64];
      snprintf( cmd, 63, "alias_import\r\n" );
      send_cmd( cmd, strlen( cmd ) );
    }

    if( ret2 == 1 ) { //backup
      char cmd[64];
      snprintf( cmd, 63, "flash_backup\r\n" );
      send_cmd( cmd, strlen( cmd ) );
    }
    if( ret2 == 2 ) { //restore
      char cmd[64];
      snprintf( cmd, 63, "flash_restore\r\n" );
      send_cmd( cmd, strlen( cmd ) );
    }
    if( ret2 == 3 ) { //gen summary
      char cmd[64];
      snprintf( cmd, 63, "gen_summary\r\n" );
      send_cmd( cmd, strlen( cmd ) );
    }
    if( ret2 == 7 ) { //screen caps
      int ret3 = get_menu_choice( 2, "DISABLE", "ENABLE", NULL, NULL, NULL, NULL, NULL, NULL );
      if( ret3 == -1 ) return;
      gen_screencaps = ret3;
    }

    clr_buttons();
    clr_screen();
  } else if( ret == 4 ) { //RESET CONFIG TO DEFAULTS
    int ret2 = get_menu_choice( 5, "CANCEL", "RESET  TO  DEFAULTS", "BACKUP", "RESTORE", "SHOW  CONFIG", NULL, NULL, NULL );
    if( ret2 == -1 ) return;

    if( ret2 == 1 ) { //CONFIRM RESET CONFIG
      int ret3 = get_menu_choice( 2, "NO / CANCEL", "YES  RESET  CONFIG", NULL, NULL, NULL, NULL, NULL, NULL );
      if( ret3 == -1 ) return;

      if( ret3 == 1 ) {
        char cmd[64];
        snprintf( cmd, 63, "lfs_fmt\r\n" );
        send_cmd( cmd, strlen( cmd ) );
        delay( 1000 );

        clr_cmd();
      }
    }

    if( ret2 == 2 ) { //backup
      char cmd[64];
      snprintf( cmd, 63, "flash_backup\r\n", m->tg_s );
      send_cmd( cmd, strlen( cmd ) );
    }
    if( ret2 == 3 ) { //restore
      char cmd[64];
      snprintf( cmd, 63, "flash_restore\r\n", m->tg_s );
      send_cmd( cmd, strlen( cmd ) );
    }
    if( ret2 == 4 ) { //show config
      draw_config_summary();
    }

    clr_buttons();
    clr_screen();
  } else if( ret == 5 ) { //AUDIO
    int ret2 = get_menu_choice( 6, "CANCEL", "VOLUME", "AUDIO  AGC", "KEEP  SPEAKER  AWAKE", "MUTE P1/P2 TONES", "LOW VOLUME", NULL, NULL );
    if( ret2 == -1 ) return;

    if( ret2 == 1 ) { //volume
      int ret3 = get_menu_choice( 8, "OFF", "VERY LOW 0.5", "LOW 1.0", "NORMAL 2.0", "LOUD 4.0", "LOUD 5.0", "LOUD 6.0", "VERY LOUD 8.0" );
      if( ret3 == -1 ) return;

      float v = 0.0f;
      if( ret3 == 0 ) v = 0.0f;
      if( ret3 == 1 ) v = 0.5f;
      if( ret3 == 2 ) v = 1.0f;
      if( ret3 == 3 ) v = 2.0f;
      if( ret3 == 4 ) v = 4.0f;
      if( ret3 == 5 ) v = 5.0f;
      if( ret3 == 6 ) v = 6.0f;
      if( ret3 == 7 ) v = 8.0f;

      char cmd[64];
      snprintf( cmd, 63, "vol %3.1f\r\n", v );
      send_cmd( cmd, strlen( cmd ) );
    }


    if( ret2 == 2 ) { //audio agc
      int ret3 = get_menu_choice( 2, "DISABLE", "ENABLE", NULL, NULL, NULL, NULL, NULL, NULL );
      if( ret3 == -1 ) return;

      char cmd[64];
      snprintf( cmd, 63, "audio_agc %u\r\n", ret3 );
      send_cmd( cmd, strlen( cmd ) );
    }

    if( ret2 == 3 ) { //brown noise
      int ret3 = get_menu_choice( 2, "DISABLE", "ENABLE", NULL, NULL, NULL, NULL, NULL, NULL );
      if( ret3 == -1 ) return;

      char cmd[64];
      snprintf( cmd, 63, "brown_noise %u\r\n", ret3 );
      send_cmd( cmd, strlen( cmd ) );
    }

    if( ret2 == 4 ) { //mute p1 tones
      int ret3 = get_menu_choice( 2, "DISABLE MUTE", "ENABLE MUTE", NULL, NULL, NULL, NULL, NULL, NULL );
      if( ret3 == -1 ) return;

      char cmd[64];
      snprintf( cmd, 63, "skip_tones %u\r\n", ret3 );
      send_cmd( cmd, strlen( cmd ) );
    }

    if( ret2 == 5 ) { //low volume
      int ret3 = get_menu_choice( 8, "  0.01", "  0.02", "  0.03", "  0.04", "  0.05", "  0.10", "  0.25", "  0.50" );
      if( ret3 == -1 ) return;

      float v = 0.00f;
      if( ret3 == 0 ) v = 0.01f;
      if( ret3 == 1 ) v = 0.02f;
      if( ret3 == 2 ) v = 0.03f;
      if( ret3 == 3 ) v = 0.04f;
      if( ret3 == 4 ) v = 0.05f;
      if( ret3 == 5 ) v = 0.10f;
      if( ret3 == 6 ) v = 0.25f;
      if( ret3 == 7 ) v = 0.50f;

      char cmd[64];
      snprintf( cmd, 63, "vol %4.2f\r\n", v );
      send_cmd( cmd, strlen( cmd ) );
    }
    clr_buttons();
    clr_screen();
  }
#if 0
  else if( ret == 6 ) { //ADVANCED
    int ret2 = get_menu_choice( 6, "CANCEL", "IP  ADDR", "GW  ADDR", "NET  MASK", "UDP  HOST  IP", "REF  FREQ  CAL", NULL, NULL );
    if( ret2 == -1 ) return;

    clr_buttons();
    clr_screen();
  }
#endif
}
