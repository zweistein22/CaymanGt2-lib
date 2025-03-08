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


#define MAP_DELAY 0 //ms
#define PIDControlRatio 300

#define OPTIONS_VANESOPENIDLE 1
#define OPTIONS_VNTOUTPUTINVERTED 2

#define EGT_COOL 165
#define EGT_WARN 930
#define EGT_ALARM 960
#define EGT_MAX_READ 1101

#define IDLE_MAX_RPM 1150
#define MIN_BOOST_SPOOLED 1600 // kPa
#define PID_CUT_IN 1520 // rpm
#define TPS_CUT_IN 18 // ~ 7%



unsigned int EXEC_DELAY = 0; 
const unsigned char versionString[] PROGMEM = "DMN-Vanbcguy Boost Ctrl v3.2.3";

bool sportModus=false;

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
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 110, 120, 130, 140, 140,140,140,
  100, 109, 133,175, 110, 120, 130, 140, 150, 150,150, 150,
  150, 159, 150,170, 170, 170, 190, 190, 190, 190,195, 200,
  160, 170, 180, 180,180, 180, 190, 190, 195, 200, 205,210,
  00, 00, 00,                // lastX,lastY,lastRet
	};


unsigned char boostRequestNormal[] = {
  'M', '2', 'D',
  0xC, 0xB, MAP_AXIS_RPM, MAP_AXIS_TPS, MAP_AXIS_KPA, // 01 - new version
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 100, 100, 100, 100, 100,100,100,
  100, 100, 100,100, 100, 110, 120, 130, 140, 140,140,140,
  100, 109, 133,175, 110, 120, 130, 140, 150, 150,150, 150,
  150, 159, 150,170, 170, 170, 170, 170, 170, 170,170, 170,
  160, 170, 170, 170,170, 170, 170, 170, 170, 170, 170,170,
  00, 00, 00,                // lastX,lastY,lastRet
  };
  
	unsigned char boostDCMax[] = {
  'M', '2', 'D',
  0x8, 0xB, MAP_AXIS_RPM, MAP_AXIS_TPS, MAP_AXIS_DUTY_CYCLE,
  
  125, 139, 185, 220, 240, 240, 240, 240,
  125, 139, 185, 220, 240, 240, 240, 240,
  125, 140, 185, 220, 240, 240, 240, 240,
  125, 140, 185, 220, 240, 240, 240, 240,
  125, 140, 185, 220, 240, 240, 240, 240,
  130, 140, 185, 220, 240, 240, 240, 240,
  130, 185, 185, 220, 240, 240, 240, 240,
  130, 185, 185, 220, 240, 240, 240, 240,
  130, 255, 255, 255, 255, 255, 255, 255,
  130, 255, 255, 255, 255, 255, 255, 255,
  130, 255, 255, 255, 255, 255, 255, 255,

  
  00, 00, 00,                // lastX,lastY,lastRet
	};

unsigned char boostDCMin[] = {
	'M', '2', 'D',
	0x8, 0xB, MAP_AXIS_RPM, MAP_AXIS_TPS, MAP_AXIS_DUTY_CYCLE,
	0,	0,	0, 0, 50, 50, 50, 50,
	0, 0, 0, 91, 83, 75, 68, 50,
	0, 0, 0, 91, 83, 75, 68, 50,
    0, 90, 90, 91, 83, 75, 68, 50,
    0, 90, 91, 83, 75, 68, 62, 50,
	0, 90, 70, 70, 70, 70, 65, 50,
    0, 90, 70, 70, 70, 70, 65, 50,
    0, 90, 70, 70, 70, 70, 65, 50,
	0, 90, 70, 70, 70, 70, 65, 50,
	0, 90, 70, 70, 70, 70, 65, 50,
    0, 90, 70, 70, 70, 70, 65, 50,
	  00, 00, 00,                // lastX,lastY,lastRet
	};

	unsigned char n75precontrolMap[] = {
	  'M', '2', 'D',
	  0xC, 0xB, MAP_AXIS_RPM, MAP_AXIS_TPS, MAP_AXIS_DUTY_CYCLE,
	 100, 125, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130,
   100, 125, 150, 180, 180, 180, 180, 180, 180, 180, 180, 180,
   100, 125, 150, 180, 180, 180, 180, 180, 180, 180, 180, 180,
   100, 125, 150, 180, 180, 180, 180, 180, 180, 180, 180, 180,
   100, 125, 150, 180, 180, 180, 180, 180, 180, 180, 180, 180,
   100, 125, 150, 180, 180, 180, 180, 180, 180, 180, 180, 180,
   100, 125, 150, 180, 180, 180, 180, 180, 180, 180, 180, 180,
   100, 160, 170, 180, 180, 180, 180, 180, 180, 180, 180, 180,
   100, 170, 180, 180, 180, 180, 180, 180, 180, 220, 220, 222,
   100, 180, 220, 230, 230, 230, 230, 240, 240, 245, 245, 245,
   100, 180, 220, 230, 230, 230, 230, 240, 240, 255, 255, 255,
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
		int rpmMax;
		
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

 int  toRpm(int raw){
  return round(((float)settings.rpmMax/255)* (float) raw);
  
 }

 int toTps(int raw){
    return int((float)raw/2.55);
    
 }




	struct avgStruct {
		unsigned char pos;
		unsigned char size;
		volatile unsigned int avgData[AVG_MAX];
	};

	avgStruct mapAvg;

	char buffer[100]; // general purpose buffer, mainly used for string storage when printing from flash
	unsigned long lastPacketTime;
		

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
		settings.tpsMin = 0;
		settings.tpsMax = 255;
		settings.mapMin = 1000;
		settings.mapMax = 2550;

		settings.egtMax = 1050;
		settings.egtMin = 0;
		
		settings.rpmMax = 7000;
		settings.options = 0;//OPTIONS_VANESOPENIDLE;//|OPTIONS_VNTOUTPUTINVERTED;
		settings.boostKp = 220;
		settings.boostKi = 7;
		settings.boostKd = 15;
	}

	unsigned char mapValues(int raw, int mapMin, int mapMax) {
		if (raw < mapMin)	return 0;
		if (raw >= mapMax){
      Serial.print("mapMax=");
      Serial.println(mapMax);
			Serial.print("raw=");
      Serial.println(raw);
			return 0xff;
		}

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

void PrintMap(unsigned char *mapData){
 unsigned char tableSizeX = *(mapData + 3);
 unsigned char tableSizeY = *(mapData + 4);
 unsigned char axisTypeX = *(mapData + 5);
 unsigned char axisTypeY = *(mapData + 6);
 unsigned char axisTypeResult = *(mapData + 7);
 unsigned char lastXpos = *(mapData + 8 + tableSizeX * tableSizeY);
 unsigned char lastYpos = *(mapData + 8 + tableSizeX * tableSizeY + 1);
 unsigned char lastValue = *(mapData + 8 + tableSizeX * tableSizeY + 2);
 if(axisTypeX == MAP_AXIS_RPM){
    Serial.print("\t");
    for(int y = 0;y < tableSizeY; y++){
        if(y == 0) {
          for(int x = 0; x < tableSizeX;x++){
            Serial.print((int)(((x+1) * (long)settings.rpmMax)/tableSizeX));
            Serial.print("\t");
          } 
          Serial.print("rpm");
          Serial.println("");
        }
        for(int x = 0; x < tableSizeX;x++){
          if(x==0){
            Serial.print("tp:");
            Serial.print((y+1)*100/tableSizeY);
            Serial.print("%");
          }
          Serial.print("\t");

          unsigned char v =*(mapData + 8 + y*tableSizeX + x);
                    
          if(axisTypeResult == MAP_AXIS_KPA){
            Serial.print((unsigned int)map(v,0,255, settings.mapMin, settings.mapMax));
             //Serial.print((int) v);
          }
          if(axisTypeResult == MAP_AXIS_DUTY_CYCLE){
             Serial.print((int) v);
          }
         
        }
        Serial.println("");
    }
 }
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
    /*
   Serial.print("controls.rpmActual=");
   Serial.print(controls.rpmActual);

   Serial.print("controls.mapInput=");
   Serial.print(controls.mapInput);

   Serial.print("controls.tpsInput=");
   Serial.println(controls.tpsInput);
   */
		controls.rpmCorrected = mapValues(controls.rpmActual, 0, settings.rpmMax);
		controls.mapCorrected = mapValues(controls.mapInput, settings.mapMin, settings.mapMax);
		controls.tpsCorrected = mapValues(controls.tpsInput, settings.tpsMin, settings.tpsMax);

		controls.vntMaxDc = mapLookUp(boostDCMax, controls.rpmCorrected, controls.tpsCorrected);
		controls.vntMinDc = mapLookUp(boostDCMin, controls.rpmCorrected, controls.tpsCorrected);

		controls.n75precontrol = mapLookUp(n75precontrolMap, controls.rpmCorrected, controls.tpsCorrected);

		/* Look up the requested boost */
   
		controls.vntTargetPressure = mapLookUp(sportModus?boostRequest:boostRequestNormal, controls.rpmCorrected, controls.tpsCorrected);

 //   Serial.print("controls.n75precontrol=");
 //  Serial.println(controls.n75precontrol);
		/* This is the available span of our DC - we can only go between min and max */
		minControl = controls.vntMinDc - controls.n75precontrol;  // this will be a negative number
		maxControl = controls.vntMaxDc - controls.n75precontrol;  // this will be a positive number

		if (minControl > 0) {
			// Our MinDC map is higher than our precontrol map; oops
     
      
			Serial.print(controls.vntMinDc);
      Serial.println(" : Our MinDC map is higher than our precontrol map; oops");
      Serial.print(", n75precontrol=");
      Serial.print(controls.n75precontrol);
      
			Serial.print("rpmCorrected=");
			Serial.print(controls.rpmCorrected);
			Serial.print(" ,tpsCorrected=");
			Serial.print(controls.tpsCorrected);
			Serial.print("rpmActual=");
			Serial.print(controls.rpmActual);
			Serial.print(" ,tpsInput=");
            Serial.println(controls.tpsInput);
  
			minControl = 0;
		}

		if (maxControl < 0) {
			// Our MaxDC map is lower than our precontrol map; oops
			Serial.print(controls.vntMaxDc);
			Serial.println(" : Our MaxDC map is lower than our precontrol map; oops");
       Serial.print("n75precontrol=");
      Serial.print(controls.n75precontrol);
			Serial.print(", rpmCorrected=");
			Serial.print(controls.rpmCorrected);
			Serial.print(" ,tpsCorrected=");
			Serial.print(controls.tpsCorrected);
			Serial.print("rpmActual=");
			Serial.print(controls.rpmActual);
			Serial.print(" ,tpsInput=");
			Serial.println(controls.tpsInput);
			maxControl = 0;
   		}

		vntPid.SetOutputLimits(minControl, maxControl);

		if ((controls.idling)) {
			// If we are at idle then we don't want any boost regardless of map

			controls.vntTargetPressure = 100;                    // Display zero target pressure on the LCD at idle
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
  //  Serial.print("controls.vntPositionRemapped = ");
  //  Serial.print(controls.vntPositionRemapped);
  //  Serial.print("\r\n");
		// Include the time we spent processing
		controls.lastTime = millis();
	}

	void controlEGT() {
		// EGT controls
		controls.egtCorrected = mapValues(controls.temp1, settings.egtMin, settings.egtMax);
		controls.auxOutput = mapLookUp(auxMap, controls.rpmCorrected, controls.egtCorrected);
	}

	
	
	typedef struct  {
		unsigned char nduty;
		unsigned char wi;
	} vntlda_data;

	unsigned int execTimeRead = 0;
	unsigned int execTimeAct = 0;

	void loop(vntlda_data & data, unsigned short map, unsigned short rpm, unsigned char wpedal, short egt) {
		//data.nduty = 0;
		//data.wi = 0;
    controls.mapInput = map;
		
		/* Actual execution will happen every EXEC_DELAY - this is where we do our actual calculations */
			controls.tpsInput = wpedal;
			controls.temp1 = egt;
		// update output values according to input
			controls.rpmActual = rpm;
			determineIdle();
			controlVNT();
			controls.temp1 = egt; 
			data.nduty = controls.vntPositionRemapped;
			data.wi = controls.auxOutput;
}

	void Init(int sampletime) {
		EXEC_DELAY = sampletime;
    
		pwmfreq = 30.64;
		maxMAP = 2200;
		vntPid.SetSampleTime(sampletime);
		mapAvg.size = AVG_MAX;
    loadDefaults();
		//initial setup of kp/ki/kd
		calcKp();
		calcKi();
		calcKd();
    Serial.println("boostRequest");
   PrintMap(boostRequest);
   Serial.println("");
    Serial.println("boostRequestNormal");
   PrintMap(boostRequestNormal);
   Serial.println("");
   Serial.println("n75precontrolMap");
   PrintMap(n75precontrolMap);

	}

}



#endif
