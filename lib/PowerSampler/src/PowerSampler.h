/*
  PowerSampler.h - Library for reading several Current transformers by analog read.
  Created by Ben J.M. de Vette, November 20, 2018.
  Released into the public domain.
*/
#ifndef PowerSampler_h
#define PowerSampler_h

#include "esp32-hal.h"
#include "freertos/FreeRTOS.h"
#include "Preferences.h"

class PowerSampler
{
  public:
	  //PowerSampler(int analogPin, int numberOfSamples, int assumedVoltage);
	  PowerSampler(int *analogPins, int numberOfPorts, int **bufferSamples, int numberOfSamples, int assumedVoltage);
	  void onBufferTimer();
	  void onSampleTimer();
	  void Start();
	  bool GetActiveBuffer(float* wattage, int* lastBufferIndex, int* currentBufferId);

  private:
	void bufferTimeFlag();
	void LoadSettings();
	void SaveSettings();

	const float Pi = 3.14159265358979323846;
	const int bufferSize = 2;

	// Configuration
	Preferences preferences;

	// Analog
	int _analogPin = 32;
	int _analogPinIndex = 0;
	int _sampleSubtraction = 1860;
	long _totalSamplingOneCycle = 0;
	int _actualSamples = 0;

	int _numberOfPins;
	int _numberOfSamples;
	float _radianPerSample;
	int _assumedVoltage;
	float* _sinus = 0;

	int **_bufferSamples;
	int *_analogPins;
	int _multiplier[8];
	int _offset[8];

	int _miniBufferSize = 20;
	int* _miniBuffer = 0;

	int _activeBufferId = 0;
	int _lastBufferIndex = 0;
	bool _bufferChanged;
	int _activeSampleIndex = 0;
	int _previousSample = 0;
	int _miniBufferIndex = 10;

	// Timer for sampling
	hw_timer_t * _sampleTimer = NULL;

	// Timer for buffer
	hw_timer_t * _bufferTimer = NULL;

	//portMUX_TYPE bufferTimeFlag = portMUX_INITIALIZER_UNLOCKED;
};

#endif
