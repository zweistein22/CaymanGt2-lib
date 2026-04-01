#include <Arduino.h>
#include "CheckSequence.h"

#define INFOSERIAL(call) Serial.call
// or do deactivate  #define INFOSERIAL(call)

void CheckSequence::Begin(){
ChecksequenceStep = 0;
INFOSERIAL(println("CheckSequence::Begin()"));
}
void CheckSequence::Reset() {
	ChecksequenceStep = -1; // sets to dormient,  is a public variable to set
	CheckEnd = 0;
	digitalWrite(VACUUM_PIN, LOW);
	digitalWrite(WATER_INJECT_VALVE_PIN, LOW);
	digitalWrite(OILPUMP_PIN, LOW);

}

void CheckSequence::Continue(unsigned long mil, float __map) {
	
	if (ChecksequenceStep >= 6) return;  // sequence done
	if (ChecksequenceStep < 0) return; // in reset state, Begin() not yet called
	if (mil > CheckEnd) {
		ChecksequenceStep++;
		if (ChecksequenceStep < 1 || ChecksequenceStep>6) {
			Reset();
			return;
		}
   
		CheckEnd = mil + RunTestMillis[ChecksequenceStep];

  String info="heckSequence::Continue ChecksequenceStep:";
  info+=String(ChecksequenceStep);
	INFOSERIAL(println(info));
	switch (ChecksequenceStep) {

   case 1:
       (*le)("Vacuum pump...");
        pvacuumpump->Start(mil);
        digitalWrite(VACUUM_PIN, HIGH);
        pHead->settings.vacuumpump = 1;
        break;
        
	case 2:
	  		pvacuumpump->Stop();
        pHead->settings.vacuumpump = 0;
        break;

   case 3:
        (*le)("WI...");
        digitalWrite(WATER_INJECT_VALVE_PIN, HIGH);
        pHead->settings.waterinjection = 1;
     		break;
         
	case 4:
			digitalWrite(WATER_INJECT_VALVE_PIN, LOW);
      pHead->settings.waterinjection = 0;
			break;

	case 5:
      if (__map > 800 && __map < 1100) {
          EEPROM_writeAnything(0, __map);
          pressurehPa = __map;
      }
      line = String(pressurehPa) + " hPa. Oilpump ...";
      (*le)(line.c_str());
     // pHead->settings.oilpump = 1; // we do read in actual state
      digitalWrite(OILPUMP_PIN, HIGH);
      break;

   case 6:
   
      digitalWrite(OILPUMP_PIN, LOW);
      //pHead->settings.oilpump = 0;
      break;

    default:
     break;
      
	}
	}


}
