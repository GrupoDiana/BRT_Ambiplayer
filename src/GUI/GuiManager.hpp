#ifndef _GUI_MANAGER_HPP_
#define _GUI_MANAGER_HPP_

#include "Logo.hpp"
#include "MenuPanel.hpp"
#include "Projection.hpp"
#include "imgui.h"
#include "ofMain.h"
#include "ofxImGui.h"

class CGuiManager {
public:
	CGuiManager() {
		gui.setup(ofGetCurrentWindow(), nullptr, true, 0, true);

		menuPanel.onPlay = [this]() { if (onPlay) onPlay(); };
		menuPanel.onStop = [this]() { if (onStop) onStop(); };
		menuPanel.onRotationChanged = [this](float yaw, float pitch, float roll) {
			if (onRotationChanged) onRotationChanged(yaw, pitch, roll);
		};
	}

	void draw(const std::vector<float> & energies) {
		gui.begin();

		ImGuiViewport * viewport = ImGui::GetMainViewport();
		float screenWidth = viewport->Size.x;
		float screenHeight = viewport->Size.y;

		logo.draw(0, 0, screenWidth, screenHeight/5);

		if (screenWidth > 1024) {
			menuPanel.draw(0, screenHeight / 5, width_menu, screenHeight - screenHeight / 5);

			lobeZX.draw(energies, width_menu, screenHeight / 5,
				(screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, XZ);

			lobeYZ.draw(energies, width_menu + (screenWidth - width_menu) / 2, screenHeight / 5,
				(screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, YZ);

			lobeXY.draw(energies, width_menu, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2,
				(screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, XY);
		} else {

			menuPanel.draw(0, screenHeight / 5, screenWidth / 4, screenHeight - screenHeight / 5);

			lobeZX.draw(energies, screenWidth / 4, screenHeight / 5,
				screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, XZ);

			lobeYZ.draw(energies, screenWidth / 4 + screenWidth * 1.5 / 4, screenHeight / 5,
				screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, YZ);

			lobeXY.draw(energies, screenWidth / 4, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2,
				screenWidth * 1.5 / 4, (screenHeight - screenHeight / 5) / 2, XY);
		}

		

		gui.end();
	}

	// --- Callbacks ---
	std::function<void()> onPlay;
	std::function<void()> onStop;
	std::function<void(float, float, float)> onRotationChanged;

private:
	ofxImGui::Gui gui;
	CLogo logo;

	CMenuPanel menuPanel;
	CProjection lobeXY, lobeYZ, lobeZX;

	int width_menu = 256;

};

#endif
