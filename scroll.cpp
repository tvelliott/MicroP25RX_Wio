
#include <Arduino.h>
#include "TFT_eSPI.h"
#include "Free_Fonts.h"
#include "metainfo.h"

extern TFT_eSPI tft;
extern TFT_eSprite scroll_spr;

static int did_init;

static int xpos;
static uint16_t col;

void draw_scroll_signals_page(uint8_t state);
void draw_scroll_mon_page(uint8_t state);

static int current_mode=-1;
extern bool TGLogScreen;  

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void scroll_tick(uint8_t p25_state) {

    if(p25_state==0x00 || p25_state==P25_STATE_SYNC) return;

    if(mptr->wio_button_mode==WIO_BUTTON_MODE_MONITOR && TGLogScreen) {

      if(!did_init || current_mode!=mptr->wio_button_mode) {
        if(did_init) scroll_spr.deleteSprite();
        scroll_spr.createSprite(90, 60);
        scroll_spr.fillSprite(TFT_BLACK);
        scroll_spr.setTextColor(TFT_WHITE); 
        current_mode = WIO_BUTTON_MODE_MONITOR;
        did_init=1;
        xpos=0;
      }

      draw_scroll_mon_page(p25_state);
    }
    else if(mptr->wio_button_mode==WIO_BUTTON_MODE_RF_GAIN) {

      if(!did_init || current_mode!=mptr->wio_button_mode) {
        if(did_init) scroll_spr.deleteSprite();
        scroll_spr.createSprite(120, 160);
        scroll_spr.fillSprite(TFT_BLACK);
        scroll_spr.setTextColor(TFT_WHITE); 
        current_mode = WIO_BUTTON_MODE_RF_GAIN;
        did_init=1;
        xpos=0;
      }

      draw_scroll_signals_page(p25_state);
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_scroll_mon_page(uint8_t p25_state) {
    if(++xpos >= 12) {
      xpos=0;
      scroll_spr.setScrollRect(0, 0, 96, 60, TFT_BLACK); 
      scroll_spr.scroll(0,-8);
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
        col = TFT_CYAN; 
      break;

      case  P25_STATE_TDU  :
        col = TFT_ORANGE; 
      break;

      case  P25_STATE_VGRANT  :
        col = TFT_PURPLE; 
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
    scroll_spr.fillRect(xpos*8, 60-8, 8,8, col);
    scroll_spr.pushSprite(120, 120);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_scroll_signals_page(uint8_t p25_state) {
    if(++xpos >= 15) {
      xpos=0;
      scroll_spr.setScrollRect(0, 0, 120, 160, TFT_BLACK); 
      scroll_spr.scroll(0,-8);     
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
        col = TFT_CYAN; 
      break;

      case  P25_STATE_TDU  :
        col = TFT_ORANGE; 
      break;

      case  P25_STATE_VGRANT  :
        col = TFT_PURPLE; 
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
    scroll_spr.fillRect(xpos*8, 160-8, 8,8, col);
    scroll_spr.pushSprite(0, 30);
}
