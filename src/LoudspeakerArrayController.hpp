#ifndef _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
#define _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_

#include "AmbisonicSoundField.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <memory>
#include <vector>

class CLoudSpeakerArrayController {
public:
	CLoudSpeakerArrayController() {}
	// Constructor: initialize a default speaker layout (Hardcoded positions for now)
	CLoudSpeakerArrayController(BRTBase::CBRTManager & brtManager, size_t nspeakers) {
		// Create 6 omnidirectional loudspeakers (vertices of an octahedron)
		speakers.resize(nspeakers);
		for (int i = 0; i < nspeakers; i++) {
			auto speaker = CreateOmnidirectionalSoundSource(brtManager,"Speaker" + std::to_string(i));
			speakers[i] = speaker;
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
	void LoadLayoutJSON(/* std::string name*/) {
		// Set speaker positions in the shape of a regular octahedron centered at (0,0,0)
		if (speakers.size() == 6) {
			// Speaker 0 -> (1, 0, 0)
			Common::CTransform speaker0 = Common::CTransform();
			speaker0.SetPosition(Common::CVector3(1.0, 0.0, 0.0));
			speakers[0]->SetSourceTransform(speaker0);

			// Speaker 1 -> (-1, 0, 0)
			Common::CTransform speaker1 = Common::CTransform();
			speaker1.SetPosition(Common::CVector3(-1.0, 0.0, 0.0));
			speakers[1]->SetSourceTransform(speaker1);

			// Speaker 2 -> (0, 1, 0)
			Common::CTransform speaker2 = Common::CTransform();
			speaker2.SetPosition(Common::CVector3(0.0, 1.0, 0.0));
			speakers[2]->SetSourceTransform(speaker2);

			// Speaker 3 -> (0, -1, 0)
			Common::CTransform speaker3 = Common::CTransform();
			speaker3.SetPosition(Common::CVector3(0.0, -1.0, 0.0));
			speakers[3]->SetSourceTransform(speaker3);

			// Speaker 4 -> (0, 0, 1)
			Common::CTransform speaker4 = Common::CTransform();
			speaker4.SetPosition(Common::CVector3(0.0, 0.0, 1.0));
			speakers[4]->SetSourceTransform(speaker4);

			// Speaker 5 -> (0, 0, -1)
			Common::CTransform speaker5 = Common::CTransform();
			speaker5.SetPosition(Common::CVector3(0.0, 0.0, -1.0));
			speakers[5]->SetSourceTransform(speaker5);
		} else if (speakers.size() == 8) {
			float norm = 1.0f / std::sqrt(3.0f);

			// Speaker 0 -> (1, 1, 1)
			Common::CTransform speaker0;
			speaker0.SetPosition(Common::CVector3(1.0f * norm, 1.0f * norm, 1.0f * norm));
			speakers[0]->SetSourceTransform(speaker0);

			// Speaker 1 -> (1, 1, -1)
			Common::CTransform speaker1;
			speaker1.SetPosition(Common::CVector3(1.0f * norm, 1.0f * norm, -1.0f * norm));
			speakers[1]->SetSourceTransform(speaker1);

			// Speaker 2 -> (1, -1, 1)
			Common::CTransform speaker2;
			speaker2.SetPosition(Common::CVector3(1.0f * norm, -1.0f * norm, 1.0f * norm));
			speakers[2]->SetSourceTransform(speaker2);

			// Speaker 3 -> (1, -1, -1)
			Common::CTransform speaker3;
			speaker3.SetPosition(Common::CVector3(1.0f * norm, -1.0f * norm, -1.0f * norm));
			speakers[3]->SetSourceTransform(speaker3);

			// Speaker 4 -> (-1, 1, 1)
			Common::CTransform speaker4;
			speaker4.SetPosition(Common::CVector3(-1.0f * norm, 1.0f * norm, 1.0f * norm));
			speakers[4]->SetSourceTransform(speaker4);

			// Speaker 5 -> (-1, 1, -1)
			Common::CTransform speaker5;
			speaker5.SetPosition(Common::CVector3(-1.0f * norm, 1.0f * norm, -1.0f * norm));
			speakers[5]->SetSourceTransform(speaker5);

			// Speaker 6 -> (-1, -1, 1)
			Common::CTransform speaker6;
			speaker6.SetPosition(Common::CVector3(-1.0f * norm, -1.0f * norm, 1.0f * norm));
			speakers[6]->SetSourceTransform(speaker6);

			// Speaker 7 -> (-1, -1, -1)
			Common::CTransform speaker7;
			speaker7.SetPosition(Common::CVector3(-1.0f * norm, -1.0f * norm, -1.0f * norm));
			speakers[7]->SetSourceTransform(speaker7);

		}
	}

	// Returns the number of speakers
	int GetNumSpeakers() const { return speakers.size(); }

	// To implement
	void DecodeToAmbisonicSoundField(const CLoudSpeakerArrayController & array);

private:
	//BRTBase::CBRTManager brtManager; // BRT global manager interface

	using SourceModelPtr = std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>;
	std::vector<SourceModelPtr> speakers;

	std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel> CreateOmnidirectionalSoundSource(BRTBase::CBRTManager & brtManager,const std::string & _soundSourceID) {
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
