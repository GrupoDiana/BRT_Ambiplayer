/**
 * \class CCircularProjection
 *
 * \brief Renders a circular energy-based projection visualization.
 *
 * \details This class generates a 2D circular projection used to visualize
 * spatial energy distributions. It draws a deformable radial shape based on
 * input energy values, typically used in spatial audio or ambisonics systems.
 *
 * \date April 2026
 *
 * \author R. Liceras-Ramírez under the supervision of A. Reyes-Lecuona and D. Gonzalez-Toledo
 * (3DI-DIANA Research Group)
 *
 * \b Institution: Universidad de Málaga
 *
 * \b Context:
 * This class was developed as part of a Bachelor's Thesis (TFG)
 * within a real-time spatial audio visualization system based on energy
 * distribution across azimuthal angles.
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

#ifndef _CIRCULAR_PROJECTION_HPP_
#define _CIRCULAR_PROJECTION_HPP_

#include "ofMain.h"
#include <cmath>
#include <vector>

class CCircularProjection {
public:
	CCircularProjection(int numAngles = 20)
		: numPoints(numAngles) { }

	void draw(const std::vector<float> & energies,
		float cx, float cy,
		float width, float height,
		Plane plane,
		const std::vector<Common::CVector3> & /*vertices*/) {
		if (energies.empty()) return;

		float maxRadius = std::min(width, height) / 2.5f;

		ofPushMatrix();

		// Marco
		ofPushStyle();
		ofNoFill();
		ofSetColor(255);
		ofSetLineWidth(1.0f);
		ofDrawRectangle(cx, cy, width - margin, height - margin);
		ofPopStyle();

		// Ejes
		float axisSize = std::min(width, height) * 0.15f;
		float x0 = cx + width - margin - axisSize;
		float y0 = cy + height - margin - axisSize / 2;
		DrawAxes(x0, y0, axisSize, plane);

		// Centro
		ofTranslate(cx + width / 2.0f, cy + height / 2.0f);

		// --- Curva deformada por energías (rosa) ---
		DrawCircularEnergy(energies, maxRadius, plane);

		// --- Círculo base gris ---
		DrawBaseCircle(maxRadius);

		ofPopMatrix();
	}

private:
	int margin = 10;
	int numPoints;

	void DrawBaseCircle(float maxRadius) {
		float rBase = maxRadius * 0.5f;

		ofPushStyle();
		ofFill();
		ofSetColor(60); //Color gris de fondo
		ofDrawCircle(0, 0, rBase);
		ofPopStyle();

		ofPushStyle();
		ofNoFill();
		ofSetColor(150);
		ofSetLineWidth(2.0f);
		ofDrawCircle(0, 0, rBase);
		ofPopStyle();
	}

	void DrawCircularEnergy(const std::vector<float> & energies,
		float maxRadius,
		Plane plane) {
		if (energies.empty()) return;

		const float minDb = -80.0f;
		const float maxDb = 0.0f;

		float maxVal =0.5f;

		float frameMaxDb = 20.0f * log10(maxVal);
		float targetScale = ofMap(frameMaxDb, minDb, maxDb, 0.0f, 1.0f, true);

		static float smoothedScale = 0.0f;
		const float alpha = 0.05f;
		smoothedScale = ofLerp(smoothedScale, targetScale, alpha);
		float globalScale = smoothedScale;

		float rBase = maxRadius * 0.5f;
		int N = energies.size();

		ImVec4 defaultColor = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabActive);

		ofColor haloFill(
			defaultColor.x * 255.f,
			defaultColor.y * 255.f,
			defaultColor.z * 255.f,
			defaultColor.w * 100.f);

		ofColor haloStroke(
			defaultColor.x * 255.f,
			defaultColor.y * 255.f,
			defaultColor.z * 255.f,
			255.f);

		ofPushStyle();

		ofFill();
		ofSetColor(haloFill);

		ofBeginShape();
		for (int i = 0; i < N + 3; ++i) {
			int idx = i % N;
			float theta = TWO_PI * idx / N;
			float normE = energies[idx] / maxVal;

			float deformation = sqrt(normE) * globalScale;
			float r = rBase + (maxRadius - rBase) * deformation;

			float px, py;
			switch (plane) {
			case XY:
				px = -sinf(theta);
				py = -cosf(theta);
				break;

			case ZY:
				px = -cosf(theta);
				py = -sinf(theta);
				break;

			case ZX:
				px = -cosf(theta);
				py = -sinf(theta);
				break;
			}

			ofCurveVertex(px * r, py * r);
		}
		ofEndShape(true);

		ofNoFill();
		ofSetColor(haloStroke);
		ofSetLineWidth(3.0f);

		ofBeginShape();
		for (int i = 0; i < N + 3; ++i) {
			int idx = i % N;
			float theta = TWO_PI * idx / N;
			float normE = energies[idx] / maxVal;

			float deformation = sqrt(normE) * globalScale;
			float r = rBase + (maxRadius - rBase) * deformation;

			float px, py;
			switch (plane) {
			case XY:
				px = -sinf(theta);
				py = -cosf(theta);
				break;

			case ZY:
				px = -cosf(theta);
				py = -sinf(theta);
				break;

			case ZX:
				px = -cosf(theta);
				py = -sinf(theta);
				break;
			}

			ofCurveVertex(px * r, py * r);
		}
		ofEndShape(true);

		ofPopStyle();
	}

	void AngleToPlaneXY(float theta, float & x, float & y) {
		x = -sinf(theta); // -Y
		y = -cosf(theta); // -X
	}

	void AngleToPlaneZY(float theta, float & x, float & y) {
		y = -sinf(theta); // -Y
		x = -cosf(theta); // -Z
	}

	void AngleToPlaneZX(float theta, float & x, float & y) {
		x = -cosf(theta); // -X
		y = -sinf(theta); // -Z
	}

	void DrawAxes(float x0, float y0, float size, Plane plane) {
		ofPushStyle();
		ofSetLineWidth(1.0f);

		switch (plane) {
		case XY:
			DrawAxis(x0, y0, size, "X", "Y", ofColor(255, 0, 0), ofColor(0, 255, 0));
			break;
		case ZY:
			DrawAxis(x0, y0, size, "Z", "Y", ofColor(0, 0, 255), ofColor(0, 255, 0));
			break;
		case ZX:
			DrawAxis(x0, y0, size, "Z", "X", ofColor(0, 0, 255), ofColor(255, 0, 0));
			break;
		}

		ofPopStyle();
	}

	void DrawAxis(float x0, float y0, float size,
		const std::string & verticalLabel,
		const std::string & horizontalLabel,
		const ofColor & vColor,
		const ofColor & hColor) {
		// Vertical
		ofSetColor(255);
		ofDrawBitmapString(verticalLabel, x0 - 4, y0 - size - 4);
		ofSetColor(vColor);
		ofDrawLine(x0, y0, x0, y0 - size);
		ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);

		// Horizontal
		ofSetColor(255);
		ofDrawBitmapString(horizontalLabel, x0 - size - 8, y0 + 4);
		ofSetColor(hColor);
		ofDrawLine(x0, y0, x0 - size, y0);
		ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 - size + 5, y0 + 3);
	}
};

#endif
