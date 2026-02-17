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

class CGuiManager {
public:
    CGuiManager() {
        gui.setup(ofGetCurrentWindow(), nullptr, true, 0, true);

        // ---- Menu panel callbacks ----
        menuPanel.onPlay = [this](bool isPlaying) { if (onPlay) onPlay(isPlaying); };
        menuPanel.onStop = [this]() { if (onStop) onStop(); };
        menuPanel.onWAV = [this](string ordering) { if (onWAV) onWAV(ordering); };
        menuPanel.onCreateSource = [this](int frames, Common::CVector3 pos) { if (onCreateSource) onCreateSource(frames, pos); };
        menuPanel.onChangeK = [this](int k) { if (onChangeK) onChangeK(k); };
        menuPanel.onSettings = [this]() { if (onSettings) onSettings(); };
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

        // ---- Controller callbacks ----
        menuPanel.onControllerPlay = [this](CMenuPanel::ControllerPtr c) { if (onControllerPlay) onControllerPlay(c); };
        menuPanel.onControllerStop = [this](CMenuPanel::ControllerPtr c) { if (onControllerStop) onControllerStop(c); };
        menuPanel.onControllerMute = [this](CMenuPanel::ControllerPtr c) { if (onControllerMute) onControllerMute(c); };
        menuPanel.onControllerSolo = [this](CMenuPanel::ControllerPtr c) { if (onControllerSolo) onControllerSolo(c); };
        menuPanel.onControllerLoop = [this](CMenuPanel::ControllerPtr c) { if (onControllerLoop) onControllerLoop(c); };
        menuPanel.onControllerRemove = [this](CMenuPanel::ControllerPtr c) { if (onControllerRemove) onControllerRemove(c); };

        // ---- OF mouse listeners ----
        ofAddListener(ofEvents().mousePressed, this, &CGuiManager::mousePressed);
        ofAddListener(ofEvents().mouseDragged, this, &CGuiManager::mouseDragged);
        ofAddListener(ofEvents().mouseReleased, this, &CGuiManager::mouseReleased);
    }

    void setSpeakerController(const CLoudSpeakerArrayController * controller) {
        loudspeakers.setController(controller);
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
            menuPanel.draw(0, screenHeight / 5, width_menu, screenHeight - screenHeight / 5);

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

        } else {
            menuPanel.draw(0, screenHeight / 5, screenWidth / 4, screenHeight - screenHeight / 5);
            loudspeakers.draw(screenWidth / 4 + screenWidth * 1.5 / 4,
                             screenHeight / 5 + (screenHeight - screenHeight / 5) / 2,
                             screenWidth * 1.5 / 4,
                             (screenHeight - screenHeight / 5) / 2,
                             speakersEnergies);
        }

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

private:
    ofxImGui::Gui gui;
    CLogo logo;
    CMenuPanel menuPanel;

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
    std::function<void(string ordering)> onWAV;
    std::function<void(int numFrames, Common::CVector3 pos)> onCreateSource;
    std::function<void(int k)> onChangeK;
    std::function<void()> onSettings;
    std::function<void(float, float, float)> onSFRotationChanged;
    std::function<void(float, float, float)> onLSRotationChanged;
    std::function<void(float yaw, float pitch, float roll)> onRotationChangedByMouse;
    std::function<void(int)> onLayoutChanged;
    std::function<void(int)> onOrderChanged;
    std::function<void(int)> onVisualizationChanged;
	std::function<void(int)> onSerialPortChanged;
	std::function<void()> onCalibrateHead;
	std::function<void()> onCalibrateCompass;

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
