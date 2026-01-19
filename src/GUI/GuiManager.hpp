#ifndef _GUI_MANAGER_HPP_
#define _GUI_MANAGER_HPP_

#include "Logo.hpp"
#include "Loudspeakers.hpp"
#include "MenuPanel.hpp"
#include "Projection.hpp"
#include "imgui.h"
#include "ofMain.h"
#include "ofxImGui.h"

class CGuiManager {
public:
	CGuiManager() {
		gui.setup(ofGetCurrentWindow(), nullptr, true, 0, true);

		// ---- Menu panel callbacks ----
		menuPanel.onPlay = [this]() { if (onPlay) onPlay(); };
		menuPanel.onStop = [this]() { if (onStop) onStop(); };
		menuPanel.onWAV = [this](string ordering) { if (onWAV) onWAV(ordering); };
		menuPanel.onCreateSource = [this](int frames, Common::CVector3 pos) { if (onCreateSource) onCreateSource(frames, pos); };
		menuPanel.onChangeK = [this](int k) { if (onChangeK) onChangeK(k); };
		menuPanel.onSettings = [this]() { if (onSettings) onSettings(); };
		menuPanel.onSFRotationChanged = [this](float yaw, float pitch, float roll) { if (onSFRotationChanged) onSFRotationChanged(yaw, pitch, roll); };
		menuPanel.onLSRotationChanged = [this](float yaw, float pitch, float roll) { if (onLSRotationChanged) onLSRotationChanged(yaw, pitch, roll); };
		menuPanel.onIRotationChanged = [this](CMenuPanel::ControllerPtr c, float yaw, float pitch, float roll) { if (onIRotationChanged) onIRotationChanged(c, yaw, pitch, roll); };
		menuPanel.onLayoutChanged = [this](int layoutIndex) { if (onLayoutChanged) onLayoutChanged(layoutIndex); };
		menuPanel.onOrderChanged = [this](int orderIndex) { if (onOrderChanged) onOrderChanged(orderIndex); };

		// ---- Controller callbacks ----
		menuPanel.onControllerPlay = [this](CMenuPanel::ControllerPtr c) { if (onControllerPlay) onControllerPlay(c); };
		menuPanel.onControllerStop = [this](CMenuPanel::ControllerPtr c) { if (onControllerStop) onControllerStop(c); };
		menuPanel.onControllerMute = [this](CMenuPanel::ControllerPtr c) { if (onControllerMute) onControllerMute(c); };
		menuPanel.onControllerSolo = [this](CMenuPanel::ControllerPtr c) { if (onControllerSolo) onControllerSolo(c); };
		menuPanel.onControllerLoop = [this](CMenuPanel::ControllerPtr c) { if (onControllerLoop) onControllerLoop(c); };
		menuPanel.onControllerRemove = [this](CMenuPanel::ControllerPtr c) { if (onControllerRemove) onControllerRemove(c); };

		// ---- Register OF mouse event listeners ----
		ofAddListener(ofEvents().mousePressed, this, &CGuiManager::mousePressed);
		ofAddListener(ofEvents().mouseDragged, this, &CGuiManager::mouseDragged);
		ofAddListener(ofEvents().mouseReleased, this, &CGuiManager::mouseReleased);
	}

	void setSpeakerController(const CLoudSpeakerArrayController * controller) {
		loudspeakers.setController(controller);
	}

	// ---- Draw GUI and speaker windows ----
	void draw(const std::vector<float> & energies, const std::vector<float> & speakersEnergies, const std::vector<Common::CVector3> & vertices) {
		gui.begin();

		ImGuiViewport * viewport = ImGui::GetMainViewport();
		float screenWidth = viewport->Size.x;
		float screenHeight = viewport->Size.y;

		logo.draw(0, 0, screenWidth, screenHeight / 5);

		if (screenWidth > 1023) {
			menuPanel.draw(0, screenHeight / 5, width_menu, screenHeight - screenHeight / 5);

			lobeYZ.draw(energies, width_menu, screenHeight / 5, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, ZY, vertices);
			//lobeZX.draw(energies, width_menu + (screenWidth - width_menu) / 2, screenHeight / 5, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, ZX);
			//lobeXY.draw(energies, width_menu, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, XY);

			loudspeakers.draw(width_menu + (screenWidth - width_menu) / 2,
				screenHeight / 5 + (screenHeight - screenHeight / 5) / 2,
				(screenWidth - width_menu) / 2,
				(screenHeight - screenHeight / 5) / 2,
				speakersEnergies);
		} else {
			menuPanel.draw(0, screenHeight / 5, screenWidth / 4, screenHeight - screenHeight / 5);

			//lobeYZ.draw(energies, screenWidth / 4, screenHeight / 5, screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, ZY);
			//lobeZX.draw(energies, screenWidth / 4 + screenWidth * 1.5 / 4, screenHeight / 5, screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, ZX);
			//lobeXY.draw(energies, screenWidth / 4, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, XY);

			loudspeakers.draw(screenWidth / 4 + screenWidth * 1.5 / 4,
				screenHeight / 5 + (screenHeight - screenHeight / 5) / 2,
				screenWidth * 1.5 / 4,
				(screenHeight - screenHeight / 5) / 2,
				speakersEnergies);
		}

		gui.end();

		// ---- Send rotation to main via callback ----
		if (onLSRotationChanged) {
			ofVec3f rot = loudspeakers.getRotation();
			onLSRotationChanged(rot.x, rot.y, rot.z);
		}
	}

	void addController(std::shared_ptr<CAmbisonicSoundFieldController> c, string filePathName) {
		menuPanel.addController(c, filePathName);
	}

	void removeController(std::shared_ptr<CAmbisonicSoundFieldController> c) {
		menuPanel.removeController(c);
	}

	void clearControllers() {
		menuPanel.clearControllers();
	}

	// ---- Mouse event handlers to forward to loudspeakers ----
	void mousePressed(ofMouseEventArgs & args) {
		// Compute the speaker window bounds
		float cx, cy, w, h;
		getSpeakerWindowBounds(cx, cy, w, h);
		loudspeakers.mousePressed(args.x, args.y, cx, cy, w, h);
	}

	void mouseDragged(ofMouseEventArgs & args) {
		loudspeakers.mouseDragged(args.x, args.y);
	}

	void mouseReleased(ofMouseEventArgs & args) {
		loudspeakers.mouseReleased();
	}

private:
	ofxImGui::Gui gui;
	CLogo logo;
	CMenuPanel menuPanel;
	CProjection lobeXY, lobeYZ, lobeZX;
	CLoudspeakers loudspeakers;

	int width_menu = 320;

	// ---- Helper function to compute loudspeaker window bounds ----
	void getSpeakerWindowBounds(float & cx, float & cy, float & w, float & h) {
		ImGuiViewport * viewport = ImGui::GetMainViewport();
		float screenWidth = viewport->Size.x;
		float screenHeight = viewport->Size.y;

		if (screenWidth > 1023) {
			cx = width_menu + (screenWidth - width_menu) / 2;
			cy = screenHeight / 5 + (screenHeight - screenHeight / 5) / 2;
			w = (screenWidth - width_menu) / 2;
			h = (screenHeight - screenHeight / 5) / 2;
		} else {
			cx = screenWidth / 4 + screenWidth * 1.5 / 4;
			cy = screenHeight / 5 + (screenHeight - screenHeight / 5) / 2;
			w = screenWidth * 1.5 / 4;
			h = (screenHeight - screenHeight / 5) / 2;
		}
	}

public:
	// ---- Callbacks to main application ----
	std::function<void()> onPlay;
	std::function<void()> onStop;
	std::function<void(string ordering)> onWAV;
	std::function<void(int numFrames, Common::CVector3 pos)> onCreateSource;
	std::function<void(int k)> onChangeK;
	std::function<void()> onSettings;
	std::function<void(float, float, float)> onSFRotationChanged;
	std::function<void(float, float, float)> onLSRotationChanged;
	std::function<void(int)> onLayoutChanged;
	std::function<void(int)> onOrderChanged;

	using ControllerPtr = std::shared_ptr<CAmbisonicSoundFieldController>;
	std::function<void(std::shared_ptr<CAmbisonicSoundFieldController>)> onAddControllerButton;
	std::function<void(ControllerPtr)> onControllerPlay;
	std::function<void(ControllerPtr)> onControllerStop;
	std::function<void(ControllerPtr)> onControllerMute;
	std::function<void(ControllerPtr)> onControllerRemove;
	std::function<void(ControllerPtr)> onControllerSolo;
	std::function<void(ControllerPtr)> onControllerLoop;
	std::function<void(ControllerPtr, float, float, float)> onIRotationChanged;
};

#endif
