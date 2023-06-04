#include <Arduino.h>
#include "DisplayStderr.h"


DisplayStderr::DisplayStderr() :fpstderr(0) {}


void DisplayStderr::println(const char *err) {
	print(err);
}

void DisplayStderr::print(const char *err) {

	if (fpstderr != 0) 	(*fpstderr)(err);

}
DisplayStderr STDERR;