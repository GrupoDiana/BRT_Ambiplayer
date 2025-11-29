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

int m_order[7] = { 0, 1, -1, 2, -2, 3, -3 };

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

		endFrame = (position + bufferSize - 1) % sourceSize;
		position = (position + bufferSize) % sourceSize;
	}

	//Ira en clase Quaternion
	CMatrix<float> R_1(Common::CQuaternion q) {

		Common::CVector3 X_axis(1, 0, 0);
		Common::CVector3 Y_axis(0, 1, 0);
		Common::CVector3 Z_axis(0, 0, 1);

		Common::CVector3 R_col1 = q.RotateVector(X_axis);
		Common::CVector3 R_col2 = q.RotateVector(Y_axis);
		Common::CVector3 R_col3 = q.RotateVector(Z_axis);

		CMatrix<float> R_1(3, 3);

		R_1 = { { R_col1.x, R_col2.x, R_col3.x }, { R_col1.y, R_col2.y, R_col3.y }, { R_col1.z, R_col2.z, R_col3.z } };

		return R_1;
	}

	// Rotate the field depending on how coordinates have changed
	void RotateField(Common::CQuaternion q) { 

		if (!setupDone) return;

		int numChannels = pow(ambisonicOrder + 1, 2);
		vector<CMonoBuffer<float>> buffer(numChannels, CMonoBuffer<float>(bufferSize, 0.0f));
		buffer[0] = soundFieldArray[0];

		CMatrix<float> R1 = CalculateRMatrix(1, R_1(q));
		CMatrix<float> R1_mapped(3, 3);

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < bufferSize; k++) {
					buffer[i + 1][k] += R1(i,j) * soundFieldArray[j + 1][k];
					R1_mapped(i, j) = R1(mapR1toMath[i], mapR1toMath[j]);
				}
			}
		}

		if (ambisonicOrder > 1) {
			CMatrix<float> R2 = CalculateRMatrix(2, R1_mapped);

			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					for (int k = 0; k < bufferSize; k++) {
						buffer[i + 4][k] += R2(i,j) * soundFieldArray[j + 4][k];
					}
				}
			}

			if (ambisonicOrder > 2) {
				CMatrix<float> R3 = CalculateRMatrix(3, R1_mapped);

				for (int i = 0; i < 7; i++) {
					for (int j = 0; j < 7; j++) {
						for (int k = 0; k < bufferSize; k++) {
							buffer[i + 9][k] += R3(i, j) * soundFieldArray[j + 9][k];
						}
					}
				}
			}
		}

		soundFieldArray = buffer;
	}

	void DownMixing(int order) {
		if (!setupDone) return;

		soundFieldArray.resize(pow(order + 1, 2));
		ambisonicOrder = order;
	}

	//TODO
	CAmbisonicSoundField & operator+=(const CAmbisonicSoundField & other) {
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

	CMatrix<float> CalculateRMatrix(int l, CMatrix<float> R_1) {
		int dim = (2 * l + 1);
		CMatrix<float> R_l(dim, dim);

		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				if (l == 1) {
					R_l(i, j) = R_1(i, j);
				} else {
					R_l(i, j) = Rmm(l, m_order[i], m_order[j], R_1);
				}
				
			}
		}

		return R_l;
	}

	inline int delta(int i, int j) {
		return (i == j) ? 1 : 0;
	}

	double Rmm(int l, int m, int mp, CMatrix<float> R_1) {
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


	float umm(int l, int m, int mp) {
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

	float Umm(int l, int m, int mp, CMatrix<float> R_1) {
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

	float vmm(int l, int m, int mp) {
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

	float Vmm(int l, int m, int mp, CMatrix<float> R_1) {
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

	float wmm(int l, int m, int mp) {
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

	float Wmm(int l, int m, int mp, CMatrix<float> R_1) {
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

	float P(int i, int l, int mu, int mp, CMatrix<float> R_1) {

		if (abs(mp) < l) {
			return R_1(i + 1, 1) * Rmm(l - 1, mu, mp, R_1);
		} else if (mp == l) {
			return R_1(i + 1, 2) * Rmm(l - 1, mu, l - 1, R_1) - R_1(i + 1, 0) * Rmm(l - 1, mu, -l + 1, R_1);
		} else if (mp == -l) {
			return R_1(i + 1, 2) * Rmm(l - 1, mu,-l + 1, R_1) + R_1(i + 1, 0) * Rmm(l - 1, mu, l - 1, R_1);
		} else {
			return 0.0f;
		}
	}
	
};

#endif // _CAMBISONIC_SOUND_FIELD_HPP_
