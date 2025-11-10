#ifndef _LOGO_HPP_
#define _LOGO_HPP_

#include "ofMain.h"

class CLogo {
public:
	CLogo() {
		string pathResources = ofToDataPath("../../../resources");

		bool imageOk = logoUMA.load(pathResources + "/UMA.png");
		bool fontOk = titleFont.load(pathResources + "/Verdana.ttf", 32);

		if (imageOk) {
			logoUMA.resize(logoUMA.getWidth() / 13, logoUMA.getHeight() / 13);
		}
	}

	void draw(float x, float y, float width, float height) {
		ofPushMatrix();
		ofPushStyle();

		// Borde blanco
		ofNoFill();
		ofSetColor(255);
		ofSetLineWidth(1.0);
		ofDrawRectangle(x, y, width, height);

		if (width > 850) {
			if (titleFont.isLoaded()) {
				string title = "Ambisonic Player";
				ofSetColor(255);
				titleFont.drawString(title, x + width / 2 - titleFont.stringWidth(title) / 2, y + height / 2 + 10);
			}

			// Imagen
			if (logoUMA.isAllocated()) {
				ofSetColor(255);
				logoUMA.draw(width - x - logoUMA.getWidth(), y + (height - logoUMA.getHeight()) / 2);
			}
		} else{
			if (titleFont.isLoaded()) {
				string title = "AmbiPlayer";
				ofSetColor(255);
				titleFont.drawString(title, x + width / 2 - titleFont.stringWidth(title) / 2, y + height / 2 + 10);
			} 
		}

		ofPopStyle();
		ofPopMatrix();
	}

private:
	ofImage logoUMA;
	ofTrueTypeFont titleFont;
};

#endif
