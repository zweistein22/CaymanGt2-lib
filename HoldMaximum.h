#ifndef HoldMaximum_h
#define HoldMaximum_h
#include "Arduino.h"
#include <limits.h>
template <typename T>
class HoldMaximum {

public:
	HoldMaximum(unsigned long holdmilliseconds) {
		_holdmilliseconds = holdmilliseconds;
		_holdtime = 0;
		if (sizeof(T) == 1) _holdvalue = CHAR_MIN;
		else if(sizeof(T)==2) _holdvalue = SHRT_MIN;
		else _holdvalue = INT_MIN;
		
		
	}
	T Current(T v) {
/*
#ifdef DEBUGSERIAL
		DEBUGSERIAL.print("Current(");
		DEBUGSERIAL.print(v);
		DEBUGSERIAL.println(")");
#endif
*/

		
		unsigned long m = millis();

		if ((m - _holdtime) < _holdmilliseconds) {

			if (v > _holdvalue) {

				_holdtime = m;
				_holdvalue = v;
			}

		}

		else {

			_holdtime = m;
			_holdvalue = v;

		}
/*
#ifdef DEBUGSERIAL
		DEBUGSERIAL.print("_holdvalue=");
		DEBUGSERIAL.println(_holdvalue);
	
#endif
	*/
		return _holdvalue;


	}
	

private:
	unsigned long _holdmilliseconds;
	unsigned long _holdtime;
	T _holdvalue;
};



#endif

