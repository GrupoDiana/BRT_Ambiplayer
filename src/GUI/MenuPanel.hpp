#ifndef _MENU_PANEL_HPP_
#define _MENU_PANEL_HPP_

#include "imgui.h"
#include "ofMain.h"
#include <functional>

class CMenuPanel {
public:
	CMenuPanel() {
		SFyawRad = SFpitchRad = SFrollRad = LSyawRad = LSpitchRad = LSrollRad = 0.0f;
	}

	void draw(float x, float y, float screenWidth, float screenHeight) {
		ImGui::SetNextWindowPos(ImVec2(x + margen, y));
		ImGui::SetNextWindowSize(ImVec2(screenWidth - 2 * margen, screenHeight - margen));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f)); // fondo gris oscuro
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)); // borde negro suave
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));

		ImGui::Begin("Ambisonic Controls", nullptr,
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);

		// --- Play/Stop Buttons ---
		if (ImGui::Button("PLAY")) {
			if (onPlay) onPlay();
		}
		ImGui::SameLine();
		if (ImGui::Button("STOP")) {
			if (onStop) onStop();
		}
		ImGui::SameLine();
		if (ImGui::Button("WAV")) {
			if (onWAV) onWAV();
		}

		ImGui::Separator();

		// --- Combo ---
		const char * items[] = { "Octahedron", "Cube", "Icosahedron", "Dodecahedron" };
		static int item_selected_idx = 0;

		ImGui::Text("Loudspeaker Layout:");
		if (ImGui::BeginCombo("##Layout", items[item_selected_idx])) {
			for (int n = 0; n < IM_ARRAYSIZE(items); ++n) {
				bool selected = (item_selected_idx == n);
				if (ImGui::Selectable(items[n], selected)) {
					item_selected_idx = n;
					if (onLayoutChanged) onLayoutChanged(n);
				}
				if (selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// --- Sliders ---
		ImGui::Separator();
		ImGui::Text("Field Rotation:");

		bool SFchanged = false;
		SFchanged |= ImGui::SliderAngle("Yaw##SF", &SFyawRad, -180, 180);
		SFchanged |= ImGui::SliderAngle("Pitch##SF", &SFpitchRad, -180, 180);
		SFchanged |= ImGui::SliderAngle("Roll##SF", &SFrollRad, -180, 180);

		if (SFchanged && onSFRotationChanged)
			onSFRotationChanged(SFyawRad, SFpitchRad, SFrollRad);

		ImGui::Separator();
		ImGui::Text("Loudspeakers Rotation:");

		bool LSchanged = false;
		LSchanged |= ImGui::SliderAngle("Yaw##LS", &LSyawRad, -180, 180);
		LSchanged |= ImGui::SliderAngle("Pitch##LS", &LSpitchRad, -180, 180);
		LSchanged |= ImGui::SliderAngle("Roll##LS", &LSrollRad, -180, 180);

		if (LSchanged && onLSRotationChanged)
			onLSRotationChanged(LSyawRad, LSpitchRad, LSrollRad);

		ImGui::End();
		ImGui::PopStyleColor(5);
	}



	// --- Callbacks ---
	std::function<void()> onPlay;
	std::function<void()> onStop;
	std::function<void()> onWAV;
	std::function<void(float, float, float)> onSFRotationChanged;
	std::function<void(float, float, float)> onLSRotationChanged;
	std::function<void(int)> onLayoutChanged;

private:
	float SFyawRad, SFpitchRad, SFrollRad, LSyawRad, LSpitchRad, LSrollRad;
	int layoutIndex;

	int margen = 10;
};

#endif
