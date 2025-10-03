#pragma once


#define SAMPLERATE 44100
#define LISTENER_ID "listener1"

#define SOURCE_FILEPATH "../resources/ambi_1OA_test.wav"

#include "ofMain.h"
#include "GUI/GuiManager.hpp"

#include "AmbisonicSoundFieldController.hpp"
#include "AmbisonicSoundField.hpp"
#include "AmbisonicProcessor.hpp"
#include "LoudSpeakerArrayController.hpp"

ofSoundStream soundStream;
Common::CGlobalParameters globalParameters; // Class where the global BRT parameters are defined.
BRTBase::CBRTManager brtManager; // BRT global manager interface
std::shared_ptr<BRTBase::CListener> listener; // Pointer to listener mode

Common::CEarPair<CMonoBuffer<float>> outputBufferStereo; // Stereo buffer containing processed audio

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

	private:
		CGuiManager guiManager;
		CLoudSpeakerArrayController speakerArrayController;
		CAmbisonicSoundFieldController ambisonicSoundFieldController;
		CAmbisonicProcessor ambisonicProcessor;

		void setRealTimePriority() {
			HANDLE hThread = GetCurrentThread();
			SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
		}
		
};




/*
1)Cargamos el WAV
CAmbisonicSoundField:: loadWav()

3)Antes de crear las fuentes, cargamos la configuración de los altavoces
Seria algo similar a CalculateSoundSourceInitialLocation() en berta pero con un vector
y sin usar el listener, porque ahi calcula la pos del vector con la posicion relativa al listener
CLoudSpeakerArrayController:: LoadLayoutJSON()

4)Creamos las fuentes
Aqui entonces deberiamos crear ya las fuentes (tipo AddSoundSource() en berta)
Esto se supone que crea una SoundSourceList que en nuestro caso sera nuestro vector de fuentes ???
CAmbisonicProcessor:: DecodeToSpeakerArray()

5)Cargamos las fuentes
Similar a berta
OfApp:: LoadSoundSourceList()

6)Conectamos al oyente
Similar a berta
OfApp:: ConnectToListener()


*/
