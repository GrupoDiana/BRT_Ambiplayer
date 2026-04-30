/**
 * \class CLogo
 *
 * \brief Manages and renders application branding logos and title header.
 *
 * \date April 2026
 *
 * \author R. Liceras-Ramírez under the supervision of A. Reyes-Lecuona and
 * D. Gonzalez-Toledo (3DI-DIANA Research Group)
 *
 * \b Institution: Universidad de Málaga
 *
 * \b Context:
 * This class was developed as part of a Bachelor's Thesis (TFG) within a
 * real-time spatial audio visualization application. It provides a graphical
 * header used across the user interface to display project identity and
 * institutional branding.
 *
 * \b License:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * \b Copyright:
 * This library includes pieces of code from the BRT Library, shared under the GPLv3 license and copyrighted (c) by the University of Málaga
 *
 * \b Acknowledgement:
 * This project has been partially funded by the Spanish Ministerio de Ciencia e Innovación
 * under the SONIX project (PID2023-152547NB-I00).
 * https://www.diana.uma.es/sonix
 */

#ifndef _LOGO_HPP_
#define _LOGO_HPP_

#include "ofMain.h"

class CLogo {
public:
	CLogo() {
		string pathResources = ofToDataPath("resources", true);

		bool imageOk = logoUMA.load(pathResources + "/UMA.png");
		bool imageDIANAOk = logoDIANA.load(pathResources + "/LogoDiana.png");
		bool imageSONIXOk = logoSONIX.load(pathResources + "/LogoSONIX.png");
		bool fontOk = titleFont.load(pathResources + "/Verdana.ttf", 32);

		if (imageOk) {
			logoUMA.resize(logoUMA.getWidth() / 13, logoUMA.getHeight() / 13);
		}

		if (imageDIANAOk) {
			logoDIANA.resize(logoDIANA.getWidth() / 4, logoDIANA.getHeight() / 4);
		}

		if (imageSONIXOk) {
			logoSONIX.resize(logoSONIX.getWidth() / 4, logoSONIX.getHeight() / 4);
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

		if (width > 850 && width < 1500) {
			if (titleFont.isLoaded()) {
				string title = "BRT AmbiPlayer";
				ofSetColor(255);
				titleFont.drawString(title, x + 2 * margen + width / 2 - titleFont.stringWidth(title) / 2, y + margen + height / 2);
			}

			// Imagen
			if (logoUMA.isAllocated()) {
				ofSetColor(255);
				logoUMA.draw(width - x - 2 * margen - logoUMA.getWidth(), y + (height - logoUMA.getHeight()) / 2);
			}

			if (logoDIANA.isAllocated()) {
				ofSetColor(255);
				logoDIANA.draw(x + 2 * margen + width / 4.5 - logoUMA.getWidth(), y + (height - logoDIANA.getHeight()) / 2);
			}

			if (logoSONIX.isAllocated()) {
				ofSetColor(255);
				logoSONIX.draw(x + 2 * margen + width / 4.5 - 1.2 * logoUMA.getWidth() + logoSONIX.getWidth(), y + (height - 0.9 * logoDIANA.getHeight()) / 2);
			}
		} else if (width < 850) {
			if (titleFont.isLoaded()) {
				string title = "BAMBI";
				ofSetColor(255);
				titleFont.drawString(title, x + 2 * margen + width / 2 - titleFont.stringWidth(title) / 2, y + margen + height / 2);
			}
		} else if (width > 1500) {
			if (titleFont.isLoaded()) {
				string title = "BRT AmbiPlayer";
				ofSetColor(255);
				titleFont.drawString(title, x + 2 * margen + width / 2 - titleFont.stringWidth(title) / 2, y + margen + height / 2);
			}

			if (logoUMA.isAllocated()) {
				ofSetColor(255);
				logoUMA.draw(width - x - 2 * margen - logoUMA.getWidth(), y + (height - logoUMA.getHeight()) / 2);
			}

			if (logoDIANA.isAllocated()) {
				ofSetColor(255);
				logoDIANA.draw(x + 2 * margen + width / 4.5 - 1.8 * logoUMA.getWidth(), y + (height - logoDIANA.getHeight()) / 2);
			}

			if (logoSONIX.isAllocated()) {
				ofSetColor(255);
				logoSONIX.draw(x + 2 * margen + width / 4.5 - 1.8 * logoUMA.getWidth() + logoSONIX.getWidth(), y + (height - 0.9 * logoDIANA.getHeight()) / 2);
			}
		}

		ofPopStyle();
		ofPopMatrix();
	}

private:
	ofImage logoUMA, logoDIANA, logoSONIX;
	ofTrueTypeFont titleFont;

	int margen = 10;
};

#endif
