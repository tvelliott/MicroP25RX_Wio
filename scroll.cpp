
#include <Arduino.h>
#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include "metainfo.h"

extern TFT_eSPI tft;
extern TFT_eSprite scroll_spr;

static int tcount = 0;

static int did_init;

static int xpos;
static int ypos;

static long ctime;
static long prev_time;
static uint16_t col;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void scroll_tick(uint8_t p25_state) {

    if(!did_init) {
      did_init=1;
      scroll_spr.createSprite(120, 160);
      scroll_spr.fillSprite(TFT_BLACK);
      scroll_spr.setTextColor(TFT_WHITE); // White text, no background
    }

    if(p25_state==0x00 || p25_state==P25_STATE_SYNC) return;

    ctime = millis();
    
    //if( ctime - prev_time > 5) {
      prev_time = ctime;

      if(++xpos == 15) {
        xpos=0;
        scroll_spr.setScrollRect(0, 0, 120, 160, TFT_BLACK); // Scroll the first 40 pixels
        scroll_spr.scroll(0,-8);     // scroll stext 0 pixel right, 1 up

        //scroll_spr.fillRect(0, 160-8, 120,8, TFT_BLACK); 
        //scroll_spr.pushSprite(0, 30);
      }
      else {
        scroll_spr.setScrollRect(0, 152, 120, 8, TFT_BLACK); // Scroll the first 40 pixels
        scroll_spr.scroll(-8,0);     // scroll stext 0 pixel right, 1 up
      }

      switch( p25_state) {
        case  0x00  :
          col = TFT_BLACK; //idle state
        break;

        case  P25_STATE_SYNC  :
          col = TFT_BLACK; 
        break;

        case  P25_STATE_TSBK  :
          col = TFT_BLUE; 
        break;
        
        case  P25_STATE_VOICE  :
          col = TFT_GREEN; 
        break;

        case  P25_STATE_TDULC  :
          col = TFT_BLACK; 
        break;

        case  P25_STATE_TDU  :
          col = TFT_BLACK; 
        break;

        case  P25_STATE_VGRANT  :
          col = TFT_CYAN; 
        break;

        case  P25_STATE_VERR1  :
          col = TFT_YELLOW; 
        break;

        case  P25_STATE_VERR2  :
          col = TFT_RED; 
        break;

        case  P25_STATE_TSBK_ERR  :
          col = TFT_RED; 
        break;
      }
      //scroll_spr.fillRect(120-8, 160-8, 8,8, rand()%0xffff);  //testing
      scroll_spr.fillRect(120-8, 160-8, 8,8, col);
      scroll_spr.pushSprite(0, 30);
    //}
}
