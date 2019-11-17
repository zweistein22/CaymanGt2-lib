#pragma once
#include <Arduino.h>
#define VACUUM_PIN 4     // switc on vacuum pump // similar to WATER_INJECT, but limit to 20 seconds in a minute.

class VacuumPump {
	bool bstarted = false;
	long started = 0;
	long maxduty = 20000; //  20 seconds
	long dutyciclePercent = 50;
public:
	void Init() {
		pinMode(VACUUM_PIN, OUTPUT);
		digitalWrite(VACUUM_PIN, LOW);
	}
	void Start(long millis) {
		if (!bstarted) {
			started = millis;
			digitalWrite(VACUUM_PIN, HIGH);
			bstarted = true;
		}
	}
	void Stop() {
		digitalWrite(VACUUM_PIN, LOW);
		bstarted = false;
	}
	void Update(long millis) {
		if (!bstarted) return;
		long diff = millis - started;

		if (diff > maxduty * 100 / dutyciclePercent) {
			started = millis;
			digitalWrite(VACUUM_PIN, HIGH);
		}
		else if (diff > maxduty) {
			digitalWrite(VACUUM_PIN, LOW);
		}
	}

};
