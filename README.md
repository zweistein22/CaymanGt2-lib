# Goldelox997Frontdisplay-lib
Using a uOLED-160-G2 Display from 4dsystems.com.au  to display addional engine data from a Porsche 997
needs https://github.com/4dsystems/Goldelox-Serial-Arduino-Library
needs https://github.com/SlashDevin/NeoSWSerial
needs https://github.com/zweistein22/arduino-breitband-lambda
needs https://github.com/zweistein22/997can-lib

For track day engine data monitoring additional  sensors are attached to the engine and displayed on a secondary display.
Some data is taken from the 997 Can bus (Antrieb or can-drive).  The can bus is also used to send private data from the back of the car to the front.

To improve the goldelox uOLED displays performance also in text mode an internal text buffer matrix (FastPrint function) is used. Only changed data is written over the slow serial connection. The uOLED display is connected to an Arduino (UNO)  with a serial connection on pin 8 and 9.

