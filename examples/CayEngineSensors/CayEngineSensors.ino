#define LAMBDASERIAL Serial2
#define LAMBDA2SERIAL Serial3
#define DEBUGSERIAL Serial
#include <BreitBandLambda.h>
#include <PString.h>
#include <Adafruit_MAX31855.h>
#include <Can997.h>

#define INFOSERIAL Serial


int EGTL_PIN_CLK = 52;
int EGTL_PIN_CS = 11;
int EGTL_PIN_DO = 50;
Adafruit_MAX31855 egt_left(EGTL_PIN_CS);
int EGTR_PIN_CLK = 52;
int EGTR_PIN_CS = 9;
int EGTR_PIN_DO = 50;   // analog out A2 , A3 A3
Adafruit_MAX31855 egt_right(EGTR_PIN_CS);
int WATERPUMPS_PIN =40;
int GBOXOILPUMP_PIN =6;

int N75 = 3;
char n75duty=0; //0%

#define AIRR_PIN A1                 // Analog Pin 1   , rechter IC
#define AIRL_PIN A2              // Analog pin 2  , linker IC, 
#define AIRBEFOREIC_PIN A3            // Analog Pin 3
#define MAP_PIN A4
#define LA_PIN A7
#define LA_START A8
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

 float xy[][2]={{0.25,110},{4.75,2540}};
 float a = (xy[1][1]-xy[0][1])/(xy[1][0]-xy[0][0]);
 float b = xy[1][1]-a *xy[1][0];
 float lgm = xy[0][1];
float ManifoldAbsolutePressurehPa(int rawADC){
 
  
  float x = AREF*rawADC/1024.0f;
  float y = a*x+b;
  if (y > xy[0][1] && y < xy[1][1]) lgm = y;
  return lgm;
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

class __WaterpumpsIC {

	int hysterese = 7;
	int onabove = 58;
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

void setup() {

  //---------------------------------------------- Set PWM frequency for D2, D3 & D5 ---------------------------
 
//TCCR3B = TCCR3B & B11111000 | B00000001;    // set timer 3 divisor to     1 for PWM frequency of 31372.55 Hz
//TCCR3B = TCCR3B & B11111000 | B00000010;    // set timer 3 divisor to     8 for PWM frequency of  3921.16 Hz
//  TCCR3B = TCCR3B & B11111000 | B00000011;    // set timer 3 divisor to    64 for PWM frequency of   490.20 Hz
//TCCR3B = TCCR3B & B11111000 | B00000100;    // set timer 3 divisor to   256 for PWM frequency of   122.55 Hz
TCCR3B = TCCR3B & B11111000 | B00000101;    // set timer 3 divisor to  1024 for PWM frequency of    30.64 Hz
  //---------------------------------------------- Set PWM frequency for D11 & D12 -----------------------------
  pinMode(GBOXOILPUMP_PIN,OUTPUT);
  pinMode(N75,OUTPUT);
  pinMode(MAP_PIN,INPUT);
  pinMode(LA_PIN, INPUT);
 // pinMode(LA_START, OUTPUT); // LAMBDA START if set to HIGH
  delay(250);
  Serial.begin(115200);
  Serial.println("CayEngineSensor.startup");
  WaterPumpIC.init(WATERPUMPS_PIN);
 // digitalWrite(LA_START, HIGH);
}





void ReadEGTs() {
	double tl, tr;
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
float maxMAP=1950;
float alambda;
int iloop=0;
#define MINLOOPIME 100
unsigned long lastloopmillis = millis();
INT8U can_rv = CAN_FAIL;

void loop() {
	//Serial.println(iloop);
	Engine.sensor.status = OK;
	  Engine.sensor.map = ManifoldAbsolutePressurehPa(analogRead(MAP_PIN));

	  if (can_rv != CAN_OK) {
		  //Serial.println("CAN_begin() called");
		  can_rv=CAN_BeginMaster();
		  if (can_rv != CAN_OK) {
			  Serial.print("CAN_BeginMaster() can_rv=");
			  Serial.println(can_rv);
			  delay(500);
		  }
	  }
	  if (can_rv == CAN_OK) {
		  can_rv = getCan242(200, can242);
		  can_rv = getCan245(150, can245);
		  Engine.sensor.nmot100 = can242.nmot /(4*100);
		  Engine.sensor.Tmot = can245.Tmot;
	  }
	 

    if(Engine.sensor.map < 1400){
      analogWrite(N75,0);
    }
    else analogWrite(N75,n75duty);
    if(Engine.sensor.map > maxMAP){
      
    }
	  Engine.sensor.iatl=(int)AirThermistor(analogRead(AIRL_PIN));       // read ADC and  convert it to Celsius
	  Engine.sensor.iatr = (int) AirThermistor(analogRead(AIRR_PIN));       // read ADC and  convert it to Celsius
	  Engine.sensor.map=(int)ManifoldAbsolutePressurehPa(analogRead(MAP_PIN));
	  Engine.sensor.iatbeforeIC = (int)AirThermistor(analogRead(AIRBEFOREIC_PIN));
	  Engine.sensor.lambdaplus100 = (byte)(100 + LAMBDA1.Lambda());
#ifdef LAMBDA2SERIAL
	  Engine.sensor.llambdaplus100 = (byte)(100 + LAMBDA2.Lambda());
#endif

	  boolean btrack=false;
    
       ftr=fTrackUse(mil,Engine.sensor.egtl,Engine.sensor.egtr, Engine.sensor.map);
       if(ftr>0.25) {btrack=true;}
           
      if(Engine.sensor.gearboxoilpump!=btrack)   Engine.sensor.gearboxoilpump=btrack;
		  
	  if (can242.nmot/4 < 500) Engine.sensor.gearboxoilpump = 0;
	  digitalWrite(GBOXOILPUMP_PIN, Engine.sensor.gearboxoilpump);
	  WaterPumpIC.Update();
	  if (can_rv == CAN_OK) {
		  can_rv = sendBothPrivateCan(Engine);
		
	  }
	 ReadEGTs(); 
	 PrintlnDataSerial(Engine.sensor,can242,can245);
	 
	  unsigned long looptime = millis() - lastloopmillis;
	  
	  if (looptime < MINLOOPIME) delay(MINLOOPIME - looptime);
	  lastloopmillis = millis();
      iloop++;
   
}
