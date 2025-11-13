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
	listenerPosition.SetPosition(Common::CVector3(0.0f, 0.0f, 0.0f));
	listener->SetListenerTransform(listenerPosition);

	/////////////////////////////////////
	// Create and connnect BRT modules
	/////////////////////////////////////
	configurationA.Setup(&brtManager, LISTENER_ID);
	configurationA.LoadResources(&brtManager, LISTENER_ID);

	/////////////////////
	// Create Sources
	/////////////////////
	int solid;
	std::cout << "Please choose the platonic solid: "<<endl;
	std::cout << "0: Octahedron" << endl;
	std::cout << "1: Cube" << endl;
	std::cout << "2: Icosahedron" << endl;
	std::cout << "3: Dodecahedron" << endl;
	std::cin >> solid;
	speakerArrayController = CLoudSpeakerArrayController();
	speakerArrayController.SetUp(brtManager, static_cast<PlatonicSolid>(solid));

	//q = Common::CQuaternion::FromYawPitchRoll(M_PI/6, 0.0f, 0.0f);
	//speakerArrayController.RotateLoudspeakerArray(q);

	for (int i = 0; i < speakerArrayController.GetNumSpeakers(); i++) {
		configurationA.ConnectSoundSource(&brtManager, "Speaker" + std::to_string(i));
	}
	
	// /////////////////////
	// Load Wav Files
	/////////////////////
	ambisonicSoundFieldController = std::make_shared<CAmbisonicSoundFieldController>();
	ambisonicSoundFieldController->SetUp(1, 48000, 512, "FuMa");
	ambisonicSoundFieldController->LoadWav(SOURCE_FILEPATH, "ACN");
	//ambisonicSoundFieldController->GenerateSyntheticYField(48000 * 10); // 10 seconds

	SFbuffer = std::make_shared<CAmbisonicSoundField>();
	SFbuffer->SetUp(ambisonicSoundFieldController->GetSoundFieldController().GetOrder(), 48000, 512);
	OutBuffer = std::make_shared<CAmbisonicSoundField>();
	OutBuffer->SetUp(ambisonicSoundFieldController->GetSoundFieldController().GetOrder(), 48000, 512);

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
		//soundStream.close();
	};

	guiManager.onRotationChanged = [this](float yaw, float pitch, float roll) {
		yawRad = yaw;
		pitchRad = pitch;
		rollRad = roll;
	};

	
}

//--------------------------------------------------------------
void ofApp::update() {
}

//--------------------------------------------------------------
void ofApp::draw() {
	guiManager.draw(directionalEnergies);
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

// Function to process audio
void ofApp::audioProcess(Common::CEarPair<CMonoBuffer<float>> & bufferOutput, int bufferSize) {

	//SFbuffer->GetSegment(wavSamplePositionSource, positionEndFrameSource, ambisonicSoundFieldController->GetSoundFieldController(), bufferSize);
	OutBuffer->GetSegment(wavSamplePositionSource, positionEndFrameSource, ambisonicSoundFieldController->GetSoundFieldController(), bufferSize);

	q = Common::CQuaternion::FromYawPitchRoll(yawRad, pitchRad, rollRad);
	ambisonicProcessor.RotateField(q, *OutBuffer);


	// Decodificación a altavoces
	ambisonicProcessor.DecodeToSpeakerArray(speakerArrayController, *OutBuffer, "basic");
	directionalEnergies = speakerArrayController.ComputeDirectionalEnergyMap(*OutBuffer, numAz, numEl);


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


//--------------------------------------------------------------
void ofApp::DrawDirectionalLobeProjected(const std::vector<float> & energies, int numAz, int numEl, float cx, float cy, float maxRadius) {
	if (energies.empty()) return;

	std::vector<float> azE(numAz, 0.0f);
	float globalMax = 0.0f;

	for (int ia = 0; ia < numAz; ++ia) {
		float maxVal = 0.0f;
		for (int ie = 0; ie < numEl; ++ie) {
			float val = energies[ia * numEl + ie];
			if (val > maxVal) maxVal = val;
		}
		azE[ia] = maxVal;
		if (maxVal > globalMax) globalMax = maxVal;
	}

	float scale = (globalMax > 0.0f) ? (maxRadius / std::sqrt(globalMax)) : 0.0f;

	ofPushStyle();
	ofNoFill();
	ofSetLineWidth(2);
	ofBeginShape();

	for (int ia = 0; ia < numAz; ++ia) {
		float az = (2.0f * M_PI * ia) / numAz + M_PI_2;
		float r = scale * std::sqrt(azE[ia]);
		float x = cx + r * std::cos(az);
		float y = cy - r * std::sin(az);
		ofVertex(x, y);
	}

	ofEndShape(true);
	ofPopStyle();
}









