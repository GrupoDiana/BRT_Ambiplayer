#ifndef _MENU_PANEL_HPP_
#define _MENU_PANEL_HPP_

#include "imgui.h"
#include "ofMain.h"
#include "AmbisonicSoundFieldController.hpp"

#include <functional>

class CMenuPanel {
public:
	CMenuPanel() {
		SFyawRad = SFpitchRad = SFrollRad = 0.0f;
		LSyawRad = LSpitchRad = LSrollRad = 0.0f;
	}

	void draw(float x, float y, float screenWidth, float screenHeight) {
		ImGui::SetNextWindowPos(ImVec2(x + margen, y));
		ImGui::SetNextWindowSize(ImVec2(screenWidth - 2 * margen, screenHeight - margen));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));

		ImGui::Begin("Ambisonic Controls", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);

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
		ImGui::SameLine();
		if (ImGui::Button("Source")) {
			showSourcePopup = true;
		}

		ImGui::Separator();

		// --- Order Combo ---
		const char * order_items[] = { "1", "2", "3"};
		static int order_selected_idx = 0;

		ImGui::Text("Order:");
		if (ImGui::BeginCombo("##Order", order_items[order_selected_idx])) {
			for (int n = 0; n < IM_ARRAYSIZE(order_items); ++n) {
				bool selected = (order_selected_idx == n);
				if (ImGui::Selectable(order_items[n], selected)) {
					order_selected_idx = n;
					if (onOrderChanged) onOrderChanged(n);
				}
				if (selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
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

		// --- Sliders Field Rotation ---
		ImGui::Separator();
		ImGui::Text("Field Rotation:");

		bool SFchanged = false;
		SFchanged |= ImGui::SliderAngle("Yaw##SF", &SFyawRad, -180, 180);
		SFchanged |= ImGui::SliderAngle("Pitch##SF", &SFpitchRad, -180, 180);
		SFchanged |= ImGui::SliderAngle("Roll##SF", &SFrollRad, -180, 180);

		if (SFchanged && onSFRotationChanged)
			onSFRotationChanged(SFyawRad, SFpitchRad, SFrollRad);

		// --- Sliders Loudspeakers Rotation ---
		ImGui::Separator();
		ImGui::Text("Loudspeakers Rotation:");

		bool LSchanged = false;
		LSchanged |= ImGui::SliderAngle("Yaw##LS", &LSyawRad, -180, 180);
		LSchanged |= ImGui::SliderAngle("Pitch##LS", &LSpitchRad, -180, 180);
		LSchanged |= ImGui::SliderAngle("Roll##LS", &LSrollRad, -180, 180);

		if (LSchanged && onLSRotationChanged)
			onLSRotationChanged(LSyawRad, LSpitchRad, LSrollRad);

		// ----------------------------------------
		ImGui::Separator();
		ImGui::Text("Loaded Controllers:");

		for (int i = 0; i < controllers.size(); ++i) {
			ImGui::PushID(i);

			std::string headerTitle = controllers[i]->GetFilePathName();
			if (ImGui::CollapsingHeader(headerTitle.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

				if (ImGui::Button("Play"))
					if (onControllerPlay) onControllerPlay(i);

				ImGui::SameLine();

				if (ImGui::Button("Stop"))
					if (onControllerStop) onControllerStop(i);

				ImGui::SameLine();

				if (ImGui::Button("Mute"))
					if (onControllerMute) onControllerMute(i);

				// Sliders para rotación
				auto & rot = controllers[i]->GetRotation();

				bool changed = false;
				changed |= ImGui::SliderAngle("Yaw", &rot.yaw, -180, 180);
				changed |= ImGui::SliderAngle("Pitch", &rot.pitch, -180, 180);
				changed |= ImGui::SliderAngle("Roll", &rot.roll, -180, 180);

				if (changed && onIRotationChanged)
					onIRotationChanged(i, rot.yaw, rot.pitch, rot.roll);
			}

			ImGui::PopID();
		}


		ImGui::End();
		// ---- POPUP FOR SOURCE ----
		
		if (showSourcePopup) {
			ImGui::OpenPopup("Create Puntual Source");
		}

		if (ImGui::BeginPopupModal("Create Puntual Source", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

			static int numFrames = 48000; // valor por defecto
			static int Azimuth = 0;
			static int Elevation = 0;

			ImGui::Text("Parameters of the source:");

			ImGui::InputInt("Azimuth", &Azimuth);
			ImGui::InputInt("Elevation", &Elevation);

			ImGui::Separator();

			if (ImGui::Button("Create")) {
				if (onCreateSource)
					onCreateSource(numFrames, SphericalToCartesian(Azimuth,Elevation));

				showSourcePopup = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel")) {
				showSourcePopup = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}


		ImGui::PopStyleColor(5);
	}

	void addController(std::shared_ptr<CAmbisonicSoundFieldController> c, std::string filePathName_) {
		controllers.push_back(c);
		controllers[controllers.size()-1]->GetFilePathName() = filePathName_;
	}

	// --- Callbacks ---
	std::function<void()> onPlay;
	std::function<void()> onStop;
	std::function<void()> onWAV;
	std::function<void(size_t numFrames, Common::CVector3 pos)> onCreateSource;
	std::function<void(float, float, float)> onSFRotationChanged;
	std::function<void(float, float, float)> onLSRotationChanged;
	std::function<void(int,float, float, float)> onIRotationChanged;
	std::function<void(int)> onLayoutChanged;
	std::function<void(int)> onOrderChanged;

	// ---Controller specific callbacks ---
	std::function<void(int)> onControllerPlay;
	std::function<void(int)> onControllerStop;
	std::function<void(int)> onControllerMute;

private:
	float SFyawRad, SFpitchRad, SFrollRad;
	float LSyawRad, LSpitchRad, LSrollRad;

	int layoutIndex;
	int margen = 10;

	std::vector<std::shared_ptr<CAmbisonicSoundFieldController>> controllers;

	bool showSourcePopup = false;

	Common::CVector3 SphericalToCartesian(float azimuthDeg, float elevationDeg, float radius = 1.0f) {
		float azim = ofDegToRad(azimuthDeg); 
		float elev = ofDegToRad(elevationDeg); 

		float x = radius * cos(elev) * cos(azim);
		float y = radius * cos(elev) * sin(azim);
		float z = radius * sin(elev);

		return Common::CVector3(x, y, z);
	}
};

#endif
