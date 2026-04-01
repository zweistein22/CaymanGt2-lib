#ifndef __NEXTIONFIRMWARE_H__
#define __NEXTIONFIRMWARE_H__
#include <Arduino.h>
#include <NeoICSerial.h>

class NextionFirmware
{
public: 
     NextionFirmware(NeoICSerial * nextion);
     bool upload_init(uint32_t fwsize,uint32_t download_baudrate);
     long upload_chunk(Stream* stream,long bytes);
     bool wait_acknowledge(Stream* stream);
      void sendCommand(const char* cmd);
      void send0Command();
      NeoICSerial* _nextion; 
      bool _searchBaudrate(uint32_t baudrate);
      void DebugOut(const char *msg);
          static bool bdebug;

          void SerialDbgRecv();
private: 
    uint32_t _getBaudrate(void);
   
    bool _setDownloadBaudrate(uint32_t baudrate);
   
    uint16_t recvRetString( uint32_t timeout = 100L,bool ack_flag = false);
  
private: /* data */ 

    static uint32_t _baudrate; /*nextion serail baudrate*/
    static uint32_t _fwsize; /*undownload byte of tft file*/
    static uint32_t _download_baudrate; /*download baudrate*/
    static char recv[180];
    static int recv_len;
};
/**
 * @}
 */

#endif /* #ifndef __NEXTIONFIRMWARE_H__ */
