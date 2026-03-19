#ifndef _MENU_PANEL_HPP_
#define _MENU_PANEL_HPP_

#include "AmbisonicSoundFieldController.hpp"
#include "TrackerController.hpp"
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
		initSerialPorts();
	}

	void draw(float x, float y, float screenWidth, float screenHeight) {
		ImGui::SetNextWindowPos(ImVec2(x + margen, y));
		ImGui::SetNextWindowSize(ImVec2(screenWidth - 2 * margen, screenHeight - margen));
		ImVec4 defaultColor = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabActive);

		ImVec4 origHeader = ImGui::GetStyleColorVec4(ImGuiCol_Header);
		ImVec4 origHeaderHover = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
		ImVec4 origHeaderActive = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);

		ImVec4 grayColor(0.5f, 0.5f, 0.5f, 1.0f);


		//ImGuiWindowFlags_AlwaysHorizontalScrollbar
		ImGui::Begin("Ambisonic", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize );

		ImGui::BeginChild("ScrollRegion",
			ImVec2(0, -30), // deja espacio abajo
			true,
			ImGuiWindowFlags_AlwaysVerticalScrollbar);

		// --- Play/Stop/WAV/Source Buttons ---
		if (isPlaying) {
			ImGui::PushStyleColor(ImGuiCol_Button, defaultColor);
		}
		if (ImGui::Button("PLAY")) {
			isPlaying = !isPlaying;
		if (onPlay) onPlay(isPlaying);
		}
		if (isPlaying) {
			ImGui::PopStyleColor();
		}
		ImGui::SameLine();
		if (ImGui::Button("STOP") && onStop) {
			onStop();
			isPlaying = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("LOAD SF")) showWAVPopup = true;
		ImGui::SameLine();
		if (ImGui::Button("LOAD PS")) showSourcePopup = true;
		ImGui::SameLine();
		if (ImGui::Button("SETTINGS")) showSettingsPopUp = true;

		ImGui::Separator();

		if (ImGui::CollapsingHeader("Audio Settings", ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::Text("Audio Output Devices:");

			if (ImGui::BeginCombo(
					"##AudioDevices",
					audio_selected_idx == -1 ? "-Select device-" : audioDevices[audio_selected_idx].name.c_str())) {

				// Placeholder
				bool placeholder_selected = (audio_selected_idx == -1);
				if (ImGui::Selectable("-Select device-", placeholder_selected)) {
					audio_selected_idx = -1;
					if (onAudioDeviceChanged)
						onAudioDeviceChanged(-1);
				}
				if (placeholder_selected)
					ImGui::SetItemDefaultFocus();

				for (int n = 0; n < (int)audioDevices.size(); ++n) {

					bool selected = (audio_selected_idx == n);

					if (ImGui::Selectable(audioDevices[n].name.c_str(), selected)) {
						audio_selected_idx = n;

						if (onAudioDeviceChanged)
							onAudioDeviceChanged(n);
					}

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}

		if (ImGui::CollapsingHeader("Visualization Settings", NULL)) {

			static int k = 5;

			const char * visualization_items[] = { "Geodesic Projection", "Radial Projection" };
			static int visualization_selected_idx = 0;

			ImGui::Text("Visualization Mode:");
			if (ImGui::BeginCombo("##Visualization Mode", visualization_items[visualization_selected_idx])) {
				for (int n = 0; n < IM_ARRAYSIZE(visualization_items); ++n) {
					bool selected = (visualization_selected_idx == n);
					if (ImGui::Selectable(visualization_items[n], selected)) {
						visualization_selected_idx = n;
						if (onVisualizationChanged) onVisualizationChanged(n);
					}
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Separator();
			ImGui::Text("For Geodesic Projection only:");
			ImGui::InputInt("k", &k);

			if (ImGui::Button("Update")) {
				onChangeK(k);
			}

		}


		if (ImGui::CollapsingHeader("Ambisonic Settings", NULL)) {


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

			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Header, grayColor);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, grayColor.w * 0.8f); 
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, grayColor.w * 0.8f); 

			// --- Overall SoundField Rotation ---
			if (ImGui::CollapsingHeader("Overall SoundField Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
				bool SFchanged = false;

				if (ImGui::Button("-##yawsf")) {
					SFyawRad -= ofDegToRad(1.0f);
					SFchanged = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("+##yawsf")) {
					SFyawRad += ofDegToRad(1.0f);
					SFchanged = true;
				}
				ImGui::SameLine();
				SFchanged |= ImGui::SliderAngle("Yaw##SF", &SFyawRad, -180, 180);
				
				if (ImGui::Button("-##pitchsf")) {
					SFpitchRad -= ofDegToRad(1.0f);
					SFchanged = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("+##pitchsf")) {
					SFpitchRad += ofDegToRad(1.0f);
					SFchanged = true;
				}
				ImGui::SameLine();
				SFchanged |= ImGui::SliderAngle("Pitch##SF", &SFpitchRad, -180, 180);

				if (ImGui::Button("-##rollsf")) {
					SFrollRad -= ofDegToRad(1.0f);
					SFchanged = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("+##rollsf")) {
					SFrollRad += ofDegToRad(1.0f);
					SFchanged = true;
				}
				ImGui::SameLine();
				SFchanged |= ImGui::SliderAngle("Roll##SF", &SFrollRad, -180, 180);

				if (SFchanged && onSFRotationChanged)
					onSFRotationChanged(SFyawRad, SFpitchRad, SFrollRad);
			}

			//     --- Total Rotation (SoundField + Tracker) ---
			ImGui::Text("Final Yaw: %.2f", ofRadToDeg(TotalyawRad));
			ImGui::Text("Final Pitch: %.2f", ofRadToDeg(TotalpitchRad));
			ImGui::Text("Final Roll: %.2f", ofRadToDeg(TotalrollRad));



			// --- Loudspeaker Rotation Sliders ---
			if (ImGui::CollapsingHeader("LoudSpeakers Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
				bool LSchanged = false;
				
				if (ImGui::Button("-##yawls")) {
					LSyawRad -= ofDegToRad(1.0f);
					LSchanged = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("+##yawls")) {
					LSyawRad += ofDegToRad(1.0f);
					LSchanged = true;
				}
				ImGui::SameLine();
				LSchanged |= ImGui::SliderAngle("Yaw##LS", &LSyawRad, -180, 180);
				
				if (ImGui::Button("-##pitchls")) {
					LSpitchRad -= ofDegToRad(1.0f);
					LSchanged = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("+##pitchls")) {
					LSpitchRad += ofDegToRad(1.0f);
					LSchanged = true;
				}
				ImGui::SameLine();
				LSchanged |= ImGui::SliderAngle("Pitch##LS", &LSpitchRad, -180, 180);
				
				if (ImGui::Button("-##rollls")) {
					LSrollRad -= ofDegToRad(1.0f);
					LSchanged = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("+##rollls")) {
					LSrollRad += ofDegToRad(1.0f);
					LSchanged = true;
				}
				ImGui::SameLine();
				LSchanged |= ImGui::SliderAngle("Roll##LS", &LSrollRad, -180, 180);

				if (LSchanged && onLSRotationChanged)
					onLSRotationChanged(LSyawRad, LSpitchRad, LSrollRad);
			}
		}

		ImGui::PopStyleColor(3);

		// --- Tracker settings ---
		if (ImGui::CollapsingHeader("Tracker Settings", NULL)){

			ImGui::Text("Serial Terminal:");
			static int serial_selected_idx = -1; 

			if (ImGui::BeginCombo(
					"##SerialPorts",
					serial_selected_idx == -1 ? "-Select COM-" : availableCOMs[serial_selected_idx].port.c_str())) {
				// Primer ítem: placeholder
				bool placeholder_selected = (serial_selected_idx == -1);
				if (ImGui::Selectable("-Select COM-", placeholder_selected)) {
					serial_selected_idx = -1; // ningún COM seleccionado
					if (onSerialPortChanged)
						onSerialPortChanged(-1);
				}
				if (placeholder_selected)
					ImGui::SetItemDefaultFocus();

				// COMs reales
				for (int n = 0; n < (int)availableCOMs.size(); ++n) {
					bool selected = (serial_selected_idx == n);

					if (ImGui::Selectable(availableCOMs[n].name.c_str(), selected)) {
						serial_selected_idx = n; 
						if (onSerialPortChanged)
							onSerialPortChanged(n);
					}

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::Text("Calibrations:");
			//Boton calibrate head
			if (ImGui::Button("HEAD") && onCalibrateHead) {
				onCalibrateHead();
			}
			ImGui::SameLine();
			//boton calibrate compass
			if (ImGui::Button("COMPASS") && onCalibrateCompass) {
				onCalibrateCompass();
			}

			ImGui::Text("LLY: %.2f", ofRadToDeg(TotalyawRad));
			ImGui::SameLine();
			ImGui::Text("LLP: %.2f", ofRadToDeg(TotalpitchRad));
			ImGui::SameLine();
			ImGui::Text("LLR: %.2f", ofRadToDeg(TotalrollRad));

		}
		ImGui::Separator();

		// --- Listener settings ---
		if (ImGui::CollapsingHeader("Listener Settings", NULL)) {
			ImGui::Text("HRTF:");
			if (ImGui::Button("HRTF")) {
				//COMPLETAR
			}
		}

		// --- Loaded Controllers ---
		ImGui::Separator();
		if (ImGui::CollapsingHeader("SoundField Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (size_t i = 0; i < controllers.size(); ++i) {
				ImGui::PushID(static_cast<int>(i));
				ControllerPtr controller = controllers[i];
				std::string headerTitle = controller->GetFilePathName();

				ImGui::PushStyleColor(ImGuiCol_Header, grayColor);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, grayColor.w * 0.8f); 
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, grayColor.w * 0.8f); 

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

					// Rotation sliders for controller with manual + / - buttons
					auto & rot = controller->GetRotation();
					bool changed = false;

					// Yaw
					if (ImGui::Button("-##yawctl")) {
						rot.yaw -= ofDegToRad(1.0f);
						changed = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("+##yawctl")) {
						rot.yaw += ofDegToRad(1.0f);
						changed = true;
					}
					ImGui::SameLine();
					changed |= ImGui::SliderAngle("Yaw##ctl", &rot.yaw, -180, 180);

					// Pitch
					if (ImGui::Button("-##pitchctl")) {
						rot.pitch -= ofDegToRad(1.0f);
						changed = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("+##pitchctl")) {
						rot.pitch += ofDegToRad(1.0f);
						changed = true;
					}
					ImGui::SameLine();
					changed |= ImGui::SliderAngle("Pitch##ctl", &rot.pitch, -180, 180);

					// Roll
					if (ImGui::Button("-##rollctl")) {
						rot.roll -= ofDegToRad(1.0f);
						changed = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("+##rollctl")) {
						rot.roll += ofDegToRad(1.0f);
						changed = true;
					}
					ImGui::SameLine();
					changed |= ImGui::SliderAngle("Roll##ctl", &rot.roll, -180, 180);

					// Callback if changed
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
			ImGui::PopStyleColor(3);
		}

		ImGui::EndChild();

		ImVec2 winSize = ImGui::GetWindowSize();
		ImVec2 cursorPos = ImGui::GetCursorPos();
		float buttonHeight = ImGui::GetFrameHeight(); // altura del botón
		ImGui::SetCursorPosY(winSize.y - buttonHeight - 10);
		if (ImGui::Button("ABOUT")) showAboutPopUp = true;

		ImGui::End();

		// --- WAV Popup ---
		const char * ordering_items[] = { "ACN", "FuMa" };
		static int ordering_selected_idx = 0;

		if (showWAVPopup) ImGui::OpenPopup("Load Ambisonic Soundfield");
		if (ImGui::BeginPopupModal("Load Ambisonic Soundfield", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			static int numFrames = 48000;
			static string ordering = "";

			ImGui::Text("Parameters of the soundfield:");
			ImGui::Text("Ordering:");
			if (ImGui::BeginCombo("##Ordering", ordering_items[ordering_selected_idx])) {
				for (int n = 0; n < IM_ARRAYSIZE(ordering_items); ++n) {
					bool selected = (ordering_selected_idx == n);
					if (ImGui::Selectable(ordering_items[n], selected)) {
						ordering_selected_idx = n; // actualizar índice
					}
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			// Determinar la cadena según el índice
			switch (ordering_selected_idx) {
			case 0:
				ordering = "ACN";
				break;
			case 1:
				ordering = "FuMa";
				break;
			}

			// Llamar onWAV solo al presionar "Create"
			if (ImGui::Button("Load") && onWAV) {
				onWAV(ordering);
				showWAVPopup = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			float buttonWidth = 100.0f;
			ImGui::SetCursorPosX(
				ImGui::GetWindowWidth() - buttonWidth / 2 - ImGui::GetStyle().WindowPadding.x);

			if (ImGui::Button("Cancel")) {
				showWAVPopup = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		// --- Source Popup ---
		if (showSourcePopup) ImGui::OpenPopup("Load Puntual Source");
		if (ImGui::BeginPopupModal("Load Puntual Source", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			static int numFrames = 48000;
			static int Azimuth = 0;
			static int Elevation = 0;

			ImGui::Text("Parameters of the source:");
			ImGui::InputInt("Azimuth", &Azimuth);
			ImGui::InputInt("Elevation", &Elevation);
			ImGui::Separator();

			if (ImGui::Button("Load") && onCreateSource) {
				onCreateSource(numFrames, SphericalToCartesian(Azimuth, Elevation));
				showSourcePopup = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			float buttonWidth = 100.0f;
			ImGui::SetCursorPosX(
				ImGui::GetWindowWidth() - buttonWidth / 2 - ImGui::GetStyle().WindowPadding.x);

			if (ImGui::Button("Cancel")) {
				showSourcePopup = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		//Settings Popup
		if (showSettingsPopUp) ImGui::OpenPopup("Settings");
		if (ImGui::BeginPopupModal("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

			const char * bufferSize_items[] = { "64", "128", "256", "512", "1024" };
			static int bufferSize_selected_idx = 2;

			const char * samplerate_items[] = { "44100", "48000", "96000"};
			static int samplerate_selected_idx = 1;

			//To implement settings options here
			ImGui::Text("Sample Rate:");
			if (ImGui::BeginCombo("##SampleRate", samplerate_items[samplerate_selected_idx])) {
				for (int n = 0; n < IM_ARRAYSIZE(samplerate_items); ++n) {
					bool selected = (samplerate_selected_idx == n);
					if (ImGui::Selectable(samplerate_items[n], selected)) {
						samplerate_selected_idx = n; // actualizar índice
					}
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Text("Buffer Size:");
			if (ImGui::BeginCombo("##BufferSize", bufferSize_items[bufferSize_selected_idx])) {
				for (int n = 0; n < IM_ARRAYSIZE(bufferSize_items); ++n) {
					bool selected = (bufferSize_selected_idx == n);
					if (ImGui::Selectable(bufferSize_items[n], selected)) {
						bufferSize_selected_idx = n; // actualizar índice
					}
					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Separator();

			if (ImGui::Button("Update SR")) {
				onChangeSampleRate(samplerate_selected_idx);
				showSettingsPopUp = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			if (ImGui::Button("Update BS")) {
				onChangeBufferSize(bufferSize_selected_idx);
				showSettingsPopUp = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			float buttonWidth = 100.0f;
			ImGui::SetCursorPosX(
				ImGui::GetWindowWidth() - buttonWidth / 2 - ImGui::GetStyle().WindowPadding.x);

			if (ImGui::Button("Close")) {
				showSettingsPopUp = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (showAboutPopUp) ImGui::OpenPopup("About");
		if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_NoResize)) {

			ImGui::Text("This player is the result of the Bachelors Thesis project by");
			ImGui::Text("Rocio Liceras Ramirez, supervised by Arcadio Reyes Lecuona");
			ImGui::Text("and co-supervised by Daniel Gonzalez Toledo at the University of Malaga.");
			

			float buttonWidth = 100.0f;
			ImGui::SetCursorPosX(
				ImGui::GetWindowWidth() - buttonWidth/2 - ImGui::GetStyle().WindowPadding.x);

			if (ImGui::Button("Close")) {
				showAboutPopUp = false;
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


	void setLSRotationSliders(float yaw, float pitch, float roll) {
		LSyawRad = yaw;
		LSpitchRad = pitch;
		LSrollRad = roll;
	}

	void setLayout(int layoutIndex) {
		layout_selected_idx = std::clamp(layoutIndex, 0, 3); 
	}

	void setTotalRotation(float yaw, float pitch, float roll) {
		TotalyawRad = yaw;
		TotalpitchRad = pitch;
		TotalrollRad = roll;
	}

	void setAudioDevices(const std::vector<ofSoundDevice> & devices) {
		audioDevices = devices;
	}

	// --- Callbacks ---
	std::function<void(bool)> onPlay;
	std::function<void()> onStop;
	std::function<void(string)> onWAV;
	std::function<void(size_t, Common::CVector3)> onCreateSource;
	std::function<void(int)> onChangeK;
	std::function<void(int)> onChangeBufferSize;
	std::function<void(int)> onChangeSampleRate;
	std::function<void()> onSettings;
	std::function<void()> onAbout;
	std::function<void(float, float, float)> onSFRotationChanged;
	std::function<void(float, float, float)> onLSRotationChanged;
	std::function<void(int)> onLayoutChanged;
	std::function<void(int)> onOrderChanged;
	std::function<void(int)> onVisualizationChanged;
	std::function<void(int)> onSerialPortChanged;
	std::function<void()> onCalibrateHead;
	std::function<void()> onCalibrateCompass;
	std::function<void(int)> onAudioDeviceChanged;

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
	float TotalyawRad, TotalpitchRad, TotalrollRad;
	int layoutIndex;
	int margen = 10;

	int layout_selected_idx = 0;
	int serial_selected_idx = 0;
	int audio_selected_idx = 0;

	bool isPlaying = false;

	std::vector<COMInfo> availableCOMs;

	std::vector<ofSoundDevice> audioDevices;

	std::vector<std::shared_ptr<CAmbisonicSoundFieldController>> controllers;
	bool showSourcePopup = false, showWAVPopup = false, showSettingsPopUp = false, showAboutPopUp = false;

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

	void initSerialPorts() {
		availableCOMs = TrackerController::listarCOMsConNombre();

		/*if (availableCOMs.empty()) {
			availableCOMs.push_back("No COM found");
		}*/

		serial_selected_idx = 0; // seleccionar el primero por defecto
	}


};

#endif // _MENU_PANEL_HPP_
