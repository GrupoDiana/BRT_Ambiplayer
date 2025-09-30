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

		static bool ImGuiChildFlags_Borders = true;

		// Simple window with a Play button
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("ChildR", ImVec2(100, 260), ImGuiChildFlags_Borders, window_flags);

		 if (ImGui::Button("PLAY")) {
			if (onPlay) onPlay(); // callback
		}
		
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}

	// Callback for the Play button
	std::function<void()> onPlay;

private:
	ofxImGui::Gui gui;
};

#endif
