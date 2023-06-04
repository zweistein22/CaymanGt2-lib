

#define NO_PRINTLNDATASERIAL

#define INFOSERIAL(call) Serial.call
// or do deactivate  #define INFOSERIAL(call)

//#define _DISABLE_CANBUS
//#define _SIMULATEDATA
#include <Can997.h>
#include <BreitBandLambda.h>
#include "CheckSequence.h"
#include "VacuumPump.h"
#include "NextionDisplay.h"
#define OILPUMPIN_PIN 6    // input

String line = String("");



VacuumPump vacuumpump;
CheckSequence checksequence;

unsigned long startupmill=1; 

NextionDisplay disp(8,9);


#define MINLOOPIME 200

void LogError(const char *msg) {
  INFOSERIAL(println(msg));
  disp.Error(msg);
}


void setup() {
  
  line.reserve(60);
  
  INFOSERIAL(begin(115200));
  INFOSERIAL(println("INFOSERIAL => Serial"));

  
  pinMode(OILPUMPIN_PIN,INPUT);
  pinMode(OILPUMP_PIN, OUTPUT);
  digitalWrite(OILPUMP_PIN,LOW);

  delay(250);
  disp.setup();
  
  vacuumpump.Init();
  checksequence.Init(&LogError,Head,vacuumpump);
  pinMode(WATER_INJECT_VALVE_PIN,OUTPUT);
  digitalWrite(WATER_INJECT_VALVE_PIN,LOW);
 
   HeadU_Zero();
  EngineMsmtU_Zero();
  startupmill = millis();
  
}

float boost = 0;
float ftr=0;
int relais=0;
bool setupdone=false;
bool EngineNeverStarted=true;
unsigned long dmil=0;
unsigned long last_millis = millis();
unsigned long mil = 0;


#ifndef NO_PRINTLNDATASERIAL
MOTOR_1 can242;
MOTOR_2 can245;
#endif

int iloop = 0;
byte can_result = CAN_FAIL;

unsigned long lastloopmillis = millis();
void loop() {
  
  
  #ifndef _DISABLE_CANBUS
  if (can_result != CAN_OK) {
      Serial.println("CAN0_BeginSlave()");
      disp.setup(); // so after can error display is reconnected.
      // can error happen usually when ignition is off for short time only
      can_result = CAN0_BeginSlave();
      delay(500);
    
  }
  #endif

  
  if (can_result == CAN_OK) can_result = CAN0_getbothPrivate(800, Engine);
  iloop++;
  boost = ((float) Engine.sensor.map) / checksequence.pressurehPa - 1.0f;

#ifdef _SIMULATEDATA
  Engine.sensor.lambdaplus100 = (byte)((int)random(-9 + 100, 230));
  Engine.sensor.egtl = random(900, 1100);
  Engine.sensor.egtr = Engine.sensor.egtl + random(-100, 100);
  boost = random(900, 1200) / 1000.0f;
  Engine.sensor.iatl = random(30,110);
  Engine.sensor.nmot100 = random(10,70);
#endif
   mil=millis();
  

  if(Engine.sensor.nmot100 * 100>500) {
       EngineNeverStarted=false;
   }
   if((mil>startupmill+1000*2) && EngineNeverStarted==true && checksequence.ChecksequenceStep==-1){
      // we start check sequence only when not started for 5 seconds
         checksequence.ChecksequenceStep=0;
   }
   checksequence.Continue(mil, Engine.sensor.map);
   Head.settings.waterinjection=false;
  if(boost>0.45 && Engine.sensor.iatl>58 &&  (Engine.sensor.egtl> 700 || Engine.sensor.egtr>700)) {
        Head.settings.waterinjection=true;
  }
  else if (boost>0.15 &&  (Engine.sensor.egtl> 880 || Engine.sensor.egtr>880)){
          Head.settings.waterinjection=true;
  }
  else if(Engine.sensor.iatl>80 &&  boost>-0.1 && (Engine.sensor.egtl> 700 || Engine.sensor.egtr>700)) {
      Head.settings.waterinjection=true;
  }
  if(Engine.sensor.nmot100*100<3000 && Engine.sensor.iatl<80) Head.settings.waterinjection=false;
  if(Engine.sensor.nmot100 * 100<2200) Head.settings.waterinjection=false;
  if(Engine.sensor.nmot100 * 100>300) {
     digitalWrite(WATER_INJECT_VALVE_PIN, Head.settings.waterinjection == true ? HIGH : LOW);
     if (Head.settings.waterinjection) vacuumpump.Start(mil);
     else vacuumpump.Stop();
     digitalWrite(OILPUMP_PIN, EngOilTemp(Engine.sensor.Tmot) > 120 ? HIGH : LOW);
  }
  vacuumpump.Update(mil);
  Head.settings.oilpump = digitalRead(OILPUMPIN_PIN);

 
  disp.EGT(Engine.sensor.egtl, Engine.sensor.egtr, (int)Engine.sensor.EGT_Status_left, (int)Engine.sensor.EGT_Status_right);
  disp.Boost(boost);
  disp.Lambda( Engine.sensor.lambdaplus100-100,Engine.sensor.llambdaplus100-100);

  int iathigher = Engine.sensor.iatl;
  if (Engine.sensor.iatr > iathigher) iathigher = Engine.sensor.iatr;
  int iatdiff = abs(Engine.sensor.iatl - Engine.sensor.iatr);
  if (iatdiff > 15) {
      line="";
      if (Engine.sensor.iatr == iathigher) line+="iat L";
      else line+="iat R";
      line+=" (-" + String(iatdiff) + ")";
      disp.Error(line.c_str());
  }
  disp.IntakeTemp(iathigher, Head.settings.waterinjection);
  disp.Pumps(Head.settings.oilpump != 0, Engine.sensor.gearboxoilpump);
  
  
unsigned long looptime = millis() - lastloopmillis;
   if (looptime < MINLOOPIME ) {
    delay(MINLOOPIME - looptime);
    looptime = millis() - lastloopmillis;
   }
   //INFOSERIAL(print("looptime (ms) : "));
   //INFOSERIAL(println(looptime));
   lastloopmillis = millis();

#ifndef NO_PRINTLNDATASERIAL
   PrintlnDataSerial(Engine.sensor,can242,can245);
#endif
}
