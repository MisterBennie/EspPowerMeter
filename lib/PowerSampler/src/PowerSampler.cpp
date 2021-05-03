#include <Arduino.h>
#include "PowerSampler.h"
#include <soc/sens_reg.h>

static PowerSampler* localPowerSampler;

PowerSampler::PowerSampler(int analogPin, int numberOfSamples, int assumedVoltage) {
	localPowerSampler = this;
	LoadSettings();

	_numberOfSamples = numberOfSamples;
	_analogPin = analogPin;
	_radianPerSample = 2.0 * Pi / (float)numberOfSamples;
	_assumedVoltage = assumedVoltage;

	_sinus = new float[numberOfSamples];
	_miniBuffer = new int[_miniBufferSize];

	for (int i = 0 ; i < 8 ; i++)
	{
		_bufferSamples[i] = new int[numberOfSamples];
	}
}

static void localOnSampleTimer() {
	localPowerSampler->onSampleTimer();
}

static void localOnBufferTimer() {
	localPowerSampler->onBufferTimer();
}

void PowerSampler::onSampleTimer() {
	long sample = analogRead(_analogPin);

	if (_activeSampleIndex >= 0 && _activeSampleIndex < _numberOfSamples)
	{
		_bufferSamples[_activeBufferId][_activeSampleIndex] = sample;
		_activeSampleIndex++;
	}
}

void PowerSampler::onBufferTimer() {
	_activeBufferId = 1 - _activeBufferId;
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

bool PowerSampler::GetActiveBuffer(int* subtract2, float* wattage, int* actualSamples, int* buffer) {
	float total0 = 0;
	long totalRaw = 0;

	if (_bufferChanged) {
		for (int index = 0; index < _actualSamples; index++)
		{
			int value = 0;
			value = _bufferSamples[_activeBufferId][index];
			total0 += (float)(abs(value - _sampleSubtraction)) * 1;
			totalRaw += value;
		}

		_sampleSubtraction = totalRaw / _actualSamples;

		*subtract2 = _sampleSubtraction;
		*wattage = ((total0 / (float)_numberOfSamples) - 1.8) * 5.464480874 ; // *8.870753;
		*actualSamples = _actualSamples;
		*buffer = 0;

		_bufferChanged = false;
		_actualSamples = 0;

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