/**
 * \class CLogo
 *
 * \brief Handles loading and rendering of application branding elements.
 *
 * \details This class is responsible for loading and displaying graphical
 * assets used in the application header, including institutional logos and
 * the main title text.
 *
 * It provides:
 *  - Loading of logo textures and font resources.
 *  - Scaled rendering of logos depending on available screen space.
 *  - Adaptive title rendering for wide and compact layouts.
 *  - Basic framed header decoration.
 *
 * \note Uses OpenFrameworks image and font rendering utilities.
 *
 * \note Resource paths are resolved relative to the application data folder.
 *
 * \date April 2026
 *
 * \author R. Liceras-Ramírez  
 * \b Tutor: A. Reyes-Lecuona and D. Gonzalez-Toledo  
 * \b Institution: Universidad de Málaga
 *
 * \b Context:
 * Developed as part of a Bachelor's Thesis (TFG), this class provides the
 * visual identity header for a real-time Ambisonic audio application.
 *
 * \b Licence:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 */

#ifndef _LOGO_HPP_
#define _LOGO_HPP_

#include "ofMain.h"

class CLogo {
public:
	CLogo() {
		string pathResources = ofToDataPath("../../../resources");

		bool imageOk = logoUMA.load(pathResources + "/UMA.png");
		bool imageDIANAOk = logoDIANA.load(pathResources + "/LogoDiana.png");
		bool fontOk = titleFont.load(pathResources + "/Verdana.ttf", 32);

		if (imageOk) {
			logoUMA.resize(logoUMA.getWidth() / 13, logoUMA.getHeight() / 13);
		}

		if (imageDIANAOk) {
			logoDIANA.resize(logoDIANA.getWidth() / 3, logoDIANA.getHeight() / 3);
		}
	}

	void draw(float x, float y, float width, float height) {
		ofPushMatrix();
		ofPushStyle();

		// Borde blanco
		ofNoFill();
		ofSetColor(255);
		ofSetLineWidth(1.0);
		ofDrawRectangle(x + margen, y + margen, width - 2 * margen, height - 2 * margen);

		if (width > 850) {
			if (titleFont.isLoaded()) {
				string title = "Ambisonic Player";
				ofSetColor(255);
				titleFont.drawString(title, x + 2 * margen + width / 2 - titleFont.stringWidth(title) / 2, y + margen + height / 2 );
			}

			// Imagen
			if (logoUMA.isAllocated()) {
				ofSetColor(255);
				logoUMA.draw(width - x -  2 * margen - logoUMA.getWidth(), y + (height - logoUMA.getHeight()) / 2);
			}

			if (logoDIANA.isAllocated()) {
				ofSetColor(255);
				logoDIANA.draw(x + 2 * margen + width / 4.5 - logoUMA.getWidth(), y + (height - logoDIANA.getHeight()) / 2);
			}
		} else{
			if (titleFont.isLoaded()) {
				string title = "AmbiPlayer";
				ofSetColor(255);
				titleFont.drawString(title, x + 2 * margen + width / 2 - titleFont.stringWidth(title) / 2, y + margen + height / 2);
			} 
		}

		ofPopStyle();
		ofPopMatrix();
	}

private:
	ofImage logoUMA, logoDIANA;
	ofTrueTypeFont titleFont;

	int margen = 10;
};

#endif
