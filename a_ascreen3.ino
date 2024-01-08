
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// We received a new metainfo structure. Time to redraw the screen 3
///////////////////////////////////////////////////////////////////////
void Screen_3( void )
{

  if( do_draw_rx && mptr->layout == 3 ) {

    __disable_irq();
    do_draw_rx = 0;
    memcpy( ( uint8_t * ) &minfo, ( uint8_t * ) buf, sizeof( metainfo ) );
    __enable_irq();

    metainfo *mptr = ( metainfo * ) &minfo;
    crc32_val = ~0L;


    uint32_t mi_crc = crc32_range( ( unsigned char * ) &minfo, sizeof( metainfo ) - 8 );

    //validate the structure with 32-bit crc
    if( mptr->crc_val == mi_crc && mptr->port == 8893 ) { //8893=metainfo structure
      rx_count++;

      tgzone = mptr->tgzone;

      if( mptr->data_type == META_DATA_TYPE_ZONEINFO ) {
        memcpy( &tg_zones[0][0], &mptr->data[0], 112 ); //tg zones 7 x 16
      }
      if( mptr->data_type == META_DATA_TYPE_META_CONFIG_INFO ) {
        memcpy( ( char * ) &minfo_config, ( char * ) &mptr->data[0], sizeof( meta_config_info ) );
      }

      memcpy( ( void * )&minfo_verified, ( void * )&minfo, sizeof( metainfo ) );

      demod = mptr->use_demod;
      follow = mptr->follow;
      mute = mptr->audio_mute;
      learn = mptr->learn_mode;
      inc_in_scan = mptr->inc_in_scan;


      extern uint8_t packet_id;
      if( packet_id == mptr->wio_packet_id ) {
        //the teensy digital board received the last command
        //sent with send_cmd( *str, int len)

        __disable_irq();
        cmd_acked = 0;
        clr_cmd(); //clear the command buffer
        __enable_irq();
      }



      if( current_button_mode != WIO_BUTTON_MODE_RF_GAIN ) {
        force_scan = 0;
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
      if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 &&
          mptr->roaming == 1 && mptr->voice_tg_timeout == 0 && ( millis() - roam_time > mptr->roaming_timeout ) ) { //ROAMING=1 // scanning

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      }
      //ROAMING MODE 2
      else if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 && ( millis() - roam_time > mptr->roaming_timeout ) &&
               mptr->roaming == 2 && ( mptr->evm_p > 10 || mptr->rssi_f < -115 ) ) { //ROAMING=2 // auto-switch-over-on-lost-sig, P+S ALL systems

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      }
      //ROAMING MODE 3
      else if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 && ( millis() - roam_time > mptr->roaming_timeout ) &&
               mptr->roaming == 3 && ( mptr->evm_p > 10 || mptr->rssi_f < -115 ) ) { //ROAMING=3 // P+S auto-switch-over-on-lost-sig, SINGLE SYSTEM

        roam_time = millis();
        char cmd_str[32];
        sprintf( cmd_str, "nfreq %05X %03X\r\n", mptr->roam_wacn, mptr->roam_sysid ); //roam mode=3 needs wacn and sysid arguments
        send_cmd( cmd_str, 15 );
      }
      //ROAMING MODE 4
      else if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 && ( millis() - roam_time > mptr->roaming_timeout ) &&
               mptr->roaming == 4 && ( mptr->evm_p > 10 || mptr->rssi_f < -115 ) ) { //ROAMING=4 // auto-switch-over-on-lost-sig, P+S+A ALL systems

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      }
      //ROAMING MODE 5
      else if( cmd_acked == 0 && ( force_scan || current_button_mode == WIO_BUTTON_MODE_MONITOR ) && follow == 0 &&
               mptr->roaming == 5 && mptr->voice_tg_timeout == 0 && ( millis() - roam_time > mptr->roaming_timeout ) ) { //ROAMING=5 // scanning, INC_IN_SCAN=1

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      } else if( mptr->voice_tg_timeout ) {
        roam_time = millis(); //hang around ater the conversation too
      }



      //check if the button / display mode changed
      current_button_mode = mptr->wio_button_mode;

      if( current_button_mode != prev_button_mode ) {
        clr_screen();
        clr_buttons();

        did_save = 0;
        b_button_press_time = 0;
        c_button_press_time = 0;
      }

      prev_button_mode = current_button_mode;




      if( current_button_mode == WIO_BUTTON_MODE_TG_ZONE ) {
        __disable_irq();
        do_draw_rx = 0;
        memcpy( ( uint8_t * ) &minfo, ( uint8_t * ) buf, sizeof( metainfo ) );
        __enable_irq();

        draw_button_modes();
        draw_tg_zones();
        goto draw_end2;  //it really is ok to use goto. don't worry about it.
      } else if( current_button_mode == WIO_BUTTON_MODE_RF_GAIN ) {

        int y_offset = 15;

        draw_button_modes();

        //////////////////////////////////////////////////////////
        //Draw FFT
        //////////////////////////////////////////////////////////
        __disable_irq();
        do_draw_rx = 0;
        memcpy( ( uint8_t * ) fft_data, ( uint8_t * ) mptr->data, FFT_M );
        __enable_irq();

///////////////////////////////////////////////////////////////////////////////////// Signal and Gain sprite ////////////////////////////////////////////
        FNT = 2;
        spr.createSprite( 170, 20 ); //allocate memory for 80 x 80 sprite
        spr.fillSprite( TFT_BLACK );
        _lna_gain = mptr->lna_gain;
        if( _lna_gain < 0 ) _lna_gain = 0;
        if( _lna_gain > 16 ) _lna_gain = 16;

        if( _lna_gain < 16 ) {
          spr.setTextColor( TFT_WHITE, TFT_BLACK );
          if( gain_select == 0 ) sprintf( disp_buf, "> %02d LNA", _lna_gain );
          else sprintf( disp_buf, "%02d LNA", _lna_gain );
          spr.drawString( disp_buf, 0, 0, FNT );
          spr.fillRect( 60, 3, 7 * _lna_gain, 7, TFT_WHITE );
        } else {
          spr.setTextColor( TFT_GREEN, TFT_BLACK );
          if( gain_select == 0 ) sprintf( disp_buf, "> LNA AUTO" );
          else sprintf( disp_buf, "LNA AUTO" );
          spr.drawString( disp_buf, 0, 0, FNT );
        }
        spr.pushSprite( 140, 50 + y_offset ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory

        spr.createSprite( 170, 20 ); //allocate memory for 80 x 80 sprite
        _mixer_gain = mptr->mixer_gain;
        if( _mixer_gain < 0 ) _mixer_gain = 0;
        if( _mixer_gain > 16 ) _mixer_gain = 16;
        if( _mixer_gain < 16 ) {
          spr.setTextColor( TFT_WHITE, TFT_BLACK );
          if( gain_select == 1 ) sprintf( disp_buf, "> %02d MIX", _mixer_gain );
          else sprintf( disp_buf, "%02d MIX", _mixer_gain );
          spr.drawString( disp_buf, 0, 0, FNT );
          spr.fillRect( 60, 3, 7 * _mixer_gain, 7, TFT_WHITE );
        } else {
          spr.setTextColor( TFT_GREEN, TFT_BLACK );
          if( gain_select == 1 ) sprintf( disp_buf, "> MIX AUTO" );
          else sprintf( disp_buf, "MIX AUTO" );
          spr.drawString( disp_buf, 0, 0, FNT );
        }
        spr.pushSprite( 140, 70 + y_offset ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory

        spr.createSprite( 170, 20 ); //allocate memory for 80 x 80 sprite
        _vga_gain = mptr->vga_gain;
        if( _vga_gain < 0 ) _vga_gain = 0;

        is_vga_auto = ( ( _vga_gain & 0x80 ) != 0 );
        _vga_gain &= 0x0f;

        if( !is_vga_auto ) {
          spr.setTextColor( TFT_WHITE, TFT_BLACK );
          if( gain_select == 2 ) sprintf( disp_buf, "> %02d VGA", _vga_gain );
          else sprintf( disp_buf, "%02d VGA", _vga_gain );
          spr.drawString( disp_buf, 0, 0, FNT );
          spr.fillRect( 60, 3, 7 * _vga_gain, 7, TFT_WHITE );
        } else {
          spr.setTextColor( TFT_GREEN, TFT_BLACK );
          //if(gain_select==2) sprintf(disp_buf,"> VGA AUTO %d dB", _vga_gain*3);
          //else sprintf(disp_buf,"VGA AUTO %d dB", _vga_gain*3);
          if( gain_select == 2 ) sprintf( disp_buf, "> VGA AUTO %d  ", _vga_gain );
          else sprintf( disp_buf, "VGA AUTO %d  ", _vga_gain );
          spr.drawString( disp_buf, 0, 0, FNT );
        }
        spr.pushSprite( 140, 90 + y_offset ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory



#if 0
        spr.createSprite( 90, 20 ); //allocate memory for 80 x 80 sprite
        if( gain_select == 3 ) sprintf( disp_buf, "> SAVE GAINS" );
        else sprintf( disp_buf, "SAVE GAINS" );
        spr.drawString( disp_buf, 0, 0, FNT );
        spr.pushSprite( 140, 110 + y_offset ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory
#endif

        FNT = 2;
        tft.setTextColor( TFT_LIGHTGREY, TFT_BLACK );
        char demod_str[8];
        demod_str[0] = 0;
        if( mptr->use_demod == 0 ) strcpy( demod_str, "LSM" );
        if( mptr->use_demod == 1 ) strcpy( demod_str, "FM" );
        if( mptr->use_demod == 2 ) strcpy( demod_str, "FMNB" );
        if( mptr->use_demod == 3 ) strcpy( demod_str, "AM" );
        if( mptr->use_demod == 4 ) strcpy( demod_str, "AM+AGC" );

        sprintf( disp_buf, "DEMOD %s   ", demod_str );
        tft.drawString( disp_buf, 140, 110 + y_offset, FNT );


        FNT = 4;
        tft.setFreeFont( NULL );
        tft.setTextColor( TFT_WHITE, TFT_BLACK );

        sprintf( disp_buf, "EVM %3.1f  RSSI %3.1f dBm    ", mptr->evm_p, mptr->rssi_f );
        tft.drawString( disp_buf, 5, 210, FNT );

        FNT = 2;
        snprintf( disp_buf, 32, "FREQ: %3.6f MHz", mptr->current_freq );
        tft.drawString( disp_buf, 140, 0 + y_offset, FNT );

        if( mptr->decoder <= 1 ) { //p25 and dmr
          if( mptr->on_control_b && mptr->total_session_time > 1500 ) {
            sprintf( disp_buf, "TSBK/SEC %u      ", mptr->tsbk_sec );
          } else {
            sprintf( disp_buf, "ERATE %1.3f      ", mptr->erate );
          }
          tft.drawString( disp_buf, 140, 15 + y_offset, FNT );
        }

        sprintf( disp_buf, "SITE %d, RFSS %d     ", mptr->site_id, mptr->rf_id );
        tft.drawString( disp_buf, 140, 30 + y_offset, FNT );

        goto draw_end2;  //it really is ok to use goto. don't worry about it.
      } //////////////////////////////////////////////////////////////////////////////////////////////// End RF Gain sprites


      //FONT SIZE 1,2,4,8
      FNT = 4;

      tft.setFreeFont( NULL );
      tft.setTextColor( mptr->col_def_fg, mptr->col_def_bg );

      if( prev_freq != mptr->current_freq ) {
        freq_changed = 1;

      }
      prev_freq = mptr->current_freq;


      tft.setTextColor( mptr->col2, mptr->col_def_bg );

      //if( !mptr->do_wio_lines ) {
      if( mptr->tg_s != prev_tgs ) {
        //clear_line2();
      }
      prev_tgs = mptr->tg_s;


      if( mptr->tg_s <= 0 ) {
//       snprintf( disp_buf2, 32, " %3.6f MHz", mptr->current_freq );   // changes for rid on line 2 - remove "FREQ:" text

        if( strncmp( ( char * )disp_buf2, ( char * )line3_str, 31 ) != 0 ) clear_line3();
        strncpy( line3_str, disp_buf2, 31 );

        if( !mptr->do_wio_lines && follow > 0 ) {
          snprintf( disp_buf, 32, "HOLDING ON TG %d", follow );
          if( strncmp( ( char * )disp_buf, ( char * )line2_str, 31 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 31 );
          tft.drawString( disp_buf, 5, 40, FNT );
        }
        if( !mptr->do_wio_lines && follow == 0 ) {
          snprintf( disp_buf, 32, "   " );
          mptr->desc[19] = 0;
          if( strncmp( ( char * )disp_buf, ( char * )line2_str, 31 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 31 );
        }
      } else {

//////
//        if( !mptr->do_wio_lines ) {
        mptr->desc[19] = 0; //  sets NULL at char position 19?
//             }

        snprintf( disp_buf2, 24, "%s", mptr->desc ); /////////////////////////////////////////////////////Line 3 TG description
        if( strncmp( ( char * )disp_buf2, ( char * )line3_str, 31 ) != 0 ) {
          clear_line3();
          tft.setTextColor( mptr->col3, mptr->col_def_bg ); // set to line 3 color
          tft.drawString( disp_buf2, 5, 70, FNT );

          snprintf( tglog_buf, 32, "%s  %d  ", disp_buf2, mptr->tg_s ); // send desc and tg number to buf
          // move the TG log down
          strncpy( TGlog8, TGlog7, 32 );
          strncpy( TGlog7, TGlog6, 32 );
          strncpy( TGlog6, TGlog5, 32 );
          strncpy( TGlog5, TGlog4, 32 );
          strncpy( TGlog4, TGlog3, 32 );
          strncpy( TGlog3, TGlog2, 32 );
          strncpy( TGlog2, TGlog1, 32 );
          strncpy( TGlog1, tglog_buf, 32 );
          sprintf( tglog_buf, " " ); // clear buf

        } //

        strncpy( line3_str, disp_buf2, 31 );
        sprintf( disp_buf2, "                              " );
        disp_buf2[0] = {0};
      }


      FNT = 4;
      tft.setTextColor( mptr->col4, mptr->col_def_bg ); // color line 4

      uint8_t nac_lock_str = 'U';
      if( mptr->lock_nac ) nac_lock_str = 'L';

      if( mptr->decoder <= 1 ) { //p25 and dmr


        if( mptr->on_control_b > 0 ) { // if on CCH

          snprintf( disp_buf, 32, " CCH  %3.6f ", mptr->current_freq );


          if( strcmp( disp_buf, line6_str ) != 0 )  {
            tft.setTextColor( mptr->col6, mptr->col_def_bg );
            tft.drawString( disp_buf, 5, 160, FNT );
          }
          strncpy( line6_str, disp_buf, 31 );

          char demod_str[8];
          demod_str[0] = 0;
          if( mptr->use_demod == 0 ) strcpy( demod_str, "LSM" );
          if( mptr->use_demod == 1 ) strcpy( demod_str, "FM" );
          disp_buf[0] = '\0';


////////// use sprite on line 6
          spr.createSprite( 60, 20 ); //allocate memory
          FNT = 2;
          spr.fillSprite( mptr->col_def_bg );
          spr.setTextColor( mptr->col6, mptr->col_def_bg );
          sprintf( disp_buf, "dBm %3.0f", mptr->rssi_f );     // rssi display
          spr.drawString( disp_buf, 0, 5, FNT );
          spr.pushSprite( 260, 160 ); //send to lcd. upper left corner of sprite
          spr.deleteSprite();  //free memory

          disp_buf[0] = '\0';
        } else if( mptr->on_control_b < 1 )  { // if not on a CCH this shows nothing on line 6
          sprintf( disp_buf, "" );

          if( strcmp( disp_buf, line6_str ) != 0 )  {
            tft.drawString( disp_buf, 5, 160, FNT );
          }

          strncpy( line6_str, disp_buf, 31 );
          sprintf( disp_buf, "" );
          disp_buf[0] = {0};
          clear_line6();

        }

      }

      snprintf( disp_buf, 32, " %s", mptr->alpha ); // TG alphatag on line 4
      if( strcmp( disp_buf, line4_str ) != 0 ) {
        clear_line4();

        tft.drawString( disp_buf, 5, 99, FNT ); // move up from 100 to 99
      }
      strcpy( line4_str, disp_buf );
      strcpy( disp_buf, "                              " );
      disp_buf[0] = {0};
      char wioZone [35]; // used for selected Zones displayed on line 7


      wioZone[0] = '\0';
      if( bitRead( mptr->tgzone, 0 ) == 1 )  {
        strcat( wioZone, "1" ); // lsb of the tgzone meta
      }
      if( bitRead( mptr->tgzone, 1 ) == 1 )  {
        strcat( wioZone, " 2" );
      }
      if( bitRead( mptr->tgzone, 2 ) == 1 )  {
        strcat( wioZone, " 3" );
      }
      if( bitRead( mptr->tgzone, 3 ) == 1 )  {
        strcat( wioZone, " 4" );
      }
      if( bitRead( mptr->tgzone, 4 ) == 1 )  {
        strcat( wioZone, " 5" );
      }
      if( bitRead( mptr->tgzone, 5 ) == 1 )  {
        strcat( wioZone, " 6" );
      }
      if( bitRead( mptr->tgzone, 6 ) == 1 )  {
        strcat( wioZone, " 7" );
      }
      if( bitRead( mptr->tgzone, 7 ) == 1 )  {
        strcat( wioZone, " 8" );
      }
      if( bitRead( mptr->tgzone, 8 ) == 1 )  {
        strcat( wioZone, " 9" );
      }
      if( bitRead( mptr->tgzone, 9 ) == 1 )  {
        strcat( wioZone, " 10" );
      }
      if( bitRead( mptr->tgzone, 10 ) == 1 ) {
        strcat( wioZone, " 11" );
      }
      if( bitRead( mptr->tgzone, 11 ) == 1 ) {
        strcat( wioZone, " 12" );
      }
      if( bitRead( mptr->tgzone, 12 ) == 1 ) {
        strcat( wioZone, " 13" );
      }
      if( bitRead( mptr->tgzone, 13 ) == 1 ) {
        strcat( wioZone, " 14" );
      }
      if( bitRead( mptr->tgzone, 14 ) == 1 ) {
        strcat( wioZone, " 15" );
      }
      if( bitRead( mptr->tgzone, 15 ) == 1 ) {
        strcat( wioZone, " 16" );
      }




      if( strcmp( wioZone, line7_str ) != 0 ) {

        clear_line7();

        if( strlen( wioZone ) >= 18 ) {
          FNT = 2;
          sprintf( disp_buf, "Z%s",  wioZone );
        } else {
          FNT = 4;
          sprintf( disp_buf, "ZONE %s",  wioZone );
        }
        tft.setTextColor( mptr->col7, mptr->col_def_bg );
        tft.drawString( disp_buf, 5, 190, FNT );

        strcpy( line7_str, wioZone );
//        disp_buf[0]='\0';
        disp_buf[0] = {0};
      }


      FNT = 4;
      tft.setTextColor( mptr->col5, mptr->col_def_bg );


      if( mptr->tg_s > 0 ) snprintf( disp_buf, 32, " TG %d", mptr->tg_s ); // TG number on line 5 if TG number is greater than 0

      if( strcmp( disp_buf, line5_str ) != 0 ) {

        clear_line5();

        tft.drawString( disp_buf, 5, 130, FNT );
        strcpy( line5_str, disp_buf );
        sprintf( disp_buf, "                              " );
        disp_buf[0] = {0};
      }


      tft.setTextColor( mptr->col6, mptr->col_def_bg );
      if( strcmp( disp_buf, line6_str ) != 0 ) {

      }


      tft.setTextColor( mptr->col7, mptr->col_def_bg );


      #if 0
      if( strcmp( disp_buf, line7_str ) != 0 ) { //does nothing
      }
      strcpy( line7_str, disp_buf );
      #endif

      FNT = 2;
      memset( disp_buf, 0x00, sizeof( disp_buf ) );
      tft.setTextColor( mptr->col8, mptr->col_def_bg );
      //   sprintf( disp_buf, "NCO1 %3.3f, NCO2 %3.2f, EVM %3.1f    ", mptr->nco_offset, mptr->loop_freq, mptr->evm_p );
      if( mptr->on_control_b == 0 && mptr->RID != 0 && mptr->wio_line2[0] == 0 ) {


        sprintf( disp_buf, "                      " );
        disp_buf[0] = {0};
      } else { // line8  showing mode/roam status
        if( current_button_mode == WIO_BUTTON_MODE_CONFIG ) {
          draw_button_modes(); //<<< added
          if( mptr->roaming ) {
            sprintf( disp_buf, "CONFIG  ROAM-PAUSE" );
          } else {
            sprintf( disp_buf, "MODE: CONFIG          " );
          }
        } else if( current_button_mode == WIO_BUTTON_MODE_MONITOR ) {
          draw_button_modes(); //<<< added


          if( mptr->hw_type < HW_TYPE_EAGLE_HH ) {
            if( mptr->roaming ) {
              //  sprintf( disp_buf, "MONITOR ROAM-ON-%u Free %u", mptr->roaming, freeMemory() );
              sprintf( disp_buf, "MONITOR ROAM-ON-%u ", mptr->roaming );
            } else {
              //  sprintf( disp_buf, "MONITOR RM-OFF %s TSBK %u dBm %3.0f", demod_str, mptr->tsbk_sec, mptr->rssi_f );
              sprintf( disp_buf, "MONITOR ROAM-OFF" );
              // sprintf( disp_buf, "MONITOR - MEM %u         ", freeMemory() ); // <<<<<<<<<<<<
            }
          } else {
            if( mptr->roaming ) {
              sprintf( disp_buf, "MONITOR ROAM-ON-%u BAT %1.2fV, GC%u, PD%u", mptr->roaming, mptr->bat_volt_f, mptr->gain_controller, mptr->peak_det + 1 ); // removed the space after "GC" and "PD". The PD number was not cleared on display.
            } else {
              // sprintf( disp_buf, "MONITOR ROAM-OFF" );
              sprintf( disp_buf, "MONITOR - BAT %1.2fV, GC %u, PD %u        ", mptr->bat_volt_f, mptr->gain_controller, mptr->peak_det + 1 ); // <<<<<<<<<<<<
            }
          }


        } else {
          sprintf( disp_buf, " " );
        }
      }
      if( strcmp( disp_buf, line8_str ) != 0 ) {
        tft.setTextColor( mptr->col8, mptr->col_def_bg );
        strcpy( line8_str, disp_buf );
        clear_line8();
        tft.drawString( disp_buf, 5, 220, 1 ); // FNT 1 from 2
      }

      //start of sprites
      init_sprites(); //best to re-init
#if 1
      //////////////////////////////////////////////////////////
      //Draw STATUS INDICATORS HOLD, MUTE,
      //////////////////////////////////////////////////////////
      spr.createSprite( 60, 30 ); //allocate sprite memory
      spr.fillSprite( mptr->col_def_bg ); //clear to black bground

      spr.setTextColor( mptr->col9, mptr->col10 );

      memset( disp_buf, 0x00, sizeof( disp_buf ) );
      if( follow || mute || inc_in_scan ) strcat( disp_buf, " " );

      if( follow > 0 ) strcat( disp_buf, "H " );
      if( mute > 0 ) strcat( disp_buf, "M " );
      if( inc_in_scan > 0 ) strcat( disp_buf, "S " );

      spr.drawString( disp_buf, 0, 0, FNT );


      spr.pushSprite( 265, 185 ); //transfer to lcd, x,y = 240,210
      spr.deleteSprite(); //free memory
#endif

#if 1
      //////////////////////////////////////////////////////////
      //Draw Status LEDS  / P1-P2 indicator
      //////////////////////////////////////////////////////////
      spr.createSprite( 80, 40 ); //allocate sprite memory
      spr.fillSprite( mptr->col_def_bg ); //clear to black bground

      spr.setTextColor( mptr->col_def_indicator, mptr->col_def_bg );

      if( mptr->decoder == 0 ) { //P25
        if( mptr->phase2 ) {
          spr.drawString( "P2", 0, 8, FNT ); //p25 p2
        } else {
          spr.drawString( "P1", 0, 8, FNT ); //p25 p1
        }
      } else if( mptr->decoder == 1 ) { //DMR
        spr.drawString( "DMR", 0, 8, FNT ); //DMR
      } else if( mptr->decoder == 2 ) { //ACARS
        spr.drawString( "ACA", 0, 8, FNT );
      } else if( mptr->decoder == 3 ) { //PAGERS
        spr.drawString( "PGR", 0, 8, FNT );
      }


      //status led
      if( mptr->status_led ) {
        spr.fillCircle( 37, 12, 10, mptr->col_def_led1_on );
      } else {
        spr.fillCircle( 37, 12, 10, mptr->col_def_led1_off );
      }

      //TG led
      if( mptr->tg_led ) {
        spr.fillCircle( 62, 12, 10, mptr->col_def_led2_on );
      } else {
        spr.fillCircle( 62, 12, 10, mptr->col_def_led2_off );
      }
      spr.pushSprite( 240, 212 ); //transfer to lcd, x,y = 240,210
      spr.deleteSprite(); //free memory
#endif

      draw_button_modes();

      tft.setTextColor( mptr->col1, mptr->col_def_bg );


      if( mptr->do_wio_lines ) {


        if( mptr->wio_line1[0] != 0 ) {
          snprintf( disp_buf, 25, "%s", mptr->wio_line1 );
          tonelookup(); //<<<<<<<<<<<<<<<<<<< TONE lookup for alias
          FNT = 2;

          if( strncmp( ( char * )line1_str, ( char * )disp_buf, 63 ) != 0 ) clear_line1();
          strncpy( line1_str, disp_buf, 63 );
          if( ( strstr( disp_buf, "TONE_A:" ) ) != NULL ) {
            tft.setTextColor( ILI9341_ORANGE, mptr->col_def_bg );
            tft.drawString( disp_buf, 9, 18, 4 );
          } else tft.drawString( disp_buf, 20, 20, FNT );
        } else {
          FNT = 2;
          mptr->sys_name[18] = 0;
          snprintf( disp_buf, 50, "%s" );

          if( strncmp( ( char * )line1_str, ( char * )disp_buf, 63 ) != 0 ) clear_line1();
          strncpy( line1_str, disp_buf, 63 );

          tft.drawString( disp_buf, 20, 20, FNT );

        }


        if( mptr->wio_line2[0] != 0 ) {
          FNT = 2;
          snprintf( disp_buf, 63,  "%s", mptr->wio_line2 );
          if( strncmp( ( char * )line2_str, ( char * )disp_buf, 63 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 63 );

          if( ( strstr( disp_buf, "FREQ_A:" ) ) != NULL ) { //do not display tone freq info wio line2 (tone lookup will display alias)
            tft.setTextColor( ILI9341_RED, mptr->col_def_bg ); // show line2 in RED during tone out
            tft.drawString( ToneAlias, 9, 44, 4 ); //
            strcpy( line2_str, ToneAlias ); // show tone alias
          }  else if( ( strstr( disp_buf, "FREQ_A:" ) ) != NULL ) {;} //do not show freq info
          else  tft.drawString( disp_buf, 5, 40, FNT );
        } else {
          FNT = 2;
          //mptr->sys_name[18]=0;
          //mptr->site_name[18]=0;
          //snprintf(disp_buf, 63,"%s / %s",mptr->sys_name,mptr->site_name);
          snprintf( disp_buf, 63, "   " );

          if( strncmp( ( char * )line2_str, ( char * )disp_buf, 63 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 63 );

          tft.drawString( disp_buf, 5, 40, FNT );
        }

      } else {
        FNT = 2;
        mptr->sys_name[18] = 0;
        mptr->site_name[18] = 0;

        //clear values
        if( freq_changed ) {
          freq_changed = 0; //keep this at the end within valid crc check
          two_tone_A = 0;
          two_tone_B = 0;
        }

#if 0
        int freq_idx = -1;

        float fm = -1.0f;
        //we wait for a value update before processing
        if( mptr->max_freq_cnt > 8 ) {
          //freq_idx = two_tone_get_idx((int) mptr->max_freq_hz);
          fm = mptr->max_freq_hz;

        }
#if 0
        if( mptr->max_freq_cnt == 0 ) {
          two_tone_A = 0;
          two_tone_B = 0;
        }
#endif

#if 1
        if( fm > 0 ) {  //valid frequency from table?
          if( two_tone_A == 0 ) {
            two_tone_A = ( int ) fm;
          } else if( two_tone_B == 0 ) {
            if( ( int ) fm != two_tone_A ) {
              two_tone_B = ( int ) fm;
            }
          }
        }

        if( two_tone_A > 0 || two_tone_B > 0 ) {
          if( two_tone_A && two_tone_B ) snprintf( disp_buf, 50, "TWO-TONE A:%u  B:%u", two_tone_A, two_tone_B )
            else snprintf( disp_buf, 50, "TWO-TONE A:%u", two_tone_A );
        } else {
          snprintf( disp_buf, 50, "%s / %s", mptr->sys_name, mptr->site_name );
        }
        if( strncmp( ( char * )line1_str, ( char * )disp_buf, 31 ) != 0 ) clear_line1();
        strncpy( line1_str, disp_buf, 31 );
        tft.drawString( disp_buf, 5, 10, FNT );
#else
        //if(freq_idx>0) {
        snprintf( disp_buf, 50, "Freq: %3.1f Hz, CNT: %u", fm, mptr->max_freq_cnt );
        if( strncmp( ( char * )line1_str, ( char * )disp_buf, 31 ) != 0 ) clear_line1();
        strncpy( line1_str, disp_buf, 31 );
        tft.drawString( disp_buf, 5, 10, FNT );
        //}
#endif
#else
        if( power_button_press_time == 0 ) {
          //if( demod == 0 || demod == 1 ) {
          snprintf( disp_buf, 50, "%s / %s   %05X-%03X-%03X%c  ", mptr->sys_name, mptr->site_name, mptr->wacn_id, mptr->sys_id, mptr->p25_sys_nac, nac_lock_str ); // <<<< moved wacn to line 1
          //} else if( demod == 2 ) {
          // snprintf( disp_buf, 50, "FMNB     " );
          //}
          if( strncmp( ( char * )line1_str, ( char * )disp_buf, 63 ) != 0 ) clear_line1();
          strncpy( line1_str, disp_buf, 63 );
          tft.drawString( disp_buf, 5, 20, FNT );
        }
#endif
      }


#if 1
    //draw audio volume bar
    if( do_update_vol || do_draw_rx ) {
      do_update_vol = 0;
      FNT = 2;
      spr.createSprite( 220, 4 ); //allocate memory for 80 x 80 sprite
      spr.fillSprite( TFT_BLACK );

      int v = ( int )( mptr->audio_volume_f * 78.0f );
      if( v > 220 ) v = 220;

      //x,y,w,h
      if( mptr->audio_volume_f < 2.1 ) spr.fillRect( 0, 0, v, 4, TFT_WHITE );
      else spr.fillRect( 0, 0, v, 4, TFT_RED );

      spr.pushSprite( 0, 230 );  //send to lcd. upper left corner of sprite
      spr.deleteSprite();  //free memory
    }
#endif

    }

draw_end2:
    status_timer = millis();

  } //////////end screen == 3
  draw_button_modes();
} // screen_3 void
