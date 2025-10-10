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
		ImGui::Begin("prueba",nullptr,0);
		
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

		ImGui::SameLine();
		 if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
			if (onPlay) onPlay(); // callback
		}

		 ImGui::SameLine();
		if (ImGui::Button("STOP")) {
			if (onStop) onStop(); // callback
		}
		
		//ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::End();
		gui.end();
	}

	// Callback for the Play button
	std::function<void()> onPlay;

	// Callback for the Stop button
	std::function<void()> onStop;

private:
	ofxImGui::Gui gui;
};

#endif
