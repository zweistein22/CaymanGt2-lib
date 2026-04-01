#include "NextionFirmware.h"

int ARDUINOSERIALBUFFERSIZE = 64;

volatile int tx_completed = 0;

void TX_Complete() {

    tx_completed = 1;
}

  NextionFirmware::NextionFirmware(NeoICSerial *  nextion){

    _nextion=nextion;
   
  }

uint32_t NextionFirmware::_baudrate; /*nextion serail baudrate*/
uint32_t NextionFirmware::_fwsize; /*undownload byte of tft file*/
uint32_t NextionFirmware::_download_baudrate; /*download baudrate*/
char NextionFirmware::recv[180];
int NextionFirmware::recv_len = 0;
bool NextionFirmware::bdebug=true;
long nuploaded=0;

void NextionFirmware::SerialDbgRecv() {
    Serial.println(recv);
    for (int i = 0; i < recv_len; i++) {
          Serial.print(recv[i], HEX);
          if (i > 0) {
              if (i % 8 == 0) Serial.print(" ");
              if (i % 32 == 0) Serial.print("\r\n");
          }
    }
}



void NextionFirmware::DebugOut(const char *msg){
  if(!bdebug) return;
  String string = "DBG: " + String(msg);
  sendCommand(string.c_str());
  
}
bool NextionFirmware::upload_init(uint32_t fwsize,uint32_t download_baudrate){
  _fwsize=fwsize;
  nuploaded=0;
  _download_baudrate=download_baudrate;
  
    if(_getBaudrate() == 0)
    {
       return 0;
    }
    uint32_t newbaud = 9600;
    if (_baudrate != newbaud) {
        String cmd = "baud=";
        cmd += String(newbaud);
        sendCommand(cmd.c_str());
        recvRetString(500, true);
        //Serial.print("Setting baud to ");
        //Serial.print(newbaud);
        //Serial.print(" from ");
        //Serial.println(_baudrate);
        delay(500);

        if (_getBaudrate() == 0)
        {
            return 0;
        }
    }
    // we change to 4800 baud!
    
    if(!_setDownloadBaudrate(_download_baudrate))
    {
        return 0;
    }
    return 1;
}

uint32_t NextionFirmware::_getBaudrate(void)
{
    uint32_t baudrate_array[7] = { 4800,9600,115200,19200,57600,38400,2400};
     
    for(uint8_t i = 0; i < 7; i++)
    {
        if(_searchBaudrate(baudrate_array[i]))
        {
            _baudrate = baudrate_array[i];
            //Serial.print("\r\nBaudrate=");
            //Serial.println(_baudrate);
            break;
        }
    }
    return _baudrate;
}


bool NextionFirmware::_searchBaudrate(uint32_t baudrate)
{
    _nextion->end();
    _nextion->begin(baudrate);
    _nextion->setTimeout(500);
    sendCommand("DRAKJHSUYDGBNCJHGJKSHBDN");
    send0Command();
    recvRetString(500, true);
    sendCommand("connect");
    recvRetString(500, true);
  
    delay(110);
    sendCommand("��");
    sendCommand("connect");
    recvRetString(500, true);
  
    char* p = strstr(recv, "comok");
     if(p==nullptr){
           //Serial.println("comok expected.");
        return false;
    }
    return true;

}


void NextionFirmware::send0Command() {
    //while (_nextion->available()) { _nextion->read(); }
    tx_completed = 0;
    _nextion->attachTxCompleteInterrupt(TX_Complete);
    _nextion->write((uint8_t)0x00);
  
    _nextion->write(0xFF); _nextion->write(0xFF); _nextion->write(0xFF);
    //Serial.print("->0x00FFFFFF");
    _nextion->flush();
    while (tx_completed == 0) {
        delay(50);
    }
    _nextion->detachTxCompleteInterrupt();
   
}


void NextionFirmware::sendCommand(const char* cmd)
{
    //while (_nextion->available()) { _nextion->read(); }
    tx_completed = 0;
    _nextion->attachTxCompleteInterrupt(TX_Complete);
    //Serial.print("->");
    //Serial.print(cmd);
    _nextion->write(cmd);
    _nextion->write(0xFF);_nextion->write(0xFF); _nextion->write(0xFF);
    //Serial.print("0xFFFFFF");
    _nextion->flush();
    while (tx_completed == 0) {
        delay(50);
    }
    _nextion->detachTxCompleteInterrupt();
 
}

uint16_t NextionFirmware::recvRetString( uint32_t timeout,bool ack_flag)
{
    uint16_t ret = 0;
    uint8_t c = 0;
    long start;
    bool exit_flag = false;
    start = millis();
    recv_len = 0;
    while (millis() - start <= timeout)
    {
        while (_nextion->available())
        {
            c = _nextion->read(); 
            recv[recv_len++]= (char)c;
           
            if(ack_flag)
            {
                if(c == 0x05)
                { 
                    exit_flag = true;
                } 
            }
        }
        if(exit_flag)
        {
            break;
        }
    }
    recv[recv_len] = 0;
    /*Serial.print(" : ");
    for (int j = 0; j < recv_len; j++) {
        Serial.print((unsigned char)recv[j], HEX);
    }
    Serial.print("\r\n");
    Serial.println(recv);
    */
    return recv_len;
}

bool NextionFirmware::_setDownloadBaudrate(uint32_t baudrate)
{
    static String cmd = "";
    cmd.reserve(70);
    delay(100);
    sendCommand("runmode=2");
    delay(60);
    //recvRetString(500, true);
    sendCommand("print \"mystop_yesABC\"");
   
    recvRetString(500, true);
  
   
    sendCommand("get sleep���get dim���get baud���prints \"ABC\",0");
    delay(500);
    recvRetString(500, true);
    //SerialDbgRecv();
       
    static String filesize_str = String(_fwsize,10);
    static String baudrate_str = String(baudrate,10);


      
    cmd = "whmi-wri ";
    cmd+=filesize_str;
    cmd+= "," + baudrate_str + ",1";
    sendCommand("00");

    sendCommand(cmd.c_str());
    _nextion->flushOutput();
    _nextion->end();
    delay(100);
    _nextion->begin(baudrate);
    _nextion->setTimeout(500);
    
  
    recvRetString(500,true);  // acknowledge in new baud rate
    if(strchr(recv,0x05)) { 
        return 1;
    } 
    return 0;
}

bool NextionFirmware::wait_acknowledge(Stream* stream) {
   
    if (bdebug) return true;
    recvRetString( 1000, true);
    if (strchr(recv,0x05)) {
        //stream->write(0x05);
        return true;
    }
    return false; 
}

long NextionFirmware::upload_chunk(Stream* stream,long bytes)
{
    int t = 0;
   
    unsigned long tstart=millis();
    unsigned long tactive = 0;
    unsigned long timeout=30000L;
    long maxavail=0;
    long pos=0;
    long bytes_sent = 0;
    if (bytes == 0) return 0;

   
    static String string = "uploading : ";

    for(int i=0;i<bytes;i++){
       
        int avail = 0;
        do {
            avail = stream->available();
            if (avail == 0) delay(20);
        } while (avail == 0);
        if (avail >= ARDUINOSERIALBUFFERSIZE-1) {
            string = " Serial buffer overflow: avail=";
            string += String(avail);
            DebugOut(string.c_str());
        }
         if(avail>maxavail) {
          maxavail=avail;
          pos= bytes_sent;
         }
        
         while(avail>0){
           char c = stream->read();
          
           if (!bdebug) {
               _nextion->write(c);
              // _nextion->flush();
               
           }
           else {
               delay(1);
           }
           avail--;
           bytes_sent++;
          // string = " bytes_sent=";
          // string += String(bytes_sent);
          // DebugOut(string.c_str());
           if(bytes_sent==bytes)  goto trasmitted;
           
         }
         tactive = millis() - tstart;
         if (tactive > timeout) { goto timeout_flag; }
       
    }

 timeout_flag:
    string += "TIMEOUT :";
    string += String(tactive);
    string += "ms , max=";
    string += String(timeout);
    DebugOut(string.c_str());
 trasmitted:
   
   
   
    //string =" maxavail=";
    //string+=String(maxavail);
    //string+=", pos=";
    //string+=String(pos);
    //DebugOut(string.c_str());
    
    if (true) /*bytes_sent != bytes)*/ {
        string = "bytes_sent=";
        string += String(bytes_sent);
        string += " bytes=";
        string += String(bytes);
       
    }
    DebugOut(string.c_str());
   
    return bytes_sent;
 }
