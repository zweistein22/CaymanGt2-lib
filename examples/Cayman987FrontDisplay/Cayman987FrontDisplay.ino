

#define NO_PRINTLNDATASERIAL
#define DEBUGSERIAL Serial
#define SERIOUSERROR Serial

#include <Can997.h>
#include <BreitBandLambda.h>
#include <Goldelox_Serial_4DLib.h>
#include <Goldelox_Const4D.h>
#include <CaymanDisplayGoldeloxB.h>
#include <CheckSequence.h>
#include <DisplayStderr.h>
#include <HoldMaximum.h>
#include <VacuumPump.h>




void Callback(int ErrCode, unsigned char ErrByte);
CaymanDisplay disp(8,9,Callback);
void LogError(const char *msg) {
#ifdef DEBUGSERIAL
  Serial.println(msg);
#endif
  disp.Error(msg);
}


#define OILPUMPIN_PIN 6    // input
char _buffer[70];
PString line(_buffer, sizeof(_buffer));

  VacuumPump vacuumpump;
  CheckSequence checksequence;
 
 
  void(*resetarduino)(void) = 0;


  int nharderror = 0;

void Callback(int ErrCode, unsigned char ErrByte)
{
  const char  str_OK[]  = "OK\0";
  const char  str_Timeout[] = "Timeout\0";
  const char  str_NAK[]  = "NAK\0";
  const char  str_Length[] = "Length\0";
  const char  str_Invalid[] = "Invalid\0";
  
  const char * const Error4DText[]  = {str_OK,str_Timeout,str_NAK,str_Length,str_Invalid} ;
  
  line.begin();
  line.print("4DLib  ") ;
  if (ErrCode >= 0 && ErrCode <= 4) {
    line.print(Error4DText[ErrCode]);
  }
  else {
    line.print(" ErrCode=");
    line.print(ErrCode);
  }
  if (ErrCode == Err4D_NAK)  {
    line.print(" returned data= ") ;
    line.println(ErrByte) ;
  
  }
#ifdef  SERIOUSERROR
  SERIOUSERROR.println(line);
  
  Serial.print("cpos[line,column]=");
  Serial.print(disp.cpy);
  Serial.print(",");
  Serial.print(disp.cpx);
  Serial.print(",char=");
  Serial.println(disp.c[disp.cpy][disp.cpx]);

#endif
  disp.Error(line);
  nharderror++;
  if (nharderror > 0) {
   
#ifdef  SERIOUSERROR
    SERIOUSERROR.println("display.init()");
#endif
    disp.init();
  }
 
}

unsigned long startupmill=1; 
void setup() {
#ifdef  SERIOUSERROR
  SERIOUSERROR.begin(115200);
  SERIOUSERROR.println("SERIOUSRERROR => Serial");
#endif
  
  pinMode(OILPUMPIN_PIN,INPUT);
  pinMode(OILPUMP_PIN, OUTPUT);
  digitalWrite(OILPUMP_PIN,LOW);

  vacuumpump.Init();
  checksequence.Init(&LogError,Head,vacuumpump);
  pinMode(WATER_INJECT_VALVE_PIN,OUTPUT);
  digitalWrite(WATER_INJECT_VALVE_PIN,LOW);
 
   STDERR.fpstderr = &LogError;
   HeadU_Zero();
  EngineMsmtU_Zero();
  disp.init();
  startupmill = millis();
  //disp.Error("abcdefghijklmnopqrstuvwxyz0123456789");
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
  
  
  
  if (can_result != CAN_OK) {
      Serial.println("CAN0_BeginSlave()");
      can_result = CAN0_BeginSlave();
      delay(500);
      //if (MCP2515_FAIL == can_result) resetarduino();
  }
  
  if (can_result == CAN_OK) can_result = CAN0_getbothPrivate(800, Engine);
  iloop++;
  boost = ((float) Engine.sensor.map) / checksequence.pressurehPa - 1.0f;
  //Engine.sensor.lambdaplus100 = (byte)((int)random(-9 + 100, 230));
  //Engine.sensor.egtl = random(900, 1100);
  //Engine.sensor.egtr = Engine.sensor.egtl + random(-100, 100);
  //boost = random(900, 1200) / 1000.0f;
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
#ifndef NO_PRINTLNDATASERIAL
   PrintlnDataSerial(Engine.sensor,can242,can245);
#endif
}
