#ifndef _LOUDSPEAKERS_HPP_
#define _LOUDSPEAKERS_HPP_

#include "LoudspeakerArrayController.hpp"
#include "MenuPanel.hpp"
#include "imgui.h"
#include "ofMain.h"

#include <functional>
#include <string>

class CLoudspeakers {
public:
	CLoudspeakers()
		: isDragging(false) { }

	void setController(const CLoudSpeakerArrayController * ctrl) {
		controller = ctrl;
	}

	// --- Draw speakers and halo ---
	void draw(float cx, float cy, float width, float height, const std::vector<float> & speakerEnergies) {
		if (!controller) return;

		float sceneScale = std::min(width, height) / 4.0f;
		float baseRadius = std::min(width, height) / 40.0f;

		// --- ORIGINAL COLOR SETUP ---
		ImVec4 defaultColor = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabActive);
		ofColor haloColor;
		haloColor.r = static_cast<unsigned char>(defaultColor.x * 255.0f);
		haloColor.g = static_cast<unsigned char>(defaultColor.y * 255.0f);
		haloColor.b = static_cast<unsigned char>(defaultColor.z * 255.0f);
		haloColor.a = static_cast<unsigned char>(defaultColor.w * 255.0f);

		ofPushMatrix();
		ofPushStyle();

		ofNoFill();
		ofSetColor(255);
		ofSetLineWidth(1.0f);
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

			float fov = 300.0f;
			float px = -y * fov / (fov - z);
			float py = -x * fov / (fov - z);

			ofFill();
			ofSetColor(128);
			ofDrawCircle(px, py, baseRadius);

			if (i < speakerEnergies.size()) {
				float energy = speakerEnergies[i] / maxEnergy;
				energy = ofClamp(energy, 0.0f, 1.0f);
				energy = pow(energy, 0.6f);

				float haloBaseRadius = baseRadius + 5.0f;
				int numSteps = 40;

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

	// --- Mouse interaction ---
	void mousePressed(int mx, int my, float cx, float cy, float width, float height) {
		// Check if inside loudspeaker window
		if (mx >= cx && mx <= cx + width && my >= cy && my <= cy + height) {
			isDragging = true;
			lastMouseX = mx;
			lastMouseY = my;
		}
	}

	void mouseDragged(int mx, int my) {
		if (!isDragging) return;

		float dx = mx - lastMouseX;
		float dy = my - lastMouseY;

		// Apply simple rotation sensitivity
		float sensitivity = 0.005f;
		rotation.y += dx * sensitivity; // yaw
		rotation.x += dy * sensitivity; // pitch

		lastMouseX = mx;
		lastMouseY = my;

		if (onRotationChangedByMouse) {
			onRotationChangedByMouse(rotation.y, rotation.x, rotation.z);
		}
	}

	void mouseReleased() {
		isDragging = false;
	}

	// --- Getter for current rotation ---
	ofVec3f getRotation() const {
		return rotation;
	}

	void setRotation(float yaw, float pitch, float roll) {
		rotation.x = pitch;
		rotation.y = yaw;
		rotation.z = roll;
	}

	void setOnRotationChangedByMouse(std::function<void(float yaw, float pitch, float roll)> cb) {
		onRotationChangedByMouse = cb;
	}

	std::function<void(float yaw, float pitch, float roll)> onRotationChangedByMouse;

private:
	const CLoudSpeakerArrayController * controller = nullptr;
	int margen = 10;

	// --- Rotation state ---
	ofVec3f rotation = { 0.0f, 0.0f, 0.0f };
	bool isDragging = false;
	int lastMouseX = 0, lastMouseY = 0;

	void DrawAxes(float x0, float y0, float size) {
		ofPushStyle();
		ofSetLineWidth(1.0);
		ofSetColor(255, 255, 255);
		ofDrawBitmapString("X", x0 - 4, y0 - size - 4);
		ofSetColor(255, 0, 0);
		ofDrawLine(x0, y0, x0, y0 - size);
		ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);
		ofSetColor(255, 255, 255);
		ofDrawBitmapString("Y", x0 - size - 8, y0 + 4);
		ofSetColor(0, 255, 0);
		ofDrawLine(x0, y0, x0 - size, y0);
		ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 - size + 5, y0 + 3);
		ofPopStyle();
	}
};

#endif
