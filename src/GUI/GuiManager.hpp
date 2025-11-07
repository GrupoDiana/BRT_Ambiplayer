#ifndef _GUI_MANAGER_HPP_
#define _GUI_MANAGER_HPP_

#include "imgui.h"
#include "ofMain.h"
#include "ofxImGui.h"

class CGuiManager {
public:
	CGuiManager() {
		gui.setup(ofGetCurrentWindow(), nullptr, true, 0, true);
		yawRad = pitchRad = rollRad = 0.0f;
	}

	void draw() {
		gui.begin();
		ImGuiViewport * viewport = ImGui::GetMainViewport();
		float screenWidth = viewport->Size.x;
		float screenHeight = viewport->Size.y;

		ImGui::SetNextWindowPos(ImVec2(10, 100)); 
		ImGui::SetNextWindowSize(ImVec2(screenWidth / 5, screenHeight *0.85));

		ImGui::Begin("Ambisonic Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		if (ImGui::Button("PLAY")) {
			if (onPlay) onPlay();
		}
		ImGui::SameLine();
		if (ImGui::Button("STOP")) {
			if (onStop) onStop();
		}

		ImGui::Separator();

		ImGui::Text("Field Rotation:");
		bool changed = false;
		changed |= ImGui::SliderAngle("Yaw", &yawRad, -180.0f, 180.0f);
		changed |= ImGui::SliderAngle("Pitch", &pitchRad, -180.0f, 180.0f);
		changed |= ImGui::SliderAngle("Roll", &rollRad, -180.0f, 180.0f);

		if (changed && onRotationChanged) {
			onRotationChanged(yawRad, pitchRad, rollRad);
		}

		ImGui::End();
		gui.end();
	}

	// --- Callbacks ---
	std::function<void()> onPlay;
	std::function<void()> onStop;
	std::function<void(float, float, float)> onRotationChanged;

	// --- Estado actual de rotaciones ---
	float yawRad, pitchRad, rollRad;

private:
	ofxImGui::Gui gui;
};

#endif
