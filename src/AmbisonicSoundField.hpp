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


	// Rotate the field depending on how coordinates have changed
	void RotateField(Common::CQuaternion q) { 

		if (!setupDone) return;

		int numChannels = pow(ambisonicOrder + 1, 2);
		vector<CMonoBuffer<float>> buffer(numChannels, CMonoBuffer<float>(bufferSize, 0.0f));
		buffer[0] = soundFieldArray[0];

		CMatrix<double> R1 = q.R_1();

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < bufferSize; k++) {
					buffer[i + 1][k] += R1(i,j) * soundFieldArray[j + 1][k];
				}
			}
		}

		if (ambisonicOrder > 1) {
			CMatrix<double> R2 = CalculateRMatrix(2, q.R_1());

			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					for (int k = 0; k < bufferSize; k++) {
						buffer[i + 4][k] += R2(i,j) * soundFieldArray[j + 4][k];
					}
				}
			}

			if (ambisonicOrder > 2) {
				CMatrix<double> R3 = CalculateRMatrix(3, q.R_1());

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

	CMatrix<double> CalculateRMatrix(int l, CMatrix<double> R_1) {
		int dim = (2 * l + 1);
		CMatrix<double> R_l(dim, dim);

		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				R_l(i, j) = Rmm(l, m_order[i], m_order[j], R_1);
			}
		}

		return R_l;
	}

	inline int delta(int i, int j) {
		return (i == j) ? 1 : 0;
	}

	double Rmm(int l, int m, int mp, CMatrix<double> R_1) {
		if (l == 1) {
			if (m < -1 || m > 1 || mp < -1 || mp > 1)
				return 0.0;
			return R_1(m + 1, mp + 1);
		}

		return umm(l, m, mp) * Umm(l, m, mp, R_1)
			+ vmm(l, m, mp) * Vmm(l, m, mp, R_1)
			+ wmm(l, m, mp) * Wmm(l, m, mp, R_1);
	}

	double umm(int l, int m, int mp) {
		double umm = 0.0f;

		if (abs(mp) < l) {
			umm = sqrt(((l + m) * (l - m)) / ((l + mp) * (l - mp)));
		} else {
			umm = sqrt(((l + m) * (l - m)) / ((2 * l) * (2 * l - 1)));
		}

		return umm;
	}

	double Umm(int l, int m, int mp, CMatrix<double> R_1) {
		double Umm = 0.0f;

		if (m == 0) {
			return P(0, l, 0, mp, R_1);
		} else if (m > 0) {
			return P(0, l, m, mp, R_1);
		} else if (m < 0) {
			return P(0, l, m, mp, R_1);
		}

		return Umm;
	}

	double vmm(int l, int m, int mp) {
		double vmm = 0.0f;

		if (abs(mp) < l) {
			vmm = 0.5 * sqrt(((1 + delta(m, 0)) * (l + abs(m) - 1) * (l + abs(m))) / ((l + mp) * (l - mp))) * (1 - 2 * delta(m, 0));
		} else {
			vmm = 0.5 * sqrt(((1 + delta(m, 0)) * (l + abs(m) - 1) * (l + abs(m))) / ((2 * l) * (2 * l - 1))) * (1 - 2 * delta(m, 0));
		}

		return vmm;
	}

	double Vmm(int l, int m, int mp, CMatrix<double> R_1) {
		double Vmm = 0.0f;

		if (m == 0) {
			return P(1, l, 1, mp, R_1) + P(-1, l, -1, mp, R_1);
		} else if (m > 0) {
			return P(1, l, m - 1, mp, R_1) * sqrt(1 + delta(m, 1)) - P(-1, l, -m + 1, mp, R_1) * (1 - delta(m, 1));
		} else if (m < 0) {
			return P(1, l, m + 1, mp, R_1) * (1 - delta(m, -1)) + P(-1, l, -m - 1, mp, R_1) * sqrt(1 - delta(m, -1));
		}

		return Vmm;
	}

	double wmm(int l, int m, int mp) {
		double wmm = 0.0f;

		if (abs(mp) < l) {
			wmm = -0.5 * sqrt(((l - abs(m) - 1) * (l - abs(m))) / ((l + mp) * (l - mp))) * (1 - delta(m, 0));
		} else {
			wmm = -0.5 * sqrt(((l - abs(m) - 1) * (l - abs(m))) / ((2 * l) * (2 * l - 1))) * (1 - delta(m, 0));
		}

		return wmm;
	}

	double Wmm(int l, int m, int mp, CMatrix<double> R_1) {
		double Wmm = 0.0f;

		if (m == 0) {
			return 0.0;
		} else if (m > 0) {
			return P(1, l, m + 1, mp, R_1) + P(-1, l, -m - 1, mp, R_1);
		} else if (m < 0) {
			return P(1, l, m - 1, mp, R_1) - P(-1, l, -m + 1, mp, R_1);
		}

		return Wmm;
	}

	double P(int i, int l, int mu, int mp, CMatrix<double> R_1) {
		if (abs(mp) < l) {
			//Se suma 1 en R_1 porque los indices van de 0 a 2 e i va de -1 a 1
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
