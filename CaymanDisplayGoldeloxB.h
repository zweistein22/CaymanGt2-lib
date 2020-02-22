#ifndef CaymanDisplay_h
#define CaymanDisplay_h
#include "Arduino.h"
//#include <SoftwareSerial.h>
//#include <NeoSWSerial.h>
#include <NeoICSerial.h>
#include <PString.h>
#include "Goldelox_const4D.h"
#include "Goldelox_Serial_4DLib.h"

//#define DEBUGSERIAL Serial

#define LINES_MAXY 5
#define LINE_WIDTH 11

class CaymanDisplay : public NeoICSerial {
public:
	CaymanDisplay(int rx, int tx, Tcallback4D cb);
	~CaymanDisplay();
	void EGT(int left, int right, int statusleft, int statusright);
	void Boost(float hpa);
	void Lambda(int lambda, int llamb);
	void IntakeTemp(int air, bool injectwater);
	void Pumps(bool front, bool rear);
	void init();
	void Error(const char *err);
	void UpdateError();
	Goldelox_Serial_4DLib Display;
private:
	bool SetTxtFGColor(word fgc);
	void MoveCursor(int cy, int cx);
	void FastPrint(char *txt, int cy, int cx, word txtcolor);
public:
	char c[LINES_MAXY][LINE_WIDTH+1];
	word ccol[LINES_MAXY][LINE_WIDTH+1];
	unsigned long lasterrortime;
	int fontwidth;
	int fontheight;
	int cpx;
	int cpy;
	word tfgcolor;
private:
	void SetFontSize(int w, int h);
};

#endif


