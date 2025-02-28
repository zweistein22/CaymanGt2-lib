#include <NeoICSerial.h>

class NextionDisplay: public NeoICSerial {
  public:
  NextionDisplay(int rx, int tx);
  void setup();
  void sendCommand(const char *cmd);
  
  ~NextionDisplay();
  void EGT(int left, int right, int statusleft, int statusright);
	void Boost(float hpa);
	void Lambda(int lambda, int llamb);
	void IntakeTemp(int air, bool injectwater);
	void Pumps(bool front, bool rear);
	void Error(const char *err);
 
  private:
  
  void _setNumber(const char *objname,int number);
 
  
};

extern NextionDisplay disp;
