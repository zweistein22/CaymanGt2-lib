
#include <PString.h>
#include <Can997.h>



MOTOR_1 can242;
MOTOR_2 can245;



unsigned long lastmill=millis();
float avgTimeMs=3*60*1000.0f;  // 3 minutes
float ftrack=0;
unsigned long mil=0;

 boolean gb=false;

void setup() {
   Serial.begin(115200);
  Serial.println("CayEngineSensor.startup");
  delay(2000);
 // digitalWrite(LA_START, HIGH);
}

  
int iloop=0;
#define MINLOOPIME 100
unsigned long lastloopmillis = millis();

void loop() {
	//Serial.println(iloop);
	Engine.sensor.status = OK;
	Engine.sensor.EGT_Status_left = MAX31855OK;
	Engine.sensor.EGT_Status_right = MAX31855OK;
	 
	  int Tmot = 78;
	  int rpm = 4000;
	  int egtl = 940;
	  int egtr = 930;
	  int map = random(930,1700);


	  Engine.sensor.egtl = egtl;
	  Engine.sensor.egtr = egtr;

	  Engine.sensor.iatl = 25;
	  Engine.sensor.iatr = 25;
	  Engine.sensor.map=map;
	  
	  can242.nmot = rpm * 4;
	  Engine.sensor.nmot100 = can242.nmot / (4* 100);
	  Engine.sensor.iatbeforeIC = 16;
	  Engine.sensor.lambdaplus100 = (byte)(100 + 97);
	  Engine.sensor.llambdaplus100 = (byte)(100 + 98);
	   	

	  PrintlnDataSerial(Engine.sensor,can242,can245);
	 
	  unsigned long looptime = millis() - lastloopmillis;
	  
	  if (looptime < MINLOOPIME) delay(MINLOOPIME - looptime);
	  lastloopmillis = millis();
      iloop++;
   
}
