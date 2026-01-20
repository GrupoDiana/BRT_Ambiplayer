#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	/// AUDIO setup
	AudioSetup();

	/// BRT Global Parameters setup
	globalParameters.SetSampleRate(SAMPLERATE);
	globalParameters.SetBufferSize(BUFFERSIZE);

	////////////////////////////
	// Listener Setup
	////////////////////////////
	brtManager.BeginSetup();
	listener = brtManager.CreateListener<BRTBase::CListener>(LISTENER_ID);
	brtManager.EndSetup();

	Common::CTransform listenerPosition;
	listenerPosition.SetPosition(Common::CVector3(0.0f, 0.0f, 0.0f));
	listener->SetListenerTransform(listenerPosition);

	////////////////////////////
	// Create and connect BRT modules
	////////////////////////////
	configurationA.Setup(&brtManager, LISTENER_ID);
	configurationA.LoadResources(&brtManager, LISTENER_ID);

	////////////////////////////
	// Speaker Array Setup
	////////////////////////////
	speakerArrayController = CLoudSpeakerArrayController();
	speakerArrayController.SetUp(brtManager, static_cast<PlatonicSolid>(0));
	originalPositions.clear();

	for (int i = 0; i < speakerArrayController.GetNumSpeakers(); i++) {
		brtManager.BeginSetup();
		configurationA.ConnectSoundSource(&brtManager,"Speaker" + std::to_string(i));
		brtManager.EndSetup();
		originalPositions.push_back(speakerArrayController.GetSpeaker(i)->GetSourceTransform().GetPosition());
	}

	vertices = speakerArrayController.kDivision(0);

	////////////////////////////
	// Create Ambisonic Buffers
	////////////////////////////
	SFbuffer = std::make_shared<CAmbisonicSoundField>();
	SFbuffer->SetUp(order, 48000, BUFFERSIZE);

	OutBuffer = std::make_shared<CAmbisonicSoundField>();
	OutBuffer->SetUp(order, 48000, BUFFERSIZE);

	audioBufferCopy = std::make_shared<CAmbisonicSoundField>();
	audioBufferCopy->SetUp(order, 48000, BUFFERSIZE);

	////////////////////////////
	// Stereo output buffer
	////////////////////////////
	outputBufferStereo.left.resize(BUFFERSIZE);
	outputBufferStereo.right.resize(BUFFERSIZE);

	////////////////////////////
	// GUI Callbacks
	////////////////////////////
	guiManager.onPlay = [this]() {
		ofLog() << "Play button pressed!";
		soundStream.start();
	};

	guiManager.onStop = [this]() {
		soundStream.stop();
	};

	guiManager.onWAV = [this](string ordering) {
		ofFileDialogResult result = ofSystemLoadDialog("Select a WAV file", false);

		if (!result.bSuccess) {
			ofLog() << "No file selected";
			return;
		}

		auto controller = std::make_shared<CAmbisonicSoundFieldController>();

		controller->SetUp(order, 48000, BUFFERSIZE, ordering);

		if (controller->LoadWav(result.getPath().c_str(), ordering, order)) {
			controller->GetWavSamplePositionSource() = 0;
			ambisonicSoundFieldControllers.push_back(controller);
			guiManager.addController(controller,
				controller->GetFilePathName());
		} else {
			ofLog() << "Failed to load WAV";
		}
	};

	guiManager.onCreateSource =
		[this](int numFrames, Common::CVector3 pos) {
			ofFileDialogResult result = ofSystemLoadDialog("Select a WAV file", false);

			if (!result.bSuccess) {
				ofLog() << "No file selected";
				return;
			}

			auto controller = std::make_shared<CAmbisonicSoundFieldController>();

			controller->SetUp(order, 48000, BUFFERSIZE, "ACN");

			if (controller->GeneratePuntualSourceField(
					order, result.getPath().c_str(), pos)) {
				controller->GetWavSamplePositionSource() = 0;
				ambisonicSoundFieldControllers.push_back(controller);
				guiManager.addController(controller,
					controller->GetFilePathName());
			}
		};

	guiManager.onChangeK = [this](int k) {
		vertices = speakerArrayController.kDivision(k);
	};

	guiManager.onControllerPlay =
		[this](CMenuPanel::ControllerPtr c) {
			if (!c) return;
			c->Play(!c->IsPlaying());
		};

	guiManager.onControllerStop =
		[this](CMenuPanel::ControllerPtr c) {
			if (!c) return;
			c->Play(false);
			c->GetWavSamplePositionSource() = 0;
		};

	guiManager.onControllerMute =
		[this](CMenuPanel::ControllerPtr c) {
			if (c) c->Mute(!c->IsMuted());
		};

	guiManager.onControllerSolo =
		[this](CMenuPanel::ControllerPtr c) {
			if (!c) return;

			bool enableSolo = !c->IsSolo();

			for (auto & ctrl : ambisonicSoundFieldControllers) {
				ctrl->Solo(false);
				ctrl->Mute(false); 
			}

			if (enableSolo) {
				c->Solo(true);

				for (auto & ctrl : ambisonicSoundFieldControllers) {
					if (ctrl != c)
						ctrl->Mute(true);
				}
			}
		};

	guiManager.onControllerLoop =
		[this](CMenuPanel::ControllerPtr c) {
			if (!c) return;
			c->Loop(!c->IsLoop());
		};

	guiManager.onControllerRemove =
		[this](CMenuPanel::ControllerPtr c) {
			if (!c) return;
			guiManager.removeController(c);
			ambisonicSoundFieldControllers.erase(
				std::remove(ambisonicSoundFieldControllers.begin(),
					ambisonicSoundFieldControllers.end(), c),
				ambisonicSoundFieldControllers.end());
		};

	guiManager.onIRotationChanged =
		[this](CMenuPanel::ControllerPtr c,
			float yaw, float pitch, float roll) {
			if (!c) return;
			c->GetQ() = Common::CQuaternion::FromYawPitchRoll(yaw, pitch, roll);
		};

	guiManager.onSFRotationChanged =
		[this](float yaw, float pitch, float roll) {
			qRotation = Common::CQuaternion::FromYawPitchRoll(yaw, pitch, roll);
		};

	guiManager.onLSRotationChanged =
		[this](float yaw, float pitch, float roll) {
			qLoudSpeakers = Common::CQuaternion::FromYawPitchRoll(yaw, pitch, roll);

			for (size_t i = 0;
				i < speakerArrayController.GetNumSpeakers(); ++i) {
				Common::CVector3 rotatedPos = qLoudSpeakers.RotationMatrix() * originalPositions[i];

				speakerArrayController.GetSpeaker(i)->SetSourceTransform(Common::CTransform(rotatedPos));
			}
		};

	guiManager.onLayoutChanged =
		[this](int layoutIndex) {
			speakerArrayController.UpdateSpeakerLayout(brtManager,static_cast<PlatonicSolid>(layoutIndex),  configurationA);

			originalPositions.clear();

			for (int i = 0;i < speakerArrayController.GetNumSpeakers(); i++) {
				originalPositions.push_back(speakerArrayController.GetSpeaker(i)->GetSourceTransform().GetPosition());
			}
		};

	guiManager.onOrderChanged =
		[this](int orderIndex) {
			soundStream.stop();

			order = orderIndex + 1;

			SFbuffer->SetOrder(order);
			OutBuffer->SetOrder(order);

			guiManager.clearControllers();
			ambisonicSoundFieldControllers.clear();

			soundStream.start();
		};



	guiManager.setSpeakerController(&speakerArrayController);

	soundStream.start();
}

//--------------------------------------------------------------
void ofApp::update() { }

//--------------------------------------------------------------
void ofApp::draw() {
	std::vector<float> energiesCopy;
	{
		std::lock_guard<std::mutex> lock(energiesMutex);
		energiesCopy = directionalEnergies;
	}

	// --- Apply loudspeaker rotation before drawing ---
	for (size_t i = 0; i < speakerArrayController.GetNumSpeakers(); ++i) {
		Common::CVector3 rotatedPos = qLoudSpeakers.RotationMatrix() * originalPositions[i];
		speakerArrayController.GetSpeaker(i)->SetSourceTransform(Common::CTransform(rotatedPos));
	}

	// --- Draw GUI and speakers ---
	guiManager.draw(energiesCopy, speakerEnergies, vertices);
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
	outputParameters.bufferSize = BUFFERSIZE;

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
	unsigned int uiBufferSize = BUFFERSIZE;
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

	for (auto & ch : SFbuffer->GetSoundFieldArray())
		std::fill(ch.begin(), ch.end(), 0.0f);

	for (size_t i = 0; i < ambisonicSoundFieldControllers.size(); i++) {
		auto & controller = ambisonicSoundFieldControllers[i];

		if (!controller->IsPlaying())
			continue;

		for (auto & ch : OutBuffer->GetSoundFieldArray())
			std::fill(ch.begin(), ch.end(), 0.0f);

		ambisonicProcessor.CalculateRotationMatrices(controller->GetQ(), *OutBuffer);

		OutBuffer->GetSegment(controller->GetWavSamplePositionSource(), controller->GetPositionEndFrameSource(), controller->GetSoundFieldController(), bufferSize, controller->IsLoop(), controller->GetPlaying());
		
		if (!controller->IsMuted())
			*SFbuffer += *OutBuffer;
	}

	ambisonicProcessor.CalculateRotationMatrices(qRotation, *SFbuffer);
	SFbuffer->GetSoundFieldArrayRotate();

	ambisonicProcessor.DecodeToSpeakerArray(speakerArrayController, *SFbuffer, "basic");
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
	directionalEnergies = speakerArrayController.ComputeDirectionalEnergyMapGeodesic(*SFbuffer);
	//directionalEnergies = speakerArrayController.ComputeDirectionalEnergyMap(*SFbuffer,numAz, numEl);

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









