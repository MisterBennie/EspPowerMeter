#include <Arduino.h>
#include "PowerSampler.h"
#include <soc/sens_reg.h>

static PowerSampler* localPowerSampler;

// PowerSampler::PowerSampler(int analogPin, int numberOfSamples, int assumedVoltage) {
// 	localPowerSampler = this;
// 	LoadSettings();

// 	_numberOfSamples = numberOfSamples;
// 	_analogPin = analogPin;
// 	_radianPerSample = 2.0 * Pi / (float)numberOfSamples;
// 	_assumedVoltage = assumedVoltage;

// 	_sinus = new float[numberOfSamples];
// 	//_miniBuffer = new int[_miniBufferSize];

// 	_bufferSamples = new int[8];
// 	for (int i = 0 ; i < 8 ; i++)
// 	{
// 		(&_bufferSamples)[i] = new int[numberOfSamples];
// 	}
// }

PowerSampler::PowerSampler(int *analogPins, int numberOfPins, int **bufferSamples, int numberOfSamples, int assumedVoltage) {
	localPowerSampler = this;
	LoadSettings();

	_numberOfSamples = numberOfSamples;
	_analogPins = analogPins;
	_numberOfPins = numberOfPins;
	_radianPerSample = 2.0 * Pi / (float)numberOfSamples;
	_assumedVoltage = assumedVoltage;

	_sinus = new float[numberOfSamples];
	_bufferSamples = bufferSamples;

	//_miniBuffer = new int[_miniBufferSize];
}

static void localOnSampleTimer() {
	localPowerSampler->onSampleTimer();
}

static void localOnBufferTimer() {
	localPowerSampler->onBufferTimer();
}

void PowerSampler::onSampleTimer() {
	long sample = analogRead(_analogPins[_analogPinIndex]);

	if (_activeSampleIndex >= 0 && _activeSampleIndex < _numberOfSamples)
	{
		(_bufferSamples)[_activeBufferId][_activeSampleIndex] = sample;
		_activeSampleIndex++;
	}
}

void PowerSampler::onBufferTimer() {
	_lastBufferIndex = _analogPinIndex;

	_analogPinIndex++;
	if (_analogPinIndex == _numberOfPins){
		_analogPinIndex = 0;
	}
	_analogPin = _analogPins[_analogPinIndex];

	_activeBufferId = _analogPinIndex;
	_actualSamples = _activeSampleIndex;
	_bufferChanged = true;
	_activeSampleIndex = 0;
}

void PowerSampler::Start() {
	// Fill the sinus list as alternative for actual measurement of the voltage
	// When 0 crossing can be detected (simple hardware) the cosPhi can be calculated,
	// together with the actual power.
	for (int i = 0; i < _numberOfSamples; i++) {
		_sinus[i] = sin(_radianPerSample * (float)i);
	}

	// Every 20 mSec, a number of samples is read into the buffer
	_sampleTimer = timerBegin(0, 80, true);
	timerAttachInterrupt(_sampleTimer, &localOnSampleTimer, true);
	timerAlarmWrite(_sampleTimer, 1000000 / 50 / _numberOfSamples, true);

	// Once every 20 mSec a new sample starts
	_bufferTimer = timerBegin(1, 80, true);
	timerAttachInterrupt(_bufferTimer, &localOnBufferTimer, true);
	timerAlarmWrite(_bufferTimer, 1000000 / 50, true);

	timerAlarmEnable(_bufferTimer);
	timerAlarmEnable(_sampleTimer);
}

bool PowerSampler::GetActiveBuffer(float* wattage, int* lastBufferIndex, int* currentBufferId) {
	if (_bufferChanged) {
		float total = 0;
		long totalRaw = 0;
		int howManyZeros = 0;

		// dit moet anders berekend worden
		// het gemiddelde van alle waardes dus.
		float _subtractor = 0;

		for (int index = 0; index < _actualSamples; index++)
		{
			int value = _bufferSamples[_lastBufferIndex][index];
			totalRaw += value;
			// maxValue = max(maxValue, value);
			// minValue = min(minValue, value);
		}
		_subtractor =  (float)(totalRaw) / (float)_numberOfSamples;

		for (int index = 0; index < _actualSamples; index++)
		{
			int value = _bufferSamples[_lastBufferIndex][index];

			if (value == 0){
				howManyZeros++;
			}

			total += (float)(abs(value - _subtractor)) * 1;
			totalRaw += value;
		}

		//- 1.66164
		*wattage = ((total / (float)_numberOfSamples) - 1.55733) * 5.464480874 ; // *8.870753;
		*lastBufferIndex = _lastBufferIndex;
		_bufferChanged = false;
		return true;
	}

	return false;
}

void PowerSampler::LoadSettings(){
	preferences.begin("my-app", true);

	for (int i= 0 ; i < 8 ; i++){
		_offset[i] = preferences.getFloat("offset" + i, 1.8);
		_multiplier[i] = preferences.getFloat("multiplier" + i, 5.464480874);
	}

	preferences.end();
}

void PowerSampler::SaveSettings(){
	preferences.begin("my-app", false);

	for (int i= 0 ; i < 8 ; i++){
		preferences.putFloat("offset" + i, _offset[i]);
		preferences.putFloat("multiplier" + i, _multiplier[i]);
	}

	preferences.end();
}