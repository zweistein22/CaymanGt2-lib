#include "NextionDisplay.h"
#include <PString.h>
#include "RunningAverage.h"

char _ebuffer[70];
PString error(_ebuffer, sizeof(_ebuffer));

#ifdef DEBUGSERIAL
#define dbSerialPrint(a)    DEBUGSERIAL.print(a)
#define dbSerialPrintln(a)  DEBUGSERIAL.println(a)
#define dbSerialBegin(a)    DEBUGSERIAL.begin(a)
#else
#define dbSerialPrint(a)    do{}while(0)
#define dbSerialPrintln(a)  do{}while(0)
#define dbSerialBegin(a)    do{}while(0)
#endif


bool _rear = true;
bool _front = true;

#define AVERAGE 5
RunningAverage egtl(AVERAGE);
RunningAverage egtr(AVERAGE);
RunningAverage iat(AVERAGE);
RunningAverage bar(AVERAGE);


#define NEX_RET_STRING_HEAD                 (0x70)
uint16_t recvRetString(char *buffer, uint16_t len, uint32_t timeout= 100)
{
    uint16_t ret = 0;
    bool str_start_flag = false;
    uint8_t cnt_0xff = 0;
    String temp = String("");
    uint8_t c = 0;
    long start;

    if (!buffer || len == 0)
    {
        goto __return;
    }
    
    start = millis();
    while (millis() - start <= timeout)
    {
        while (disp.available())
        {
            c = disp.read();
            if (str_start_flag)
            {
                if (0xFF == c)
                {
                    cnt_0xff++;                    
                    if (cnt_0xff >= 3)
                    {
                        break;
                    }
                }
                else
                {
                    temp += (char)c;
                }
            }
            else if (NEX_RET_STRING_HEAD == c)
            {
                str_start_flag = true;
            }
        }
        
        if (cnt_0xff >= 3)
        {
            break;
        }
    }

    ret = temp.length();
    ret = ret > len ? len : ret;
    strncpy(buffer, temp.c_str(), ret);
    
__return:

    dbSerialPrint("recvRetString[");
    dbSerialPrint(temp.length());
    dbSerialPrint(",");
    dbSerialPrint(temp);
    dbSerialPrintln("]");

    return ret;
}

NextionDisplay::NextionDisplay(int tx, int rx){
lasterrortime=millis();


}

NextionDisplay::~NextionDisplay(){

	NeoICSerial::end();
	
}

void NextionDisplay::_setNumber(char *objname,int number){

  char buf[10] = {0};
  utoa(number, buf, 10);
   disp.write(objname);
   disp.write(".val=");
      disp.write(buf);
      disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
      disp.write(0xff);
      disp.write(0xff);
  
}
void NextionDisplay::EGT(int left, int right, int statusleft, int statusright){

      //Serial.print("EGTL=");
     // Serial.println(left);
       egtl.addValue(left);
       _setNumber("egtl", egtl.getAverage());
       egtr.addValue(right);
      _setNumber("egtr",egtr.getAverage());

      

  

  }
	void NextionDisplay::Boost(float hpa){
     char _fbuffer[4];
     PString f(_fbuffer, sizeof(_fbuffer));
      bar.addValue((hpa));

       Serial.print("hpa=");
      Serial.println(bar.getAverage());
      int ihpa = hpa*100;
      Serial.print("ihpa=");
      Serial.println(ihpa);
      _setNumber("boost",ihpa);
      
  }
	void NextionDisplay::Lambda(int lambda, int llamb){
    /*
      disp.write("lamb.val=");

      disp.write(lambda);b
      disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
      disp.write(0xff);
      disp.write(0xff);
      
      disp.write("llamb.val=");

      disp.write(llamb);
      disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
      disp.write(0xff);
      disp.write(0xff);
*/      
  };
	void NextionDisplay::IntakeTemp(int air, bool injectwater){

      iat.addValue(air);
      _setNumber("iat",iat.getAverage());
       disp.write("iat.bco=");
       disp.write(injectwater?"BLUE":"BLACK");         
       disp.write(0xff);disp.write(0xff);disp.write(0xff);
      
  }
	void NextionDisplay::Pumps(bool front, bool rear){
        if (front != _front) {
            _front = front;
            disp.write("hotoil.pic=");
            disp.write(front ? "0" : "1");
            disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
            disp.write(0xff);
            disp.write(0xff);
        }

        if (rear != _rear) {
            _rear = rear;
            disp.write("gear.pic=");
            disp.write(rear ? "2" : "1");
            disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
            disp.write(0xff);
            disp.write(0xff);
        }
//    disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
//      disp.write(0xff);
//      disp.write(0xff);
  }
	void NextionDisplay::Error(const char *err){
    
    disp.write("get err.text");
    disp.write(0xff);  disp.write(0xff);  disp.write(0xff);
    recvRetString(_ebuffer,70);
    Serial.print("get error=");
    Serial.println(error);

    if(error==err){
      return;
    }    
    
    error.begin();
    error.print(err);   
    lasterrortime=millis(); 

    disp.write("err.txt=\"");
    disp.write(err);
    disp.write("\"");
    disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
    disp.write(0xff);
    disp.write(0xff);
  }
  void NextionDisplay::UpdateError(){
    if(error=="") return;
   if(millis()-lasterrortime > 15000) {
       Error("");  
       }    
  }