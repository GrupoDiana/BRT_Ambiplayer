#ifndef _CAMBISONIC_PROCESSOR_HPP_
#define _CAMBISONIC_PROCESSOR_HPP_

#include "AmbisonicSoundField.hpp"
#include "LoudSpeakerArrayController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <Common/Buffer.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>



class CAmbisonicProcessor {
public:
	CAmbisonicProcessor(){}

	// Rotate the field depending on listener rotation
	void RotateField(float yaw, float pitch, float roll,CAmbisonicSoundField & OutBuffer) {
		OutBuffer.RotateField(yaw, pitch, roll);
	}

	void DecodeToSpeakerArray(CLoudSpeakerArrayController & speakerArray,
		const CAmbisonicSoundField & OutBuffer, std::string decoding) {
		speakerArray.DecodeToSpeakerArray(OutBuffer, decoding);
	}

	// Accessor for the internal ambisonic sound field
	CAmbisonicSoundField & GetSoundFieldProcessor() { return ambisonicProcessor; }

	const CAmbisonicSoundField & GetSoundFieldProcessor() const { return ambisonicProcessor; }

	// Setter para el ambisonicProcessor
	void SetSoundFieldProcessor(const CAmbisonicSoundField & sf) {
		ambisonicProcessor = sf;
	}

private:
	/* size_t ambisonicOrder;
	size_t sampleRate;
	int blockSize;
	int cont = 0;*/

	CAmbisonicSoundField ambisonicProcessor;

};

#endif // _CAMBISONIC_PROCESSOR_HPP_
