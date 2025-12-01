#ifndef _MENU_PANEL_HPP_
#define _MENU_PANEL_HPP_

#include "imgui.h"
#include "ofMain.h"
#include <functional>

class CMenuPanel {
public:
	CMenuPanel() {
		yawRad = pitchRad = rollRad = 0.0f;
	}

	void draw(float x, float y, float screenWidth, float screenHeight) {
		ImGui::SetNextWindowPos(ImVec2(x + margen, y ));
		ImGui::SetNextWindowSize(ImVec2(screenWidth - 2 * margen, screenHeight -  margen));

		ImGui::Begin("Ambisonic Controls", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		// --- Play/Stop Buttons ---
		if (ImGui::Button("PLAY")) {
			if (onPlay) onPlay();
		}
		ImGui::SameLine();
		if (ImGui::Button("STOP")) {
			if (onStop) onStop();
		}

		ImGui::Separator();

		// --- ComboBox ---
		const char * items[] = { "Octahedron", "Cube", "Icosahedron", "Dodecahedron" };
		static int item_selected_idx = 0;
		static ImGuiComboFlags flags = 0;

		ImGui::Text("Loudspeaker Layout:");

		const char * combo_preview_value = items[item_selected_idx];
		if (ImGui::BeginCombo("", items[item_selected_idx], flags)) {
			for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
				const bool is_selected = (item_selected_idx == n);
				if (ImGui::Selectable(items[n], is_selected)) {
					item_selected_idx = n;
					layoutIndex = n; // actualizamos el índice
					if (onLayoutChanged) // llamamos al callback
						onLayoutChanged(layoutIndex);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}


		// --- Sliders---
		ImGui::Text("Field Rotation:");
		bool changed = false;

		ImGui::PushItemWidth(screenWidth - 3 * margen);

		ImGui::Text("Yaw");
		changed |= ImGui::SliderAngle("Yaw", &yawRad, -180.0f, 180.0f);

		ImGui::Text("Pitch");
		changed |= ImGui::SliderAngle("Pitch", &pitchRad, -180.0f, 180.0f);

		ImGui::Text("Roll");
		changed |= ImGui::SliderAngle("Roll", &rollRad, -180.0f, 180.0f);

		ImGui::PopItemWidth();

		if (changed && onRotationChanged) {
			onRotationChanged(yawRad, pitchRad, rollRad);
		}

		ImGui::End();
	}

	// --- Callbacks ---
	std::function<void()> onPlay;
	std::function<void()> onStop;
	std::function<void(float, float, float)> onRotationChanged;
	std::function<void(int)> onLayoutChanged;

private:
	float yawRad, pitchRad, rollRad;
	int layoutIndex;

	int margen = 10;
};

#endif
