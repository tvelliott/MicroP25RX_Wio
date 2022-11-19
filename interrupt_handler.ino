

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



volatile char cmd_buf[256];
volatile uint8_t cmd_buf_idx;
volatile uint8_t cmd_buf_boff;
volatile uint8_t cmd_buf_off;
volatile uint8_t packet_id;

uint8_t right_but;
uint8_t left_but;
uint8_t press_but;
uint8_t up_but;
uint8_t down_but;
int left_but_pressed;
int right_but_pressed;
int press_but_pressed;
int down_but_pressed;
int up_but_pressed;
uint8_t A_but;
uint8_t B_but;
uint8_t C_but;
int A_but_pressed;
int B_but_pressed;
int C_but_pressed;

volatile int cmd_acked;


uint32_t b_button_press_time;
uint32_t c_button_press_time;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void clr_buttons() {
  right_but=0;
  left_but=0;
  press_but=0;
  up_but=0;
  down_but=0;
  left_but_pressed=0;
  right_but_pressed=0;
  press_but_pressed=0;
  down_but_pressed=0;
  up_but_pressed=0;
  A_but=0;
  B_but=0;
  C_but=0;
  A_but_pressed=0;
  B_but_pressed=0;
  C_but_pressed=0;
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
void clr_cmd() {
  __disable_irq();
  memset((char *)cmd_buf,0x00,sizeof(cmd_buf));
  cmd_acked=0;
  __enable_irq();
}

////////////////////////////////////////////////////////////////////////
// call this from loop()
////////////////////////////////////////////////////////////////////////
void check_buttons() {
   //button press
  if (right_but == 0xff ) {
    right_but_pressed = 1;
  }
  //button press
  if (left_but == 0xff) {
    left_but_pressed = 1;
  }
  //button press
  if (press_but == 0xff) {
    press_but_pressed = 1;
  }
  //button press
  if (up_but == 0xff) {
    up_but_pressed = 1;
  }
  //button press
  if (down_but == 0xff) {
    down_but_pressed = 1;
  }

  //pressed
  if (A_but == 0xff) {
    A_but_pressed = 1;
  }
  if (B_but == 0xff) {
    B_but_pressed = 1;
    if(b_button_press_time==0) b_button_press_time = millis();
  }
  if (C_but == 0xff) {
    C_but_pressed = 1;
    if(c_button_press_time==0) c_button_press_time = millis();
  }
}

//////////////////////////////////////////////////////////////////////////
// send command to teensy. The command will be continually repeated
// until the next command. The teensy uses the packet_id to drop the 
// repeats
//////////////////////////////////////////////////////////////////////////
void send_cmd(const char *cmd, int len) {

  if(cmd_acked) return;

  __disable_irq();

  cmd_acked=1;

  if(len > 48) len = 48;
  memset((uint8_t *) cmd_buf,0x00,256);
  
  cmd_buf[0] = 0x7f;
  cmd_buf[1] = 0x1c;
  cmd_buf[2] = 0xf1;
  cmd_buf[3] = 0x39; //sync word
  cmd_buf[4] = ++packet_id;

  memcpy((uint8_t *) &cmd_buf[5], cmd, len);
  
  crc32_val = ~0L;
  uint32_t crc = crc32_range( (unsigned char *) &cmd_buf[0], 60  );
  memcpy((uint8_t *) &cmd_buf[61], (uint8_t *) &crc32_val, 4);
  cmd_buf_idx=0;
  cmd_buf_boff=0;
  cmd_buf_off=0;
  
  __enable_irq();
}
////////////////////////////////////////////////////////////////////////////////////////
//clock out commands on the SPI_SS signal being used as a clock from the Teensy 4.1
// ISR handler
////////////////////////////////////////////////////////////////////////////////////////
void spi_miso_int(void) {
  //sclk_count++;
  //if(sclk_count&0x01) digitalWrite(PIN_SPI_MISO,1);
  //  else digitalWrite(PIN_SPI_MISO,0);
  
  if( (cmd_buf[cmd_buf_idx]<<cmd_buf_boff++ & 0x80)==0x80 ) {
   digitalWrite(PIN_SPI_MISO,1);
  }
  else {
   digitalWrite(PIN_SPI_MISO,0);
  }
  if(cmd_buf_boff==8) {
    cmd_buf_boff=0;
    cmd_buf_idx++;
  }
	if(cmd_buf_idx>=65) cmd_buf_idx=0;

  sclk_count++; //debugging

    //debounce without delay
  left_but <<= 1;
  right_but <<= 1;
  press_but <<= 1;
  up_but <<= 1;
  down_but <<= 1;
  if (digitalRead(WIO_5S_UP) == LOW) {
		up_but |= 1;
  }
  else if (digitalRead(WIO_5S_DOWN) == LOW) {
		down_but |= 1;
  }
  else if (digitalRead(WIO_5S_LEFT) == LOW) {
    left_but |= 1;
  }
  else if (digitalRead(WIO_5S_RIGHT) == LOW) {
    right_but |= 1;
  }
  else if (digitalRead(WIO_5S_PRESS) == LOW) {
    press_but |= 1;
  }

  
  A_but <<= 1;
  B_but <<= 1;
  C_but <<= 1;
  if (digitalRead(WIO_KEY_A) == LOW) {
    A_but |= 1;
  }
  else if (digitalRead(WIO_KEY_B) == LOW) {
    B_but |= 1;
  }
  else if (digitalRead(WIO_KEY_C) == LOW) {
    C_but |= 1;
  }
  
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void SERCOM5_0_Handler() {
 SERCOM5->SPI.DATA.reg = 0x55;
}

void SERCOM5_1_Handler() {
  // SPI Data Transmit Complete
  SERCOM5->SPI.INTFLAG.bit.TXC = 1; //clear transmit complete interrupt
}

void SERCOM5_2_Handler() {
  SERCOM5->SPI.INTFLAG.bit.RXC = 1; //clear receive complete interrupt

  uint8_t val = (uint8_t) SERCOM5->SPI.DATA.reg;  //this clears the interrupt

  if (buf_idx > sizeof(metainfo)) {
    buf_idx = 0;
    spi_state = 0;
  }
  if (do_draw_rx == 0) {
    buf[buf_idx++] = val;
  }

  if ( do_draw_rx == 0 && spi_state == 0 && val == 0x53 ) {
    spi_state++;
    buf_idx = 0;
    buf[buf_idx++] = val;
  }
  else if (spi_state == 1 && val == 0xd1) spi_state++;
  else if (spi_state == 2 && val == 0x74) spi_state++;
  else if (spi_state == 3 && val == 0x98) {

    spi_state++;
  }
  else if (spi_state == 4) {
    meta_port = val;
    spi_state++;
  }
  else if (spi_state == 5) {
    meta_port |= val << 8;
    spi_state++;
  }
  else if (spi_state == 6) {
    meta_len = val;
    spi_state++;
  }
  else if (spi_state == 7) {
    meta_len |= val << 8;
    spi_state++;
  }
  else if (spi_state == 8) {
    if (meta_port == 8893 && meta_len < sizeof(buf)) {
      spi_state++;
    }
    else {
      spi_state = 0;
    }
  }
  else if (spi_state == 9) {
    if ( buf_idx == meta_len) {
      spi_state = 0;
      buf_idx = 0;
      do_draw_rx = 1;

    }
  }

}

void SERCOM5_3_Handler() {
  // SSL Interupt
  SERCOM5->SPI.INTFLAG.bit.SSL = 1; //clear slave select interrupt
  //spi_state = 0;
  //buf_idx = 0;
}
void SERCOM5_7_Handler() {
  SERCOM5->SPI.INTFLAG.bit.ERROR = 1;
}
