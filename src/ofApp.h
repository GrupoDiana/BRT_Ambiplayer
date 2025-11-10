#pragma once


#define SAMPLERATE 48000
#define LISTENER_ID "listener1"

#define SOURCE_FILEPATH "../resources/ambi_2OA_test.wav"

#include "ofMain.h"
#include "GUI/GuiManager.hpp"
#include "ConfigurationA.hpp"

#include "AmbisonicSoundFieldController.hpp"
#include "AmbisonicSoundField.hpp"
#include "AmbisonicProcessor.hpp"
#include "LoudSpeakerArrayController.hpp"

ofSoundStream soundStream;
Common::CGlobalParameters globalParameters; 
BRTBase::CBRTManager brtManager; 
std::shared_ptr<BRTBase::CListener> listener; 
size_t currentSample = 0; 
std::string decoding;
Common::CQuaternion q;

std::vector<float> directionalEnergies;


CConfigurationA configurationA;

Common::CEarPair<CMonoBuffer<float>> outputBufferStereo; 
unsigned int wavSamplePositionSource; 
unsigned int positionEndFrameSource;

float yawRad = 0.0f, pitchRad = 0.0f, rollRad = 0.0f;

int numAz = 72; // resolución angular
int numEl = 36;


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
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
		ofSoundDevice ShowSelectAudioDeviceMenu();
		void DrawDirectionalLobeProjected(const std::vector<float> & energies, int numAz, int numEl, float cx, float cy, float maxRadius);

	private:
		CGuiManager guiManager;
		std::shared_ptr<CAmbisonicSoundFieldController> ambisonicSoundFieldController;
		std::shared_ptr<CAmbisonicSoundField> SFbuffer, OutBuffer;
		CLoudSpeakerArrayController speakerArrayController;
		CAmbisonicProcessor ambisonicProcessor;

		void setRealTimePriority() {
			HANDLE hThread = GetCurrentThread();
			SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
		}
		
};
