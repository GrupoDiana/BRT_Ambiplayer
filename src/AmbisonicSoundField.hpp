/**
 * \class CAmbisonicSoundField
 *
 * \brief Core class for representing and manipulating Ambisonic sound fields.
 * 
 * \details This class defines the internal data structure and operations
 * for Ambisonic sound fields. It provides methods for initialization,
 * buffer management, rotation using quaternions, channel manipulation,
 * and Ambisonic order downmixing.
 * 
 * The implementation assumes **FuMa channel ordering**, ensuring
 * compatibility with standard first- and higher-order Ambisonic workflows.
 * 
 * \date November 2025
 *
 * \author R. Liceras-Ramírez  
 * \b Tutor: A. Reyes-Lecuona (University of Málaga)  
 * \b Institution: Universidad de Málaga
 *
 * \b Contact: areyes@uma.es
 *
 * \b Copyright: © 2025 University of Málaga. All rights reserved.
 *
 * \b Note: This class was developed as part of the Bachelor's Thesis of 
 * R. Liceras-Ramírez, under the supervision of A. Reyes-Lecuona.  
 * It does not belong to any external funded project.
 * 
 * \b Remarks:
 * - The Ambisonic field is internally stored as a set of mono buffers,
 *   one per channel, following FuMa ordering.  
 * - Rotations are performed via quaternion-derived rotation matrices.  
 * - Channel indexing and R matrix computation follow the Ambisonic
 *   rotation formalism for orders up to 3.
 *
 * \b Licence: This program is free software; you can redistribute it 
 * and/or modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation, either version 3 of 
 * the License, or (at your option) any later version.
 */


#ifndef _CAMBISONIC_SOUND_FIELD_HPP_
#define _CAMBISONIC_SOUND_FIELD_HPP_

#include "LoudspeakerArrayController.hpp"
#include "ofxAudioFile.h"
#include "ofxBRTLibrary.h"
#include "Matrix.hpp"

#include <Common/Buffer.hpp>
#include <cmath>
#include <string>
#include <vector>

using namespace Common;

int mapR1toMath[3] = { 1, 2, 0 }; 


class CAmbisonicSoundField {
public:
	CAmbisonicSoundField(): sampleRate(-1), ambisonicOrder(-1), bufferSize(-1), setupDone {false} {}

	void SetUp(int ambisonicOrder_, size_t sampleRate_, size_t bufferSize_){
		ambisonicOrder = ambisonicOrder_;
		sampleRate = sampleRate_;
		bufferSize = bufferSize_;

		// Initialize buffers for each channel
		int calculatedChannels = pow(ambisonicOrder + 1, 2);
		soundFieldArray = std::vector<CMonoBuffer<float>>(calculatedChannels, CMonoBuffer<float>(bufferSize, 0.0f));
		
		setupDone = true;
		std::cout << "CAmbisonicSoundField created with "
				  << calculatedChannels << " channels, order " << ambisonicOrder
				  << ", sampleRate " << sampleRate
				  << ", buffer size " << bufferSize << std::endl;
	}


	// Returns the field order
	int GetOrder() const {
		if (!setupDone) return -1;
		return ambisonicOrder;
	}

	void SetOrder(int order) {
		if (!setupDone) return;
		ambisonicOrder=order;
	}

	int GetBufferSize() const {
		if (!setupDone) return -1;
		return bufferSize;
	}

	std::vector<CMonoBuffer<float>> & GetSoundFieldArray() {
		if (!setupDone) return std::vector<CMonoBuffer<float>>();
		return soundFieldArray;
	}

	std::vector<CMonoBuffer<float>> & CAmbisonicSoundField::GetSoundFieldArrayRotate() {
		if (!setupDone)
			return soundFieldArray; 

		size_t numChannels = soundFieldArray.size();
		std::vector<CMonoBuffer<float>> buffer(numChannels, CMonoBuffer<float>(bufferSize, 0.0f));
		buffer[0] = soundFieldArray[0];
		RotateFieldRecursive(1, soundFieldArray, buffer, 1);

		soundFieldArray = buffer;

		ambisonicRotationMatrices.clear();

		return soundFieldArray;
	}

	const std::vector<CMonoBuffer<float>> & GetSoundFieldArray() const {
		if (!setupDone) return std::vector<CMonoBuffer<float>>();
		return soundFieldArray;
	}

	void SetSoundField(const CAmbisonicSoundField & other) {
		if (!setupDone) return;
			sampleRate = other.sampleRate;
			soundFieldArray = other.soundFieldArray;
	}

	void FillChannel(int channel, CMonoBuffer<float> data) {
		if (!setupDone) return;
		soundFieldArray[channel].resize(data.size());
		soundFieldArray[channel] = data;
	}

	CMonoBuffer<float> GetChannel(int channel) {
		return soundFieldArray[channel];
	}

	/// Fill multi-channel soundFieldArray from multi-channel samples
	void GetSegment(unsigned int & position, unsigned int & endFrame, CAmbisonicSoundField & source, size_t bufferSize) {
		if (!setupDone) return;

		const auto & sourceBuffers = source.GetSoundFieldArray();
		size_t numChannels = sourceBuffers.size();
		size_t sourceSize = sourceBuffers[0].size();

		for (size_t i = 0; i < numChannels; i++) {
			if (soundFieldArray[i].size() != bufferSize)
				soundFieldArray[i].resize(bufferSize, 0.0f);
		}

		for (size_t ch = 0; ch < numChannels; ++ch) {
			const auto & src = sourceBuffers[ch];
			auto & dest = soundFieldArray[ch];
			for (size_t i = 0; i < bufferSize; ++i) {
				size_t idx = (position + i) % sourceSize;
				dest[i] = src[idx];
			}
		}

		std::vector<CMonoBuffer<float>> buffer(numChannels, CMonoBuffer<float>(bufferSize, 0.0f));
		buffer[0] = soundFieldArray[0]; 
		RotateFieldRecursive(1, soundFieldArray, buffer, 1);

		soundFieldArray = buffer;


		endFrame = (position + bufferSize - 1) % sourceSize;
		position = (position + bufferSize) % sourceSize;
	}



	// Rotate the field depending on how coordinates have changed
	//Cambiar a CalculateRotationMatrices()
	void CalculateRotationMatrices(Common::CQuaternion q) {
		if (!setupDone) return;

		ambisonicRotationMatrices.clear();

		CMatrix<float> R1 = MapR1(q.RotationMatrix());

		CalculateRMatrix(ambisonicOrder, R1);

	}

	//Falta metodo que sea returnSoundFieldArrayRotated o algo asi, que te devuelva el campo rotado y las matrices vacias
	//que lo usara la mezcla para rotarse


	void DownMixing(int order) {
		if (!setupDone) return;

		soundFieldArray.resize(pow(order + 1, 2));
		ambisonicOrder = order;
	}

	CAmbisonicSoundField & CAmbisonicSoundField::operator+=(const CAmbisonicSoundField & other) {
		if (!setupDone || !other.setupDone) return *this;

		size_t numChannels = soundFieldArray.size();
		if (other.soundFieldArray.size() != numChannels) {
			std::cerr << "Channel mismatch in operator+=" << std::endl;
			return *this;
		}

		for (size_t ch = 0; ch < numChannels; ++ch) {
			auto & dest = soundFieldArray[ch];
			const auto & src = other.soundFieldArray[ch];

			if (dest.size() != src.size()) dest.resize(src.size(), 0.0f);

			for (size_t i = 0; i < src.size(); ++i) {
				dest[i] += src[i]; // suma canal a canal
			}
		}

		return *this;
	}



	CAmbisonicSoundField operator+(const CAmbisonicSoundField & other) const {
		CAmbisonicSoundField result = *this;
		result += other;
		return result;
	}

	CAmbisonicSoundField operator-(const CAmbisonicSoundField & other) const {
		CAmbisonicSoundField result = *this;
		result = result - other;
		return result;
	}

private:
	bool setupDone;
	int ambisonicOrder;
	int sampleRate;
	int bufferSize;

	std::vector<CMonoBuffer<float>> soundFieldArray;

	std::vector<CMatrix<float>> ambisonicRotationMatrices;
	//metodo que aplique la rotacion pero borre las matriz

	//Añadir comentarios
	void RotateFieldRecursive(int l, const std::vector<CMonoBuffer<float>> & source, std::vector<CMonoBuffer<float>> & dest, int offset) {
		if (l > ambisonicOrder) return;

		const CMatrix<float> & R_l = ambisonicRotationMatrices[l - 1]; 
		int dim = 2 * l + 1;

		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				for (int k = 0; k < dest[i + offset].size(); k++) {
					dest[i + offset][k] += R_l(i, j) * source[j + offset][k];
				}
			}
		}

		RotateFieldRecursive(l + 1, source, dest, offset + dim);
	}

	//Cambiar nombre a XYZtoYZX 
	CMatrix<float> MapR1(const CMatrix<float> & R1) {
		CMatrix<float> R1_mapped(3, 3);
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				R1_mapped(i, j) = R1(mapR1toMath[i], mapR1toMath[j]);
			}
		}
		return R1_mapped;
	}

	std::vector<int> range_l(int l) {
		std::vector<int> v;
		for (int i = -l; i <= l; ++i)
			v.push_back(i);
		return v;
	}

	 CMatrix<float> CalculateRMatrix(int l, const CMatrix<float> & R_1) {

		if (l == 1) {
			 if (ambisonicRotationMatrices.empty())
				ambisonicRotationMatrices.push_back(R_1);
			return R_1;
		}

		CMatrix<float> R_prev = CalculateRMatrix(l - 1, R_1);

		int dim = 2 * l + 1;
		CMatrix<float> R_l(dim, dim);
		std::vector<int> range = range_l(l);

		for (int i = 0; i < dim; ++i) {
			for (int j = 0; j < dim; ++j) {
				R_l(i, j) = Rmm(l, range[i], range[j], R_1);
			}
		}

		ambisonicRotationMatrices.push_back(R_l);
		return R_l;
	}

	inline int delta(float i, float j) {
		return (i == j) ? 1 : 0;
	}

	double Rmm(float l, float m, float mp, CMatrix<float> R_1) {
		if (l == 1) {
			return R_1(m +1 , mp +1 );
		}

		float u = umm(l, m, mp);
		float v = vmm(l, m, mp);
		float w = wmm(l, m, mp);

		float result = 0.0;
		constexpr float EPS = 1e-9; 

		if (std::abs(u) > EPS) {
			result += u * Umm(l, m, mp, R_1);
		}

		if (std::abs(v) > EPS) {
			result += v * Vmm(l, m, mp, R_1);
		}

		if (std::abs(w) > EPS) {
			result += w * Wmm(l, m, mp, R_1);
		}

		return result;
	}


	float umm(float l, float m, float mp) {
		float umm = 0.0f;

		if (abs(mp) < l) {
			umm = sqrt(((l + m) * (l - m)) / ((l + mp) * (l - mp)));
		} else if (abs(mp) == l) {
			umm = sqrt(((l + m) * (l - m)) / ((2 * l) * (2 * l - 1)));
		} else {
			umm = 0.0f;
		}

		return umm;
	}

	float Umm(float l, float m, float mp, CMatrix<float> R_1) {
		float Umm = 0.0f;

		if (m == 0) {
			return P(0, l, 0, mp, R_1);
		} else if (m > 0) {
			return P(0, l, m, mp, R_1);
		} else if (m < 0) {
			return P(0, l, m, mp, R_1);
		}

		return Umm;
	}

	float vmm(float l, float m, float mp) {
		float vmm = 0.0f;

		if (abs(mp) < l) {
			vmm = 0.5 * sqrt(((1 + delta(m, 0)) * (l + abs(m) - 1) * (l + abs(m))) / ((l + mp) * (l - mp))) * (1 - 2 * delta(m, 0));
		} else if (abs(mp) == l) {
			vmm = 0.5 * sqrt(((1 + delta(m, 0)) * (l + abs(m) - 1) * (l + abs(m))) / ((2 * l) * (2 * l - 1))) * (1 - 2 * delta(m, 0));
		} else {
			vmm = 0.0f;
		}

		return vmm;
	}

	float Vmm(float l, float m, float mp, CMatrix<float> R_1) {
		float Vmm = 0.0f;

		if (m == 0) {
			return P(1, l, 1, mp, R_1) + P(-1, l, -1, mp, R_1);
		} else if (m > 0) {
			return P(1, l, m - 1, mp, R_1) * sqrt(1 + delta(m, 1)) - P(-1, l, -m + 1, mp, R_1) * (1 - delta(m, 1));
		} else if (m < 0) {
			return P(1, l, m + 1, mp, R_1) * (1 - delta(m, -1)) + P(-1, l, -m - 1, mp, R_1) * sqrt(1 + delta(m, -1));
		}

		return Vmm;
	}

	float wmm(float l, float m, float mp) {
		float wmm = 0.0f;

		if (abs(mp) < l) {
			wmm = -0.5 * sqrt(((l - abs(m) - 1) * (l - abs(m))) / ((l + mp) * (l - mp))) * (1 - delta(m, 0));
		} else if(abs(mp) == l) {
			wmm = -0.5 * sqrt(((l - abs(m) - 1) * (l - abs(m))) / ((2 * l) * (2 * l - 1))) * (1 - delta(m, 0));
		} else {
			wmm = 0.0f;
		}

		return wmm;
	}

	float Wmm(float l, float m, float mp, CMatrix<float> R_1) {
		float Wmm = 0.0f;

		if (m == 0) {
			return 0.0;
		} else if (m > 0) {
			return P(1, l, m + 1, mp, R_1) + P(-1, l, -m - 1, mp, R_1);
		} else if (m < 0) {
			return P(1, l, m - 1, mp, R_1) - P(-1, l, -m + 1, mp, R_1);
		}

		return Wmm;
	}

	float P(int i, int l, int mu, int mp, const CMatrix<float> & R_1) {
		const CMatrix<float> & R_prev = ambisonicRotationMatrices[l - 2];

		auto idx = [&](int m) { return m + (l - 1); };

		if (abs(mp) < l) {
			return R_1(i + 1, 1) * R_prev(idx(mu), idx(mp));

		} else if (mp == l) {
			return R_1(i + 1, 2) * R_prev(idx(mu), idx(l - 1)) - R_1(i + 1, 0) * R_prev(idx(mu), idx(-(l - 1)));

		} else if (mp == -l) {
			return R_1(i + 1, 2) * R_prev(idx(mu), idx(-(l - 1))) + R_1(i + 1, 0) * R_prev(idx(mu), idx(l - 1));

		} else {
			return 0.0f;
		}
	}
};

#endif // _CAMBISONIC_SOUND_FIELD_HPP_


