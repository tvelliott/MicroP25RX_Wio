///////////////////////////////////////////////////////////////////////
// We received a new metainfo structure. Time to redraw the screen
///////////////////////////////////////////////////////////////////////

void Screen_4( void )
{

  if( do_draw_rx && mptr->layout == 4 ) {

    if( prev_layout <= 3 ) {
      clr_screen();
    }

    Screen4_first_check();

    current_layout = mptr->layout;
    if( prev_layout != current_layout ) {
      prev_layout = current_layout;
    }

//if (do_draw_rx) {

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



      extern uint8_t packet_id;
      if( packet_id == mptr->wio_packet_id ) {
        //the teensy digital board received the last command
        //sent with send_cmd( *str, int len)

        __disable_irq();
        cmd_acked = 0;
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
      if( cmd_acked == 0 && current_button_mode == WIO_BUTTON_MODE_MONITOR && follow == 0 &&
          mptr->roaming == 1 && mptr->voice_tg_timeout == 0 && ( millis() - roam_time > mptr->roaming_timeout ) ) { //ROAMING=1 // scanning

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      }
      //ROAMING MODE 2
      else if( cmd_acked == 0 && current_button_mode == WIO_BUTTON_MODE_MONITOR && follow == 0 && ( millis() - roam_time > mptr->roaming_timeout ) &&
               mptr->roaming == 2 && ( mptr->evm_p > 10 || mptr->rssi_f < -115 ) ) { //ROAMING=2 // auto-switch-over-on-lost-sig, P+S ALL systems

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      }
      //ROAMING MODE 3
      else if( cmd_acked == 0 && current_button_mode == WIO_BUTTON_MODE_MONITOR && follow == 0 && ( millis() - roam_time > mptr->roaming_timeout ) &&
               mptr->roaming == 3 && ( mptr->evm_p > 10 || mptr->rssi_f < -115 ) ) { //ROAMING=3 // P+S auto-switch-over-on-lost-sig, SINGLE SYSTEM

        roam_time = millis();
        char cmd_str[32];
        sprintf( cmd_str, "nfreq %05X %03X\r\n", mptr->roam_wacn, mptr->roam_sysid ); //roam mode=3 needs wacn and sysid arguments
        send_cmd( cmd_str, 15 );
      }
      //ROAMING MODE 4
      else if( cmd_acked == 0 && current_button_mode == WIO_BUTTON_MODE_MONITOR && follow == 0 && ( millis() - roam_time > mptr->roaming_timeout ) &&
               mptr->roaming == 4 && ( mptr->evm_p > 10 || mptr->rssi_f < -115 ) ) { //ROAMING=4 // auto-switch-over-on-lost-sig, P+S+A ALL systems

        roam_time = millis();
        send_cmd( "nfreq", 15 );
      }
      //ROAMING MODE 5
      else if( cmd_acked == 0 && current_button_mode == WIO_BUTTON_MODE_MONITOR && follow == 0 &&
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
        // button_press_time=0;
        b_button_press_time = 0;
        c_button_press_time = 0;
      }

      prev_button_mode = current_button_mode;



      demod = mptr->use_demod;
      follow = mptr->follow;
      mute = mptr->audio_mute;
      learn = mptr->learn_mode;
      inc_in_scan = mptr->inc_in_scan;

      if( current_button_mode == WIO_BUTTON_MODE_TG_ZONE ) {
        __disable_irq();
        do_draw_rx = 0;
        memcpy( ( uint8_t * ) &minfo, ( uint8_t * ) buf, sizeof( metainfo ) );
        __enable_irq();

        draw_button_modes();
        draw_tg_zones();
        goto draw_end4;
      } else if( current_button_mode == WIO_BUTTON_MODE_RF_GAIN ) {

        current_button_mode = mptr->wio_button_mode;

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

        goto draw_end4;  //it really is ok to use goto. don't worry about it.
      } //////////////////////////////////////////////////////////////////////////////////////////////// End RF Gain sprites
      //FONT SIZE 1,2,4,8
      FNT = 4;

      tft.setFreeFont( NULL );
      // tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextColor( mptr->col_def_fg, mptr->col_def_bg );
      if( prev_freq != mptr->current_freq ) {
        freq_changed = 1;
      }
      prev_freq = mptr->current_freq;

//////////////////////////// line2

      tft.setTextColor( mptr->col2, mptr->col_def_bg );


      //if( !mptr->do_wio_lines ) {
      if( mptr->tg_s != prev_tgs ) {
        clear_line2();
      }
      prev_tgs = mptr->tg_s;


      if( mptr->tg_s <= 0 ) {
        snprintf( disp_buf2, 32, "FREQ: %3.6f MHz", mptr->current_freq );

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

        if( !mptr->do_wio_lines ) {
          snprintf( disp_buf, 32, "TG: %d, %s", mptr->tg_s, mptr->alpha );
          mptr->desc[19] = 0;
          if( strncmp( ( char * )disp_buf, ( char * )line2_str, 31 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 31 );
        }

        snprintf( disp_buf2, 45, "%s", mptr->desc );
        if( strncmp( ( char * )disp_buf2, ( char * )line3_str, 31 ) != 0 ) clear_line3();
        strncpy( line3_str, disp_buf2, 31 );

        if( !mptr->do_wio_lines ) {
          tft.setTextColor( mptr->col2, mptr->col_def_bg );
          tft.drawString( disp_buf, 5, 40, FNT );
        }
      }

//////////////////////////// line3
      if( mptr->tg_s <= 0 ) {
        tft.setTextColor( 0xF643, mptr->col_def_bg ); // set color to a yellow (0xF643) for on CCH freq
      } else {
        tft.setTextColor( mptr->col3, mptr->col_def_bg ); // set to line 3 color
      }

      tft.drawString( disp_buf2, 5, 70, FNT );

//////////////////////////// line4
      FNT = 2;

      tft.setTextColor( mptr->col4, mptr->col_def_bg );
      uint8_t nac_lock_str = 'U';
      if( mptr->lock_nac ) nac_lock_str = 'L';

      sprintf( disp_buf, "%05X-%03X-%03X%c  RSSI %3.1f dBm    ", mptr->wacn_id, mptr->sys_id, mptr->p25_sys_nac, nac_lock_str, mptr->rssi_f );
      if( strcmp( disp_buf, line4_str ) != 0 ) {
        //clear_line4(); //space at the end of this line is better solution
        tft.drawString( disp_buf, 5, 100, FNT );

      }
      strcpy( line4_str, disp_buf );

//////////////////////// line 5
      FNT = 2;

      tft.setTextColor( mptr->col5, mptr->col_def_bg );
      char demod_str[8];
      demod_str[0] = 0;
      if( mptr->use_demod == 0 ) strcpy( demod_str, "LSM" );
      if( mptr->use_demod == 1 ) strcpy( demod_str, "FM" );
      if( mptr->use_demod == 2 ) strcpy( demod_str, "FMNB" );
      if( mptr->use_demod == 3 ) strcpy( demod_str, "AM" );
      if( mptr->use_demod == 4 ) strcpy( demod_str, "AM+AGC" );

      sprintf( disp_buf, "SITE %d, RFSS %d  DEMOD %s               ", mptr->site_id, mptr->rf_id, demod_str );

      if( strcmp( disp_buf, line5_str ) != 0 ) {
        //  clear_line5();
        tft.drawString( disp_buf, 5, 130, FNT );
      }
      strcpy( line5_str, disp_buf );

//////////////////////////// line6
      sprintf( disp_buf, "NCO1 %3.3f, NCO2 %3.2f, EVM %3.1f    ", mptr->nco_offset, mptr->loop_freq, mptr->evm_p );
      if( strcmp( disp_buf, line6_str ) != 0 ) {
        //clear_line6(); //space at the end of this line is better solution
        tft.setTextColor( mptr->col6, mptr->col_def_bg );
        tft.drawString( disp_buf, 5, 160, FNT );

      }
      strcpy( line6_str, disp_buf );

//////////////////////////// line7
      if( mptr->on_control_b ) {
        sprintf( disp_buf, "TSBK/SEC %u    REF %u        ", mptr->tsbk_sec, mptr->ref_freq_cal );
      } else {
        sprintf( disp_buf, "FREQ %3.6f MHz  ERATE %1.3f       ", mptr->current_freq, mptr->erate );
      }
      if( strcmp( disp_buf, line7_str ) != 0 ) {
        //clear_line7(); //space at the end of this line is better solution
        tft.setTextColor( mptr->col7, mptr->col_def_bg );
        tft.drawString( disp_buf, 5, 190, FNT );
      }
      strcpy( line7_str, disp_buf );

//////////////////////////// line8
      FNT = 2;
      memset( disp_buf, 0x00, sizeof( disp_buf ) );

      tft.setTextColor( mptr->col8, mptr->col_def_bg );
      if( mptr->on_control_b == 0 && mptr->RID != 0 && mptr->alias != NULL ) {
        sprintf( disp_buf, "RID %u, %s", mptr->RID, mptr->alias );
      } else {
        if( current_button_mode == WIO_BUTTON_MODE_CONFIG ) {
          if( mptr->roaming ) {
            sprintf( disp_buf, "CONFIG  ROAM-PAUSE" );
          } else {
            sprintf( disp_buf, "CONFIG" );
          }
        } else if( current_button_mode == WIO_BUTTON_MODE_MONITOR ) {
          if( mptr->roaming ) {
            sprintf( disp_buf, "MONITOR ROAM-ON-%u", mptr->roaming );
          } else {
            sprintf( disp_buf, "MONITOR ROAM-OFF" );
          }
        }

        else {
          sprintf( disp_buf, " " );
        }

      }
      if( strcmp( disp_buf, line8_str ) != 0 ) {
        strcpy( line8_str, disp_buf );
        clear_line8();
        tft.drawString( disp_buf, 5, 220, FNT );
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

#if 1
      //////////////////////////////////////////////////////////
      //Draw IQ Constellation
      //////////////////////////////////////////////////////////
      int idx = 0;
      int ii;
      int qq;
      int8_t *ptr;

      if( mptr->data_type == META_DATA_TYPE_CONST ) { //incoming data is symbol constellation?

        ptr = ( int8_t * ) &mptr->data[0]; //pointer to incoming data 128 bytes
        spr.createSprite( 80, 80 ); //allocate memory for 80 x 80 sprite
        spr.fillSprite( TFT_BLACK );
        //grid lines
        //spr.drawLine(40, 0, 40, 80, TFT_DARKGREY);
        //spr.drawLine(0, 40, 80, 40, TFT_DARKGREY);
        //draw 64-symbol constellation
        idx = 0;
        for( int i = 0; i < 64; i++ ) {
          ii = *ptr++ / 2; //scale to +/- 32 range
          qq = *ptr++ / 2;

          if( mptr->draw_const_circles ) {
            spr.fillCircle( 40 + ii, 40 + qq, 2, mptr->col_def_const ); //symbols
          } else {
            spr.drawPixel( 40 + ii, 40 + qq, mptr->col_def_const ); // use Pixel rather than Circle
          }


        }
        spr.pushSprite( 233, 98 ); //send to lcd. upper left corner of sprite
        spr.deleteSprite();  //free memory
      }
#endif

      tft.setTextColor( TFT_GREEN, TFT_BLACK );

      if( mptr->do_wio_lines ) {


        if( mptr->wio_line1[0] != 0 ) {
          snprintf( disp_buf, 25, "%s", mptr->wio_line1 );
          FNT = 2;

          if( strncmp( ( char * )line1_str, ( char * )disp_buf, 31 ) != 0 ) clear_line1();
          strncpy( line1_str, disp_buf, 31 );

          tft.drawString( disp_buf, 5, 20, FNT );
        } else {
          FNT = 2;
          mptr->sys_name[18] = 0;
          snprintf( disp_buf, 25, "%s" );

          if( strncmp( ( char * )line1_str, ( char * )disp_buf, 31 ) != 0 ) clear_line1();
          strncpy( line1_str, disp_buf, 31 );

          tft.drawString( disp_buf, 5, 20, FNT );
        }


        if( mptr->wio_line2[0] != 0 ) {
          FNT = 2;
          snprintf( disp_buf, 63,  "%s", mptr->wio_line2 );
          if( strncmp( ( char * )line2_str, ( char * )disp_buf, 63 ) != 0 ) clear_line2();
          strncpy( line2_str, disp_buf, 63 );

          tft.drawString( disp_buf, 5, 40, FNT );
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
        snprintf( disp_buf, 50, "%s / %s", mptr->sys_name, mptr->site_name );

        if( strncmp( ( char * )line1_str, ( char * )disp_buf, 31 ) != 0 ) clear_line1();
        strncpy( line1_str, disp_buf, 31 );

        tft.drawString( disp_buf, 5, 20, FNT );
      }

      freq_changed = 0; //keep this at the end within valid crc check
    }



draw_end4:
    status_timer = millis();

  }

  draw_button_modes();
}


void Screen4_first_check( void )
{

  if( Screen4_first_write == true ) {  // force a line5 write at screen startup
    char demod_str[8];
    demod_str[0] = 0;
    if( mptr->use_demod == 0 ) strcpy( demod_str, "LSM" );
    if( mptr->use_demod == 1 ) strcpy( demod_str, "FM" );
    if( mptr->use_demod == 2 ) strcpy( demod_str, "FMNB" );
    if( mptr->use_demod == 3 ) strcpy( demod_str, "AM" );
    if( mptr->use_demod == 4 ) strcpy( demod_str, "AM+AGC" );
    sprintf( disp_buf, "SITE %d, RFSS %d  DEMOD %s", mptr->site_id, mptr->rf_id, demod_str );
    strcpy( line5_str, " * " ); // force a line5 write at screen startup
    if( strcmp( disp_buf, line5_str ) != 0 ) {
      Screen4_first_write = false;
    }
    clear_line5();
    tft.setTextColor( mptr->col5, mptr->col_def_bg );
    tft.drawString( disp_buf, 5, 130, FNT );
  }
}
