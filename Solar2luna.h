#ifndef SOLAR2LUNA_H
#define SOLAR2LUNA_H

// include core Wiring API and now Arduino
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
class converSolar2luna
{
public:
	void Solar2Lunar(unsigned int SolarDate, unsigned int SolarMonth, unsigned int SolarYear);
	unsigned int get_lunar_dd();
	unsigned int get_lunar_mm();
	unsigned int get_lunar_yy();
private:
	unsigned int lunar_dd;
	unsigned int lunar_mm;
	unsigned int lunar_yy;
	const int BEGINNING_YEAR = 16;
};

#endif
