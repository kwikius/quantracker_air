#include "config.hpp"

#if QUANTRACKER_AIR_OUTPUT_TYPE == QUANTRACKER_AIR_OUTPUT_TYPE_3DR_MODEM

#include "telemetry_modem.cpp"

void output_message()
{
   telem_send_message();
}


#else
// FrSky
#include "frsky.cpp"

void output_message()
{
   FrSky_send_message();
}


#endif