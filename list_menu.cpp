
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

static int choice=0;
static int max_choice=8;
static char *s1;
static char *s2;
static char *s3;
static char *s4;
static char *s5;
static char *s6;
static char *s7;
static char *s8;

static int FNT=4;

#define ST 5
#define INC 30 

#define ILI9341_BLACK       0x0000      /*   0,   0,   0 */
#define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
#define ILI9341_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define ILI9341_MAROON      0x7800      /* 128,   0,   0 */
#define ILI9341_PURPLE      0x780F      /* 128,   0, 128 */
#define ILI9341_OLIVE       0x7BE0      /* 128, 128,   0 */
#define ILI9341_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define ILI9341_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define ILI9341_BLUE        0x001F      /*   0,   0, 255 */
#define ILI9341_GREEN       0x07E0      /*   0, 255,   0 */
#define ILI9341_CYAN        0x07FF      /*   0, 255, 255 */
#define ILI9341_RED         0xF800      /* 255,   0,   0 */
#define ILI9341_MAGENTA     0xF81F      /* 255,   0, 255 */
#define ILI9341_YELLOW      0xFFE0      /* 255, 255,   0 */
#define ILI9341_WHITE       0xFFFF      /* 255, 255, 255 */
#define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define ILI9341_PINK        0xF81F

#define TFT_DARKBLUE        0x000F      /*   0,   0, 255 */
#define BG_COLOR TFT_DARKBLUE
#define FG_COLOR TFT_WHITE
#define HL_COLOR TFT_WHITE

void menu_set_font_size(int size) {
  FNT = size;
}

static void draw_menu(int idx) 
{
  if(idx==0) tft.setTextColor(BG_COLOR, HL_COLOR);
    else tft.setTextColor(FG_COLOR, BG_COLOR);

	tft.drawString(s1, 5, ST, FNT);

  if(idx==1) tft.setTextColor(BG_COLOR, HL_COLOR);
    else tft.setTextColor(FG_COLOR, BG_COLOR);
	tft.drawString(s2, 5, (ST+INC*1), FNT);

  if(idx==2) tft.setTextColor(BG_COLOR, HL_COLOR);
    else tft.setTextColor(FG_COLOR, BG_COLOR);
	tft.drawString(s3, 5, (ST+INC*2), FNT);

  if(idx==3) tft.setTextColor(BG_COLOR, HL_COLOR);
    else tft.setTextColor(FG_COLOR, BG_COLOR);
	tft.drawString(s4, 5, (ST+INC*3), FNT);

  if(idx==4) tft.setTextColor(BG_COLOR, HL_COLOR);
    else tft.setTextColor(FG_COLOR, BG_COLOR);
	tft.drawString(s5, 5, (ST+INC*4), FNT);

  if(idx==5) tft.setTextColor(BG_COLOR, HL_COLOR);
    else tft.setTextColor(FG_COLOR, BG_COLOR);
	tft.drawString(s6, 5, (ST+INC*5), FNT);

  if(idx==6) tft.setTextColor(BG_COLOR, HL_COLOR);
    else tft.setTextColor(FG_COLOR, BG_COLOR);
	tft.drawString(s7, 5, (ST+INC*6), FNT);

  if(idx==7) tft.setTextColor(BG_COLOR, HL_COLOR);
    else tft.setTextColor(FG_COLOR, BG_COLOR);
	tft.drawString(s8, 5, (ST+INC*7), FNT);

}

int get_menu_choice(int nchoices, char *str1, char *str2, char *str3, char *str4, char *str5, char *str6, char *str7, char *str8) 
{

  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);

  s1 = str1;
  s2 = str2;
  s3 = str3;
  s4 = str4;
  s5 = str5;
  s6 = str6;
  s7 = str7;
  s8 = str8;

  max_choice=nchoices;


  choice=0;
  int db_delay=50;
  int rep_delay=50;

    tft.fillScreen(BG_COLOR);  //background
    draw_menu(choice); 

    delay(db_delay);

    // keyboard operation
    while (1) {

      if (digitalRead(WIO_5S_UP) == LOW) {
        delay(db_delay);
        if (digitalRead(WIO_5S_UP) == LOW) {
          choice--;
          if(choice<0) choice=nchoices-1;

          draw_menu(choice);
          delay(rep_delay);
        }
      } else if (digitalRead(WIO_5S_DOWN) == LOW) {
        delay(db_delay);
        if (digitalRead(WIO_5S_DOWN) == LOW) {
          choice++;
          if(choice==max_choice) choice=0;

          draw_menu(choice);
          delay(rep_delay);
        }
      } else if (digitalRead(WIO_5S_LEFT) == LOW) {
        delay(db_delay);
        if (digitalRead(WIO_5S_LEFT) == LOW) {
          draw_menu(choice);
          delay(rep_delay);
        }
      } else if (digitalRead(WIO_5S_RIGHT) == LOW) {
        delay(db_delay);
        if (digitalRead(WIO_5S_RIGHT) == LOW) {
          draw_menu(choice);
          delay(rep_delay);
        }
      } else if (digitalRead(WIO_KEY_B) == LOW) {
        delay(db_delay);
        if (digitalRead(WIO_KEY_B) == LOW) {
          return -1; //cancel
        }
      } else if (digitalRead(WIO_5S_PRESS) == LOW) {
        delay(db_delay);
        if (digitalRead(WIO_5S_PRESS) == LOW) {
          break;
        }
      }

    }

    return choice;
}
