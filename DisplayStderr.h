#ifndef __DISPLAYSTDERR_H
#define __DISPLAYSTDERR_H

typedef void(*Tstderr)(const char *);
class DisplayStderr {
public:
	void begin(int param);
	void print(const char *err);
	void println(const char *err);
	DisplayStderr();
	Tstderr fpstderr;
}; 
extern DisplayStderr STDERR;

#endif

