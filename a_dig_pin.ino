////
/* turned off
void DPinSetup()
{
//////////////////////////////////digital pins/////////////////
//d0 - d9
  pinMode( D0, OUTPUT ); // PIN13
  pinMode( D1, OUTPUT ); // PIN15
  pinMode( D2, OUTPUT ); // PIN16
  pinMode( D3, OUTPUT ); // PIN18
  pinMode( D4, OUTPUT ); // PIN22
  pinMode( D5, OUTPUT ); // PIN32
  pinMode( D6, OUTPUT ); // PIN33
  pinMode( D7, OUTPUT ); // PIN36
  pinMode( D8, OUTPUT ); // PIN37

  digitalWrite( D0, LOW );
  digitalWrite( D1, LOW );
  digitalWrite( D2, LOW );
  digitalWrite( D3, LOW );
  digitalWrite( D4, LOW );
  digitalWrite( D5, LOW );
  digitalWrite( D6, LOW );
  digitalWrite( D7, LOW );
  digitalWrite( D8, LOW );

  D0_Timer = millis();
  D1_Timer = millis();
  D2_Timer = millis();
  D3_Timer = millis();
  D4_Timer = millis();
  D5_Timer = millis();
  D6_Timer = millis();
  D7_Timer = millis();
  D8_Timer = millis();

  DPinHold = 30000; //30 sec hold time
  DPinTimeOut = 60000; // after 60 secs reset all pins low

  D0_State = false; // true D Pin is high
  D1_State = false; // true D Pin is high
  D2_State = false; // true D Pin is high
  D3_State = false; // true D Pin is high
  D4_State = false; // true D Pin is high
  D5_State = false; // true D Pin is high
  D6_State = false; // true D Pin is high
  D7_State = false; // true D Pin is high
  D8_State = false; // true D Pin is high

//  Serial.begin(115000);

///////////////////////////////////////////////////////////
}



void DPinHold_check()

{
  //Serial.println("DPHOLD CHECK ");

  unsigned long totaltime9 = ( millis() - Last_Timer );

  unsigned long totaltime5 = ( millis() - D5_Timer );
  unsigned long totaltime6 = ( millis() - D6_Timer );
  unsigned long totaltime7 = ( millis() - D7_Timer );
  unsigned long totaltime8 = ( millis() - D8_Timer );




  if( millis() - Last_Timer >= DPinTimeOut ) {
    digitalWrite( D0, LOW );
    digitalWrite( D1, LOW );
    digitalWrite( D2, LOW );
    digitalWrite( D3, LOW );
    digitalWrite( D4, LOW );
    digitalWrite( D5, LOW );
    digitalWrite( D6, LOW );
    digitalWrite( D7, LOW );
    digitalWrite( D8, LOW );

    D0_Timer = millis();
    D1_Timer = millis();
    D2_Timer = millis();
    D3_Timer = millis();
    D4_Timer = millis();
    D5_Timer = millis();
    D6_Timer = millis();
    D7_Timer = millis();
    D8_Timer = millis();

    D0_State = false; // true D Pin is high
    D1_State = false; // true D Pin is high
    D2_State = false; // true D Pin is high
    D3_State = false; // true D Pin is high
    D4_State = false; // true D Pin is high
    D5_State = false; // true D Pin is high
    D6_State = false; // true D Pin is high
    D7_State = false; // true D Pin is high
    D8_State = false; // true D Pin is high

    Serial.print( "All RESET : TIMER =" );
    Serial.println( totaltime9 );
    Last_Timer = millis();

  }


  if( D5_State == true )  {
    if( millis() - D5_Timer >= DPinHold ) {
      digitalWrite( D5, LOW );
      D5_State = false;
      Serial.print( " D5 reset!! e.t = " );
      Serial.println( totaltime5 );
      D5_Timer = millis();
      Serial.print( " D5 time now = " );
      Serial.print( D5_Timer );
      Serial.print( " D5 state = " );
      Serial.println( D5_State );
      tft.drawString( "                                ", 5, 215, 2 );
    }
  }

  if( D6_State == true )  {
    if( millis() - D6_Timer >= DPinHold ) {
      digitalWrite( D6, LOW );
      D6_State = false;
      Serial.print( " D6 reset!! e.t = " );
      Serial.println( totaltime6 );
      D6_Timer = millis();
      Serial.print( " D6 time now = " );
      Serial.print( D6_Timer );
      Serial.print( " D6 state = " );
      Serial.println( D6_State );
      tft.drawString( "                                ", 5, 215, 2 );
    }
  }

  if( D7_State == true )  {
    if( millis() - D7_Timer >= DPinHold ) {
      digitalWrite( D7, LOW );
      D7_State = false;
      Serial.print( " D7 reset!! e.t = " );
      Serial.println( totaltime7 );
      D7_Timer = millis();
      Serial.print( " D7 time now = " );
      Serial.print( D7_Timer );
      Serial.print( " D7 state = " );
      Serial.println( D7_State );
      tft.drawString( "                                ", 5, 215, 2 );
    }
  }

  if( D8_State == true )  {
    if( millis() - D8_Timer >= DPinHold ) {
      digitalWrite( D8, LOW );
      D8_State = false;
      Serial.print( " D8 reset!! e.t = " );
      Serial.println( totaltime8 );
      D8_Timer = millis();
      Serial.print( " D8 time now = " );
      Serial.print( D8_Timer );
      Serial.print( " D8 state = " );
      Serial.println( D8_State );
      tft.drawString( "                                ", 5, 215, 2 );
    }
  }

}

void DPinSprite()
{
/////////////// D0-D8 Status Sprite //////////////////////////

  spr.createSprite( 200, 27 ); //allocate sprite memory
  spr.fillSprite( mptr->col_def_bg ); //clear to black bground

  spr.setTextColor( mptr->col6, mptr->col_def_bg );
  // spr.setTextColor(TFT_BLACK, TFT_GREEN);

  if( D0_State == true ) {
    spr.fillCircle( 11, 13, 11, TFT_GREEN );
    spr.setTextColor( TFT_BLACK, TFT_GREEN );
    spr.drawString( "D0", 4, 5, 2 );
  } else {
    spr.setTextColor( mptr->col6, mptr->col_def_bg );
    spr.drawString( "D0", 4, 5, 2 );
  }
  if( D1_State == true ) {
    spr.fillCircle( 33, 13, 11, TFT_GREEN );
    spr.setTextColor( TFT_BLACK, TFT_GREEN );
    spr.drawString( "D1", 26, 5, 2 );
  } else {
    spr.setTextColor( mptr->col6, mptr->col_def_bg );
    spr.drawString( "D1", 26, 5, 2 );
  }
  if( D2_State == true ) {
    spr.fillCircle( 55, 13, 11, TFT_GREEN );
    spr.setTextColor( TFT_BLACK, TFT_GREEN );
    spr.drawString( "D2", 48, 5, 2 );
  } else {
    spr.setTextColor( mptr->col6, mptr->col_def_bg );
    spr.drawString( "D2", 48, 5, 2 );
  }
  if( D3_State == true ) {
    spr.fillCircle( 77, 13, 11, TFT_GREEN );
    spr.setTextColor( TFT_BLACK, TFT_GREEN );
    spr.drawString( "D3", 70, 5, 2 );
  } else {
    spr.setTextColor( mptr->col6, mptr->col_def_bg );
    spr.drawString( "D3", 70, 5, 2 );
  }
  if( D4_State == true ) {
    spr.fillCircle( 99, 13, 11, TFT_GREEN );
    spr.setTextColor( TFT_BLACK, TFT_GREEN );
    spr.drawString( "D4", 92, 5, 2 );
  } else {
    spr.setTextColor( mptr->col6, mptr->col_def_bg );
    spr.drawString( "D4", 92, 5, 2 );
  }
  if( D5_State == true ) {
    spr.fillCircle( 121, 13, 11, TFT_GREEN );
    spr.setTextColor( TFT_BLACK, TFT_GREEN );
    spr.drawString( "D5", 114, 5, 2 );
  } else {
    spr.setTextColor( mptr->col6, mptr->col_def_bg );
    spr.drawString( "D5", 114, 5, 2 );
  }
  if( D6_State == true ) {
    spr.fillCircle( 144, 13, 11, TFT_GREEN );
    spr.setTextColor( TFT_BLACK, TFT_GREEN );
    spr.drawString( "D6", 136, 5, 2 );
  } else {
    spr.setTextColor( mptr->col6, mptr->col_def_bg );
    spr.drawString( "D6", 136, 5, 2 );
  }
  if( D7_State == true ) {
    spr.fillCircle( 166, 13, 11, TFT_GREEN );
    spr.setTextColor( TFT_BLACK, TFT_GREEN );
    spr.drawString( "D7", 158, 5, 2 );
  } else {
    spr.setTextColor( mptr->col6, mptr->col_def_bg );
    spr.drawString( "D7", 158, 5, 2 );
  }
  if( D8_State == true ) {
    spr.fillCircle( 188, 13, 11, TFT_GREEN );
    spr.setTextColor( TFT_BLACK, TFT_GREEN );
    spr.drawString( "D8", 180, 5, 2 );
  } else {
    spr.setTextColor( mptr->col6, mptr->col_def_bg );
    spr.drawString( "D8", 180, 5, 2 );
  }

  spr.pushSprite( 2, 160 ); //transfer to lcd
  spr.deleteSprite(); //free memory

}

*/
