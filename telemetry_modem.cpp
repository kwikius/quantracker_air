#include "config.hpp"
#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include <quan/tracker/zapp4/position.hpp>
#include "aircraft.hpp"
#include "asynch_tx.hpp"

uint16_t get_update_period_ms(){ return 100;}

namespace {
   typedef quan::angle_<int32_t>::deg10e7                   lat_lon_type;
   typedef quan::length_<int32_t>::mm                       altitude_type;
   typedef quan::uav::position<lat_lon_type,altitude_type> zapp4_position_type;
}
/*
   aircraft gps type
       lat/lon = quan::angle_<int32_t>::deg10e7, 
       alt = quan::length_<int32_t>::mm

   zapp4 gps type 
      lat/lon  = quan::angle_<int32_t>::deg10e7
      alt = quan::length_<int32_t>::mm;
*/

void telem_send_message()
{
#if (1)
   zapp4_position_type norm_pos{
      the_aircraft.location.gps_lat,
      the_aircraft.location.gps_lon,
      the_aircraft.location.gps_alt
   };
   uint8_t encoded [19];
   quan::tracker::zapp4::encode_position(norm_pos,encoded);
   asynch_tx_write((const char*)encoded,19);
   asynch_tx_write_byte(static_cast<char>(0));

#else // debug
  const char str[] = "<12345678901234567>";
  asynch_tx_write(str,19);
  asynch_tx_write_byte('\n');
#endif
}

