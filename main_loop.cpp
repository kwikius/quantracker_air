
/*
 Copyright (c) 2013 Andy Little 

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>
*/

#include <Arduino.h>
#include <ctype.h>

#include "aircraft.hpp"
#include "asynch_tx.hpp"
#include "gps.hpp"
#include "frsky.hpp"


#if defined __AVR_ATmega32U4__
#warning "Building for Due or Micro"
HardwareSerial & serial_port = Serial1;
#else
#warning "Building for mini or nano"
HardwareSerial & serial_port = Serial;
#endif
namespace {

   static const int HeartbeatLed = 13; // On all boards?

   static bool new_data = false;
}

void on_gps(gps& g)
{
    if ( g.m_latitude_deg10e7.changed()) {
        the_aircraft.location.gps_lat = quan::angle_<int32_t>::deg10e7{ g.m_latitude_deg10e7.get_value() };
        g.m_latitude_deg10e7.clear_change();
        new_data = true;
    }

    if ( g.m_longtitude_deg10e7.changed()) {
        the_aircraft.location.gps_lon = quan::angle_<int32_t>::deg10e7{ g.m_longtitude_deg10e7.get_value()};
        g.m_longtitude_deg10e7.clear_change();
        new_data = true;
    }

    if(g.m_fix_type.changed()) {
        the_aircraft.gps.fix_type = g.m_fix_type.get_value();
        g.m_fix_type.clear_change();
        new_data = true;
    }
    
    if(g.m_sats_used.changed()){
       the_aircraft.gps.num_sats = g.m_sats_used.get_value();
       g.m_sats_used.clear_change();
       new_data = true;
    }

    if(g.m_altitude_mm.changed()){
      the_aircraft.location.gps_alt = quan::length_<int32_t>::mm{g.m_altitude_mm.get_value()};
      g.m_altitude_mm.clear_change();
      new_data = true;
    }
}

gps the_gps(&serial_port, on_gps);

namespace {


   void do_startup_leds(int n, int delay_val)
   {
      pinMode( HeartbeatLed, OUTPUT);   
      // do something to show we are starting up!
      for ( int i = 0; i < n; ++i){
         digitalWrite(HeartbeatLed,HIGH);
         delay(delay_val);
         digitalWrite(HeartbeatLed,LOW);
         delay(delay_val);
      }
   }

   void setup_GPS()
   {
   
// need to scan the various baudrates
// coudl start at top?
      serial_port.begin(38400); 

      //do MTK factory cold start 
      serial_port.write("$PMTK104*37\r\n");
      serial_port.flush();
      //reset NMEA to default output
      serial_port.write("$PMTK314,-1*04\r\n");
      serial_port.flush();

     //send only VTG and GGA once every fix
     // (seems rmc is sent also)
      serial_port.write("$PMTK314,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n");	
      serial_port.flush();

      //set NMEA update rate to 5Hz
      serial_port.write("$PMTK220,200*2C\r\n");
      serial_port.flush();

      //MTK set fix interval to 200 ms
      serial_port.write("$PMTK300,200,0,0,0,0*2F\r\n");
      serial_port.flush();

      serial_port.end();
      serial_port.begin(38400); 
// ardupilot
  //  serial_port.write("$PMTK314,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n");	// GGA & VTG once every fix
//    "$PMTK330,0*2E\r\n"										// datum = WGS84
//    "$PMTK313,1*2E\r\n"									// SBAS on
//    "$PMTK301,2*2E\r\n"									// use SBAS data for DGPS
//    "");
//      //MTK  set NMEA update rate to 5Hz
//      serial_port.write("$PMTK220,200*2C\r\n");
//      if (get_ack("220",300)){
//          asynch_tx_write("set rate to 5Hz\n");
//      }
     
//      return true;
      //MTK set fix interval to 200 ms
      // recalc ck for 200 ms
//      serial_port.write("$PMTK300,200,0,0,0,0*2F\r\n");
//      if (get_ack("300",300)){
//        asynch_tx_write("set fix interval to 200 ms\n");
//      }
     
      //MTK only send GGA
//      serial_port.write("$PMTK314,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
//      if (! get_ack("314",300)){
//         asynch_tx_write("set required data\n");
//      }
//     
//      // try to speed up to 38400 baud
//     // 3329 supports baud rates of 4800/9600/38400/57600/115200
//      serial_port.write("$PMTK251,38400*27\r\n");
//      if (get_ack("251",300)){
//         serial_port.end();
//         serial_port.begin(38400);
//         return true;
//      }
 //     return false;

   }

   void read_GPS(){ 
      the_gps.parse();
   }
}

void setup()
{ 

   do_startup_leds(6,167);
   cli();
   //##############
   // for FrSky invert the output
   asynch_tx_setup(9600, true);
   //##################
   sei();
   // help the far end to sync
   for ( int8_t i = 0; i < 3; ++i){
      asynch_tx_write_byte(0x7E);
   }
 
   delay(500);
 
   setup_GPS();
   do_startup_leds(5,500);

   delay(500);

// wait here blinking led slowly until there are 5 satellites...
   unsigned long loop_time = 0;
   bool led_state = false;
   for(;;){
      read_GPS();
      if ( millis() - loop_time > 1000){
         loop_time = millis();
         if ( led_state == false){
            digitalWrite(HeartbeatLed,HIGH);
            led_state = true;
         }else{
            digitalWrite(HeartbeatLed,LOW);
            led_state = false;
         }
      }
      if ( the_aircraft.gps.num_sats >=5){
         digitalWrite(HeartbeatLed,HIGH);
         new_data = false;
         break; // GO!
      }
   }
}

void loop()
{
   static unsigned long loop_time = 0;
   static unsigned long led_time = 0;
   static bool led_state = true;
   
   read_GPS();
 
   if ( millis() - loop_time > 19){
     loop_time = millis();
     FrSky_send_message();
   }

   // blink if new data else led stops
   if ( new_data){
      if(led_state){
         if(( millis() - led_time ) > 100){
            led_time = millis();
            digitalWrite(HeartbeatLed,LOW);
            led_state = false;
            new_data = false;
         }
      }else{
          if(( millis() - led_time ) > 400){
            led_time = millis();
            digitalWrite(HeartbeatLed,HIGH);
            led_state = true;
            new_data = false;
         }
      }
   }
}
