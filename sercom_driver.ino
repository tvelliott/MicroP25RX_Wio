
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



#include "wiring_private.h"

void initSPI() {
  //SERCOM5->SPI.CTRLA.bit.ENABLE = 0; //disable
  //while (SERCOM5->SPI.SYNCBUSY.bit.ENABLE==1);

  //SERCOM5->SPI.CTRLA.bit.SWRST = 1; //sw reset
  //while (SERCOM5->SPI.CTRLA.bit.SWRST);


	//detachInterrupt(digitalPinToInterrupt(PIN_SPI_SCK));

  //see wio_terminal/variant.h
  //pinPeripheral(PIN_SPI_MISO, PIO_SERCOM_ALT);
  pinPeripheral(PIN_SPI_MOSI, PIO_SERCOM_ALT);
  pinPeripheral(PIN_SPI_SCK, PIO_SERCOM_ALT);
  pinPeripheral(PIN_SPI_SS, PIO_SERCOM_ALT);

/*
  PORT->Group[PORTB].PINCFG[0].bit.PMUXEN = 0x1; //Enable Peripheral Multiplexing for SERCOM5 SPI PB00
  PORT->Group[PORTB].PMUX[0].bit.PMUXE = 0x3; //SERCOM 5 is selected for peripheral use of this pad (0x3 selects peripheral function D: SERCOM)
  PORT->Group[PORTB].PINCFG[1].bit.PMUXEN = 0x1;
  PORT->Group[PORTB].PMUX[0].bit.PMUXO = 0x3;
  PORT->Group[PORTB].PINCFG[2].bit.PMUXEN = 0x1;
  PORT->Group[PORTB].PMUX[1].bit.PMUXE = 0x3;
  PORT->Group[PORTB].PINCFG[3].bit.PMUXEN = 0x1;
  PORT->Group[PORTB].PMUX[1].bit.PMUXO = 0x3;
  */



  SERCOM5->SPI.CTRLA.bit.DORD = 0; //MSB first
  SERCOM5->SPI.CTRLA.bit.CPOL = 0; //SCK is low when idle, leading edge is rising edge
  SERCOM5->SPI.CTRLA.bit.CPHA = 1; //data sampled on leading sck edge and changed on a trailing sck edge
  SERCOM5->SPI.CTRLA.bit.FORM = 0x0; //Frame format = SPI
  SERCOM5->SPI.CTRLA.bit.MODE = 0x2; // set SPI Operating Mode to slave
  SERCOM5->SPI.CTRLA.bit.DIPO = 0; //DATA PAD MOSI is used as input (slave mode)
  SERCOM5->SPI.CTRLA.bit.DOPO = 2; //DATA PAD MISO is used as output (slave mode) DOESN'T WORK!!!!
  SERCOM5->SPI.CTRLA.bit.IBON = 0x1; //Buffer Overflow notification
  SERCOM5->SPI.CTRLA.bit.RUNSTDBY = 1; //wake on receiver complete


  SERCOM5->SPI.CTRLB.bit.SSDE = 0; //Slave Select Detection Enabled
  SERCOM5->SPI.CTRLB.bit.CHSIZE = 0; //character size 8 Bit
  SERCOM5->SPI.CTRLB.bit.MSSEN = 0;

  //Set up SPI interrupts
  SERCOM5->SPI.INTENSET.bit.SSL = 1; //Enable Slave Select low interrupt
  SERCOM5->SPI.INTENSET.bit.RXC = 1; //Receive complete interrupt
  SERCOM5->SPI.INTENSET.bit.TXC = 1;
  SERCOM5->SPI.INTENSET.bit.ERROR = 0; //
  SERCOM5->SPI.INTENSET.bit.DRE = 0;


  //Setting up NVIC
  //NVIC_EnableIRQ(SERCOM5_0_IRQn);
  //NVIC_SetPriority(SERCOM5_0_IRQn,2);

  NVIC_EnableIRQ(SERCOM5_1_IRQn);
  NVIC_SetPriority(SERCOM5_1_IRQn, 2);

  NVIC_EnableIRQ(SERCOM5_2_IRQn);
  NVIC_SetPriority(SERCOM5_2_IRQn, 2);

  NVIC_EnableIRQ(SERCOM5_3_IRQn);
  NVIC_SetPriority(SERCOM5_3_IRQn, 2);



  //Enable SPI
  SERCOM5->SPI.CTRLA.bit.ENABLE = 1;
  //while (SERCOM5->SPI.SYNCBUSY.bit.ENABLE);

  SERCOM5->SPI.CTRLB.bit.RXEN = 0x1; //Enable Receiver, this is done here due to errate issue
  //while (SERCOM5->SPI.SYNCBUSY.bit.CTRLB); //wait until receiver is enabled
}

void deInitSPI() {

  SERCOM5->SPI.CTRLB.bit.RXEN = 0; //disable
  return;

  SERCOM5->SPI.CTRLA.bit.ENABLE = 0; //disable
  while (!SERCOM5->SPI.SYNCBUSY.bit.ENABLE);

  
  SERCOM5->SPI.CTRLB.bit.RXEN = 0; //disable

  NVIC_DisableIRQ(SERCOM5_0_IRQn);
  NVIC_DisableIRQ(SERCOM5_1_IRQn);
  NVIC_DisableIRQ(SERCOM5_2_IRQn);
  NVIC_DisableIRQ(SERCOM5_3_IRQn);

  
  PORT->Group[PORTB].PINCFG[0].bit.PMUXEN = 0x0; //Enable Peripheral Multiplexing for SERCOM5 SPI PB00
  PORT->Group[PORTB].PMUX[0].bit.PMUXE = 0x0; //SERCOM 5 is selected for peripheral use of this pad (0x3 selects peripheral function D: SERCOM)
  PORT->Group[PORTB].PINCFG[1].bit.PMUXEN = 0x0;
  PORT->Group[PORTB].PMUX[0].bit.PMUXO = 0x0;
  PORT->Group[PORTB].PINCFG[2].bit.PMUXEN = 0x0;
  PORT->Group[PORTB].PMUX[1].bit.PMUXE = 0x0;
  PORT->Group[PORTB].PINCFG[3].bit.PMUXEN = 0x0;
  PORT->Group[PORTB].PMUX[1].bit.PMUXO = 0x0;


  //SERCOM5->SPI.CTRLA.bit.SWRST = 1; //sw reset
  //while (SERCOM5->SPI.CTRLA.bit.SWRST);


  //Set up SPI interrupts
  SERCOM5->SPI.INTENSET.bit.SSL = 0; //Enable Slave Select low interrupt
  SERCOM5->SPI.INTENSET.bit.RXC = 0; //Receive complete interrupt
  SERCOM5->SPI.INTENSET.bit.TXC = 0;
  SERCOM5->SPI.INTENSET.bit.ERROR = 0; //
  SERCOM5->SPI.INTENSET.bit.DRE = 0;


      spi_state = 0;
      buf_idx = 0;
      do_draw_rx = 0;

  /*
  SERCOM5->SPI.INTFLAG.bit.TXC=1;
  SERCOM5->SPI.INTFLAG.bit.RXC=1;
  SERCOM5->SPI.INTFLAG.bit.SSL=1;
  SERCOM5->SPI.INTFLAG.bit.ERROR=1;
  SERCOM5->SPI.INTFLAG.bit.DRE=1;
  */
  

	//attachInterrupt(digitalPinToInterrupt(PIN_SPI_SCK), spi_miso_int, FALLING);
}
