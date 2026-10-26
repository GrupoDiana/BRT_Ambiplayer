#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	outputDevices.clear();

	// Enumerar por separado las API de Windows
	const auto wasapiDevices = soundStream.getDeviceList(ofSoundDevice::MS_WASAPI);

	const auto directSoundDevices = soundStream.getDeviceList(ofSoundDevice::MS_DS);

	const auto asioDevices = soundStream.getDeviceList(ofSoundDevice::MS_ASIO);

	auto addOutputDevices = [this](const std::vector<ofSoundDevice> & devices) {
		for (const auto & device : devices) {
			if (device.outputChannels > 0) {
				outputDevices.push_back(device);
			}
		}
	};

	addOutputDevices(wasapiDevices);
	addOutputDevices(directSoundDevices);
	addOutputDevices(asioDevices);

	guiManager.setup();
	guiManager.setAudioDevices(outputDevices);

	// El resto de tu setup continúa igual...

	brtManager = std::make_shared<BRTBase::CBRTManager>();
	setupBRT();

	vertices = speakerArrayController.kDivision(5);

	////////////////////////////
	// Create Ambisonic Buffers
	////////////////////////////
	SFbuffer = std::make_shared<CAmbisonicSoundField>();
	SFbuffer->SetUp(order, SAMPLERATE, BUFFERSIZE);

	OutBuffer = std::make_shared<CAmbisonicSoundField>();
	OutBuffer->SetUp(order, SAMPLERATE, BUFFERSIZE);

	audioBufferCopy = std::make_shared<CAmbisonicSoundField>();
	audioBufferCopy->SetUp(order, SAMPLERATE, BUFFERSIZE);

	////////////////////////////
	// Stereo output buffer
	////////////////////////////
	outputBufferStereo.left.resize(BUFFERSIZE);
	outputBufferStereo.right.resize(BUFFERSIZE);

	planarEnergies.resize(3);

	////////////////////////////
	// GUI Callbacks
	////////////////////////////
	guiManager.onPlay = [this](bool isPlaying) {
		ofLog() << "Play button pressed!";
		soundStream.start();

		for (int i = 0; i < ambisonicSoundFieldControllers.size(); ++i) {
			ambisonicSoundFieldControllers[i]->Play(isPlaying);
		}
	};

	guiManager.onStop = [this]() {
		soundStream.stop();

		for (int i = 0; i < ambisonicSoundFieldControllers.size(); ++i) {
			ambisonicSoundFieldControllers[i]->Play(false);
			ambisonicSoundFieldControllers[i]->GetWavSamplePositionSource() = 0;
			
		}
	};

	guiManager.onREC = [this]() {
		// Solo alternamos estado
		isRecordingMic = !isRecordingMic;

		if (isRecordingMic) {
			ofLog() << "Mic recording START";

			// IMPORTANTE: no tocar audio stream
			micBuffer.clear();

			soundStream.start(); // aseguramos que está corriendo

		} else {
			ofLog() << "Mic recording STOP";

			// opcional: congelar controllers, pero NO parar stream
			for (auto & c : ambisonicSoundFieldControllers) {
				c->Play(false);
			}

			// protección básica
			if (micBuffer.size() < 256) {
				ofLog() << "Mic buffer too small for convolution";
				return;
			}

			ofFileDialogResult result = ofSystemLoadDialog("Select an ambisonic IR", false);

			if (!result.bSuccess) {
				ofLog() << "No IR selected";
				return;
			}

			auto controllerIR = std::make_shared<CAmbisonicSoundFieldController>();
			controllerIR->SetUp(order, SAMPLERATE, BUFFERSIZE, "ACN");

			if (!controllerIR->LoadWav(result.getPath().c_str(), "ACN", order)) {
				ofLog() << "Failed to load IR";
				return;
			}

			controllerIR->GetWavSamplePositionSource() = 0;

			auto convolutionController = std::make_shared<CAmbisonicSoundFieldController>();

			//convolutionController = ambisonicProcessor.Convolution(micBuffer, *controllerIR);

			convolutionController->GetWavSamplePositionSource() = 0;

			ambisonicSoundFieldControllers.push_back(convolutionController);

			guiManager.addController(
				convolutionController,
				convolutionController->GetFilePathName());
		}
	};

	guiManager.onWAV = [this](string ordering) {
		ofFileDialogResult result = ofSystemLoadDialog("Select a WAV file", false);

		if (!result.bSuccess) {
			ofLog() << "No file selected";
			return;
		}

		auto controller = std::make_shared<CAmbisonicSoundFieldController>();

		controller->SetUp(order, SAMPLERATE, BUFFERSIZE, ordering);

		if (controller->LoadWav(result.getPath().c_str(), ordering, order)) {
			controller->GetWavSamplePositionSource() = 0;
			ambisonicSoundFieldControllers.push_back(controller);
			guiManager.addController(controller,
				controller->GetFilePathName());
		} else {
			ofLog() << "Failed to load WAV";
		}
	};

	guiManager.onHRTF = [this]() {
		ofFileDialogResult result = ofSystemLoadDialog("Select an HRTF SOFA file", false);
		if (!result.bSuccess) {
			ofLog() << "No file selected";
			return;
		}
		configurationA.LoadResources(brtManager.get(), LISTENER_ID, result.getPath().c_str());
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

	guiManager.onChangeBufferSize = [this](int BufferSize) {
		BUFFERSIZE = static_cast<float>(pow(2, 6 + BufferSize));

		soundStream.stop();
		soundStream.close();

		AudioSetup();
		rebuildAudioSystem();
		guiManager.onChangeSampleRate(SAMPLERATE);

	};

	guiManager.onChangeSampleRate = [this](int SampleRate) {
		string sampleRateStr;
		if (SampleRate == 0) {
			SAMPLERATE = 44100.0f;
			sampleRateStr = "44100";
		} else if (SampleRate == 1) {
			SAMPLERATE = 48000.0f;
			sampleRateStr = "48000";
		}	
		else if (SampleRate == 2) {
			SAMPLERATE = 96000.0f;
			sampleRateStr = "96000";
		}

		soundStream.stop();
		soundStream.close();
		AudioSetup();
		rebuildAudioSystem();

		string HRTFName = ofToDataPath("resources/3DTI_HRTF_SADIE_II_D2_256s_" + sampleRateStr + "Hz.sofa", true);

		configurationA.LoadResources(brtManager.get(), LISTENER_ID, HRTFName);
	};

	guiManager.onControllerPlay =
		[this](CMenuPanel::ControllerPtr c) {
			if (!c) return;
			c->Play(!c->IsPlaying());

			updateGlobalPlayState();
		};

	guiManager.onControllerStop =
		[this](CMenuPanel::ControllerPtr c) {
			if (!c) return;
			c->Play(false);
			c->GetWavSamplePositionSource() = 0;

			updateGlobalPlayState();
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

			updateGlobalPlayState();
		};

	guiManager.onControllerCONV =
		[this](CMenuPanel::ControllerPtr c) {
		ofFileDialogResult result = ofSystemLoadDialog("Select an ambisonic IR", false);

		if (!result.bSuccess) {
			ofLog() << "No file selected";
			return;
		}

		auto controllerIR = std::make_shared<CAmbisonicSoundFieldController>();

		controllerIR->SetUp(order, 48000, BUFFERSIZE, "ACN");

		if (controllerIR->LoadWav(result.getPath().c_str(), "ACN", order)) {
			controllerIR->GetWavSamplePositionSource() = 0;
		} else {
			ofLog() << "Failed to load WAV";
		}

		auto convolutionController = std::make_shared<CAmbisonicSoundFieldController>();

		//*convolutionController = ambisonicProcessor.Convolution(c->GetSoundFieldController().GetSoundFieldArray()[0], *controllerIR);
		*convolutionController = ambisonicProcessor.Convolution(*c, *controllerIR);
		convolutionController->GetWavSamplePositionSource() = 0;
		convolutionController->SaveToWav("reverb_speech.wav");
		ambisonicSoundFieldControllers.push_back(convolutionController);

		guiManager.addController(convolutionController, convolutionController->GetFilePathName());
	};

	guiManager.onIRotationChanged =
		[this](CMenuPanel::ControllerPtr c,
			float yaw, float pitch, float roll) {
			if (!c) return;
			c->GetQ() = Common::CQuaternion::FromYawPitchRoll(yaw, pitch, roll);
		};

	guiManager.onSFRotationChanged =
		[this](float yaw, float pitch, float roll) {
			qField = Common::CQuaternion::FromYawPitchRoll(yaw, pitch, roll);
		};

	guiManager.onLSRotationChanged = [this](float yaw, float pitch, float roll) {
		qLoudSpeakers = Common::CQuaternion::FromYawPitchRoll(yaw, pitch, roll);

		for (size_t i = 0; i < speakerArrayController.GetNumSpeakers(); ++i) {
			Common::CVector3 rotatedPos = qLoudSpeakers.RotationMatrix() * originalPositions[i];
			speakerArrayController.GetSpeaker(i)->SetSourceTransform(Common::CTransform(rotatedPos));
		}

	};

	guiManager.onLayoutChanged =
		[this](int layoutIndex) {
			speakerArrayController.UpdateSpeakerLayout(*brtManager, static_cast<PlatonicSolid>(layoutIndex), configurationA);

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

			if (order == 2) {
				if (guiManager.onLayoutChanged) {
					guiManager.onLayoutChanged(2);
					guiManager.setLayout(2);
				}
			}

			if (order == 3) {
				if (guiManager.onLayoutChanged) {
					guiManager.onLayoutChanged(3);
					guiManager.setLayout(3);
				}
			}

			soundStream.start();
	};

	guiManager.onVisualizationChanged =
		[this](int visualizationIndex) {

			visualization = visualizationIndex;
			
	};

	guiManager.onSerialPortChanged =
		[this](int serialIndex) {
			auto coms = TrackerController::listCOMPorts();

			if (serialIndex == -1) {
				std::cout << "Ningun COM seleccionado.\n";
				tracker.disconnect();
				qHead = CQuaternion::UNIT();
				cout << "Llega a aqui" << endl;
			}

			if (serialIndex < 0 || serialIndex >= coms.size()) {
				std::cout << "Indice COM invalido\n";
				return;
			}

			std::wstring portName(coms[serialIndex].port.begin(), coms[serialIndex].port.end());
			portName = L"\\\\.\\" + portName; 

			if (tracker.setup(portName, 115200)) {
				std::cout << "Tracker conectado a "<< std::string(coms[serialIndex].port.begin(), coms[serialIndex].port.end())<< "\n";
			} else {
				std::cout << "No se pudo conectar a "<< std::string(coms[serialIndex].port.begin(), coms[serialIndex].port.end())<< "\n";
			}
		};


	guiManager.onCalibrateHead = [this]() {
		tracker.calibrateCenter(64);
	};

	guiManager.onCalibrateCompass = [this]() {
		tracker.calibrateSensor(240);
	};

	guiManager.onAudioDeviceChanged = [this](int index) {
		soundStream.close();
		changeAudioDevice(index);
	};

	guiManager.setSpeakerController(&speakerArrayController);

	auto controller = std::make_shared<CAmbisonicSoundFieldController>();

	controller->SetUp(order, 48000, BUFFERSIZE, "ACN");

	string SOURCEName = ofToDataPath("resources/ambi_1OA_test.wav", true);
	if (controller->LoadWav(SOURCEName, "ACN", 1)) {
		controller->GetWavSamplePositionSource() = 0;
		ambisonicSoundFieldControllers.push_back(controller);
		guiManager.addController(controller,
			controller->GetFilePathName());
	} else {
		ofLog() << "Failed to load WAV";
	}

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

	for (size_t i = 0; i < speakerArrayController.GetNumSpeakers(); ++i) {
		Common::CVector3 rotatedPos = qLoudSpeakers.RotationMatrix() * originalPositions[i];
		speakerArrayController.GetSpeaker(i)->SetSourceTransform(Common::CTransform(rotatedPos));
	}

	if (visualization == 0) {
		guiManager.draw(visualization, energiesCopy, {}, speakerEnergies, vertices);
		guiManager.getHeadRotation(qHead);
	} else {
		guiManager.draw(visualization, {}, planarEnergies, speakerEnergies, vertices);
		guiManager.getHeadRotation(qHead);
	}

	if (tracker.isConnected()) {
		tracker.readLine();
		qHead = tracker.getLastQuat().Inverse();
	}
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

	ofSoundStreamSettings settings;

	settings.numOutputChannels = 2;
	settings.numInputChannels = 1;

	if (selectedAudioDeviceIndex >= 0 && selectedAudioDeviceIndex < outputDevices.size()) {
		settings.setOutDevice(outputDevices[selectedAudioDeviceIndex]);
		settings.numOutputChannels = std::min(2,static_cast<int>(outputDevices[selectedAudioDeviceIndex].outputChannels));
	}

	settings.sampleRate = SAMPLERATE;
	settings.bufferSize = BUFFERSIZE;
	settings.numBuffers = 4;

	settings.setInListener(this);
	settings.setOutListener(this);

	setRealTimePriority();

	try {
		soundStream.setup(settings);
		soundStream.stop();
	} catch (const std::exception & e) {
		std::cout << "\nERROR: Unable to setup sound stream.\n"
				  << e.what() << std::endl;
	}
}

void ofApp::audioIn(ofSoundBuffer & input) {

	if (!isRecordingMic) return;

	std::lock_guard<std::mutex> lock(micMutex);

	for (size_t i = 0; i < input.getNumFrames(); ++i) {
		micBuffer.push_back(input[i]);
	}
}

// audioOut
void ofApp::audioOut(ofSoundBuffer & buffer) {
	// Setting the output buffer as float
	std::vector<float> & bufferData = buffer.getBuffer();

	//Pointer to the first element of the vector
	float * floatOutputBuffer = &bufferData[0];

	//Overflow/underflow
	if (buffer.getNumFrames() == 0) {
		std::cout << "Stream underflow detected!" << std::endl;
	}

	// Initializes buffer with zeros
	outputBufferStereo.left.Fill(BUFFERSIZE, 0.0f);
	outputBufferStereo.right.Fill(BUFFERSIZE, 0.0f);

	audioProcess(outputBufferStereo, BUFFERSIZE);

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

		float vol = controller->GetPlaybackVolume();

		for (auto & ch : OutBuffer->GetSoundFieldArray()) {
			for (auto & sample : ch) {
				sample *= vol;
			}
		}
		
		if (!controller->IsMuted())
			*SFbuffer += *OutBuffer;
	}

	qRotation = qField * qHead;
	ambisonicProcessor.CalculateRotationMatrices(qRotation, *SFbuffer);

	SFbuffer->GetSoundFieldArrayRotate();

	ambisonicProcessor.DecodeToSpeakerArray(speakerArrayController, *SFbuffer, "basic");
	speakerEnergies.clear();
	for (size_t i = 0; i < speakerArrayController.GetNumSpeakers(); ++i) {
		const auto & buf = speakerArrayController.GetSpeaker(i)->GetBuffer();
		float energy = 0.0f;
		for (auto s : buf)
			energy += s * s;
		energy = sqrt(energy / buf.size()); 
		speakerEnergies.push_back(energy);
	}

	std::lock_guard<std::mutex> lock(energiesMutex);

	if (visualization == 0) {
		directionalEnergies = speakerArrayController.ComputeDirectionalEnergyMap(*SFbuffer, speakerArrayController.GetVerticesSubdivided());
	} else {
		planarEnergies[0] = speakerArrayController.ComputeDirectionalEnergyMap(*SFbuffer, speakerArrayController.GeneratePlanarDirections(Audio::ProjectionPlane::ZY, 20));
		planarEnergies[1] = speakerArrayController.ComputeDirectionalEnergyMap(*SFbuffer, speakerArrayController.GeneratePlanarDirections(Audio::ProjectionPlane::ZX, 20));
		planarEnergies[2] = speakerArrayController.ComputeDirectionalEnergyMap(*SFbuffer, speakerArrayController.GeneratePlanarDirections(Audio::ProjectionPlane::XY, 20));
	}


	Common::CEarPair<CMonoBuffer<float>> bufferProcessed;

	brtManager->ProcessAll();
	listener->GetBuffers(bufferProcessed.left, bufferProcessed.right);

	bufferOutput.left += bufferProcessed.left;
	bufferOutput.right += bufferProcessed.right;
}

void ofApp::changeAudioDevice(int index) {
	if (index < 0 || index >= outputDevices.size()) {
		return;
	}

	selectedAudioDeviceIndex = index;

	const auto & device = outputDevices[index];

	ofSoundStreamSettings settings;
	settings.setOutListener(this);
	settings.setOutDevice(device);
	settings.sampleRate = SAMPLERATE;
	settings.bufferSize = BUFFERSIZE;
	settings.numOutputChannels = 2;
	settings.numBuffers = 4;

	soundStream.stop();
	soundStream.close();

	if (!soundStream.setup(settings)) {
		ofLogError("Audio") << "No se pudo abrir: " << device.name;
	}
}

void ofApp::setupBRT() {

	globalParameters.SetSampleRate(SAMPLERATE);
	globalParameters.SetBufferSize(BUFFERSIZE);

	brtManager->BeginSetup();

	listener = brtManager->CreateListener<BRTBase::CListener>(LISTENER_ID);

	brtManager->EndSetup();

	configurationA.Setup(brtManager.get(), LISTENER_ID);
	string hrtfPath = ofToDataPath("resources/3DTI_HRTF_SADIE_II_D2_256s_48000Hz.sofa",true);

	configurationA.LoadResources(brtManager.get(), LISTENER_ID, hrtfPath);

	Common::CTransform listenerPosition;
	listenerPosition.SetPosition(Common::CVector3(0.0f, 0.0f, 0.0f));
	listener->SetListenerTransform(listenerPosition);

	speakerArrayController.SetUp(*brtManager, static_cast<PlatonicSolid>(0));

	originalPositions.clear();

	for (int i = 0; i < speakerArrayController.GetNumSpeakers(); i++) {
		brtManager->BeginSetup();
		configurationA.ConnectSoundSource(brtManager.get(), "Speaker" + std::to_string(i));
		brtManager->EndSetup();

		originalPositions.push_back(
			speakerArrayController.GetSpeaker(i)
				->GetSourceTransform()
				.GetPosition());
	}
}

void ofApp::rebuildAudioSystem() {
	soundStream.stop();

	
	soundStream.close();

	brtManager.reset();
	brtManager = std::make_shared<BRTBase::CBRTManager>();
	listener.reset();
	listener = nullptr;
	setupBRT();

	SFbuffer = std::make_shared<CAmbisonicSoundField>();
	SFbuffer->SetUp(order, SAMPLERATE, BUFFERSIZE);

	OutBuffer = std::make_shared<CAmbisonicSoundField>();
	OutBuffer->SetUp(order, SAMPLERATE, BUFFERSIZE);

	audioBufferCopy = std::make_shared<CAmbisonicSoundField>();
	audioBufferCopy->SetUp(order, SAMPLERATE, BUFFERSIZE);

	outputBufferStereo.left.resize(BUFFERSIZE);
	outputBufferStereo.right.resize(BUFFERSIZE);

	AudioSetup();
	soundStream.start();
}

void ofApp::updateGlobalPlayState() {
	bool anyPlaying = false;

	for (auto & c : ambisonicSoundFieldControllers) {
		if (c->IsPlaying()) {
			anyPlaying = true;
			break;
		}
	}

	guiManager.setGlobalPlayState(anyPlaying);
}
