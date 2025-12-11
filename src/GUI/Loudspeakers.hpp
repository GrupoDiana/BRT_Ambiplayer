#ifndef _LOUDSPEAKERS_HPP_
#define _LOUDSPEAKERS_HPP_

#include "ofMain.h"

#include "ofApp.h"
#include "LoudspeakerArrayController.hpp"
#include <string>

class CLoudspeakers {
public:
	CLoudspeakers() { }

	void setController(const CLoudSpeakerArrayController * ctrl) {
		controller = ctrl;
	}

void draw(float cx, float cy, float width, float height, const std::vector<float> & speakerEnergies) {
		if (!controller) return;

		float fov = 300.0f;
		float sceneScale = 100.0f;
		float baseRadius = 5.0f;

		ofPushMatrix();
		ofPushStyle();

		ofNoFill();
		ofSetColor(255);
		ofSetLineWidth(1.0);
		ofDrawRectangle(cx, cy, width - margen, height - margen);

		const auto & speakers = controller->GetSpeakers();
		size_t numSpeakers = speakers.size();

		float maxEnergy = 0.0f;
		for (size_t i = 0; i < numSpeakers; ++i) {
			if (i < speakerEnergies.size()) maxEnergy = std::max(maxEnergy, speakerEnergies[i]);
		}
		maxEnergy = std::max(maxEnergy, 1e-6f);

		float axisSize = std::min(width, height) * 0.15f;
		float x0 = cx + width - margen - axisSize;
		float y0 = cy + height - margen - axisSize / 2;
		DrawAxes(x0, y0, axisSize);

		ofTranslate(cx + width / 2.0f, cy + height / 2.0f);

		for (size_t i = 0; i < numSpeakers; ++i) {
			if (!speakers[i]) continue;
			Common::CVector3 pos = speakers[i]->GetSourceTransform().GetPosition();

			float x = pos.x * sceneScale;
			float y = pos.y * sceneScale;
			float z = pos.z * sceneScale;

			// Proyección simple en perspectiva
			float px = -y * fov / (fov - z);
			float py = -x * fov / (fov - z);

			ofFill();
			ofSetColor(128);
			ofDrawCircle(px, py, baseRadius);

			// Halo de energía
			if (i < speakerEnergies.size()) {
				float energy = speakerEnergies[i] / maxEnergy;
				energy = ofClamp(energy, 0.0f, 1.0f);

				energy = pow(energy, 0.6f); 

				float haloBaseRadius = baseRadius + 5.0f;
				int numSteps = 40;

				ofColor haloColor = ofColor(179, 102, 127);

				for (int s = 0; s < numSteps; ++s) {
					float t = float(s) / float(numSteps - 1);
					if (t > energy) break;

					float r = haloBaseRadius + pow(t, 1.0f) * 10.0f;

					ofNoFill();
					ofSetColor(haloColor);
					ofSetLineWidth(2.0f);
					ofDrawCircle(px, py, r);
				}
			}


		}

		ofPopStyle();
		ofPopMatrix();
	}

private:
	const CLoudSpeakerArrayController * controller = nullptr;
	int margen = 10;

	void DrawAxes(float x0, float y0, float size) {
		ofPushStyle();
		ofSetLineWidth(1.0);
		ofSetColor(255, 0, 0);
		ofDrawLine(x0, y0, x0, y0 - size);
		ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);
		ofSetColor(0, 255, 0);
		ofDrawLine(x0, y0, x0 - size, y0);
		ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 - size + 5, y0 + 3);
		ofPopStyle();
	}
};

#endif
