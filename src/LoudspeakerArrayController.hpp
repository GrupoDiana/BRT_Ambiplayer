#ifndef _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
#define _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_

#include "AmbisonicSoundField.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <memory>
#include <vector>

class CLoudSpeakerArrayController {
public:
	// Constructor: initialize a default speaker layout (Hardcoded positions for now)
	CLoudSpeakerArrayController() {
		// Create 6 omnidirectional loudspeakers (vertices of an octahedron)
		for (int i = 0; i < 6; i++) {
			auto speaker = CreateOmnidirectionalSoundSource("Speaker" + std::to_string(i));
			if (speaker) {
				speakers.push_back(speaker);
			}
		}

		// Set speaker positions in the shape of a regular octahedron centered at (0,0,0)
		if (speakers.size() >= 6) {
			// Speaker 0 -> (1, 0, 0)
			Common::CTransform speaker0;
			speaker0.SetPosition(Common::CVector3(1.0, 0.0, 0.0));
			speakers[0]->SetSourceTransform(speaker0);

			// Speaker 1 -> (-1, 0, 0)
			Common::CTransform speaker1;
			speaker1.SetPosition(Common::CVector3(-1.0, 0.0, 0.0));
			speakers[1]->SetSourceTransform(speaker1);

			// Speaker 2 -> (0, 1, 0)
			Common::CTransform speaker2;
			speaker2.SetPosition(Common::CVector3(0.0, 1.0, 0.0));
			speakers[2]->SetSourceTransform(speaker2);

			// Speaker 3 -> (0, -1, 0)
			Common::CTransform speaker3;
			speaker3.SetPosition(Common::CVector3(0.0, -1.0, 0.0));
			speakers[3]->SetSourceTransform(speaker3);

			// Speaker 4 -> (0, 0, 1)
			Common::CTransform speaker4;
			speaker4.SetPosition(Common::CVector3(0.0, 0.0, 1.0));
			speakers[4]->SetSourceTransform(speaker4);

			// Speaker 5 -> (0, 0, -1)
			Common::CTransform speaker5;
			speaker5.SetPosition(Common::CVector3(0.0, 0.0, -1.0));
			speakers[5]->SetSourceTransform(speaker5);
		}
	}

	// Get a single speaker by index
	std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel> GetSpeaker(size_t index) const {
		if (index >= speakers.size()) {
			throw std::out_of_range("Speaker index out of range");
		}
		return speakers[index];
	}

	// Get the entire vector of speakers
	const std::vector<std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>> & GetSpeakers() const {
		return speakers;
	}



	// Depending on the chosen layout (octahedron, cube, icosahedron, or dodecahedron) position the sources
	void LoadLayoutJSON(std::string name) {
		/*
        if (name == "icosahedron") {
            // Load the icosahedron .json
            speakers.clear();
			Common::CTransform source1 = Common::CTransform();
			sourc1.SetPosition(...);
            source1BRT->SetSourceTransform(source1
			EN BERTA ESTA EN OFAPP.CPP EN METODO AddSoundSource()
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
	BRTBase::CBRTManager brtManager; // BRT global manager interface

	using SourceModelPtr = std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>;
	std::vector<SourceModelPtr> speakers;

	std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel> CreateOmnidirectionalSoundSource(std::string _soundSourceID) {
		brtManager.BeginSetup();
		std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel> _brtSoundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceOmnidirectionalModel>(_soundSourceID);
		brtManager.EndSetup();
		if (_brtSoundSource == nullptr) {
			std::cout << "Error creating sound source" << std::endl;
		}
		return _brtSoundSource;
	}
	
};

#endif // _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
