#pragma once


#define SAMPLERATE 48000
#define LISTENER_ID "listener1"

#define SOURCE_FILEPATH "../resources/ambi_1OA_test.wav"

#include "ofMain.h"
#include "GUI/GuiManager.hpp"
#include "ConfigurationA.hpp"

#include "AmbisonicSoundFieldController.hpp"
#include "AmbisonicSoundField.hpp"
#include "AmbisonicProcessor.hpp"
#include "LoudSpeakerArrayController.hpp"

ofSoundStream soundStream;
Common::CGlobalParameters globalParameters; // Class where the global BRT parameters are defined.
BRTBase::CBRTManager brtManager; // BRT global manager interface
std::shared_ptr<BRTBase::CListener> listener; // Pointer to listener mode
size_t currentSample = 0; // posición de reproducción global
std::string decoding;

CConfigurationA configurationA;

Common::CEarPair<CMonoBuffer<float>> outputBufferStereo; // Stereo buffer containing processed audio
unsigned int wavSamplePositionSource; // Storages, respectively, the starting and ending position of the frame being rendered for each source
unsigned int positionEndFrameSource;


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
		void FillBuffer(CMonoBuffer<float> & output, unsigned int & position, unsigned int & endFrame, std::vector<float> & samplesVector);

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
