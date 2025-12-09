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
	/* int solid;
	std::cout << "Please choose the platonic solid: "<<endl;
	std::cout << "0: Octahedron" << endl;
	std::cout << "1: Cube" << endl;
	std::cout << "2: Icosahedron" << endl;
	std::cout << "3: Dodecahedron" << endl;
	std::cin >> solid;*/
	speakerArrayController = CLoudSpeakerArrayController();
	speakerArrayController.SetUp(brtManager, static_cast<PlatonicSolid>(0));

	for (int i = 0; i < speakerArrayController.GetNumSpeakers(); i++) {
		configurationA.ConnectSoundSource(&brtManager, "Speaker" + std::to_string(i));
	}
	for (auto & sp : speakerArrayController.GetSpeakers()) {
		originalPositions.push_back(sp->GetSourceTransform().GetPosition());
	}

	
	// /////////////////////
	// Load Wav Files
	/////////////////////
	ambisonicSoundFieldController = std::make_shared<CAmbisonicSoundFieldController>();
	ambisonicSoundFieldController->SetUp(1, 48000, 512, "ACN");
	//ambisonicSoundFieldController->LoadWav(SOURCE_FILEPATH, "ACN");
	ambisonicSoundFieldController->GenerateSyntheticYField(48000 * 10); // 10 seconds
	//ambisonicSoundFieldController->GeneratePuntualSourceField(48000 * 10); // 10 seconds

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

	guiManager.onLayoutChanged = [this](int layoutIndex) {
		speakerArrayController.UpdateSpeakerLayout(brtManager, static_cast<PlatonicSolid>(layoutIndex));
		originalPositions.clear();
		for (int i = 0; i < speakerArrayController.GetNumSpeakers(); i++) {
			configurationA.ConnectSoundSource(&brtManager, "Speaker" + std::to_string(i));
			originalPositions.push_back(speakerArrayController.GetSpeaker(i)->GetSourceTransform().GetPosition());
		}
	};

	guiManager.setSpeakerController(&speakerArrayController);
	
}

//--------------------------------------------------------------
void ofApp::update() {
}

//--------------------------------------------------------------
void ofApp::draw() {
	std::vector<float> energiesCopy;
	{
		std::lock_guard<std::mutex> lock(energiesMutex);
		energiesCopy = directionalEnergies;
	}
	guiManager.draw(energiesCopy, speakerEnergies);
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
	//ofSoundDevice device = ShowSelectAudioDeviceMenu();
	auto outDevices = soundStream.getMatchingDevices("Default");
	if (!outDevices.empty()) {
		outputParameters.setOutDevice(outDevices[0]); 
	}
	

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
	for (size_t i = 0; i < speakerArrayController.GetNumSpeakers(); ++i) {
		Common::CVector3 rotatedPos = q.RotationMatrix() * originalPositions[i];
		speakerArrayController.GetSpeaker(i)->SetSourceTransform(Common::CTransform(rotatedPos));
	}


	// Decodificación a altavoces
	ambisonicProcessor.DecodeToSpeakerArray(speakerArrayController, *OutBuffer, "basic");
	speakerEnergies.clear();
	for (size_t i = 0; i < speakerArrayController.GetNumSpeakers(); ++i) {
		const auto & buf = speakerArrayController.GetSpeaker(i)->GetBuffer();
		float energy = 0.0f;
		for (auto s : buf)
			energy += s * s; // suma de cuadrados
		energy = sqrt(energy / buf.size()); // RMS
		speakerEnergies.push_back(energy);
	}


	std::lock_guard<std::mutex> lock(energiesMutex);
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









