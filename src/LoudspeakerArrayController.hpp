#ifndef _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
#define _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_

#include "AmbisonicSoundField.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"

#include <cmath>
#include <iostream>
#include <vector>

const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;

enum PlatonicSolid {
	octahedron,
	cube,
	icosahedron,
	dodecahedron
};

class CLoudSpeakerArrayController {
public:
	CLoudSpeakerArrayController() { }

	void SetUp(BRTBase::CBRTManager & brtManager, PlatonicSolid solid) {
		std::vector<Common::CVector3> vertices;

		switch (solid) {
		case PlatonicSolid::octahedron:
			vertices = {
				{ 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 }
			};
			break;

		case PlatonicSolid::cube: {
			float norm = 1.0f / std::sqrt(3.0f);
			vertices = {
				{ 1 * norm, 1 * norm, 1 * norm }, { -1 * norm, -1 * norm, 1 * norm }, { -1 * norm, 1 * norm, -1 * norm }, { 1 * norm, -1 * norm, -1 * norm },
				{ -1 * norm, 1 * norm, 1 * norm }, { 1 * norm, -1 * norm, 1 * norm }, { 1 * norm, 1 * norm, -1 * norm }, { -1 * norm, -1 * norm, -1 * norm }
			};
			break;
		}

		case PlatonicSolid::icosahedron: {
			float norm = 1.0f / std::sqrt(1 + phi * phi);
			vertices = {
				{ 0, 1 * norm, phi * norm }, { 0, 1 * norm, -phi * norm }, { 0, -1 * norm, phi * norm }, { 0, -1 * norm, -phi * norm },
				{ 1 * norm, phi * norm, 0 }, { 1 * norm, -phi * norm, 0 }, { -1 * norm, phi * norm, 0 }, { -1 * norm, -phi * norm, 0 },
				{ phi * norm, 0, 1 * norm }, { phi * norm, 0, -1 * norm }, { -phi * norm, 0, 1 * norm }, { -phi * norm, 0, -1 * norm }
			};
			break;
		}

		case PlatonicSolid::dodecahedron: {
			float norm = 1.0f / std::sqrt(3.0f);
			float normPhi = 1.0f / std::sqrt(1 + phi * phi);
			vertices = {
				{ 1 * norm, 1 * norm, 1 * norm }, { -1 * norm, 1 * norm, 1 * norm },
				{ 1 * norm, -1 * norm, 1 * norm }, { -1 * norm, -1 * norm, 1 * norm },
				{ 1 * norm, 1 * norm, -1 * norm }, { -1 * norm, 1 * norm, -1 * norm },
				{ 1 * norm, -1 * norm, -1 * norm }, { -1 * norm, -1 * norm, -1 * norm },

				{ 0, 1 * normPhi / phi, phi * normPhi }, { 0, -1 * normPhi / phi, phi * normPhi },
				{ 0, 1 * normPhi / phi, -phi * normPhi }, { 0, -1 * normPhi / phi, -phi * normPhi },

				{ 1 * normPhi / phi, phi * normPhi, 0 }, { -1 * normPhi / phi, phi * normPhi, 0 },
				{ 1 * normPhi / phi, -phi * normPhi, 0 }, { -1 * normPhi / phi, -phi * normPhi, 0 },

				{ phi * normPhi, 0, 1 * normPhi / phi }, { -phi * normPhi, 0, 1 * normPhi / phi },
				{ phi * normPhi, 0, -1 * normPhi / phi }, { -phi * normPhi, 0, -1 * normPhi / phi }
			};
			break;
		}
		}

		speakers.resize(vertices.size());
		for (size_t i = 0; i < vertices.size(); ++i) {
			speakers[i] = CreateOmnidirectionalSoundSource(brtManager, "Speaker" + std::to_string(i));
			speakers[i]->SetSourceTransform(Common::CTransform(vertices[i]));
		}

		setupDone = true;
	}

	std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel> GetSpeaker(size_t index) const {
		if (!setupDone) return nullptr;
		if (index >= speakers.size()) throw std::out_of_range("Speaker index out of range");
		return speakers[index];
	}

	const std::vector<std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>> & GetSpeakers() const {
		if (!setupDone) return std::vector<std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>>();
		return speakers;
	}

	int GetNumSpeakers() const {
		if (!setupDone) return -1;
		return speakers.size();
	}

	void RotateLoudspeakerArray(Common::CQuaternion & q) {
		if (!setupDone) return;
		CMatrix<double> R(3,3,0.0);
		R = q.R_1();
		for (auto & speaker : speakers) {
			Common::CVector3 pos = speaker->GetSourceTransform().GetPosition();
			Common::CVector3 rotatedPos = R * pos;
			speaker->SetSourceTransform(Common::CTransform(rotatedPos));
		}
	}

private:
	bool setupDone;

	using SourceModelPtr = std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel>;
	std::vector<SourceModelPtr> speakers;

	std::shared_ptr<BRTSourceModel::CSourceOmnidirectionalModel> CreateOmnidirectionalSoundSource(
		BRTBase::CBRTManager & brtManager, const std::string & _soundSourceID) {
		brtManager.BeginSetup();
		auto _brtSoundSource = brtManager.CreateSoundSource<BRTSourceModel::CSourceOmnidirectionalModel>(_soundSourceID);
		brtManager.EndSetup();
		if (!_brtSoundSource) std::cout << "Error creating sound source" << std::endl;
		return _brtSoundSource;
	}
};


#endif // _CLOUDSPEAKER_ARRAY_CONTROLLER_HPP_
