// vntlda.h

#ifndef _VNTLDA_h
#define _VNTLDA_h
#include <arduino.h>
#include <avr/pgmspace.h>
#include <PID_v1.h>
#include <EEPROM.h>
namespace vntlda {
	float pwmfreq;
	float maxMAP;


#define AVG_MAX 15
#define MAP_AXIS_TPS 0xDE
#define MAP_AXIS_RPM 0xAD
#define MAP_AXIS_KPA 0xDD
#define MAP_AXIS_CELSIUS 0xAA
#define MAP_AXIS_VOLTAGE 0xAB
#define MAP_AXIS_DUTY_CYCLE 0xAC
#define MAP_AXIS_RAW 0x0
#define MAP_AXIS_EGT 0xAE

#define EXEC_DELAY 50 //ms
#define MAP_DELAY 10 //ms
#define PIDControlRatio 300

#define OPTIONS_VANESOPENIDLE 1
#define OPTIONS_VNTOUTPUTINVERTED 2

#define EGT_COOL 165
#define EGT_WARN 700
#define EGT_ALARM 775
#define EGT_MAX_READ 1101

#define IDLE_MAX_RPM 1150
#define MIN_BOOST_SPOOLED 10 // kPa
#define PID_CUT_IN 1520 // rpm
#define TPS_CUT_IN 18 // ~ 7%


const unsigned char versionString[] PROGMEM = "DMN-Vanbcguy Boost Ctrl v3.2.3";

unsigned char auxMap[] = {
  'M', '2', 'D',
  0x6, 0x8, MAP_AXIS_RPM, MAP_AXIS_EGT, MAP_AXIS_DUTY_CYCLE, // 01 - new version
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  60, 60, 60, 60, 210, 210,
  210, 210, 210, 210, 210, 210,
  00, 00, 00,                // lastX,lastY,lastRet
};

	unsigned char boostRequest[] = {
  'M', '2', 'D',
  0xC, 0xB, MAP_AXIS_RPM, MAP_AXIS_TPS, MAP_AXIS_KPA, // 01 - new version
  0, 0, 0, 15, 34, 44, 46, 46, 46, 43, 39, 37,
  0, 5, 15, 27, 38, 49, 51, 51, 51, 48, 43, 41,
  0, 5, 20, 32, 42, 54, 57, 57, 57, 53, 48, 45,
  0, 5, 20, 40, 55, 60, 63, 63, 63, 59, 53, 50,
  0, 5, 22, 43, 60, 67, 70, 70, 70, 65, 59, 55,
  0, 5, 24, 47, 65, 74, 78, 78, 78, 72, 66, 61,
  0, 6, 27, 58, 75, 92, 98, 98, 98, 90, 83, 76,
  0, 7, 30, 70, 92, 110, 123, 123, 123, 113, 104, 95,
  0, 9, 33, 75, 112, 138, 154, 154, 154, 141, 130, 119,
  0, 11, 41, 87, 128, 173, 193, 193, 193, 176, 162, 149,
  0, 12, 45, 85, 145, 192, 214, 214, 214, 195, 180, 166,
  00, 00, 00,                // lastX,lastY,lastRet
	};

	unsigned char boostDCMax[] = {
  'M', '2', 'D',
  0x8, 0xB, MAP_AXIS_RPM, MAP_AXIS_TPS, MAP_AXIS_DUTY_CYCLE,
  0, 204, 204, 180, 155, 140, 120, 70,
  0, 204, 204, 180, 155, 140, 120, 70,
  0, 204, 204, 175, 155, 140, 120, 70,
  0, 204, 190, 160, 135, 130, 120, 70,
  0, 204, 185, 160, 135, 130, 120, 70,
  0, 204, 180, 160, 135, 130, 120, 70,
  0, 204, 180, 155, 135, 130, 120, 70,
  0, 204, 175, 150, 135, 130, 120, 70,
  0, 204, 175, 145, 135, 125, 120, 70,
  0, 204, 180, 145, 135, 125, 120, 70,
  0, 204, 185, 145, 135, 125, 120, 70,
  00, 00, 00,                // lastX,lastY,lastRet
	};

	unsigned char boostDCMin[] = {
	  'M', '2', 'D',
	  0x9, 0xB, MAP_AXIS_RPM, MAP_AXIS_TPS, MAP_AXIS_DUTY_CYCLE,
	  0, 50, 50, 50, 50, 50, 50, 50, 50,
	  0, 121, 110, 100, 91, 83, 75, 68, 50,
	  0, 121, 110, 100, 91, 83, 75, 68, 50,
	  0, 121, 110, 100, 91, 83, 75, 68, 50,
	  0, 110, 100, 91, 83, 75, 68, 62, 50,
	  0, 99, 90, 70, 70, 70, 70, 65, 50,
	  0, 99, 90, 70, 70, 70, 70, 65, 50,
	  0, 99, 90, 70, 70, 70, 70, 65, 50,
	  0, 99, 90, 70, 70, 70, 70, 65, 50,
	  0, 99, 90, 70, 70, 70, 70, 65, 50,
	  0, 99, 90, 70, 70, 70, 70, 65, 50,
	  00, 00, 00,                // lastX,lastY,lastRet
	};

	unsigned char n75precontrolMap[] = {
	  'M', '2', 'D',
	  0xC, 0xB, MAP_AXIS_RPM, MAP_AXIS_TPS, MAP_AXIS_DUTY_CYCLE,
	  0, 90, 90, 110, 110, 100, 90, 75, 62, 50, 50, 50,
	  0, 204, 204, 204, 182, 163, 145, 130, 122, 115, 106, 70,
	  0, 204, 204, 198, 165, 148, 137, 126, 118, 112, 102, 70,
	  0, 204, 204, 176, 160, 142, 130, 117, 107, 102, 95, 70,
	  0, 204, 204, 145, 124, 115, 109, 101, 94, 87, 82, 70,
	  0, 204, 204, 145, 115, 112, 104, 94, 90, 85, 78, 70,
	  0, 204, 204, 132, 111, 109, 103, 94, 87, 80, 76, 70,
	  0, 204, 204, 132, 110, 108, 102, 94, 86, 80, 76, 70,
	  0, 204, 204, 138, 107, 103, 98, 91, 85, 80, 76, 70,
	  0, 204, 204, 142, 128, 119, 110, 103, 97, 89, 82, 70,
	  0, 204, 204, 160, 150, 140, 132, 121, 110, 98, 86, 70,
	  00, 00, 00,              // lastX,lastY,lastRet
	};



	struct controlsStruct {
		// inputs
		volatile int tpsInput;
		unsigned char tpsCorrected;
		volatile int mapInput;
		double mapCorrected;
		volatile int egtInput;
		unsigned char egtCorrected;
		volatile int empInput;
		unsigned char empCorrected;
		char mode; // operating mode

		// outputs

		double vntTargetPressure;
		unsigned char vntPositionRemapped;
		unsigned char vntPositionDC;
		int vntMinDc;
		int vntMaxDc;
		int n75precontrol;

		// calculated value
		volatile int rpmActual;
		volatile unsigned char rpmCorrected;
		unsigned char statusBits;

		bool idling;
		int temp1;

		unsigned char auxOutput;

		float boostCalculatedP;
		float boostCalculatedI;
		float boostCalculatedD;

		double pidOutput;

		unsigned long lastTime;
		float lastInput;
	};

	controlsStruct controls;

	struct settingsStruct {
		int tpsMin;
		int tpsMax;
		int mapMin;
		int mapMax;
		int egtMin;
		int egtMax;
		int empMin;
		int empMax;
		int rpmMax;
		int rpmTeethsPerRotation;
		unsigned char mode;
		char options;
		int boostKp;
		int boostKi;
		int boostKd;
	};

	settingsStruct settings;

	double Kp;
	double Ki;
	double Kd;

	// set up VNT PID control
	PID vntPid(&controls.mapCorrected, &controls.pidOutput, &controls.vntTargetPressure, Kp, Ki, Kd, P_ON_E, DIRECT);


	struct avgStruct {
		unsigned char pos;
		unsigned char size;
		volatile unsigned int avgData[AVG_MAX];
	};

	avgStruct mapAvg;

	char buffer[100]; // general purpose buffer, mainly used for string storage when printing from flash
	unsigned long lastPacketTime;
	const unsigned char mapVisualitionHelp[] PROGMEM = "Top Left is 0,0 (press: L - toggle live mode)";

	unsigned char page = 0;
	const char* pages[] = {
	  "About", "Adaptation", "Actuator Fine-tune", "Edit map: boostRequest", "Edit map: boostDCMin", "Edit map: boostDCMax", "Edit map: n75preControl", "Edit map: Aux. device PWM map", "Output Tests"
	};

	unsigned char* editorMaps[] = {
	  boostRequest, boostDCMin, boostDCMax, n75precontrolMap, auxMap
	};

	

	void calcKp() {
		Kp = (float)(settings.boostKp) / PIDControlRatio;
	}

	void calcKi() {
		Ki = (float)(settings.boostKi) / PIDControlRatio;
	}

	void calcKd() {
		Kd = (float)(settings.boostKd) / PIDControlRatio;
	}

	void loadDefaults() {
		memset(&settings, 0, sizeof(settingsStruct));
		settings.tpsMin = 85;
		settings.tpsMax = 970;
		settings.mapMin = 55;
		settings.mapMax = 975;
		settings.empMax = 1023;
		settings.egtMax = 970;
		settings.egtMin = 0;
		settings.rpmTeethsPerRotation = 4;
		settings.rpmMax = 6000;
		settings.options = 0;
		settings.boostKp = 220;
		settings.boostKi = 7;
		settings.boostKd = 15;
	}

	unsigned char mapValues(int raw, int mapMin, int mapMax) {
		if (raw < mapMin)
			return 0;
		if (raw >= mapMax)
			return 0xff;

		return map(raw, mapMin, mapMax, 0, 255);
	}

	unsigned char mapValuesSqueeze(int raw, int mapMin, int mapMax) {
		return map(raw, 0, 255, mapMin, mapMax);
	}

	unsigned char mapInterpolate(unsigned char p1, unsigned char p2, unsigned char pos) {
		return (p1 * (100 - pos) + p2 * pos) / 100;
	}

	unsigned char mapLookUp(unsigned char* mapData, unsigned char x, unsigned char y) {
		unsigned char isInterpolated = *(mapData + 2);
		unsigned char tableSizeX = *(mapData + 3);
		unsigned char tableSizeY = *(mapData + 4);
		unsigned char yPos;
		*(mapData + 8 + tableSizeX * tableSizeY) = x;
		*(mapData + 8 + tableSizeX * tableSizeY + 1) = y;

		if (tableSizeY) {
			yPos = y / (256 / (tableSizeY - 1));
		}
		else {
			yPos = 0;
		}
		unsigned char xPos = (x / (256 / (tableSizeX - 1)));
		int ofs = 8; // skip headers

		unsigned char p1 = *(mapData + ofs + (yPos * tableSizeX) + xPos);
		unsigned char p2 = *(mapData + ofs + (yPos * tableSizeX) + (((xPos + 1) >= tableSizeX) ? xPos : xPos + 1));
		unsigned char p3 = *(mapData + ofs + ((((yPos + 1) >= tableSizeY) ? yPos : yPos + 1) * tableSizeX) + xPos);
		unsigned char p4 = *(mapData + ofs + ((((yPos + 1) >= tableSizeY) ? yPos : yPos + 1) * tableSizeX) + (((xPos + 1) >= tableSizeX) ? xPos : xPos + 1));

		unsigned char ret;
		if (isInterpolated == 'D') {
			int amountX = (x % (256 / (tableSizeX - 1))) * (10000 / (256 / (tableSizeX - 1)));
			if (tableSizeY) {
				// 2D
				int amountY = (y % (256 / (tableSizeY - 1))) * (10000 / (256 / (tableSizeY - 1)));
				char y1 = mapInterpolate(p1, p2, amountX / 100);
				char y2 = mapInterpolate(p3, p4, amountX / 100);
				ret = mapInterpolate(y1, y2, amountY / 100);
			}
			else {
				// 1D
				ret = mapInterpolate(p1, p2, amountX / 100);
			}
		}
		else {
			ret = p1;
		}
		*(mapData + 8 + tableSizeX * tableSizeY + 2) = ret;
		return ret;
	}


	char mapDebugCharValue(unsigned char c) {
		if (c < 5) {
			return ' ';
		}
		else if (c < 20) {
			return '.';
		}
		else if (c < 60) {
			return ':';
		}
		else if (c < 128) {
			return '!';
		}
		else if (c < 180) {
			return 'o';
		}
		else if (c < 220) {
			return 'O';
		}
		else {
			return '@';
		}
	}

	int EEPROMwriteData(int offset, byte* ptr, int size) {
		int i;
		for (i = 0; i < size; i++)
			EEPROM.write(offset++, *(ptr++));
		return i;
	}

	int EEPROMreadData(int offset, byte* ptr, int size) {
		int i;
		for (i = 0; i < size; i++)
			*(ptr++) = EEPROM.read(offset++);
		return i;
	}

	void saveToEEPROM() {
		int ofs = 0;
		// write magic header
		strcpy_P(buffer, (PGM_P)&versionString);
		ofs += EEPROMwriteData(0, (byte*)&buffer, strlen(buffer));
		// write control struct
		ofs += EEPROMwriteData(ofs, (byte*)&settings, sizeof(settingsStruct));

		ofs += EEPROMwriteData(ofs, (byte*)&auxMap, sizeof(auxMap));
		ofs += EEPROMwriteData(ofs, (byte*)&boostRequest, sizeof(boostRequest));
		ofs += EEPROMwriteData(ofs, (byte*)&boostDCMin, sizeof(boostDCMin));
		ofs += EEPROMwriteData(ofs, (byte*)&boostDCMax, sizeof(boostDCMax));
		ofs += EEPROMwriteData(ofs, (byte*)&n75precontrolMap, sizeof(n75precontrolMap));

		//printFromFlash(ANSIclearEolAndLf);
		Serial.print(ofs, DEC);
		Serial.print(F("SAVED "));
		Serial.print(ofs);
		Serial.print(F(" BYTES."));

		delay(1000);
	}

	bool loadFromEEPROM(bool force) {
		int ofs = 0;
		// if reset pin is active, no not load anything from eeprom
		
		// Check magic header to prevent data corruption of blank board or wrong version save file
		if (!force) {
			strcpy_P(buffer, (PGM_P)&versionString);
			for (ofs = 0; ofs < strlen(buffer); ofs++) {
				if (EEPROM.read(ofs) != buffer[ofs])
					return false;
			}
		}
		ofs = strlen(buffer);
		ofs += EEPROMreadData(ofs, (byte*)&settings, sizeof(settingsStruct));

		ofs += EEPROMreadData(ofs, (byte*)&auxMap, sizeof(auxMap));
		ofs += EEPROMreadData(ofs, (byte*)&boostRequest, sizeof(boostRequest));
		ofs += EEPROMreadData(ofs, (byte*)&boostDCMin, sizeof(boostDCMin));
		ofs += EEPROMreadData(ofs, (byte*)&boostDCMax, sizeof(boostDCMax));
		ofs += EEPROMreadData(ofs, (byte*)&n75precontrolMap, sizeof(n75precontrolMap));

		return true;
	}

	int getFilteredAverage(struct avgStruct* a) {
		int minVal = 0;
		int maxVal = 255;
		long int avgAll = 0;

		for (int i = 0; i < a->size; i++) {

			if (a->avgData[i] < minVal) {
				minVal = a->avgData[i];
			}
			if (a->avgData[i] > maxVal) {
				maxVal = a->avgData[i];
			}

			avgAll += a->avgData[i];
		}
		avgAll = (int)(avgAll / a->size);
		return avgAll;

	}

	
	void determineIdle() {
		if (controls.tpsCorrected > 0) {
			controls.idling = false;
		}
		else if (controls.rpmActual < IDLE_MAX_RPM) {    // Accelerator is at zero and we are in the idle speed band
			controls.idling = true;
		}
		else {
			controls.idling = false;                         // Most likely coasting right now; continue proportional behavior
		}
	}

	void controlVNT() {

		double minControl;
		double maxControl;

		double toControlVNT;

		controls.rpmCorrected = mapValues(controls.rpmActual, 0, settings.rpmMax);
		controls.mapCorrected = mapValues(controls.mapInput, settings.mapMin, settings.mapMax);
		controls.tpsCorrected = mapValues(controls.tpsInput, settings.tpsMin, settings.tpsMax);

		controls.vntMaxDc = mapLookUp(boostDCMax, controls.rpmCorrected, controls.tpsCorrected);
		controls.vntMinDc = mapLookUp(boostDCMin, controls.rpmCorrected, controls.tpsCorrected);

		controls.n75precontrol = mapLookUp(n75precontrolMap, controls.rpmCorrected, controls.tpsCorrected);

		/* Look up the requested boost */
		controls.vntTargetPressure = mapLookUp(boostRequest, controls.rpmCorrected, controls.tpsCorrected);

		/* This is the available span of our DC - we can only go between min and max */
		minControl = controls.vntMinDc - controls.n75precontrol;  // this will be a negative number
		maxControl = controls.vntMaxDc - controls.n75precontrol;  // this will be a positive number

		if (minControl > 0) {
			// Our MinDC map is higher than our precontrol map; oops
			minControl = 0;
		}

		if (maxControl < 0) {
			// Our MaxDC map is lower than our precontrol map; oops
			maxControl = 0;
		}

		vntPid.SetOutputLimits(minControl, maxControl);

		if ((controls.idling)) {
			// If we are at idle then we don't want any boost regardless of map

			controls.vntTargetPressure = 0;                    // Display zero target pressure on the LCD at idle
			controls.mode = 0;                                 // System status = idling
			controls.pidOutput = 0;

			vntPid.SetMode(MANUAL);                            // Disable PID controller at idle

			if (settings.options & OPTIONS_VANESOPENIDLE) {
				toControlVNT = minControl;
			}
			else {
				toControlVNT = maxControl;
			}

		}
		else if (controls.mapCorrected <= MIN_BOOST_SPOOLED || controls.rpmActual < PID_CUT_IN || controls.tpsCorrected < TPS_CUT_IN) {
			// If the turbo hasn't spooled up yet we're going to end up winding up the control loop; the precontrol map
			// should be more than sufficient to get things spinning

			controls.mode = 1;                                // We haven't spooled, don't integrate yet

			vntPid.SetMode(AUTOMATIC);
			vntPid.SetTunings(Kp, 0.0, Kd);

		}
		else {

			vntPid.SetMode(AUTOMATIC);
			vntPid.SetTunings(Kp, Ki, Kd);

			vntPid.Compute();

			if (controls.pidOutput == minControl) {
				// We are at minimum
				controls.mode = 4;
			}
			else if (controls.pidOutput == maxControl) {
				// We are at maximum
				controls.mode = 3;
			}
			else {
				// Normal in-range running
				controls.mode = 2;
			}

		}

		toControlVNT = round(controls.pidOutput) + controls.n75precontrol;

		controls.vntPositionDC = toControlVNT;

		/* This loop should never ever be true - a 100% output should be diff between min and max + min which should equal max
		  but I'm not quite ready to remove this */
		if (controls.vntPositionDC > controls.vntMaxDc)
			controls.vntPositionDC = controls.vntMaxDc;

		/* Display these as real numbers - will make the logs more useful as we can try different values */
		controls.boostCalculatedP = vntPid.GetKp();
		controls.boostCalculatedI = vntPid.GetKi();
		controls.boostCalculatedD = vntPid.GetKd();

		unsigned char finalPos;
		finalPos = controls.vntPositionDC;

		if (settings.options & OPTIONS_VNTOUTPUTINVERTED) {
			controls.vntPositionRemapped = 255 - finalPos;
		}
		else {
			controls.vntPositionRemapped = finalPos;
		}

		// Include the time we spent processing
		controls.lastTime = millis();
	}

	void controlEGT() {
		// EGT controls
		controls.egtCorrected = mapValues(controls.temp1, settings.egtMin, settings.egtMax);
		controls.auxOutput = mapLookUp(auxMap, controls.rpmCorrected, controls.egtCorrected);
	}

	
	bool freezeModeEnabled = false;

	unsigned long serialLoop = 0;
	unsigned long execLoop = 0;
	unsigned long displayLoop = 0;
	unsigned long mapLoop = 0;

	typedef struct  {
		char nduty;
		unsigned char wi;
	} vntlda_data;

	unsigned int execTimeRead = 0;
	unsigned int execTimeAct = 0;

	vntlda_data loop(unsigned short map, unsigned short rpm, char wpedal, short egt) {
		vntlda_data data;
		data.nduty = 0;
		data.wi = 0;

		if ((millis() - mapLoop) >= MAP_DELAY) {
			controls.mapInput = map;
			mapLoop = millis();
		}

		/* Actual execution will happen every EXEC_DELAY - this is where we do our actual calculations */
		if ((millis() - execLoop) >= EXEC_DELAY) {


			execTimeRead = millis();
			controls.tpsInput = wpedal;
			controls.temp1 = egt;
			execTimeRead = millis() - execTimeRead;


			execTimeAct = millis();
			// We will actually process our values and change actuators every EXEC_DELAY milliseconds
			if (freezeModeEnabled) {
				Serial.print(F("\rFREEZE "));
			}
			else {
				// update output values according to input
				controls.rpmActual = rpm;
				determineIdle();
				controlVNT();
				controls.temp1 = egt; 
				data.nduty = controls.vntPositionRemapped;
				data.wi = controls.auxOutput;
				
			}
			execLoop = millis();
			execTimeAct = execLoop - execTimeAct;
		}

	
	}

	void Init() {
		//---------------------------------------------- Set PWM frequency for D2, D3 & D5 ---------------------------

		//TCCR3B = TCCR3B & B11111000 | B00000001;    // set timer 3 divisor to     1 for PWM frequency of 31372.55 Hz
		//TCCR3B = TCCR3B & B11111000 | B00000010;    // set timer 3 divisor to     8 for PWM frequency of  3921.16 Hz
		//  TCCR3B = TCCR3B & B11111000 | B00000011;    // set timer 3 divisor to    64 for PWM frequency of   490.20 Hz
		//TCCR3B = TCCR3B & B11111000 | B00000100;    // set timer 3 divisor to   256 for PWM frequency of   122.55 Hz
		//TCCR3B = TCCR3B & B11111000 | B00000101;    // set timer 3 divisor to  1024 for PWM frequency of    30.64 Hz
		  //---------------------------------------------- Set PWM frequency for D11 & D12 -----------------------------
		pwmfreq = 30.64;
		maxMAP = 1950;
		vntPid.SetSampleTime(100);
		mapAvg.size = AVG_MAX;

		//initial setup of kp/ki/kd
		calcKp();
		calcKi();
		calcKd();

	}

}



#endif

