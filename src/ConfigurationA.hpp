
#include "ofxbrtlibrary.h"
#include "AppUtils.hpp"

#ifndef _CONFIGURATION_A_HPP_
#define _CONFIGURATION_A_HPP_

#define LISTENER_HRTF_MODEL_ID "listenerHRTF1"

class CConfigurationA
{
public:
    CConfigurationA() {};    


    void Setup(BRTBase::CBRTManager* brtManager, const std::string& listenerID) {
        		
        brtManager->BeginSetup();
        
        std::shared_ptr<BRTListenerModel::CListenerDirectHRTFConvolutionModel> listenerModel = brtManager->CreateListenerModel<BRTListenerModel::CListenerDirectHRTFConvolutionModel>(LISTENER_HRTF_MODEL_ID);
        
        std::shared_ptr<BRTBase::CListener> listener = brtManager->GetListener(listenerID);
        bool control = listener->ConnectListenerModel(LISTENER_HRTF_MODEL_ID);
        if (!control) {
            std::cout << "Error connecting listener model" << std::endl;
        }

        brtManager->EndSetup();		
    }

    void LoadResources(BRTBase::CBRTManager * brtManager, const std::string & listenerID, const std::string & _filePath) {
        std::shared_ptr<BRTBase::CListener> listener = brtManager->GetListener(listenerID);
        
        /// Load HRTF
        std::shared_ptr<BRTServices::CHRTF> hrtf1 = std::make_shared<BRTServices::CHRTF>();
		bool hrtfSofaLoaded1 = AppUtils::LoadHRTFSofaFile(_filePath, hrtf1);        
        // Set one for the listener. We can change it at runtime    
        if (hrtfSofaLoaded1) {
            listener->SetHRTF(hrtf1);
        }
    }

    void ConnectSoundSource(BRTBase::CBRTManager* brtManager, const std::string& _soundSourceID) {
        std::shared_ptr<BRTListenerModel::CListenerModelBase> listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(LISTENER_HRTF_MODEL_ID);
        if (listenerModel != nullptr) {
            bool control = listenerModel->ConnectSoundSource(_soundSourceID);
        }
    }

	void DisconnectSoundSource(BRTBase::CBRTManager * brtManager, const std::string & _soundSourceID) {
		std::shared_ptr<BRTListenerModel::CListenerModelBase> listenerModel = brtManager->GetListenerModel<BRTListenerModel::CListenerModelBase>(LISTENER_HRTF_MODEL_ID);
		if (listenerModel != nullptr) {
			bool control = listenerModel->DisconnectSoundSource(_soundSourceID);
		}
	}

	void DisonnectListenerModel(BRTBase::CBRTManager * brtManager, const std::string & listenerID) {
		brtManager->BeginSetup();

		std::shared_ptr<BRTBase::CListener> listener = brtManager->GetListener(listenerID);
		bool control = listener->DisconnectListenerModel(LISTENER_HRTF_MODEL_ID);
		if (!control) {
			std::cout << "Error connecting listener model" << std::endl;
		}

		brtManager->EndSetup();
	}

private:

};

#endif
