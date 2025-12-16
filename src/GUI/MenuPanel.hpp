#ifndef _MENU_PANEL_HPP_
#define _MENU_PANEL_HPP_

#include "AmbisonicSoundFieldController.hpp"
#include "imgui.h"
#include "ofMain.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

class CMenuPanel {
public:
	CMenuPanel() {
		SFyawRad = SFpitchRad = SFrollRad = 0.0f;
		LSyawRad = LSpitchRad = LSrollRad = 0.0f;
	}

	void draw(float x, float y, float screenWidth, float screenHeight) {
		ImGui::SetNextWindowPos(ImVec2(x + margen, y));
		ImGui::SetNextWindowSize(ImVec2(screenWidth - 2 * margen, screenHeight - margen));
		ImVec4 defaultColor = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabActive);

		ImGui::Begin("Ambisonic Controls", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar);

		// --- Play/Stop/WAV/Source Buttons ---
		if (ImGui::Button("PLAY") && onPlay) onPlay();
		ImGui::SameLine();
		if (ImGui::Button("STOP") && onStop) onStop();
		ImGui::SameLine();
		if (ImGui::Button("WAV") && onWAV) onWAV();
		ImGui::SameLine();
		if (ImGui::Button("Source")) showSourcePopup = true;

		ImGui::Separator();

		// --- Order Combo ---
		const char * order_items[] = { "1", "2", "3" };
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

		// --- Loudspeaker Layout Combo ---
		const char * layout_items[] = { "Octahedron", "Cube", "Icosahedron", "Dodecahedron" };
		static int layout_selected_idx = 0;

		ImGui::Text("Loudspeaker Layout:");
		if (ImGui::BeginCombo("##Layout", layout_items[layout_selected_idx])) {
			for (int n = 0; n < IM_ARRAYSIZE(layout_items); ++n) {
				bool selected = (layout_selected_idx == n);
				if (ImGui::Selectable(layout_items[n], selected)) {
					layout_selected_idx = n;
					if (onLayoutChanged) onLayoutChanged(n);
				}
				if (selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// --- Field Rotation Sliders ---
		ImGui::Separator();
		ImGui::Text("Field Rotation:");
		bool SFchanged = false;
		SFchanged |= ImGui::SliderAngle("Yaw##SF", &SFyawRad, -180, 180);
		SFchanged |= ImGui::SliderAngle("Pitch##SF", &SFpitchRad, -180, 180);
		SFchanged |= ImGui::SliderAngle("Roll##SF", &SFrollRad, -180, 180);
		if (SFchanged && onSFRotationChanged)
			onSFRotationChanged(SFyawRad, SFpitchRad, SFrollRad);

		// --- Loudspeaker Rotation Sliders ---
		ImGui::Separator();
		ImGui::Text("Loudspeakers Rotation:");
		bool LSchanged = false;
		LSchanged |= ImGui::SliderAngle("Yaw##LS", &LSyawRad, -180, 180);
		LSchanged |= ImGui::SliderAngle("Pitch##LS", &LSpitchRad, -180, 180);
		LSchanged |= ImGui::SliderAngle("Roll##LS", &LSrollRad, -180, 180);
		if (LSchanged && onLSRotationChanged)
			onLSRotationChanged(LSyawRad, LSpitchRad, LSrollRad);

		// --- Loaded Controllers ---
		ImGui::Separator();
		ImGui::Text("Loaded Controllers:");
		for (size_t i = 0; i < controllers.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			ControllerPtr controller = controllers[i];
			std::string headerTitle = controller->GetFilePathName();

			if (ImGui::CollapsingHeader(headerTitle.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

				if (controller->IsPlaying()) {
					ImGui::PushStyleColor(ImGuiCol_Button, defaultColor); 
				}
				if (ImGui::Button("Play")) {
					if (onControllerPlay) onControllerPlay(controller);
				}
				if (controller->IsPlaying()) ImGui::PopStyleColor();
				ImGui::SameLine();

				if (ImGui::Button("Stop") && onControllerStop) onControllerStop(controller);
				ImGui::SameLine();

				if (controller->IsMuted()) {
					ImGui::PushStyleColor(ImGuiCol_Button, defaultColor); 
				}
				if (ImGui::Button("Mute")) {
					if (onControllerMute) onControllerMute(controller);
				}
				if (controller->IsMuted()) ImGui::PopStyleColor();
				ImGui::SameLine();

				if (controller->IsSolo()) {
					ImGui::PushStyleColor(ImGuiCol_Button, defaultColor);
				}
				if (ImGui::Button("Solo")) {
					if (onControllerSolo) onControllerSolo(controller);
				}
				if (controller->IsSolo()) ImGui::PopStyleColor();
				ImGui::SameLine();

				if (controller->IsLoop()) {
					ImGui::PushStyleColor(ImGuiCol_Button, defaultColor);
				}
				if (ImGui::Button("Loop")) {
					if (onControllerLoop) onControllerLoop(controller);
				}
				if (controller->IsLoop()) ImGui::PopStyleColor();
				ImGui::SameLine();

				if (ImGui::Button("x") && onControllerRemove) {
					onControllerRemove(controller);
					ImGui::PopID();
					break;
				}

				// Rotation sliders for controller
				auto & rot = controller->GetRotation();
				bool changed = false;
				changed |= ImGui::SliderAngle("Yaw", &rot.yaw, -180, 180);
				changed |= ImGui::SliderAngle("Pitch", &rot.pitch, -180, 180);
				changed |= ImGui::SliderAngle("Roll", &rot.roll, -180, 180);
				if (changed && onIRotationChanged)
					onIRotationChanged(controller, rot.yaw, rot.pitch, rot.roll);

				float currentSec = controller->GetCurrentTimeSeconds();
				float totalSec = controller->GetTotalTimeSeconds();

				std::string timeText = FormatTime(currentSec) + " / " + FormatTime(totalSec);

				// Playback progress bar + time
				float progress = controller->GetPlaybackProgress();

				float timeWidth = ImGui::CalcTextSize(timeText.c_str()).x + 8.0f;
				ImVec2 barSize(ImGui::GetContentRegionAvail().x - timeWidth, 8.0f);

				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, defaultColor);

				ImGui::ProgressBar(progress, barSize, "");

				if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
					ImVec2 min = ImGui::GetItemRectMin();
					ImVec2 max = ImGui::GetItemRectMax();

					float norm = (ImGui::GetMousePos().x - min.x) / (max.x - min.x);
					norm = std::clamp(norm, 0.0f, 1.0f);

					controller->SetPlaybackProgress(norm);
				}

				ImGui::SameLine();
				ImGui::TextUnformatted(timeText.c_str());

				ImGui::PopStyleColor(2);
			}

			ImGui::PopID();
		}

		ImGui::End();

		// --- Source Popup ---
		if (showSourcePopup) ImGui::OpenPopup("Create Puntual Source");
		if (ImGui::BeginPopupModal("Create Puntual Source", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			static int numFrames = 48000;
			static int Azimuth = 0;
			static int Elevation = 0;

			ImGui::Text("Parameters of the source:");
			ImGui::InputInt("Azimuth", &Azimuth);
			ImGui::InputInt("Elevation", &Elevation);
			ImGui::Separator();

			if (ImGui::Button("Create") && onCreateSource) {
				onCreateSource(numFrames, SphericalToCartesian(Azimuth, Elevation));
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

	void addController(std::shared_ptr<CAmbisonicSoundFieldController> c, const std::string & filePathName) {
		controllers.push_back(c);
		controllers.back()->GetFilePathName() = filePathName;
	}

	void removeController(std::shared_ptr<CAmbisonicSoundFieldController> c) {
		controllers.erase(std::remove(controllers.begin(), controllers.end(), c), controllers.end());
	}

	void clearControllers() {
		controllers.clear();
	}


	// --- Callbacks ---
	std::function<void()> onPlay;
	std::function<void()> onStop;
	std::function<void()> onWAV;
	std::function<void(size_t, Common::CVector3)> onCreateSource;
	std::function<void(float, float, float)> onSFRotationChanged;
	std::function<void(float, float, float)> onLSRotationChanged;
	std::function<void(int)> onLayoutChanged;
	std::function<void(int)> onOrderChanged;

	// --- Controller-specific callbacks ---
	using ControllerPtr = std::shared_ptr<CAmbisonicSoundFieldController>;
	std::function<void(ControllerPtr)> onControllerPlay;
	std::function<void(ControllerPtr)> onControllerStop;
	std::function<void(ControllerPtr)> onControllerMute;
	std::function<void(ControllerPtr)> onControllerRemove;
	std::function<void(ControllerPtr)> onControllerSolo;
	std::function<void(ControllerPtr)> onControllerLoop;
	std::function<void(ControllerPtr, float, float, float)> onIRotationChanged;

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

	std::string FormatTime(float seconds) {
	int min = static_cast<int>(seconds) / 60;
	int sec = static_cast<int>(seconds) % 60;

	char buf[16];
	snprintf(buf, sizeof(buf), "%02d:%02d", min, sec);
	return std::string(buf);
	}

};

#endif // _MENU_PANEL_HPP_
