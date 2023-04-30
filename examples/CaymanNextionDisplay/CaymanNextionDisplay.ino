

#define NO_PRINTLNDATASERIAL
#define DEBUGSERIAL Serial
#define SERIOUSERROR Serial
//#define _DISABLE_CANBUS
//#define _SIMULATEDATA
#include <Can997.h>
#include <BreitBandLambda.h>
#include "CheckSequence.h"
#include "VacuumPump.h"
#include "NextionDisplay.h"
#define OILPUMPIN_PIN 6    // input
char _buffer[70];
PString line(_buffer, sizeof(_buffer));

VacuumPump vacuumpump;
CheckSequence checksequence;

unsigned long startupmill=1; 

NextionDisplay disp(8,9);



void LogError(const char *msg) {
#ifdef DEBUGSERIAL
  Serial.println(msg);
#endif
  disp.Error(msg);
}


void setup() {
#ifdef  SERIOUSERROR
  SERIOUSERROR.begin(115200);
  SERIOUSERROR.println("SERIOUSRERROR => Serial");
#endif
  
  pinMode(OILPUMPIN_PIN,INPUT);
  pinMode(OILPUMP_PIN, OUTPUT);
  digitalWrite(OILPUMP_PIN,LOW);

 disp.begin(9600);  // Start serial comunication at baud=9600
  // I am going to change the Serial baud to a faster rate.
  delay(500);  // This dalay is just in case the nextion display didn't start yet, to be sure it will receive the following command.
  disp.print("baud=115200");  // Set new baud rate of nextion to 115200, but it's temporal. Next time nextion is power on,
                                // it will retore to default baud of 9600.
                                // To take effect, make sure to reboot the arduino (reseting arduino is not enough).
                                // If you want to change the default baud, send the command as "bauds=115200", instead of "baud=115200".
                                // If you change the default baud, everytime the nextion is power ON is going to have that baud rate, and
                                // would not be necessery to set the baud on the setup anymore.
  disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
  disp.write(0xff);
  disp.write(0xff);
  disp.end();  // End the serial comunication of baud=9600
  disp.begin(115200);  // Start serial comunication at baud=115200

  vacuumpump.Init();
  checksequence.Init(&LogError,Head,vacuumpump);
  pinMode(WATER_INJECT_VALVE_PIN,OUTPUT);
  digitalWrite(WATER_INJECT_VALVE_PIN,LOW);
 
   STDERR.fpstderr = &LogError;
   HeadU_Zero();
  EngineMsmtU_Zero();
  startupmill = millis();
  disp.Error("");
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
void loop() {
  
  
  #ifndef _DISABLE_CANBUS
  if (can_result != CAN_OK) {
      Serial.println("CAN0_BeginSlave()");
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
  disp.UpdateError();
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
      line.begin();
      if (Engine.sensor.iatr == iathigher) line.print("iat L");
      else line.print("iat R");
      line.print(" (-");
      line.print(iatdiff);
      line.print(")");
      disp.Error(line);
  }
  disp.IntakeTemp(iathigher, Head.settings.waterinjection);
  disp.Pumps(Head.settings.oilpump != 0, Engine.sensor.gearboxoilpump);
  
  Serial.println(iloop);
  delay(140);
#ifndef NO_PRINTLNDATASERIAL
   PrintlnDataSerial(Engine.sensor,can242,can245);
#endif
}
