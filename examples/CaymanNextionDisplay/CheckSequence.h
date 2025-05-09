#ifndef __CHECKSEQUENCE_H
#define __CHECKSEQUENCE_H

#include <EngineMsmt.h>
#include "VacuumPump.h"
#include <EEPROM.h>

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
	const byte* p = (const byte*)(const void*)&value;
	int i;
	for (i = 0; i < sizeof(value); i++)
		EEPROM.write(ee++, *p++);
	return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
	byte* p = (byte*)(void*)&value;
	int i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(ee++);
	return i;
}


typedef void(*fpLogError)(const char *);

#define DEFPRESSURE 1013

class CheckSequence {
public:
	void Begin();
	void Setup(fpLogError logerror, HeadU &_head,VacuumPump &_vac) {
		le=logerror;
		pHead=&_head;
		pvacuumpump = &_vac;
		float pressure = 0;
		EEPROM_readAnything(0, pressure);
		if (isnan(pressure)) {
#ifdef SERIOUSERROR
			SERIOUSERROR.println("error reading pressure from EEPROM");
#endif
			pressurehPa = DEFPRESSURE;
		}
		if (pressure > 800 && pressure < 1100) pressurehPa = pressure;
	}
	void Reset();
	void Continue(unsigned long mil, float __map);
	int ChecksequenceStep=-1;
	float pressurehPa = DEFPRESSURE;
private:
	fpLogError le;
	HeadU * pHead;
	VacuumPump *pvacuumpump;
	unsigned long RunTestMillis[7] = { 0,5000,3000 /*pause*/,5000,3000 /*pause*/,5000,0 };
	
	unsigned long CheckEnd = 0;
	

};


#define OILPUMP_PIN 5    
#define WATER_INJECT_VALVE_PIN 3

extern String line;
#endif
