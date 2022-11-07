
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
#include "meta_config_info.h"
#include "metainfo.h"

extern TFT_eSPI tft;
extern int current_button_mode;
extern TFT_eSprite spr;
void init_sprites(void);
void draw_config_summary(void);

extern metainfo minfo_verified;
extern meta_config_info minfo_config;
meta_config_info *mi;

static int x=0;
static int y=0;


static uint8_t str1[32];
static uint8_t str2[32];
static uint8_t str3[32];
static uint8_t str4[32];
static uint8_t str5[32];
static uint8_t str6[32];
static uint8_t str7[32];
static uint8_t str8[32];
static uint8_t str9[32];
static uint8_t str10[32];
static uint8_t str11[32];
static uint8_t str12[32];
static uint8_t str13[32];
static uint8_t str14[32];
static uint8_t str15[32];
static uint8_t str16[32];

//////////////////////////////
//////////////////////////////
void draw_config_summary() {
   #if 1
   uint8_t FNT=2;

   tft.fillScreen(TFT_BLACK);

   init_sprites();

   str1[31]=0;
   str2[31]=0;
   str3[31]=0;
   str4[31]=0;
   str5[31]=0;
   str6[31]=0;
   str7[31]=0;
   str8[31]=0;
   str9[31]=0;
   str10[31]=0;
   str11[31]=0;
   str12[31]=0;
   str13[31]=0;
   str14[31]=0;
   str15[31]=0;
   str16[31]=0;

   metainfo *m = &minfo_verified;

   mi = &minfo_config;

#if 0
typedef struct {
  uint32_t fw_version;
  uint8_t ip_addr[4];
  uint8_t net_mask[4];
  uint8_t gw_addr[4];
  uint8_t udp_host[4];
  uint16_t udp_port[5];
  uint8_t do_brown_noise;
  uint8_t do_audio_agc;
  uint8_t learn_mode;
  uint8_t skip_tones;
  uint32_t tgtimeout;
  float audio_vol;
  uint8_t  roaming_mode;
} meta_config_info;
#endif

   //sprintf((char *)str1,"ROAMING TO  %u ms", m->roaming_timeout);
   sprintf((char *)str1,"IP %u.%u.%u.%u", mi->ip_addr[0], mi->ip_addr[1], mi->ip_addr[2], mi->ip_addr[3] ); 
   sprintf((char *)str2,"GW %u.%u.%u.%u", mi->gw_addr[0], mi->gw_addr[1], mi->gw_addr[2], mi->gw_addr[3] ); 
   sprintf((char *)str3,"NM %u.%u.%u.%u", mi->net_mask[0], mi->net_mask[1], mi->net_mask[2], mi->net_mask[3] ); 
   sprintf((char *)str4,"UDP HOST %u.%u.%u.%u", mi->udp_host[0], mi->udp_host[1], mi->udp_host[2], mi->udp_host[3] ); 
   sprintf((char *)str5,"PORTS %u %u %u", mi->udp_port[0], mi->udp_port[1], mi->udp_port[2]);
   sprintf((char *)str6,"%u %u", mi->udp_port[3], mi->udp_port[4]);
   sprintf((char *)str7,"SPKR AWAKE %u", mi->do_brown_noise);
   sprintf((char *)str8,"SKIP TONES %u", mi->skip_tones);
   sprintf((char *)str9,"TG HOLD %u ms", mi->tgtimeout);
   sprintf((char *)str10,"ROAM MODE %u", mi->roaming_mode);
   sprintf((char *)str11,"ROAM TO %u ms", m->roaming_timeout);
   sprintf((char *)str12,"AUD VOL %3.0f", mi->audio_vol);
   sprintf((char *)str13,"AUD AGC %u", mi->do_audio_agc);
   sprintf((char *)str14,"LEARN MODE %u", mi->learn_mode);
   sprintf((char *)str15,"RX FW %u", mi->fw_version);
   sprintf((char *)str16,"WIO FW 2022110601");

   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str1, 5, 5, FNT); 

   spr.pushSprite(5,0); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif


   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str2, 5, 5, FNT); 

   spr.pushSprite(165,0); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str3, 5, 5, FNT); 

   spr.pushSprite(5,30); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str4, 5, 5, FNT); 

   spr.pushSprite(165,30); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str5, 5, 5, FNT); 

   spr.pushSprite(5,60); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str6, 5, 5, FNT); 

   spr.pushSprite(165,60); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str7, 5, 5, FNT); 

   spr.pushSprite(5,90); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str8, 5, 5, FNT); 

   spr.pushSprite(165,90); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str9, 5, 5, FNT); 

   spr.pushSprite(5,120); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str10, 5, 5, FNT); 

   spr.pushSprite(165,120); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str11, 5, 5, FNT); 

   spr.pushSprite(5,150); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str12, 5, 5, FNT); 

   spr.pushSprite(165,150); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str13, 5, 5, FNT); 

   spr.pushSprite(5,180); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str14, 5, 5, FNT); 

   spr.pushSprite(165,180); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str15, 5, 5, FNT); 

   spr.pushSprite(5,210); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

   #if 1
   //////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////
   spr.createSprite(160,35);   //allocate sprite memory
   spr.fillSprite(TFT_BLACK); //clear to black bground

   spr.setTextColor(TFT_WHITE, TFT_BLACK);
   spr.drawString((const char *)str16, 5, 5, FNT); 

   spr.pushSprite(165,210); //transfer to lcd, x,y = 240,210 
   spr.deleteSprite(); //free memory
   #endif

    // keyboard operation
    while (1) {

      if (digitalRead(WIO_5S_UP) == LOW) {
        delay(100);
        return;
      } else if (digitalRead(WIO_5S_DOWN) == LOW) {
        delay(100);
        return;
      } else if (digitalRead(WIO_5S_LEFT) == LOW) {
        delay(100);
        return;
      } else if (digitalRead(WIO_5S_RIGHT) == LOW) {
        delay(100);
        return;
      } else if (digitalRead(WIO_KEY_B) == LOW) {
        delay(100);
        return;
      } else if (digitalRead(WIO_5S_PRESS) == LOW) {
        delay(100);
        return;
      }

   }
}

