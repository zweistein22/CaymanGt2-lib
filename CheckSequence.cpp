#include <Arduino.h>
#include "CheckSequence.h"




void CheckSequence::Reset() {
	ChecksequenceStep = 3;
	CheckEnd = 0;
	digitalWrite(VACUUM_PIN, LOW);
	digitalWrite(WATER_INJECT_VALVE_PIN, LOW);
	digitalWrite(OILPUMP_PIN, LOW);

}

void CheckSequence::Continue(unsigned long mil, float __map) {
	
	if (ChecksequenceStep > 3) return;
	if (ChecksequenceStep < 0) return;
	if (mil > CheckEnd) {
		ChecksequenceStep++;
		if (ChecksequenceStep < 1 || ChecksequenceStep>3) {
			Reset();
			return;
		}
		CheckEnd = mil + RunTestMillis[ChecksequenceStep - 1] + Pause;
		StepStarted = false;
		StepStopped = false;

	}
	switch (ChecksequenceStep) {
	case 1:
		if (mil > CheckEnd - Pause && !StepStopped) {
			(*le)("");
			pvacuumpump->Stop();
			StepStopped = true;
		}

		else {
			if (!StepStarted) {
				(*le)("Vacuum pump...");
				pvacuumpump->Start(mil);
				digitalWrite(VACUUM_PIN, HIGH);
				StepStarted = true;
			}
		}
		break;
	case 2:
		if (mil > CheckEnd - Pause && !StepStopped) {
			(*le)("");
			digitalWrite(WATER_INJECT_VALVE_PIN, LOW);
			StepStopped = true;
		}
		else {
			if (!StepStarted) {
				(*le)("WI...");
				digitalWrite(WATER_INJECT_VALVE_PIN, HIGH);
				StepStarted = true;
			}
		}
		break;
	case 3:
		if (mil > CheckEnd - Pause && !StepStopped) {
			(*le)("");
			pHead->settings.oilpump = 0;
			digitalWrite(OILPUMP_PIN, LOW);
			StepStopped = true;
		}
		else {
			if (!StepStarted) {
				if (__map > 920 && __map < 1050) {
					EEPROM_writeAnything(0, __map);
					pressurehPa = __map;
				}
				line.begin();
				line.print(pressurehPa, 1);
				line.print("hPa. Oilpump ...");
				(*le)(line);
				pHead->settings.oilpump = 1;
				digitalWrite(OILPUMP_PIN, HIGH);
				StepStarted = true;
			}
		}
		break;


	default: break;

	}



}
