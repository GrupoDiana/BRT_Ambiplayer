#ifndef _GUI_MANAGER_HPP_
#define _GUI_MANAGER_HPP_

#include "imgui.h"
#include "ofMain.h"
#include "ofxImGui.h"

class CGuiManager {
public:
	CGuiManager() {
		gui.setup(ofGetCurrentWindow(), nullptr, true, ImGuiConfigFlags_ViewportsEnable, true);
	}

	void draw() {
		gui.begin();

		// Simple window with a Play button
		ImGui::Begin("Control Panel");

		if (ImGui::Button("PLAY")) {
			if (onPlay) onPlay(); // callback
		}

		ImGui::End();
		gui.end();
	}

	// Callback for the Play button
	std::function<void()> onPlay;

private:
	ofxImGui::Gui gui;
};

#endif
