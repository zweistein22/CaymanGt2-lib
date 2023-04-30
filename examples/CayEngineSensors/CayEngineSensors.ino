#include <EEPROM.h>
#include <ResponsiveAnalogRead.h>
#include <PID_v1.h>
#define LAMBDASERIAL Serial3
#define LAMBDA2SERIAL Serial2
//#define DEBUGSERIAL Serial

#define INFOSERIAL Serial
#include <BreitBandLambda.h>
#include <PString.h>
#include <Adafruit_MAX31855.h>
#include <Can997.h>
#include "vntlda.hpp"

#define MINLOOPIME 100  //ms better more than 140ms 

#define SKIPREADEGTEACH 3


unsigned int nduty;




int EGTL_PIN_CLK = 52;
int EGTL_PIN_CS = 9;
int EGTL_PIN_DO = 50;
Adafruit_MAX31855 egt_left(EGTL_PIN_CS);
int EGTR_PIN_CLK = 52;
int EGTR_PIN_CS = 11;
int EGTR_PIN_DO = 50;   // analog out A2 , A3 A3
Adafruit_MAX31855 egt_right(EGTR_PIN_CS);
int WATERPUMPS_PIN =40;
int GBOXOILPUMP_PIN =38;

int N75 = 44;


#define AIRR_PIN A1                 // Analog Pin 1   , rechter IC
#define AIRL_PIN A2              // Analog pin 2  , linker IC, 
#define AIRBEFOREIC_PIN A3            // Analog Pin 3
#define MAP_PIN A4
//#define LA_PIN A7
//#define LA_START A8
/*
 * 
 * 
 * Inputs ADC Value from Thermistor and outputs Temperature in Celsius
 *  requires: include <math.h>
 * Utilizes the Steinhart-Hart Thermistor Equation:
 *    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]3}
 *    where A = 0.001129148, B = 0.000234125 and C = 8.76741E-08
 *
 * These coefficients seem to work fairly universally, which is a bit of a 
 * surprise. 
 *
 * Schematic:
 *
 *
 *   [VSupply (5 V)] -- [1k-pad-resistor] -- | -- [thermistor] --[Ground]
 *                                     
 *                                           |
 *                                           *                                           |
 *                                      Analog Pin 1  - 100nF -- [Ground]
 *
 
 */
float AREF = 4.98;                       // only used for display purposes, if used
float pad = 1010;                       // balance/pad resistor value, set this to
                                        // the measured resistance of your pad resistor
// Bosch 0280130039  , same as in MAF
float STEINH_A0=-2.50526E-4;
float STEINH_A1=8.919852E-4;
float STEINH_A2=-8.44704E-5;
float STEINH_A3=3.870370E-6;

long lAREF = 498;
 long lxy[][2]={{30,20000},{480,250000}}; // multiply 100
 long la = (lxy[1][1]-lxy[0][1])/(lxy[1][0]-lxy[0][0]);
 long lb = lxy[1][1]-la *lxy[1][0];
 long llgm = lxy[0][1];
int iManifoldAbsolutePressurehPa(int rawADC){
 
  long x = (lAREF * rawADC);
  long y = (la*x)/1024+lb;

  if (y < llgm) y = llgm;
  if (y > lxy[1][1])  y= lxy[1][1];
  return (int)(y/100);
}

float AnalogLambda(int RawADC){
	//Serial.print("RawADC=");
	//Serial.println(RawADC);
  float v=0.7f+  (0.6f *RawADC)/1024.0f ; //*vcc/5.0f;
  return v;
  
}
float AirResistance(int RawADC){
  // takes into account pad resistance
  float r;  
  float vpin=RawADC*AREF/1024.0f;
  r= (pad*vpin)/(AREF-vpin); 
  return r;
  
}
float AirThermistor(int RawADC) {
  float Temp;  // Dual-Purpose variable to save space.
  float r;
  Temp = log(AirResistance(RawADC)); // Saving the Log(resistance) so not to calculate  it 4 times later
  r = STEINH_A0+STEINH_A1*Temp+STEINH_A2*Temp*Temp+STEINH_A3*Temp*Temp*Temp;
   return 1.0/r-273.15;                                      // Return the Temperature
}



MOTOR_1 can242;
MOTOR_2 can245;
//BREMSE_1 can14A;

class __WaterpumpsIC {

	int hysterese = 7;
	int onabove = 53;
	long maxnachlauf = 2*60*1000; // 5 min
	long lastnmot = 0;
	bool started = false;
public:
	 bool init(int pin) {
		 pinMode(pin, OUTPUT);
		 digitalWrite(pin, started);
#ifdef INFOSERIAL
	    if(started)  INFOSERIAL.print("NTK WP started -1\n\r");
#endif
	}

	 bool Update() {

		 if (started) {
			 if (max(Engine.sensor.iatl,Engine.sensor.iatr) < onabove - hysterese ) {
				 digitalWrite(WATERPUMPS_PIN, LOW);
         		 started = false;
#ifdef INFOSERIAL
				 INFOSERIAL.print(Engine.sensor.iatl);
				 INFOSERIAL.print(" (iatl),");
				 INFOSERIAL.print(Engine.sensor.iatr);
				 INFOSERIAL.print(" (iatr) :");
				 INFOSERIAL.print("NTK WP stopped, \n\r");
#endif
			 }

			 if (can242.nmot == 0) {
				 if (lastnmot == 0) lastnmot = millis();

				 if (millis() > lastnmot + maxnachlauf) {
					 digitalWrite(WATERPUMPS_PIN, LOW);
#ifdef INFOSERIAL
					 INFOSERIAL.print("NTK WP stopped1\n\r");
#endif
					 started = false;
				 }
			 }
			 else {
				 if (lastnmot != 0) lastnmot = 0;
			 }
			 
		 }

		 else {
			 if (max(Engine.sensor.iatl, Engine.sensor.iatr) > (word)onabove && (can242.nmot / 4 > 500)) {
				 digitalWrite(WATERPUMPS_PIN, HIGH);
#ifdef INFOSERIAL
				 INFOSERIAL.print(Engine.sensor.iatl);
				 INFOSERIAL.print(" (iatl),");
				 INFOSERIAL.print(Engine.sensor.iatr);
				 INFOSERIAL.print(" (iatr) :");
				 INFOSERIAL.print("NTK WP started, \n\r");
#endif
				 started = true;
			 }
		 }
		
		

	}

};

__WaterpumpsIC WaterPumpIC;


unsigned long lastmill=millis();
float avgTimeMs=3*60*1000.0f;  // 3 minutes
float ftrack=0;
unsigned long mil=0;
bool gboilpumpon = false;

float fTrackUse(unsigned long mil,float egtl,float egtr,float map){
  unsigned long dmi=mil-lastmill;
  float v=0;
  if(egtl>700 && egtr>700 && map>1400)  v=1;
  if(map>1600) v=1;
#ifdef DEBUGSERIAL
  DEBUGSERIAL.print("dmi=");
  DEBUGSERIAL.print(dmi);
  DEBUGSERIAL.print("avgTime=");
  DEBUGSERIAL.print(avgTimeMs);
#endif
  float p=dmi/avgTimeMs;
  if(p==0) return ftrack;
  ftrack=  p*v+ (1-p)*ftrack;
  lastmill=mil;
  return ftrack;
}

 boolean gb=false;


#if  defined(__AVR_ATmega2560__)
 void setPwmFrequencyMEGA2560(int pin, int divisor) {
	 byte mode;
	 switch (divisor) {
	 case 1: mode = 0x01; break;
	 case 2: mode = 0x02; break;
	 case 3: mode = 0x03; break;
	 case 4: mode = 0x04; break;
	 case 5: mode = 0x05; break;
	 case 6: mode = 0x06; break;
	 case 7: mode = 0x07; break;
	 default: return;
	 }

	 switch (pin) {
	 case 2:  TCCR3B = TCCR3B & 0b11111000 | mode; break;
	 case 3:  TCCR3B = TCCR3B & 0b11111000 | mode; break;
	 case 4:  TCCR0B = TCCR0B & 0b11111000 | mode; break;
	 case 5:  TCCR3B = TCCR3B & 0b11111000 | mode; break;
	 case 6:  TCCR4B = TCCR4B & 0b11111000 | mode; break;
	 case 7:  TCCR4B = TCCR4B & 0b11111000 | mode; break;
	 case 8:  TCCR4B = TCCR4B & 0b11111000 | mode; break;
	 case 9:  TCCR2B = TCCR0B & 0b11111000 | mode; break;
	 case 10: TCCR2B = TCCR2B & 0b11111000 | mode; break;
	 case 11: TCCR1B = TCCR1B & 0b11111000 | mode; break;
	 case 12: TCCR1B = TCCR1B & 0b11111000 | mode; break;
	 case 13: TCCR0B = TCCR0B & 0b11111000 | mode; break;
	 default: return;
	 }

 }

#endif


void setup() {
  
  pinMode(GBOXOILPUMP_PIN,OUTPUT);
  pinMode(N75,OUTPUT);
  pinMode(MAP_PIN,INPUT);
//  pinMode(LA_PIN, INPUT);
 // pinMode(LA_START, OUTPUT); // LAMBDA START if set to HIGH
  delay(250);
  Serial.begin(115200);
  Serial.println("CayEngineSensor.startup");
  vntlda::Init(MINLOOPIME);
  delay(4000);
  WaterPumpIC.init(WATERPUMPS_PIN);
 // digitalWrite(LA_START, HIGH);
  setPwmFrequencyMEGA2560(N75,5);
}




int rskip=0;
void ReadEGTs() {
	double tl, tr;

  if(!(rskip++%SKIPREADEGTEACH)) return;
	tl = egt_left.readCelsius();
	Engine.sensor.egtl = (int)tl;
	// Engine.sensor.egtl=random(280,1030);
	tr = egt_right.readCelsius();
	Engine.sensor.egtr = (int)tr;
	Engine.sensor.EGT_Status_left = MAX31855OK;

	if (isnan(tl)) {
		uint8_t v = egt_left.readError();
		if (v & 0x04) { Engine.sensor.EGT_Status_left = SCV; }
		else if (v & 0x02) { Engine.sensor.EGT_Status_left = SCG; }
		else if (v & 0x01) Engine.sensor.EGT_Status_left = OC;


	}
	Engine.sensor.EGT_Status_right = MAX31855OK;
	if (isnan(tr)) {
		uint8_t v = egt_right.readError();
		if (v & 0x04) { Engine.sensor.EGT_Status_right = SCV; }
		else if (v & 0x02) { Engine.sensor.EGT_Status_right = SCG; }
		else if (v & 0x01) Engine.sensor.EGT_Status_right = OC;


	}


}

  
float ftr = 0;

float alambda;
int iloop=0;

unsigned long lastloopmillis = millis();
INT8U can_rv = CAN_FAIL;
INT8U can1_rv = CAN_FAIL;

unsigned int oldduty = 0;

vntlda::vntlda_data vd;

int can0retry=0;
int can1retry=0;
int serretry=0;
int ser2retry=0;

int CAN0RETRYAFTER = 100;
int CAN1RETRYAFTER = 100;
int SERRETRYAFTER = 100;
int SER2RETRYAFTER = 100;

void loop() {
	//Serial.println(iloop);
	Engine.sensor.status = OK;
	  if ((can_rv != CAN_OK) && ( !(can0retry%CAN0RETRYAFTER))) {
		  //Serial.println("CAN_begin() called");
		  can_rv=CAN0_BeginMaster();
		  if (can_rv != CAN_OK) {
			  Serial.print("CAN0_BeginMaster() can_rv=");
			  Serial.println(can_rv);
       can0retry++;
			  
		  }
     else can0retry=0;
	  }
	  if (can_rv == CAN_OK) {
		  can_rv = CAN0_get242(200, can242);
		  can_rv = CAN0_get245(150, can245);
      //can_rv = CAN0_get14A(150,can14A);
		  Engine.sensor.nmot100 = can242.nmot /(4*100);
		  Engine.sensor.Tmot = can245.Tmot;
	  }
	 

   
	  Engine.sensor.iatl=(int)AirThermistor(analogRead(AIRL_PIN));       // read ADC and  convert it to Celsius
	  Engine.sensor.iatr = (int) AirThermistor(analogRead(AIRR_PIN));       // read ADC and  convert it to Celsius
	  Engine.sensor.map=(int)iManifoldAbsolutePressurehPa(analogRead(MAP_PIN));

	  
	  vntlda::loop(vd,Engine.sensor.map, can242.nmot/4, can242.Wped_fgr , max(Engine.sensor.egtl,Engine.sensor.egtr));

    nduty = (unsigned int) vd.nduty;

       /*
	  if (can242.nmot/4  < 1250 || can242.Wped_fgr< 63) { // 63/254 is roughly 25%
		  vd.nduty = 0;
	  }
	  else vd.nduty = 215; // ~ 0,8 bar 
	  */
	  if (oldduty != vd.nduty) {
		  analogWrite(N75, oldduty);
		  oldduty = vd.nduty;
	  }
	 
	  Engine.sensor.wi = vd.wi;

	  Engine.sensor.iatbeforeIC = (int)AirThermistor(analogRead(AIRBEFOREIC_PIN));
#ifdef LAMBDASERIAL
    if(!(serretry%SERRETRYAFTER)){
        Engine.sensor.lambdaplus100 = (byte)(100 + LAMBDA1.Lambda());
        if (Engine.sensor.llambdaplus100 == 100 - SerialReadChksumError) {
              serretry++;
              LAMBDA1.initdone=false;
        }
        else serretry=0;
    }
    else serretry++;
 #endif   
    
	  
#ifdef LAMBDA2SERIAL

   if(!(ser2retry%SER2RETRYAFTER)){
    Engine.sensor.llambdaplus100 = (byte)(100 + LAMBDA2.Lambda());
    if (Engine.sensor.llambdaplus100 == 100 - SerialReadChksumError) {
        ser2retry++;
        LAMBDA2.initdone = false;
    }
    #ifndef LAMBDASERIAL
       Engine.sensor.lambdaplus100 = Engine.sensor.llambdaplus100;
    #endif
   }
   else ser2retry++;
	  
#endif
	 
	  boolean btrack=false;
    
       ftr=fTrackUse(mil,Engine.sensor.egtl,Engine.sensor.egtr, Engine.sensor.map);
       if(ftr>0.25) {btrack=true;}
           
      if(Engine.sensor.gearboxoilpump!=btrack)   Engine.sensor.gearboxoilpump=btrack;
		  
	  if (can242.nmot/4 < 500) Engine.sensor.gearboxoilpump = 0;
	  if (can_rv != CAN_OK) { Engine.sensor.gearboxoilpump = 0; }
	  if (Engine.sensor.gearboxoilpump != gboilpumpon) {
		  digitalWrite(GBOXOILPUMP_PIN, Engine.sensor.gearboxoilpump);
		  gboilpumpon = Engine.sensor.gearboxoilpump;
	  }
	  
	 
	  WaterPumpIC.Update();

	  if (can1_rv != CAN_OK && !(can1retry%100)) {
		  //Serial.println("CAN_begin() called");
		  can1_rv = CAN1_BeginMaster();
		  if (can1_rv != CAN_OK) {
        can1retry++;
			  Serial.print("CAN1_BeginMaster() can1_rv=");
			  Serial.println(can1_rv);
			 // delay(500);
		  }
     else can1retry=0;
	  }
	  if (can1_rv == CAN_OK)  can1_rv = CAN1_sendbothPrivate(Engine);
	    
	 ReadEGTs(); 
	 PrintlnDataSerial(Engine.sensor,can242,can245);
	 unsigned long looptime = millis() - lastloopmillis;
	 if (looptime < MINLOOPIME) delay(MINLOOPIME - looptime);
	 lastloopmillis = millis();
   iloop++;
   
}
