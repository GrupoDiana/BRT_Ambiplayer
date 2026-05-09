/**
 * \class CGuiManager
 *
 * \brief Central manager for GUI rendering and interaction in the application.
 *
 * \details This class integrates and coordinates all graphical user interface
 * components, including menus, visualization panels, and interactive 3D/2D
 * spatial audio displays. It acts as the main bridge between user input,
 * rendering, and application-level callbacks.
 *
 * \note Built on top of OpenFrameworks, ofxImGui, and GLFW.
 *
 * \date April 2026
 *
 * \author R. Liceras-Ramírez under the supervision of A. Reyes-Lecuona and D. Gonzalez-Toledo (3DI-DIANA Research Group)
 * \b Institution: Universidad de Málaga
 *
 * \b Context:
 * Developed as part of a Bachelor's Thesis (TFG), this class is responsible for
 * the user interface layer of a real-time spatial audio and Ambisonic system.
 *
  * \b License:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * \b Copyright:
 * This library includes pieces of code from the BRT Library, shared under the GPLv3 license and copyrighted (c) by the University of Málaga
 *
 * \b Acknowledgement:
 * This project has been partially funded by the Spanish Ministerio de Ciencia e Innovación under the SONIX project (PID2023-152547NB-I00). https://www.diana.uma.es/sonix
 */

#ifndef _GUI_MANAGER_HPP_
#define _GUI_MANAGER_HPP_

#include "Logo.hpp"
#include "Loudspeakers.hpp"
#include "MenuPanel.hpp"
#include "GeodesicProjection.hpp"
#include "CircularProjection.hpp"
#include "imgui.h"
#include "ofMain.h"
#include "ofxImGui.h"
#include <GLFW/glfw3.h>
#include "ofAppGLFWWindow.h"
#include <memory>

class CGuiManager {
public:
    CGuiManager() {

        // ---- Menu panel callbacks ----
        menuPanel.onPlay = [this](bool isPlaying) { if (onPlay) onPlay(isPlaying); };
        menuPanel.onStop = [this]() { if (onStop) onStop(); };
        menuPanel.onWAV = [this](string ordering) { if (onWAV) onWAV(ordering); };
		menuPanel.onHRTF = [this]() { if (onHRTF) onHRTF(); };
        menuPanel.onCreateSource = [this](int frames, Common::CVector3 pos) { if (onCreateSource) onCreateSource(frames, pos); };
        menuPanel.onChangeK = [this](int k) { if (onChangeK) onChangeK(k); };
		menuPanel.onChangeBufferSize = [this](int bufferSize) { if (onChangeBufferSize) onChangeBufferSize(bufferSize); };
		menuPanel.onChangeSampleRate = [this](int sampleRate) { if (onChangeSampleRate) onChangeSampleRate(sampleRate); };
        menuPanel.onSettings = [this]() { if (onSettings) onSettings(); };
		menuPanel.onAbout = [this]() { if (onAbout) onAbout(); };
        menuPanel.onSFRotationChanged = [this](float yaw, float pitch, float roll) { if (onSFRotationChanged) onSFRotationChanged(yaw, pitch, roll); };

        menuPanel.onLSRotationChanged = [this](float yaw, float pitch, float roll) {
            loudspeakers.setRotation(yaw, pitch, roll);
            if (onLSRotationChanged)
                onLSRotationChanged(yaw, pitch, roll);
        };

        loudspeakers.setOnRotationChangedByMouse([this](float yaw, float pitch, float roll) {
            menuPanel.setLSRotationSliders(yaw, pitch, roll);
            if (onLSRotationChanged)
                onLSRotationChanged(yaw, pitch, roll);
        });

        menuPanel.onIRotationChanged = [this](CMenuPanel::ControllerPtr c, float yaw, float pitch, float roll) {
            if (onIRotationChanged) onIRotationChanged(c, yaw, pitch, roll);
        };
        menuPanel.onLayoutChanged = [this](int layoutIndex) { if (onLayoutChanged) onLayoutChanged(layoutIndex); };
        menuPanel.onOrderChanged = [this](int orderIndex) { if (onOrderChanged) onOrderChanged(orderIndex); };
        menuPanel.onVisualizationChanged = [this](int visualizationIndex) { if (onVisualizationChanged) onVisualizationChanged(visualizationIndex); };
		menuPanel.onSerialPortChanged = [this](int serialIndex) { if (onSerialPortChanged) onSerialPortChanged(serialIndex); };
		menuPanel.onCalibrateHead = [this]() { if (onCalibrateHead) onCalibrateHead(); };
		menuPanel.onCalibrateCompass = [this]() { if (onCalibrateCompass) onCalibrateCompass(); };
		menuPanel.onAudioDeviceChanged = [this](int index) {
			if (onAudioDeviceChanged)
				onAudioDeviceChanged(index);
		};

        // ---- Controller callbacks ----
        menuPanel.onControllerPlay = [this](CMenuPanel::ControllerPtr c) { if (onControllerPlay) onControllerPlay(c); };
        menuPanel.onControllerStop = [this](CMenuPanel::ControllerPtr c) { if (onControllerStop) onControllerStop(c); };
        menuPanel.onControllerMute = [this](CMenuPanel::ControllerPtr c) { if (onControllerMute) onControllerMute(c); };
        menuPanel.onControllerSolo = [this](CMenuPanel::ControllerPtr c) { if (onControllerSolo) onControllerSolo(c); };
        menuPanel.onControllerLoop = [this](CMenuPanel::ControllerPtr c) { if (onControllerLoop) onControllerLoop(c); };
		menuPanel.onControllerVol = [this](CMenuPanel::ControllerPtr c) { if (onControllerVol) onControllerVol(c); };
		menuPanel.onControllerCONV = [this](CMenuPanel::ControllerPtr c) { if (onControllerCONV) onControllerCONV(c); };
        menuPanel.onControllerRemove = [this](CMenuPanel::ControllerPtr c) { if (onControllerRemove) onControllerRemove(c); };

        // ---- OF mouse listeners ----
        ofAddListener(ofEvents().mousePressed, this, &CGuiManager::mousePressed);
        ofAddListener(ofEvents().mouseDragged, this, &CGuiManager::mouseDragged);
        ofAddListener(ofEvents().mouseReleased, this, &CGuiManager::mouseReleased);

		 auto window = static_cast<ofAppGLFWWindow *>(ofGetCurrentWindow().get());

		if (window) {
			GLFWwindow * glfwWindow = window->getGLFWWindow();
			glfwSetWindowSizeLimits(glfwWindow, 500, 500, GLFW_DONT_CARE, GLFW_DONT_CARE);
		}
    }


    void setSpeakerController(const CLoudSpeakerArrayController * controller) {
        loudspeakers.setController(controller);
    }

	void setAudioDevices(const std::vector<ofSoundDevice> & devices) {
		menuPanel.setAudioDevices(devices);
	}

    // ---- Draw GUI and speaker windows ----
    void draw(int visualization,
              const std::vector<float> & energies,
              const std::vector<std::vector<float>> & planarEnergies,
              const std::vector<float> & speakersEnergies,
              const std::vector<Common::CVector3> & vertices) {
        
        gui.begin();

        ImGuiViewport * viewport = ImGui::GetMainViewport();
        float screenWidth = viewport->Size.x;
        float screenHeight = viewport->Size.y;

        logo.draw(0, 0, screenWidth, screenHeight / 5);

        if (screenWidth > 1023) {
            menuPanel.draw(0, screenHeight / 5, width_menu + 15, screenHeight - screenHeight / 5);

            if (visualization == 0) {
                // ---- Geodesic ----
                lobeYZ.draw(energies, width_menu, screenHeight / 5, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, ZY, vertices);
                lobeZX.draw(energies, width_menu + (screenWidth - width_menu) / 2, screenHeight / 5, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, ZX, vertices);
                lobeXY.draw(energies, width_menu, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, XY, vertices);
            } else {
                // ---- Planar / Circular ----
                circularYZ.draw(planarEnergies[0], width_menu, screenHeight / 5, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, ZY, vertices);
                circularZX.draw(planarEnergies[1], width_menu + (screenWidth - width_menu) / 2, screenHeight / 5, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, ZX, vertices);
                circularXY.draw(planarEnergies[2], width_menu, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, (screenWidth - width_menu) / 2, (screenHeight - screenHeight / 5) / 2, XY, vertices);
            }

            loudspeakers.draw(width_menu + (screenWidth - width_menu) / 2,
                             screenHeight / 5 + (screenHeight - screenHeight / 5) / 2,
                             (screenWidth - width_menu) / 2,
                             (screenHeight - screenHeight / 5) / 2,
                             speakersEnergies);

        } else if (screenWidth > 690) {
            menuPanel.draw(0, screenHeight / 5, screenWidth / 4, screenHeight - screenHeight / 5);
			if (visualization == 0) {
				// ---- Geodesic ----
				lobeYZ.draw(energies, screenWidth / 4, screenHeight / 5, (screenWidth - screenWidth / 4) / 2, (screenHeight - screenHeight / 5) / 2, ZY, vertices);
				lobeZX.draw(energies, screenWidth / 4 + (screenWidth - screenWidth / 4) / 2, screenHeight / 5, (screenWidth - screenWidth / 4) / 2, (screenHeight - screenHeight / 5) / 2, ZX, vertices);
				lobeXY.draw(energies, screenWidth / 4, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, (screenWidth - screenWidth / 4) / 2, (screenHeight - screenHeight / 5) / 2, XY, vertices);
			} else {
				// ---- Planar / Circular ----
				circularYZ.draw(planarEnergies[0], screenWidth / 4, screenHeight / 5, (screenWidth - screenWidth / 4) / 2, (screenHeight - screenHeight / 5) / 2, ZY, vertices);
				circularZX.draw(planarEnergies[1], screenWidth / 4 + (screenWidth - screenWidth / 4) / 2, screenHeight / 5, (screenWidth - screenWidth / 4) / 2, (screenHeight - screenHeight / 5) / 2, ZX, vertices);
				circularXY.draw(planarEnergies[2], screenWidth / 4, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, (screenWidth - screenWidth / 4) / 2, (screenHeight - screenHeight / 5) / 2, XY, vertices);
			}
            loudspeakers.draw(screenWidth / 4 + screenWidth * 1.5 / 4,
                             screenHeight / 5 + (screenHeight - screenHeight / 5) / 2,
                             screenWidth * 1.5 / 4,
                             (screenHeight - screenHeight / 5) / 2,
                             speakersEnergies);
		} else {
			menuPanel.draw(0, screenHeight / 5, screenWidth / 4, screenHeight - screenHeight / 5);
			if (visualization == 0) {
				// ---- Geodesic ----
				lobeYZ.draw(energies, 172, screenHeight / 5, (screenWidth - 172) , (screenHeight - screenHeight / 5) / 2, ZY, vertices);
				lobeXY.draw(energies, 172, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, (screenWidth - 172), (screenHeight - screenHeight / 5) / 2, XY, vertices);
				
			} else {
				// ---- Planar / Circular ----
				circularYZ.draw(planarEnergies[0], 172, screenHeight / 5, (screenWidth - 172), (screenHeight - screenHeight / 5) / 2, ZY, vertices);
				circularXY.draw(planarEnergies[2], 172, screenHeight / 5 + (screenHeight - screenHeight / 5) / 2, (screenWidth - 172), (screenHeight - screenHeight / 5) / 2, XY, vertices);
			}
			
		}

		setHeadRotation();

        gui.end();
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

    // ---- Mouse handlers forwarded to loudspeakers ----
    void mousePressed(ofMouseEventArgs & args) {
        float cx, cy, w, h;
        getSpeakerWindowBounds(cx, cy, w, h);
        loudspeakers.mousePressed(args.x, args.y, cx, cy, w, h);
    }
    void mouseDragged(ofMouseEventArgs & args) { loudspeakers.mouseDragged(args.x, args.y); }
    void mouseReleased(ofMouseEventArgs & args) { loudspeakers.mouseReleased(); }

    void setLayout(int layoutIndex) { menuPanel.setLayout(layoutIndex); }

	void getHeadRotation(CQuaternion _qRotation) {
		qRotation = _qRotation;
	}

	void setHeadRotation() {
		qRotation.ToYawPitchRoll(Totalyaw, Totalpitch, Totalroll);
		menuPanel.setHeadRotation(Totalyaw, Totalpitch, Totalroll);
	}

	void setGlobalPlayState(bool playing) {
		menuPanel.setGlobalPlay(playing);
	}

	void setup() {
		gui.setup(ofGetCurrentWindow(), nullptr, true, 0, true);
	}

private:
    ofxImGui::Gui gui;
    CLogo logo;
    CMenuPanel menuPanel;
	CQuaternion qRotation;
	float Totalyaw, Totalpitch, Totalroll;

    // Proyecciones
    CGeodesicProjection lobeXY, lobeYZ, lobeZX;
    CCircularProjection circularXY, circularYZ, circularZX;

    CLoudspeakers loudspeakers;
    int width_menu = 320;

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
    std::function<void(bool)> onPlay;
    std::function<void()> onStop;
	std::function<void()> onHRTF;
    std::function<void(string ordering)> onWAV;
    std::function<void(int numFrames, Common::CVector3 pos)> onCreateSource;
    std::function<void(int k)> onChangeK;
	std::function<void(int)> onChangeBufferSize;
	std::function<void(int)> onChangeSampleRate;
    std::function<void()> onSettings;
	std::function<void()> onAbout;
    std::function<void(float, float, float)> onSFRotationChanged;
    std::function<void(float, float, float)> onLSRotationChanged;
    std::function<void(float yaw, float pitch, float roll)> onRotationChangedByMouse;
    std::function<void(int)> onLayoutChanged;
    std::function<void(int)> onOrderChanged;
    std::function<void(int)> onVisualizationChanged;
	std::function<void(int)> onSerialPortChanged;
	std::function<void()> onCalibrateHead;
	std::function<void()> onCalibrateCompass;
	std::function<void(int)> onAudioDeviceChanged;

    using ControllerPtr = std::shared_ptr<CAmbisonicSoundFieldController>;
    std::function<void(std::shared_ptr<CAmbisonicSoundFieldController>)> onAddControllerButton;
    std::function<void(ControllerPtr)> onControllerPlay;
    std::function<void(ControllerPtr)> onControllerStop;
    std::function<void(ControllerPtr)> onControllerMute;
    std::function<void(ControllerPtr)> onControllerRemove;
    std::function<void(ControllerPtr)> onControllerSolo;
    std::function<void(ControllerPtr)> onControllerLoop;
	std::function<void(ControllerPtr)> onControllerVol;
	std::function<void(ControllerPtr)> onControllerCONV;
    std::function<void(ControllerPtr, float, float, float)> onIRotationChanged;
};

#endif
