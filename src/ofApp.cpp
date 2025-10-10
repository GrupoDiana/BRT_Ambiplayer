#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

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

	/////////////////////////////////////
	// Create and connnect BRT modules
	/////////////////////////////////////
	configurationA.Setup(&brtManager, LISTENER_ID);
	configurationA.LoadResources(&brtManager, LISTENER_ID);

	/////////////////////
	// Create Sources
	/////////////////////
	int nspeakers;
	std::cout << "Please choose the number of virtual speakers: ";
	std::cin >> nspeakers;
	speakerArrayController = CLoudSpeakerArrayController(brtManager, nspeakers);

	for (int i = 0; i < speakerArrayController.GetNumSpeakers(); i++) {
		configurationA.ConnectSoundSource(&brtManager, "Speaker" + std::to_string(i));
	}

	/////////////////////
	// Setup Sources
	/////////////////////
	speakerArrayController.LoadLayoutJSON();

	// /////////////////////
	// Load Wav Files
	/////////////////////
	ambisonicSoundFieldController = std::make_shared<CAmbisonicSoundFieldController>();
	ambisonicSoundFieldController->LoadWav(SOURCE_FILEPATH, "SN3D");

	SFbuffer = std::make_shared<CAmbisonicSoundField>();
	OutBuffer = std::make_shared<CAmbisonicSoundField>();

	/////////////////////
	// Start AUDIO Render
	/////////////////////
	// Declaration and initialization of stereo buffer
	outputBufferStereo.left.resize(512);
	outputBufferStereo.right.resize(512);

	// Setup GUI callbacks
	guiManager.onPlay = [this]() {
		ofLog() << "Play button pressed!";
		// Starting the stream
		soundStream.start();
	};
	
	guiManager.onStop = [this]() {
		// Stopping and closing the stream
		soundStream.stop();
		soundStream.close();
	};
	
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


// audioOut
void ofApp::audioOut(ofSoundBuffer & buffer) {
	unsigned int uiBufferSize = 512;
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

	//outputParameters.getOutDevice();
	ofSoundDevice device = ShowSelectAudioDeviceMenu();
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
void ofApp::audioProcess(Common::CEarPair<CMonoBuffer<float>> & bufferOutput, int bufferSize) {

	//SFbuffer->FillBuffer(wavSamplePositionSource, positionEndFrameSource, ambisonicSoundFieldController->GetSoundFieldController(), bufferSize);
	OutBuffer->FillBuffer(wavSamplePositionSource, positionEndFrameSource, ambisonicSoundFieldController->GetSoundFieldController(), bufferSize);

	// Decodificación a altavoces
	ambisonicProcessor.DecodeToSpeakerArray(speakerArrayController, *OutBuffer);

	Common::CEarPair<CMonoBuffer<float>> bufferProcessed;

	brtManager.ProcessAll();
	listener->GetBuffers(bufferProcessed.left, bufferProcessed.right);

	bufferOutput.left += bufferProcessed.left;
	bufferOutput.right += bufferProcessed.right;
}


	/// Function to show the user a menu to choose the audio output device
ofSoundDevice ofApp::ShowSelectAudioDeviceMenu() {

	auto devices = soundStream.getDeviceList();
	std::cout << "Number of audio devices found: " << devices.size() << std::endl;
	int connectedAudioDevices = soundStream.getNumOutputChannels();

	std::cout << std::endl
			  << std::endl;
	std::cout << "----------------------------------------" << std::endl;
	std::cout << "     List of available audio outputs" << std::endl;
	std::cout << "----------------------------------------" << std::endl
			  << std::endl;

	for (int i = 0; i < devices.size(); ++i) {
		std::cout << "Device #" << i << ": " << devices[i].name << std::endl;
		std::cout << "Output channels: " << devices[i].outputChannels << " ";
		std::cout << "Input channels: " << devices[i].inputChannels << " ";
		std::cout << "Default output: " << (devices[i].isDefaultOutput ? "Yes" : "No") << std::endl;
		std::cout << std::endl;
	}

	std::cout << std::endl;
	int selectAudioDevice;
	do {
		std::cout << "Please choose which audio output you wish to use: ";
		std::cin >> selectAudioDevice;
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
	} while (!(selectAudioDevice > -1 && selectAudioDevice <= devices.size()));
	std::cout << std::endl;
	return devices[selectAudioDevice];
}


