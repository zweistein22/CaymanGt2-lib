#include "NextionDisplay.h"
#include "RunningAverage.h"


String error = String("");


#define DEBUGSERIAL(call) Serial.call

// #define DEBUGSERIAL 


bool _rear = true;
bool _front = true;

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
    lasterrortime=millis();
 
    error = "ÿ"; // so != existing value

}

void NextionDisplay::setup() {
    

    disp.begin(9600);  // Start serial comunication at baud=9600

    disp.print("rest"); // reset device
    disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
    disp.write(0xff);
    disp.write(0xff);
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

      DEBUGSERIAL(print("EGTL="));
      DEBUGSERIAL(println(left));
       egtl.addValue(left);
       _setNumber("egtl", egtl.getAverage());
       egtr.addValue(right);
      _setNumber("egtr",egtr.getAverage());

      

  

  }
	void NextionDisplay::Boost(float hpa){
      bar.addValue((hpa));
      int ihpa = bar.getAverage() *100;
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
            disp.write(_front ? "0" : "1");
            disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
            disp.write(0xff);
            disp.write(0xff);
            DEBUGSERIAL(print("hotoil.pic= "));
            DEBUGSERIAL(println(_front ? "0" : "1"));

            
        }

        if (rear != _rear) {
            _rear = rear;
            disp.write("gear.pic=");
            disp.write(_rear ? "2" : "1");
            disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
            disp.write(0xff);
            disp.write(0xff);
              DEBUGSERIAL(print("gear.pic= "));
            DEBUGSERIAL(println(_rear ? "2" : "1"));
        }
//    disp.write(0xff);  // We always have to send this three lines after each command sent to nextion.
//      disp.write(0xff);
//      disp.write(0xff);
  }

 
	void NextionDisplay::Error(const char *err){
    
    
    
     if(!strcmp(err,error.c_str())){
      return; // same string, so do not send to nextion 
    }    
    
     error = err;

     disp.write("page 1");
    disp.write("e0.txt=\"");
    disp.write(error.c_str());
    disp.write("\"");
    disp.write(0xff); disp.write(0xff);disp.write(0xff);
    
  }
  
