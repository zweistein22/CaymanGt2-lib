#include "NextionDisplay.h"
#include "RunningAverage.h"


String error = String("");


//#define DEBUGSERIAL(call) Serial.call

#define DEBUGSERIAL(call) 


bool _rear = true;
bool _front = true;
bool _wi=false;
#define AVERAGE 1
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

    DEBUGSERIAL(print("recvRetString["));
    DEBUGSERIAL(print(temp.length()));
    DEBUGSERIAL(print(","));
    DEBUGSERIAL(print(temp));
    DEBUGSERIAL(println("]"));

    return ret;
}




NextionDisplay::NextionDisplay(int tx, int rx){
    error.reserve(60);
    error = "x"; // so != existing value

}


void NextionDisplay::setup() {
    
    disp.begin(9600);  // Start serial comunication at baud=9600
  // I am going to change the Serial baud to a faster rate.
    delay(500);  // This dalay is just in case the nextion display didn't start yet, to be sure it will receive the following command.
    sendCommand("baud=57600");
    // Set new baud rate of nextion to 115200, but it's temporal. Next time nextion is power on,
                                  // it will retore to default baud of 9600.
                                  // To take effect, make sure to reboot the arduino (reseting arduino is not enough).
                                  // If you want to change the default baud, send the command as "bauds=115200", instead of "baud=115200".
                                  // If you change the default baud, everytime the nextion is power ON is going to have that baud rate, and
                                  // would not be necessery to set the baud on the setup anymore.
    disp.end();  // End the serial comunication of baud=9600
    disp.begin(57600);  // Start serial comunication at baud=115200
    delay(250);

   
}

void NextionDisplay::sendCommand(const char *cmd){

   while (disp.available())
   {
        disp.read();
   }
   disp.print(cmd); 
   disp.write(0xFF);
   disp.write(0xFF);
   disp.write(0xFF);
   DEBUGSERIAL(print("sendCommand("));
   DEBUGSERIAL(print(cmd));
   DEBUGSERIAL(println(")"));
}


NextionDisplay::~NextionDisplay(){

	NeoICSerial::end();
	
}

void NextionDisplay::_setNumber(const char *objname,int number){

  String cmd=String(objname);
  cmd+=".val=";
  cmd+=String(number);
  sendCommand(cmd.c_str());
   
}
void NextionDisplay::EGT(int left, int right, int statusleft, int statusright){

      //DEBUGSERIAL(print("EGTL="));
      //DEBUGSERIAL(println(left));
       egtl.addValue(left);
       _setNumber("egtl", egtl.getAverage());
       egtr.addValue(right);
      _setNumber("egtr",egtr.getAverage());

      

  

  }
	void NextionDisplay::Boost(float hpa){
      bar.addValue((hpa));
      int ihpa = bar.getAverage() *100;
      //Serial.print("ihpa=");
      //Serial.println(ihpa);
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
      if(_wi!=injectwater){
        _wi=injectwater;
       String cmd="iat.bco=";
       cmd+=injectwater?"BLUE":"BLACK";
       sendCommand(cmd.c_str());
      }
  }

	void NextionDisplay::Pumps(bool front, bool rear){
        if (front != _front) {
            _front = front;
             String cmd="hotoil.pic=";
             cmd+=_front ? "0" : "1";
             sendCommand(cmd.c_str());
        }

        if (rear != _rear) {
            _rear = rear;
              String cmd="gear.pic=";
             cmd+=_rear ? "2" : "1";
             sendCommand(cmd.c_str());
        }

  }

 
	void NextionDisplay::Error(const char *err){
    if(!strcmp(err,error.c_str())){
      return; // same string, so do not send to nextion 
    }    
    //DEBUGSERIAL(print("NextionDisplay::Error("));
    //DEBUGSERIAL(println(err));
    error = err;
    sendCommand("page 1");
    sendCommand("tm0.en=0");
    String cmd="e0.txt=\"";
    cmd+=error;
    cmd+="\"";
    sendCommand(cmd.c_str());
    //sendCommand("tm0.tim=20000");
    sendCommand("tm0.en=1");
   // sendCommand("ref e0");
    
  }
  
