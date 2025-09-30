#ifndef _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
#define _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_

#include "AmbisonicSoundFieldController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRT.h"

#include <BRTLibrary.h>
#include <memory>
#include <vector>

class CLoudSpeakerArrayController {
public:
	// Constructor: initialize a default speaker layout
	CLoudSpeakerArrayController() {
		// Example: create a default speaker source
		// You can add more speakers as needed
		auto speaker1 = std::make_shared<BRTSourceModel::CSourceSimpleModel>();
		// TODO: configure speaker1 properties (position, azimuth, elevation, etc.)
		speakers.push_back(speaker1);
	}

	// Depending on the chosen layout (octahedron, cube, icosahedron, or dodecahedron), position the sources
	void LoadLayoutJSON(std::string name) {
		/*
        if (name == "icosahedron") {
            // Load the icosahedron .json
            speakers.clear();
            speakers.push_back({azimuth1, elevation1 });
            speakers.push_back({ azimuth2 ,  elevation2 });
            ...
        }
    */
	}

	// Returns the number of speakers
	int GetNumSpeakers() const { return speakers.size(); }

	// Should it be re-implemented or converted back to field, summed, and returned to speakers?
	void AddSoundField(/*...*/);

	// To implement
	void DecodeToAmbisonicSoundField(const CLoudSpeakerArrayController & array);

private:
	using SourceModelPtr = std::shared_ptr<BRTSourceModel::CSourceSimpleModel>;
	std::vector<SourceModelPtr> speakers;
	// In the constructor, something like speakers[0] = CreateSimpleSoundSource("speaker1") should be done
};

#endif // _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
