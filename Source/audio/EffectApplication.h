#pragma once
#include "../shape/OsciPoint.h"
#include <JuceHeader.h>

class EffectApplication {
public:
	EffectApplication() {};

	virtual OsciPoint apply(int index, OsciPoint input, const std::vector<std::atomic<double>>& values, double sampleRate) = 0;
	
	void resetPhase();
	double nextPhase(double frequency, double sampleRate);
private:
	double phase = 0.0;
};
