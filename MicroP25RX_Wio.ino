
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



#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include "Free_Keybord.h"
#include "list_menu.h"
#include "handle_menus.h"
#include "handle_button_mode.h"
#include "meta_config_info.h"

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);
Keybord mykey; // Cleate a keybord


#include "buttons.h"

#include <SPI.h>
#include <wiring_private.h>

#include <stdint.h>
#include "crc32.h"  
#include "metainfo.h"

meta_config_info minfo_config;

static uint32_t roam_time;

extern volatile int cmd_acked;

uint8_t tg_zones[16][7];
void draw_tg_zones(void);
void tg_toggle_select(void);
int get_sel_zone(void);

volatile uint8_t buf[1500];
metainfo minfo;
metainfo minfo_verified;
metainfo minfo_copy;
static int clean_wio_line2;

extern uint32_t button_press_time;
static int did_save;
int current_button_mode=-1;
static int prev_button_mode=-1;

volatile uint32_t rx_count;
volatile int spi_state;
volatile int do_draw_rx;
volatile int buf_idx;

static double freq_val;

volatile uint16_t meta_port;
volatile uint16_t meta_len;
volatile uint32_t sclk_count;
/////////////////////////////////////////////////////
//
void reset_info(void);
void change_freq(int freq, int is_inc);
int is_valid_freq( double freq );
char disp_buf[256];
char disp_buf2[256];
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


static uint32_t init_wdog1; //const
static uint32_t init_wdog2; //sysinfo
static double freq_table[8];


static int FNT = 4;

static uint8_t demod;
static uint8_t inc_in_scan;
static uint8_t learn;
static int32_t follow;
static int32_t mute;
uint16_t tgzone;

static double prev_freq;
static int32_t prev_tgs;
static int freq_changed;

void tgz_inc_x();
void tgz_dec_x();
void tgz_inc_y();
void tgz_dec_y();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void init_sprites() {
  spr = TFT_eSprite(&tft);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clr_screen() {
  tft.fillScreen(TFT_BLACK);  //background
  memset(line1_str,0x00,32);
  memset(line2_str,0x00,32);
  memset(line3_str,0x00,32);
  memset(line4_str,0x00,32);
  memset(line5_str,0x00,32);
  memset(line6_str,0x00,32);
  memset(line7_str,0x00,32);
  memset(line8_str,0x00,32);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line1() {
  tft.fillRect(0, 0, 320, 30, TFT_BLACK);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line2() {
  tft.fillRect(0, 30, 320, 30, TFT_BLACK);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line3() {
  tft.fillRect(0, 60, 320, 30, TFT_BLACK);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line4() {
  tft.fillRect(0, 90, 320, 30, TFT_BLACK);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line5() {
  tft.fillRect(0, 120, 320, 30, TFT_BLACK);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line6() {
  tft.fillRect(0, 150, 320, 30, TFT_BLACK);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line7() {
  tft.fillRect(0, 180, 320, 30, TFT_BLACK);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_line8() {
  tft.fillRect(0, 210, 235, 30, TFT_BLACK);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void setup()
{
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);  //background
  tft.setFreeFont(FS18);

 
  tft.fillScreen(TFT_BLACK);  //background

  init_sprites();

  //joystick
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);

  //3-buttons
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);


  tft.setFreeFont(NULL);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  sprintf(disp_buf, "%s","BlueTail    MicroP25RX");
  FNT=4;
  tft.drawString(disp_buf, 5, 10, FNT);

  //SPI.begin();
  initSPI();

  //we have to control this manually due to silicon bug?? in SAMD51
  pinMode(PIN_SPI_MISO, OUTPUT);
  digitalWrite(PIN_SPI_MISO,HIGH);
  
  pinMode(PIN_SPI_SS, INPUT_PULLUP);
  //interrupt handler for Wio -> to -> Teensy
  attachInterrupt(digitalPinToInterrupt(PIN_SPI_SS), spi_miso_int, FALLING);

  delay(100);//let button-inputs settle after power-up

  clr_buttons();
  did_save=0;
  button_press_time=0;
  
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void loop()
{

	check_buttons();



  //middle-top button held for more than 3 seconds?
  if(!did_save && B_but_pressed && (millis()-button_press_time > 3000) && B_but==0xff) {
    send_cmd("save",4);  //save config
    did_save=1;
  }
  else if(did_save && B_but==0x00) {
    did_save=0;
    button_press_time=0;
    B_but_pressed=0;
  }

  /////////////////////////////////////////////////////////////////////////
  // CONFIG MODE
  /////////////////////////////////////////////////////////////////////////
  if( current_button_mode == WIO_BUTTON_MODE_CONFIG) {
  
      //pressed and released
      if (press_but_pressed && press_but == 0x00) { //select button mode menu
        press_but_pressed = 0;
        memcpy((void *)&minfo_copy, (void *)&minfo_verified, sizeof(metainfo));
        metainfo *m = &minfo_copy;
        int ret = handle_button_mode();
        if(ret>=0) {
          current_button_mode = ret; 
        }
        else {
          B_but=0;
          B_but_pressed=0;
          clr_screen();
        }
      }
      //pressed and released
      if (right_but_pressed && right_but == 0x00) {
        right_but_pressed = 0;
        send_cmd("nfreq",5);  //next primary/active frequency
      }
      //pressed and released
      if (left_but_pressed && left_but == 0x00) {
        left_but_pressed = 0;
        send_cmd("pfreq",5); //previous primary/active frequency
      }
      //pressed and released
      if (up_but_pressed && up_but == 0x00) {
        up_but_pressed = 0;
        char cmd_str[32];
        metainfo *mptr = &minfo_verified;
        sprintf(cmd_str,"nsfreq %05X %03X\r\n", mptr->wacn_id, mptr->sys_id);  //roam mode=3 needs wacn and sysid arguments
        int len = strlen(cmd_str);
        send_cmd(cmd_str,len);  //next primary/active frequency
      }
      //pressed and released
      if (down_but_pressed && down_but == 0x00) {
        down_but_pressed = 0;
        char cmd_str[32];
        metainfo *mptr = &minfo_verified;
        sprintf(cmd_str,"psfreq %05X %03X\r\n", mptr->wacn_id, mptr->sys_id);  //roam mode=3 needs wacn and sysid arguments
        int len = strlen(cmd_str);
        send_cmd(cmd_str,len);  //next primary/active frequency
      }


      //pressed, released
      //left-most button
      if (C_but_pressed && C_but == 0x00) {
        C_but_pressed = 0;

        metainfo *mptr = &minfo_verified;
        char cmd[32];
        if( mptr->roaming==5 ) {
          snprintf(cmd, 31, "inc_scan %05X %03X %u %u 1\r\n", mptr->wacn_id, mptr->sys_id, mptr->site_id, mptr->rf_id); 
          send_cmd( (const char *) cmd,strlen(cmd));
        }
        else {
          snprintf(cmd, 31, "learn %d\r\n", (learn^0x01) ); //learn mode on/off
          send_cmd( (const char *) cmd,strlen(cmd));
        }
      }
      //middle-most button
      if(B_but_pressed && B_but == 0x00) { //configuration menu
        B_but_pressed = 0;
        button_press_time=0;


        memcpy((void *)&minfo_copy, (void *)&minfo_verified, sizeof(metainfo));
        metainfo *m = &minfo_copy;
        handle_main_menu(m);

        clr_screen();
      }
      //right-most button
      if (A_but_pressed && A_but == 0x00) { //demod mode LSM/FM
        A_but_pressed = 0;
        metainfo *mptr = &minfo_verified;
        char cmd[32];
        if( mptr->roaming==5 ) {
          snprintf(cmd, 31, "inc_scan %05X %03X %u %u 0\r\n", mptr->wacn_id, mptr->sys_id, mptr->site_id, mptr->rf_id); 
          send_cmd( (const char *) cmd,strlen(cmd));
        }
        else {
          snprintf(cmd, 31, "demod %d\r\n", (demod^0x01) );
          send_cmd( (const char *) cmd,strlen(cmd));
        }
      }
  }
  /////////////////////////////////////////////////////////////////////////
  // TG ZONE MODE
  /////////////////////////////////////////////////////////////////////////
  else if( current_button_mode == WIO_BUTTON_MODE_TG_ZONE) {
    //pressed and released
    if (press_but_pressed && press_but == 0x00) { //select button mode menu
      press_but_pressed = 0;
      int ret = handle_button_mode();
      if(ret>=0) {
        current_button_mode = ret; 
      }
      else {
        B_but=0;
        B_but_pressed=0;
        clr_screen();
      }
    }
    //pressed and released
    if (right_but_pressed && right_but==0x00) {
      right_but_pressed = 0;
      tgz_inc_x();
    }
    //pressed and released
    if (left_but_pressed && left_but==0x00) {
      left_but_pressed = 0;
      tgz_dec_x();
    }
    //pressed and released
    if (up_but_pressed && up_but==0x00) {
      up_but_pressed = 0;
      tgz_dec_y();
    }
    //pressed and released
    if (down_but_pressed && down_but==0x00) {
      down_but_pressed = 0;
      tgz_inc_y();
    }
    if (A_but_pressed && A_but == 0x00) { 
      A_but_pressed = 0;
      char cmd[64];
      snprintf(cmd,63,"wio_but_mode 0\r\n");
      send_cmd(cmd,15);
    }
    if (C_but_pressed && C_but == 0x00) { 
      C_but_pressed = 0;
      tg_toggle_select();
    }
    if (B_but_pressed && B_but == 0x00) { 
      B_but_pressed = 0;
      int zone = get_sel_zone();

      tft.setTextColor(TFT_GREEN, TFT_BLACK); 
      clr_screen();
      FNT=4;
      tft.drawString("Zone Name", 5, 10, FNT);
      mykey.set_flag(2);	//start with alphanum
      mykey.set_cur(28);	//start key
      draw_keybord(mykey, 0, 110, 320, 120, 32, 1);
      zone &= 0x0f;

      char zone_name[8];
      zone_name[7]=0;
      memcpy(zone_name,&tg_zones[zone][0],7);
      String zone_str = text_input_5waySwitch(mykey,35,60, String((char *) zone_name));

      zone_str.replace(' ','_');

      memcpy(zone_name,zone_str.c_str(),7);

      char cmd[32];
      snprintf(cmd, 31, "zonename %u %s\r\n",zone,zone_name); 
      send_cmd( (const char *) cmd,strlen(cmd));
    }
  }
  /////////////////////////////////////////////////////////////////////////
  // MONITOR MODE
  /////////////////////////////////////////////////////////////////////////
  else if( current_button_mode == WIO_BUTTON_MODE_MONITOR) {
    //pressed and released
    if (press_but_pressed && press_but == 0x00) { //select button mode menu
      press_but_pressed = 0;
      memcpy((void *)&minfo_copy, (void *)&minfo_verified, sizeof(metainfo));
      metainfo *m = &minfo_copy;
      int ret = handle_button_mode();
      if(ret>=0) {
        current_button_mode = ret; 
      }
      else {
        B_but=0;
        B_but_pressed=0;
        clr_screen();
      }
    }
    //pressed and released
    if (right_but_pressed && right_but == 0x00) {
      right_but_pressed = 0;
      send_cmd("nfreq",5);  //next primary/active frequency
    }
    //pressed and released
    if (left_but_pressed && left_but == 0x00) {
      left_but_pressed = 0;
      send_cmd("pfreq",5); //previous primary/active frequency
    }
    //pressed, released
    //left-most button
    if (C_but_pressed && C_but == 0x00) { //TG HOLD
      C_but_pressed = 0;
      char cmd[32];
      if(follow==0) {
        snprintf(cmd, 31, "f %d\r\n",prev_tgs); 
        send_cmd( (const char *) cmd,strlen(cmd));
      }
      else {
        snprintf(cmd, 31, "f\r\n"); 
        send_cmd( (const char *) cmd,strlen(cmd));
      }
    }
    //middle-most button
    if(B_but_pressed && B_but == 0x00) { //MUTE
      B_but_pressed = 0;
      button_press_time=0;

      char cmd[32];
      snprintf(cmd, 31, "audio_mute %u\r\n", (mute^0x01) );
      send_cmd( (const char *) cmd,strlen(cmd));
    }
    //right-most button
    if (A_but_pressed && A_but == 0x00) { //SKIP
      A_but_pressed = 0;

      char cmd[32];
      if(prev_tgs>0) {
        snprintf(cmd, 31, "s %d\r\n", prev_tgs);
        send_cmd( (const char *) cmd,strlen(cmd));
      }
      else {
        snprintf(cmd, 31, "s\r\n", prev_tgs);
        send_cmd( (const char *) cmd,strlen(cmd));
      }
    }
  }

	///////////////////////////////////////////////////////////////////////
	// We received a new metainfo structure. Time to redraw the screen
	///////////////////////////////////////////////////////////////////////
  if (do_draw_rx) {

    __disable_irq();
    do_draw_rx = 0;
    memcpy( (uint8_t *) &minfo, (uint8_t *) buf, sizeof(metainfo) );
    __enable_irq();

    metainfo *mptr = (metainfo *) &minfo;
    crc32_val = ~0L;
    uint32_t mi_crc = crc32_range( (unsigned char *) &minfo, sizeof(metainfo) - 8 );

    //validate the structure with 32-bit crc
    if (  mptr->crc_val == mi_crc && mptr->port==8893 ) { //8893=metainfo structure
      rx_count++;

      tgzone = mptr->tgzone;

      if( mptr->data_type == META_DATA_TYPE_ZONEINFO) { 
        memcpy( &tg_zones[0][0], &mptr->data[0], 112); //tg zones 7 x 16
      }
      if( mptr->data_type == META_DATA_TYPE_META_CONFIG_INFO) { 
        memcpy( (char *) &minfo_config, (char *) &mptr->data[0], sizeof(meta_config_info) );
      }

      memcpy((void *)&minfo_verified, (void *)&minfo, sizeof(metainfo));



      extern uint8_t packet_id;
      if( packet_id == mptr->wio_packet_id ) {
        //the teensy digital board received the last command
        //sent with send_cmd( *str, int len)

        __disable_irq();
        cmd_acked=0;
        clr_cmd(); //clear the command buffer
        __enable_irq();
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
      if(cmd_acked==0 && current_button_mode==WIO_BUTTON_MODE_MONITOR && follow==0 && 
        mptr->roaming==1 && mptr->voice_tg_timeout==0 && (millis()-roam_time > mptr->roaming_timeout) ) { //ROAMING=1 // scanning

        roam_time=millis();
        send_cmd("nfreq",15);
      }
      //ROAMING MODE 2
      else if(cmd_acked==0 && current_button_mode==WIO_BUTTON_MODE_MONITOR && follow==0 && (millis()-roam_time > mptr->roaming_timeout) &&
        mptr->roaming==2 && (mptr->evm_p>10 || mptr->rssi_f<-115) ) { //ROAMING=2 // auto-switch-over-on-lost-sig, P+S ALL systems

        roam_time=millis();
        send_cmd("nfreq",15);
      }
      //ROAMING MODE 3
      else if(cmd_acked==0 && current_button_mode==WIO_BUTTON_MODE_MONITOR && follow==0 && (millis()-roam_time > mptr->roaming_timeout) &&
        mptr->roaming==3 && (mptr->evm_p>10 || mptr->rssi_f<-115) ) { //ROAMING=3 // P+S auto-switch-over-on-lost-sig, SINGLE SYSTEM 

        roam_time=millis();
        char cmd_str[32];
        sprintf(cmd_str,"nfreq %05X %03X\r\n", mptr->roam_wacn, mptr->roam_sysid);  //roam mode=3 needs wacn and sysid arguments
        send_cmd(cmd_str,15);
      }
      //ROAMING MODE 4
      else if(cmd_acked==0 && current_button_mode==WIO_BUTTON_MODE_MONITOR && follow==0 && (millis()-roam_time > mptr->roaming_timeout) &&
        mptr->roaming==4 && (mptr->evm_p>10 || mptr->rssi_f<-115) ) { //ROAMING=4 // auto-switch-over-on-lost-sig, P+S+A ALL systems

        roam_time=millis();
        send_cmd("nfreq",15);
      }
      //ROAMING MODE 5
      else if(cmd_acked==0 && current_button_mode==WIO_BUTTON_MODE_MONITOR && follow==0 && 
        mptr->roaming==5 && mptr->voice_tg_timeout==0 && (millis()-roam_time > mptr->roaming_timeout) ) { //ROAMING=5 // scanning, INC_IN_SCAN=1

        roam_time=millis();
        send_cmd("nfreq",15);
      }
      else if(mptr->voice_tg_timeout) {
        roam_time=millis(); //hang around ater the conversation too
      }



      //check if the button / display mode changed
      current_button_mode = mptr->wio_button_mode;

      if( current_button_mode != prev_button_mode ) {
        clr_screen();
        clr_buttons();

        did_save=0;
        button_press_time=0;
      }

      prev_button_mode = current_button_mode;



			demod = mptr->use_demod;
      follow = mptr->follow;
      mute = mptr->audio_mute;
      learn = mptr->learn_mode;
      inc_in_scan = mptr->inc_in_scan;

      if( current_button_mode == WIO_BUTTON_MODE_TG_ZONE) {
          __disable_irq();
          do_draw_rx = 0;
          memcpy( (uint8_t *) &minfo, (uint8_t *) buf, sizeof(metainfo) );
          __enable_irq();

          draw_tg_zones();
          goto draw_end; 
      }


      //FONT SIZE 1,2,4,8
      FNT=4;

      tft.setFreeFont(NULL);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);

      if( prev_freq != mptr->current_freq ) {
        freq_changed=1;
      }
      prev_freq=mptr->current_freq;


      tft.setTextColor(TFT_YELLOW, TFT_BLACK);

      //if( !mptr->do_wio_lines ) {
        if(mptr->tg_s!=prev_tgs) {
          clear_line2();
        }
        prev_tgs = mptr->tg_s;


        if (mptr->tg_s <= 0) {
          snprintf(disp_buf2, 32, "FREQ: %3.6f MHz", mptr->current_freq );

          if( strncmp((char *)disp_buf2,(char *)line3_str, 31)!=0 ) clear_line3();
          strncpy(line3_str,disp_buf2,31);

          if( !mptr->do_wio_lines && follow>0 ) {
            snprintf(disp_buf,32, "HOLDING ON TG %d", follow);
            if( strncmp((char *)disp_buf,(char *)line2_str, 31)!=0 ) clear_line2();
            strncpy(line2_str,disp_buf,31);
            tft.drawString(disp_buf, 5, 40, FNT);
          }
          if(!mptr->do_wio_lines && follow==0) {
            snprintf(disp_buf,32, "   ");
            mptr->desc[19]=0;
            if( strncmp((char *)disp_buf,(char *)line2_str, 31)!=0 ) clear_line2();
            strncpy(line2_str,disp_buf,31);
          }
        }
        else {

          if(!mptr->do_wio_lines) {
            snprintf(disp_buf,32, "TG: %d, %s", mptr->tg_s, mptr->alpha );
            mptr->desc[19]=0;
            if( strncmp((char *)disp_buf,(char *)line2_str, 31)!=0 ) clear_line2();
            strncpy(line2_str,disp_buf,31);
          }

          snprintf(disp_buf2,45, "%s", mptr->desc );
          if( strncmp((char *)disp_buf2,(char *)line3_str, 31)!=0 ) clear_line3();
          strncpy(line3_str,disp_buf2,31);

          if(!mptr->do_wio_lines) {
            tft.drawString(disp_buf, 5, 40, FNT);
          }
        }
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);
        tft.drawString(disp_buf2, 5, 70, FNT);
      //}

      FNT=2;
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      
      sprintf(disp_buf, "%05X-%03X-%03X  RSSI %3.1f dBm    ", mptr->wacn_id, mptr->sys_id, mptr->p25_sys_nac, mptr->rssi_f);
      if( strcmp(disp_buf,line4_str)!=0 ) { 
        //clear_line4(); //space at the end of this line is better solution
        tft.drawString(disp_buf, 5, 100, FNT);
      }
      strcpy(line4_str, disp_buf);

      FNT=2;
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      char demod_str[8];
      demod_str[0]=0;
      if( mptr->use_demod==0) strcpy(demod_str,"LSM");
      if( mptr->use_demod==1) strcpy(demod_str,"FM");
      
      sprintf(disp_buf, "SITE %d, RFSS %d  DEMOD %s", mptr->site_id, mptr->rf_id, demod_str );
      if( strcmp(disp_buf,line5_str)!=0 ) { 
        clear_line5();
        tft.drawString(disp_buf, 5, 130, FNT);
      }
      strcpy(line5_str, disp_buf);

      sprintf(disp_buf, "NCO1 %3.3f, NCO2 %3.2f, EVM %3.1f    ", mptr->nco_offset, mptr->loop_freq, mptr->evm_p );
      if( strcmp(disp_buf,line6_str)!=0 ) { 
        //clear_line6(); //space at the end of this line is better solution
        tft.drawString(disp_buf, 5, 160, FNT);
      }
      strcpy(line6_str, disp_buf);

      if(mptr->on_control_b && mptr->total_session_time>1500) {
        sprintf(disp_buf, "TSBK/SEC %u    REF %u        ", mptr->tsbk_sec, mptr->ref_freq_cal );
      }
      else {
        sprintf(disp_buf, "FREQ %3.6f MHz  ERATE %1.3f       ", mptr->current_freq, mptr->erate );
      }
      if( strcmp(disp_buf,line7_str)!=0 ) { 
        //clear_line7(); //space at the end of this line is better solution
        tft.drawString(disp_buf, 5, 190, FNT);
      }
      strcpy(line7_str, disp_buf);
      
      FNT=2;
      memset(disp_buf,0x00,sizeof(disp_buf));
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      if(mptr->on_control_b==0 && mptr->RID!=0 && mptr->alias!=NULL) {
        sprintf(disp_buf, "RID %u, %s", mptr->RID, mptr->alias );
      }
      else {
        if( current_button_mode==WIO_BUTTON_MODE_CONFIG ) {
          if(mptr->roaming) {
            sprintf(disp_buf, "MODE: CONFIG  ROAM-PAUSE" );
          }
          else {
              sprintf(disp_buf, " " );
          }
        }
        else if( current_button_mode==WIO_BUTTON_MODE_MONITOR ) {
          if(mptr->roaming) {
            sprintf(disp_buf, "MODE: MONITOR ROAM-ON-%u",mptr->roaming );
          }
          else {
            sprintf(disp_buf, "MODE: MONITOR ROAM-OFF" );
          }
        }
        else {
            sprintf(disp_buf, " " );
        }
      }
      if( strcmp(disp_buf,line8_str)!=0 ) { 
        strcpy(line8_str,disp_buf);
        clear_line8();
        tft.drawString(disp_buf, 5, 220, FNT);
      }

      //start of sprites
       init_sprites(); //best to re-init
       #if 1
       //////////////////////////////////////////////////////////
       //Draw STATUS INDICATORS HOLD, MUTE,  
       //////////////////////////////////////////////////////////
       spr.createSprite(60,30);   //allocate sprite memory
       spr.fillSprite(TFT_BLACK); //clear to black bground

       spr.setTextColor(TFT_BLACK, TFT_GREEN); 

       memset(disp_buf,0x00,sizeof(disp_buf));
       if(follow || mute || inc_in_scan) strcat(disp_buf, " ");

       if( follow>0) strcat(disp_buf, "H ");
       if( mute>0 ) strcat(disp_buf, "M ");
       if( inc_in_scan>0 ) strcat(disp_buf, "S ");
       
       spr.drawString(disp_buf, 0, 0, FNT);
       

       spr.pushSprite(265,185); //transfer to lcd, x,y = 240,210 
       spr.deleteSprite(); //free memory
       #endif

       #if 1
       //////////////////////////////////////////////////////////
       //Draw Status LEDS  / P1-P2 indicator
       //////////////////////////////////////////////////////////
       spr.createSprite(80,40);   //allocate sprite memory
       spr.fillSprite(TFT_BLACK); //clear to black bground

       spr.setTextColor(TFT_GREEN, TFT_BLACK); 
       if(mptr->phase2) {
        spr.drawString("P2", 0, 8, FNT); //p25 p2   
       }
       else {
        spr.drawString("P1", 0, 8, FNT); //p25 p1
       }
      
			//status led
       if ( mptr->status_led ) {
				 spr.fillCircle( 35, 12, 10, TFT_GREEN);
			 }
       else {
				 spr.fillCircle( 35, 12, 10, TFT_DARKGREY);
			 }

			//TG led
       if ( mptr->tg_led ) {
          spr.fillCircle( 60, 12, 10, TFT_YELLOW);
       } else {
          spr.fillCircle( 60, 12, 10, TFT_DARKGREY);
       }
       spr.pushSprite(240,212); //transfer to lcd, x,y = 240,210 
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

       if( mptr->data_type == META_DATA_TYPE_CONST) { //incoming data is symbol constellation? 

         ptr = (int8_t *) &mptr->data[0];  //pointer to incoming data 128 bytes
         spr.createSprite(80,80);  //allocate memory for 80 x 80 sprite
         spr.fillSprite(TFT_BLACK);
         //grid lines
         spr.drawLine(40, 0, 40, 80, TFT_DARKGREY);
         spr.drawLine(0, 40, 80, 40, TFT_DARKGREY);
         //draw 64-symbol constellation
         idx = 0;
         for (int i = 0; i < 64; i++) {
           ii = *ptr++/2; //scale to +/- 32 range
           qq = *ptr++/2;

           spr.fillCircle(40+ii, 40+qq, 1, TFT_YELLOW);  //symbols
         }
         spr.pushSprite(233,98);  //send to lcd. upper left corner of sprite
         spr.deleteSprite();  //free memory
       }
       #endif
        
      tft.setTextColor(TFT_GREEN, TFT_BLACK); 

			if( mptr->do_wio_lines ) {


				if(mptr->wio_line1[0]!=0) {
					snprintf(disp_buf,25, "%s", mptr->wio_line1);
					FNT=4;

          if(strncmp((char *)line1_str,(char *)disp_buf,31)!=0) clear_line1();
          strncpy(line1_str,disp_buf,31);

					tft.drawString(disp_buf, 5, 10, FNT);
				}
        else {
					FNT=4;
          mptr->sys_name[18]=0;
          snprintf(disp_buf, 25,"%s");

          if(strncmp((char *)line1_str,(char *)disp_buf,31)!=0) clear_line1();
          strncpy(line1_str,disp_buf,31);

					tft.drawString(disp_buf, 5, 10, FNT);
        }


				if(mptr->wio_line2[0]!=0) {
					FNT=2;
					snprintf(disp_buf,63,  "%s", mptr->wio_line2);
          if(strncmp((char *)line2_str,(char *)disp_buf,63)!=0) clear_line2();
          strncpy(line2_str,disp_buf,63);

					tft.drawString(disp_buf, 5, 40, FNT);
				}
        else {
					FNT=2;
          //mptr->sys_name[18]=0;
          //mptr->site_name[18]=0;
          //snprintf(disp_buf, 63,"%s / %s",mptr->sys_name,mptr->site_name);
          snprintf(disp_buf, 63,"   ");

          if(strncmp((char *)line2_str,(char *)disp_buf,63)!=0) clear_line2();
          strncpy(line2_str,disp_buf,63);

					tft.drawString(disp_buf, 5, 40, FNT);
        }
			}
      else {
        FNT=4;
        mptr->sys_name[18]=0;
        mptr->site_name[18]=0;
        snprintf(disp_buf, 50,"%s / %s",mptr->sys_name,mptr->site_name);

        if(strncmp((char *)line1_str,(char *)disp_buf,31)!=0) clear_line1();
        strncpy(line1_str,disp_buf,31);

        tft.drawString(disp_buf, 5, 10, FNT);
      }

      freq_changed=0; //keep this at the end within valid crc check
    }

draw_end: 
    status_timer = millis();

  }

  if (status_timer > 0 && millis() - status_timer > 1000) {
    status_timer = 0;

    __disable_irq();
    spi_state = 0;
    __enable_irq();

  }

}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void change_freq(int freq, int is_inc) {

  clr_screen();

  int i;

  for (i = 0; i < 8; i++) { //try all 8 until we find a valid one
    freq_idx &= 0x07; //limit from 0-7
    double next_freq = freq_table[freq_idx];


    if (is_valid_freq(next_freq)) { //only send new frequency if it is in a valid ranage
      //give it a chance to catch up after a freq change
      init_wdog1 = millis(); //voice
      init_wdog2 = millis(); //const

      sprintf(disp_buf, "freq %3.6f\r\n", next_freq);
      tft.setFreeFont(FS18);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);

      //send frequency command
   

      memcpy(disp_buf, "FREQ", 4); //keep this after the command. P25RX wants lower case for the command.
      tft.drawString(disp_buf, 10, 10);
      return;
    }

    if (is_inc) freq_idx++; //last one wasn't valid, so try the next one
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

  if ( freq >= 25.0 && freq <= 512.0 ) band = 1; //band 1
  if ( freq >= 758.0 && freq <= 824.0 ) band = 2; //band 2
  if ( freq >= 849.0 && freq <= 869.0 ) band = 3; //band 3
  if ( freq >= 894.0 && freq <= 960.0 ) band = 4; //band 4
  if ( freq >= 1240.0 && freq <= 1300.0 ) band = 5; //band 5


  return band;
  //return 1;
}