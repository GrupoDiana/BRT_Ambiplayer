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

	//Este podria ser el setup
	CLoudSpeakerArrayController(BRTBase::CBRTManager & brtManager, size_t nspeakers) {
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

			speakers[0]->SetSourceTransform(Common::CTransform(Common::CVector3(1.0f, 0.0f, 0.0f)));
			speakers[1]->SetSourceTransform(Common::CTransform(Common::CVector3(-1.0f, 0.0f, 0.0f)));
			speakers[2]->SetSourceTransform(Common::CTransform(Common::CVector3(0.0f, 1.0f, 0.0f)));
			speakers[3]->SetSourceTransform(Common::CTransform(Common::CVector3(0.0f, -1.0f, 0.0f)));
			speakers[4]->SetSourceTransform(Common::CTransform(Common::CVector3(0.0f, 0.0f, 1.0f)));
			speakers[5]->SetSourceTransform(Common::CTransform(Common::CVector3(0.0f, 0.0f, -1.0f)));

		} else if (speakers.size() == 8) {
			float norm = 1.0f / std::sqrt(3.0f);

			speakers[0]->SetSourceTransform(Common::CTransform(Common::CVector3(1.0f * norm, 1.0f * norm, 1.0f * norm)));
			speakers[1]->SetSourceTransform(Common::CTransform(Common::CVector3(-1.0f * norm, -1.0f * norm, 1.0f * norm)));
			speakers[2]->SetSourceTransform(Common::CTransform(Common::CVector3(-1.0f * norm, 1.0f * norm, -1.0f * norm)));
			speakers[3]->SetSourceTransform(Common::CTransform(Common::CVector3(1.0f * norm, -1.0f * norm, -1.0f * norm)));
			speakers[4]->SetSourceTransform(Common::CTransform(Common::CVector3(-1.0f * norm, 1.0f * norm, 1.0f * norm)));
			speakers[5]->SetSourceTransform(Common::CTransform(Common::CVector3(1.0f * norm, -1.0f * norm, 1.0f * norm)));
			speakers[6]->SetSourceTransform(Common::CTransform(Common::CVector3(1.0f * norm, 1.0f * norm, -1.0f * norm)));
			speakers[7]->SetSourceTransform(Common::CTransform(Common::CVector3(-1.0f * norm, -1.0f * norm, -1.0f * norm)));
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
