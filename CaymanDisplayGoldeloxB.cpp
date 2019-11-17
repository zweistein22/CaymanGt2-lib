#include "Arduino.h"
#include "CaymanDisplayGoldeloxB.h"
#include <PString.h>
 
#include "Goldelox_Serial_4DLib.h"
#include "Goldelox_const4D.h"
#include <HoldMaximum.h>

#define GOLDELOX_RESET_PIN 10
#define MAXERRSTR 60
char _lasterror[MAXERRSTR];
//#define DEBUGSERIAL Serial
const int fontw=2;
const int fonth=3;


int MStrlen(const char *p){
	int l=0;
	for(int i=0;i<MAXERRSTR-1;i++) {
		if(*p==0) return l;
		p++;
		l++;
	}
	return l;
}



char _al_buffer[LINE_WIDTH];
PString _al_line=PString(_al_buffer, sizeof(_al_buffer));


char _al_bufftemp[MAXERRSTR];
PString _al_temp=PString(_al_bufftemp, sizeof(_al_bufftemp));

int showerrorseconds=10;

HoldMaximum<int> h_egt(5000);
HoldMaximum<float> h_boost(1000);

int TimeOutGoldelox = 1500; // 500 milliseconds
unsigned long lastfastprint = millis();


void CaymanDisplay::FastPrint(char *txt, int cy, int cx, word txtcolor) {
	//if (cy > 1) return;
	//if (cx > 0) return;
	bool shortenough = false;
	for (int k = 0; k <= LINE_WIDTH - cx; k++) {
		if (txt[k] == 0) shortenough = true;
	}
	
#ifdef DEBUGSERIAL
	if (!shortenough) DEBUGSERIAL.println("FastPrint txt too long!");
	DEBUGSERIAL.print("FastPrint(");
	DEBUGSERIAL.print('\"');
	DEBUGSERIAL.print(txt);
	DEBUGSERIAL.print('\"');
	DEBUGSERIAL.print(",");
	DEBUGSERIAL.print(cy);
	DEBUGSERIAL.print(",");
	DEBUGSERIAL.print(cx);
	DEBUGSERIAL.print(",");
	DEBUGSERIAL.print("0x");
	DEBUGSERIAL.print(txtcolor, HEX);
	DEBUGSERIAL.println(")");
#endif

	if (!shortenough) return;
	
	for (int j = 0; j < 2; j++) {
		char *p = txt;
		int len = MStrlen(txt);
		char _s = 0;
		bool txtwritten = false;
		for (int i = cx; i < cx+len; i++) {
			if (_s == 0) {
				if (*p == 0) _s = ' ';
			}
			char oldch = c[cy][i];
			char newch = (_s == 0 ? *p : _s);
			if ((oldch == newch)  && (ccol[cy][i] == txtcolor)) {
				
#ifdef DEBUGSERIAL_2
				DEBUGSERIAL.print("do nothing, old==new: ");
				DEBUGSERIAL.print('\'');
				DEBUGSERIAL.print(c[cy][i]);
				DEBUGSERIAL.print('\'');
				DEBUGSERIAL.print(",");
				DEBUGSERIAL.print('\'');
				DEBUGSERIAL.print(newch);
				DEBUGSERIAL.print('\'');
				DEBUGSERIAL.print(",");
				DEBUGSERIAL.print(cy);
				DEBUGSERIAL.print(",");
				DEBUGSERIAL.print(i);
				DEBUGSERIAL.println(")");

#endif
			}
			else {
				// not same, write char in bgcolor so it disappears
				if (j == 0) {
					
#ifdef DEBUGSERIAL
					DEBUGSERIAL.print(
						((oldch == newch)&& ccol[cy][i]== txtcolor) 
						?	"skip ":"BLACK out old !=new: ");
					DEBUGSERIAL.print('\'');
					DEBUGSERIAL.print(c[cy][i]);
					DEBUGSERIAL.print('\'');
					DEBUGSERIAL.print(",");
					DEBUGSERIAL.print('\'');
					DEBUGSERIAL.print(newch);
					DEBUGSERIAL.print('\'');
					DEBUGSERIAL.print(",");
					DEBUGSERIAL.print(cy);
					DEBUGSERIAL.print(",");
					DEBUGSERIAL.print(i);
					DEBUGSERIAL.println(")");
#endif
					if (oldch == newch && ccol[cy][i] == txtcolor) {
						continue;
					}
					SetTxtFGColor(BLACK);
					if(cy!=cpy || i!=cpx )	Display.txt_MoveCursor(cy, i);
					Display.putCH(oldch);
					lastfastprint = millis();
					cpy = cy;
					cpx = i + 1;
					
					c[cy][i] = ' ';
					ccol[cy][i] = BLACK;
				}
				if (j == 1) {
					SetTxtFGColor(txtcolor);
					if (!txtwritten && i >= cx && (oldch!=newch)) {
						txtwritten = true;
#ifdef DEBUGSERIAL
						DEBUGSERIAL.print("Display.putStr(");
						DEBUGSERIAL.print('\"');
						DEBUGSERIAL.print(p);
						DEBUGSERIAL.print('\"');
						DEBUGSERIAL.print(",");
						DEBUGSERIAL.print(cy);
						DEBUGSERIAL.print(",");
						DEBUGSERIAL.print(i);
						DEBUGSERIAL.println(")");
#endif
						if (cy != cpy || i != cpx)	Display.txt_MoveCursor(cy, i);
						Display.putstr(p);
						lastfastprint = millis();
						cpy = cy;
						cpx = i + MStrlen(p);
						
						
					}
					c[cy][i] = (_s == 0 ? *p : _s);
					ccol[cy][i] = txtcolor;
				}
			}
			if (_s == 0) p++; //for safety only
		}


	}
}


CaymanDisplay::CaymanDisplay(int rx, int tx,Tcallback4D cb):NeoICSerial(),Display(this){

//CaymanDisplay::CaymanDisplay(int rx, int tx,Tcallback4D cb):NeoSWSerial(rx,tx),Display(this),bmaskonly(false){
lasterrortime=millis();
_lasterror[0]=0;
Display.Callback4D =  cb ; // NULL ;
Display.TimeLimit4D = TimeOutGoldelox;


}

CaymanDisplay::~CaymanDisplay(){

	NeoICSerial::end();
	
}



void CaymanDisplay::UpdateError(){
	unsigned long mi = millis();

	if (mi - lastfastprint > 2*TimeOutGoldelox/3) {
		word pmm = Display.sys_GetVersion();
		Serial.print("GetVersion=");
		Serial.println(pmm);
		lastfastprint = mi;
	}
	int len = MStrlen(_lasterror);
	if (len == 0) return;

	int istart=(mi-lasterrortime)/500;

	int maxline=LINE_WIDTH;

	int is=0;
	
	if(len>maxline) {
		is=istart%(len-maxline+1);
	}

	
	if ((millis() - lasterrortime) > showerrorseconds * 1000) {
			_lasterror[0] = 0;
	}
	else {
		_al_line.begin();
		for(int f=0;f<maxline;f++){
			char c=_lasterror[is+f];
			_al_line.write(c);
			if(c==0) break;
		}
		FastPrint(_al_line,0,0,RED) ;
	}
	// start can vary from 0 until Strlen(_lasterrror)-11;
}

void CaymanDisplay::Error(const char *err){
	const char *p = err;
	int i=0;
	p=err;
	for(i=0;i<MAXERRSTR-1;i++){
		_lasterror[i]=*p;
		if(*p==0) break;
		p++;
	}
	_lasterror[i+1]=0;
	lasterrortime=millis();
	
}




void CaymanDisplay::init(){
	_lasterror[0] = 0;
#ifdef DEBUGSERIAL
    DEBUGSERIAL.println("CaymanDisplay::init()");
#endif
	pinMode(GOLDELOX_RESET_PIN, OUTPUT);
	digitalWrite(GOLDELOX_RESET_PIN, LOW);
	delay(50);
	digitalWrite(GOLDELOX_RESET_PIN, HIGH);
	delay(3000); // recommended by manual
	NeoICSerial::begin(9600);
	//word Newrate = 120; //25000 Baud
	word Newrate=BAUD_31250;
	//word Newrate=BAUD_19200;
	//word Newrate = BAUD_9600;
	print((char)(F_setbaudWait >> 8));
	print((char)(F_setbaudWait));
	print((char)(Newrate >> 8));
	print((char)(Newrate));

	int br ;
	flush() ;
	NeoICSerial::end() ;
	
  switch(Newrate)
  {
    case BAUD_110    : br = 110 ;
      break ;
    case BAUD_300    : br = 300 ;
      break ;
    case BAUD_600    : br = 600 ;
      break ;
    case BAUD_1200   : br = 1200 ;
      break ;
    case BAUD_2400   : br = 2400 ;
      break ;
    case BAUD_4800   : br = 4800 ;
      break ;
    case BAUD_9600   : br = 9600 ;
      break ;
    case BAUD_14400  : br = 14400 ;
      break ;
    case BAUD_19200  : br = 19200 ;
      break ;
    case BAUD_31250  : br = 31250 ;
      break ;
    case BAUD_38400  : br = 38400 ;
      break ;
    case BAUD_56000  : br = 56000 ;
      break ;
    case BAUD_57600  : br = 57600 ;
      break ;
    case BAUD_115200 : br = 115200 ;
      break ;
    case BAUD_128000 : br = 133928 ; // actual rate is not 128000 ;
		  break ;
	case 14:
		br = 200000;
		break;
	case 120:
		br = 25000;
		break;
	case BAUD_256000 : br = 281250 ; // actual rate is not  256000 ;
      break ;
    case BAUD_300000 : br = 312500 ; // actual rate is not  300000 ;
      break ;
    case BAUD_375000 : br = 401785 ; // actual rate is not  375000 ;
      break ;
    case BAUD_500000 : br = 562500 ; // actual rate is not  500000 ;
      break ;
    case BAUD_600000 : br = 703125 ; // actual rate is not  600000 ;
      break ;
  }
  NeoICSerial::begin(br) ;
  delay(50) ; // Display sleeps for 100
  flush() ;
  Display.TimeLimit4D = 2000;
   Display.GetAck() ;

    fontwidth=1;
    fontheight=1;
	tfgcolor=LIMEGREEN; // this color is never choosen
#ifdef DEBUGSERIAL
	DEBUGSERIAL.print("OK baud=");
		DEBUGSERIAL.println(br);
#endif
		int ol = Display.TimeLimit4D;
		Display.TimeLimit4D = 500;
		Display.gfx_Cls();
		Display.TimeLimit4D = ol;


		SetFontSize(fontw, fonth);
		for (int i = 0; i < LINES_MAXY; i++) {
			for (int j = 0; j < LINE_WIDTH; j++) {
				c[i][j] = ' ';
				ccol[i][j] = BLACK;
			}
		}

		cpx = 0;
		cpy = 0;

	
	
	
}

	void CaymanDisplay::SetFontSize(int w,int h){
		if(h!=fontheight){
			Display.txt_Height(h) ;
			fontheight=h;
		}
		if(w!=fontwidth){
		Display.txt_Width(w) ;
		fontwidth=w;
		}

	}
	bool CaymanDisplay::SetTxtFGColor(word fgc){
		if(fgc!=tfgcolor){
		Display.txt_Set(TEXT_COLOUR, fgc) ;
		tfgcolor=fgc;
		return true;
		}
		return false;
	}




long lasterror = millis();

void CaymanDisplay::EGT(int left,int right,int statusleft,int statusright){
		
	if(_lasterror[0] != 0) return;
	FastPrint("EGT ", 0, 0, LIMEGREEN);
		
	int WarnTemp=950;

	if(statusleft!=0) left=-40;
	if(statusright!=0) right=-40;

#ifdef DEBUGSERIAL
	DEBUGSERIAL.print("EGT(");
	DEBUGSERIAL.print(left);
	DEBUGSERIAL.print(",");
	DEBUGSERIAL.print(right);
	DEBUGSERIAL.print(",");
	DEBUGSERIAL.print(statusleft);
	DEBUGSERIAL.print(",");
	DEBUGSERIAL.print(statusright);
	DEBUGSERIAL.println(")");
#endif

	if (statusleft != 0 || statusright != 0) {
		_al_temp.begin();
		_al_temp.print("EGT:");
		if (statusleft == 1) _al_temp.print("L OC");
		if (statusleft == 2) _al_temp.print("L SCG");
		if (statusleft == 4) _al_temp.print("L SCV");
		if (statusleft != 0) _al_temp.print(", ");
		if (statusright == 1) _al_temp.print("R OC");
		if (statusright == 2) _al_temp.print("R SCG");
		if (statusright == 4) _al_temp.print("R SCV");
		//Error(_al_temp);
		return;

	}
	int higher=left;
	int lower = right;
	int shown;
	_al_line.begin();
		if(higher>lower) {
			shown = h_egt.Current(higher);
			_al_line.print(shown);
			if((higher-lower)>60 && left != -40) {
				 _al_line.print(" L ");
			}
			else if((higher-lower)>30 && left != -40) {
			_al_line.print(" l ");
		}
		else {
			 _al_line.print("  ");
		}
	}

	else {
		shown = h_egt.Current(lower);
		_al_line.print(shown);
		if((lower-higher)>60 && right!=-40) {
		 _al_line.print(" R ");
		}
		else if((lower-higher)>30 && right!=-40) {
		 _al_line.print(" r ");
		}
		else {
			 _al_line.print("  ");
		}


	}
	while(_al_line.length() < 7) _al_line.print(" ");
	//_al_line.print("#");
	FastPrint(_al_line,0,4, (shown > WarnTemp)?RED:LIGHTGREY);
   
	
}







void CaymanDisplay::Boost(float hpa){
	
	FastPrint("Bar ", 1,0, LIGHTGREY);
	_al_line.begin();
	_al_line.print(h_boost.Current(hpa),2);
	while (_al_line.length() < 7) _al_line.print(" ");
	FastPrint(_al_line,1,4, LIGHTGREY);
}

void CaymanDisplay::Lambda(int lambda,int llamb){
	//Serial.print(lambda);
	//Serial.print(",");
	//Serial.println(llamb);
	if (lambda < -9 || llamb < -9) return;
	FastPrint("Lamb", 2,0, LIGHTGREY);
	_al_line.begin();
	if (lambda < 0) {
		if (lambda == -2) {
			_al_line.print("NotConn");
		}
		else if (lambda == -3) {
			_al_line.print("HeatOff");
		}
		else if (lambda == -4) {
			_al_line.print("Heating");
		}
		else {
			_al_line.print(" E");
			_al_line.print(lambda);
			_al_line.print("   ");
		}
		
	}
	else {
		_al_line.print(" ");
		int diff = lambda - llamb;
		if (diff >= 0) {
			_al_line.print(lambda<100?"0.":"1.");
			if (lambda >= 100 && lambda < 110) _al_line.print("0");
			_al_line.print(lambda<100?lambda:lambda-100);
			
			if (diff > 2) {
				_al_line.print(" R");
			}
			else if (diff >= 1) {
				_al_line.print(" r");
			}
			

		}
		else {
			
			_al_line.print(llamb < 100 ? "0." : "1.");
			if (llamb >= 100 && llamb < 110) _al_line.print("0");
			_al_line.print(llamb < 100 ? llamb :llamb - 100 );

									
			if (diff < -2) {
				_al_line.print(" L");
			}
			else if (diff<= -1) {
				_al_line.print(" l");
			}
		}

		
	}
	
	FastPrint(_al_line,2, 4, LIGHTGREY);

}
void CaymanDisplay::IntakeTemp(int air,bool injectwater){
	_al_line.begin();
	if(injectwater)  _al_line.print("W");
	else _al_line.print("I");
	_al_line.print(air);
	_al_line.print(" ");
	int len = _al_line.length();
	FastPrint(_al_line, 3,0, injectwater ? BLUE : WHITE);

	
}

void CaymanDisplay::Pumps( bool front, bool rear) {
	_al_line.begin();
	if (front) {
		_al_line.print("ENG");
	}
	else _al_line.print("   ");
	if (rear) {
		_al_line.print(" GB");
	}
	else _al_line.print("   ");
	FastPrint(_al_line, 4, 4, WHITE);
}


