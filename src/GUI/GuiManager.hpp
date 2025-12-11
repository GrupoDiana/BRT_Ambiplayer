#ifndef _GUI_MANAGER_HPP_
#define _GUI_MANAGER_HPP_

#include "Logo.hpp"
#include "MenuPanel.hpp"
#include "Projection.hpp"
#include "Loudspeakers.hpp"
#include "imgui.h"
#include "ofMain.h"
#include "ofxImGui.h"

class CGuiManager {
public:
	CGuiManager() {
		gui.setup(ofGetCurrentWindow(), nullptr, true, 0, true);

		menuPanel.onPlay = [this]() { if (onPlay) onPlay(); };
		menuPanel.onStop = [this]() { if (onStop) onStop(); };
		menuPanel.onWAV = [this]() { if (onWAV) onWAV(); };
		menuPanel.onCreateSource = [this](int frames, Common::CVector3 pos) {
			if (onCreateSource) onCreateSource(frames, pos);
		};
		menuPanel.onSFRotationChanged = [this](float yaw, float pitch, float roll) {
			if (onSFRotationChanged) onSFRotationChanged(yaw, pitch, roll);
		};
		menuPanel.onLSRotationChanged = [this](float yaw, float pitch, float roll) {
			if (onLSRotationChanged) onLSRotationChanged(yaw, pitch, roll);
		};
		menuPanel.onLayoutChanged = [this](int layoutIndex) {
			if (onLayoutChanged) onLayoutChanged(layoutIndex);
		};
		menuPanel.onControllerPlay = [this](int idx) {
			if (onControllerPlay) onControllerPlay(idx);
		};
		menuPanel.onControllerStop = [this](int idx) {
			if (onControllerStop) onControllerStop(idx);
		};
		menuPanel.onControllerMute = [this](int idx) {
			if (onControllerMute) onControllerMute(idx);
		};

	}

	void setSpeakerController(const CLoudSpeakerArrayController * controller) {
		loudspeakers.setController(controller);
	}

	void draw(const std::vector<float> & energies, const std::vector<float> & speakersEnergies) {
		gui.begin();

		ImGuiViewport * viewport = ImGui::GetMainViewport();
		float screenWidth = viewport->Size.x;
		float screenHeight = viewport->Size.y;

		logo.draw(0, 0, screenWidth, screenHeight/5);

		if (screenWidth > 1024) {
			menuPanel.draw(0, screenHeight / 5, width_menu, screenHeight - screenHeight / 5);

			lobeYZ.draw(energies, width_menu, screenHeight / 5, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, YZ);

			lobeZX.draw(energies, width_menu + (screenWidth - width_menu) / 2, screenHeight / 5, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, XZ);

			lobeXY.draw(energies, width_menu, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, XY);

			loudspeakers.draw(width_menu + (screenWidth - width_menu) / 2, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, speakersEnergies);
		} else {

			menuPanel.draw(0, screenHeight / 5, screenWidth / 4, screenHeight - screenHeight / 5);

			lobeYZ.draw(energies, screenWidth / 4, screenHeight / 5, screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, YZ);

			lobeZX.draw(energies, screenWidth / 4 + screenWidth * 1.5 / 4, screenHeight / 5, screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, XZ);

			lobeXY.draw(energies, screenWidth / 4, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, XY);

			loudspeakers.draw(screenWidth / 4 + screenWidth * 1.5 / 4, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, speakersEnergies);
		}

		

		gui.end();
	}

	void addController(std::shared_ptr<CAmbisonicSoundFieldController> c) {
		menuPanel.addController(c);
	}

	// --- Callbacks ---
	std::function<void()> onPlay;
	std::function<void()> onStop;
	std::function<void()> onWAV;
	std::function<void(int numFrames, Common::CVector3 pos)> onCreateSource;
	std::function<void(float, float, float)> onSFRotationChanged;
	std::function<void(float, float, float)> onLSRotationChanged;
	std::function<void(int)> onLayoutChanged;

	std::function<void(std::shared_ptr<CAmbisonicSoundFieldController>)> onAddControllerButton;
	std::function<void(int)> onControllerPlay;
	std::function<void(int)> onControllerStop;
	std::function<void(int)> onControllerMute;

private:
	ofxImGui::Gui gui;
	CLogo logo;

	CMenuPanel menuPanel;
	CProjection lobeXY, lobeYZ, lobeZX;
	CLoudspeakers loudspeakers;

	int width_menu = 256;

};

#endif
