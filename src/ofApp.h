#pragma once

#define LISTENER_ID "listener1"

#include "ofMain.h"
#include "GUI/GuiManager.hpp"
#include "ConfigurationA.hpp"

#include "AmbisonicSoundFieldController.hpp"
#include "AmbisonicSoundField.hpp"
#include "AmbisonicProcessor.hpp"
#include "LoudSpeakerArrayController.hpp"
#include "TrackerController.hpp"

ofSoundStream soundStream;
Common::CGlobalParameters globalParameters; 
std::shared_ptr < BRTBase::CBRTManager> brtManager;
std::shared_ptr<BRTBase::CListener> listener;
int id = 0;
std::string ListenerID = std::string(LISTENER_ID) + std::to_string(id);
size_t currentSample = 0;
std::string decoding;
Common::CQuaternion qRotation, qLoudSpeakers, qField, qHead;

std::vector<float> directionalEnergies;
std::vector < std::vector<float> > planarEnergies;
std::vector<float> speakerEnergies;
int visualization;
std::vector<Common::CVector3> interpolatedVertices;
std::vector<float> interpolatedEnergies;

int selectedAudioDeviceIndex = -1;

float BUFFERSIZE = 256.0f;
float SAMPLERATE = 48000.0f;

ofSoundDevice audioDevices;

CConfigurationA configurationA;
int cont = 0;

TrackerController tracker;

Common::CEarPair<CMonoBuffer<float>> outputBufferStereo; 

float SFyawRad = 0.0f, SFpitchRad = 0.0f, SFrollRad = 0.0f, LSyawRad = 0.0f, LSpitchRad = 0.0f, LSrollRad = 0.0f;

int numAz = 72;
int numEl = 36;

std::vector<Common::CVector3> vertices;
std::vector<Edge> edges;

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void AudioSetup();
	void audioOut(ofSoundBuffer & buffer);
	void audioProcess(Common::CEarPair<CMonoBuffer<float>> & bufferOutput, int uiBufferSize);
	void changeAudioDevice(int index);
	void setupBRT();
	void rebuildAudioSystem();
	void updateGlobalPlayState();

private:
	CGuiManager guiManager;
	std::shared_ptr<CAmbisonicSoundFieldController> ambisonicSoundFieldController;
	vector<std::shared_ptr<CAmbisonicSoundFieldController>> ambisonicSoundFieldControllers;
	std::shared_ptr<CAmbisonicSoundField> SFbuffer, OutBuffer, audioBufferCopy;
	CLoudSpeakerArrayController speakerArrayController;
	CAmbisonicProcessor ambisonicProcessor;

	int order=1;

	std::mutex energiesMutex;
	std::vector<Common::CVector3> originalPositions;

	void setRealTimePriority() {
		HANDLE hThread = GetCurrentThread();
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	}



};

