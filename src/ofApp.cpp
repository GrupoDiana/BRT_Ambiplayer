#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	// Setup GUI callbacks
	guiManager.onPlay = [this]() {
		ofLog() << "Play button pressed!";
		//Play logic
	};

	/// AUDIO setup
	AudioSetup();

	/// BRT Global Parametert setup
	globalParameters.SetSampleRate(SAMPLERATE); // Setting sample rate
	globalParameters.SetBufferSize(512); // Setting buffer size

	// //////////////////////////
	// // Listener Setup
	// //////////////////////////
	brtManager.BeginSetup();
	listener = brtManager.CreateListener<BRTBase::CListener>(LISTENER_ID);
	brtManager.EndSetup();
	Common::CTransform listenerPosition = Common::CTransform(); // Setting listener in (0,0,0)
	listenerPosition.SetPosition(Common::CVector3(0, 0, 0));
	listener->SetListenerTransform(listenerPosition);

	/////////////////////
	// Create and Setup Sources
	/////////////////////
	speakerArrayController = CLoudSpeakerArrayController();

	// /////////////////////
	// Load Wav Files
	/////////////////////
	ambisonicSoundFieldController.LoadWav(SOURCE_FILEPATH, "SN3D");

	/////////////////////
	// Start AUDIO Render
	/////////////////////
	// Declaration and initialization of stereo buffer
	outputBufferStereo.left.resize(512);
	outputBufferStereo.right.resize(512);

	// Informing user by the console to press any key to end the execution
	std::cout << std::endl
			  << std::endl;
	std::cout << "Press any key to start and later press ENTER to finish..." << std::endl;
	std::cout << std::endl
			  << std::endl;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer
	std::cin.get(); // Wait for a key press

	// Starting the stream
	soundStream.start();

	// Wait enter to finish
	std::cin.ignore();
	char temp = getchar();

	// Stopping and closing the stream
	soundStream.stop();
	soundStream.close();
}

//--------------------------------------------------------------
void ofApp::update() {
}

//--------------------------------------------------------------
void ofApp::draw() {
	guiManager.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
}

void ofApp::audioOut(ofSoundBuffer & buffer) {
	unsigned int iBufferSize = 512;
	unsigned int uiBufferSize = iBufferSize;
	// Setting the output buffer as float
	std::vector<float> & bufferData = buffer.getBuffer();

	//Pointer to the first element of the vector
	float * floatOutputBuffer = &bufferData[0];

	//Overflow/underflow
	if (buffer.getNumFrames() == 0) {
		std::cout << "Stream underflow detected!" << std::endl;
	}

	// Initializes buffer with zeros
	outputBufferStereo.left.Fill(uiBufferSize, 0.0f);
	outputBufferStereo.right.Fill(uiBufferSize, 0.0f);

	audioProcess(outputBufferStereo, uiBufferSize);

	// Declaration and initialization of interlaced audio vector for correct stereo output
	CStereoBuffer<float> iOutput;
	iOutput.Interlace(outputBufferStereo.left, outputBufferStereo.right);

	// Buffer filling loop
	for (auto it = iOutput.begin(); it != iOutput.end(); it++) {
		floatOutputBuffer[0] = *it; // Setting of value in actual buffer position
		floatOutputBuffer = &floatOutputBuffer[1]; // Updating pointer to next buffer position
	}
}

/// Audio output configuration, using ofxAudioFile
void ofApp::AudioSetup() {

	// Setting the output parameters
	ofSoundStreamSettings outputParameters;
	outputParameters.numOutputChannels = 2;

	auto devices = soundStream.getDeviceList();
	ofSoundDevice device = devices[0];
	outputParameters.setOutDevice(device);

	// Setting real-time audio output.
	setRealTimePriority();
	outputParameters.numBuffers = 4;

	outputParameters.sampleRate = SAMPLERATE;
	outputParameters.bufferSize = 512;

	outputParameters.setOutListener(this);

	try {
		soundStream.setup(outputParameters);
		soundStream.stop();
	} catch (const std::exception & e) {
		std::cout << "\nERROR: Unable to setup sound stream." << '\n'
				  << "Exception: " << e.what() << std::endl;
	}
}

// Function to process audio
void ofApp::audioProcess(Common::CEarPair<CMonoBuffer<float>> & bufferOutput, int uiBufferSize) {
	// -----------------------------------------------
	// Step 1: Decode ambisonic channels to the speaker array
	// -----------------------------------------------
	ambisonicProcessor.DecodeToSpeakerArray(speakerArrayController);

	// -----------------------------------------------
	// Step 2: Process all BRT modules (listener binauralization)
	// -----------------------------------------------
	Common::CEarPair<CMonoBuffer<float>> bufferProcessed;
	brtManager.ProcessAll();
	listener->GetBuffers(bufferProcessed.left, bufferProcessed.right);

	// -----------------------------------------------
	// Step 3: Sum the binaural output to the provided output buffer
	// -----------------------------------------------
	bufferOutput.left += bufferProcessed.left;
	bufferOutput.right += bufferProcessed.right;
}
