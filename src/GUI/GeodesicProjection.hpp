/**
 * \class CGeodesicProjection
 *
 * \brief Visualizes spatial energy distribution using a geodesic projection.
 *
 * \details This class renders a 3D-to-2D geodesic projection of spatial energy
 * values over a predefined set of directional vertices. It is typically used in
 * Ambisonic or spatial audio analysis systems to represent energy distribution
 * across a spherical sampling grid.
 *
 * The visualization converts spherical coordinates (azimuth/elevation) into
 * Cartesian projections and scales them according to energy magnitude,
 * producing a spatial representation of directional intensity.
 *
 * \note Energy values are normalized and mapped using a logarithmic (dB-like)
 * scale before projection.
 *
 * \date April 2026
 *
 * \author R. Liceras-Ramírez under the supervision of A. Reyes-Lecuona and D. Gonzalez-Toledo (3DI-DIANA Research Group)
 * \b Institution: Universidad de Málaga
 *
 * \b Context:
 * Developed as part of a Bachelor's Thesis (TFG), this class is intended for
 * real-time spatial audio visualization and geodesic Ambisonic analysis tools.
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
 * This project has also been partially funded by the Spanish Ministerio de Ciencia e Innovación under the SONIX project (PID2023-152547NB-I00). https://www.diana.uma.es/sonix
 */

#ifndef _GEODESIC_PROJECTION_HPP_
#define _GEODESIC_PROJECTION_HPP_

#include "ofMain.h"
#include <cmath>
#include <vector>

enum Plane { XY,
	ZY,
	ZX };

class CGeodesicProjection {
public:
	CGeodesicProjection(int numAzimuth = 72, int numElevation = 36)
		: numAz(numAzimuth)
		, numEl(numElevation) { }

	void draw(const std::vector<float> & energies,
		float cx, float cy, float width, float height,
		Plane plane, const std::vector<Common::CVector3> & vertices) {
		float maxRadius = std::min(width, height) / 2.5f;

		ofPushMatrix();

		ofPushStyle();
		ofNoFill();
		ofSetColor(255);
		ofSetLineWidth(1.0);
		ofDrawRectangle(cx, cy, width - margen, height - margen);
		ofPopStyle();

		float axisSize = std::min(width, height) * 0.15f;
		float x0 = cx + width - margen - axisSize;
		float y0 = cy + height - margen - axisSize / 2;
		DrawAxes(x0, y0, axisSize, plane);

		ofTranslate(cx + width / 2.0f, cy + height / 2.0f);

		DrawDirectionalLobeProjectedPointsGeodesic(energies, maxRadius, plane, vertices);

		ofPopMatrix();
	}

private:
	int margen = 10;
	int numAz;
	int numEl;

void DrawDirectionalLobeProjectedPointsGeodesic(
		const std::vector<float> & energies,
		float maxRadius,
		Plane plane,
		const std::vector<Common::CVector3> & vertices) {
		if (energies.empty()) return;

		const float minDb = -80.0f;
		const float maxDb = 0.0f;

		float maxVal = 0.0f;
		for (float e : energies)
			if (e > maxVal) maxVal = e;
		if (maxVal <= 0.0f) maxVal = 1e-12f;

		float frameMaxDb = 20.0f * log10(maxVal);
		float targetScale = ofMap(frameMaxDb, minDb, maxDb, 0.0f, 1.0f, true);

		static float smoothedScale = 0.0f;
		const float alpha = 0.05f;
		smoothedScale = ofLerp(smoothedScale, targetScale, alpha);
		float globalScale = smoothedScale;

		ofPushStyle();
		ofNoFill();
		ofSetLineWidth(1.0f);

		for (int i = 0; i < vertices.size(); ++i) {

			float val = std::max(energies[i], 0.0f);
			float rShape = std::sqrt(val / maxVal);

			float r = rShape * globalScale;

			float az = ofDegToRad(CartesianToSpherical(vertices[i]).x);
			float el = ofDegToRad(CartesianToSpherical(vertices[i]).y);

			float x3D = r * cos(el) * cos(az);
			float y3D = r * cos(el) * sin(az);
			float z3D = r * sin(el);

			float px, py;
			if (plane == ZY) {
				px = -y3D * maxRadius;
				py = -z3D * maxRadius;
			} else if (plane == XY) {
				px = -y3D * maxRadius;
				py = -x3D * maxRadius;
			} else {
				px = -x3D * maxRadius;
				py = -z3D * maxRadius;
			}

			ofDrawCircle(px, py, 1.5f);
		}

		ofPopStyle();
	}

	void DrawGeodesicVertices(const std::vector<Common::CVector3> & vertices, float scale = 200.0f) {
		glPointSize(5.0f);
		glBegin(GL_POINTS);
		glColor3f(1.0f, 0.2f, 0.2f);
		for (const auto & v : vertices) {
			glVertex3f(v.x * scale, v.y * scale, v.z * scale);
		}
		glEnd();
	}

	void DrawAxes(float x0, float y0, float size, Plane plane) {
		ofPushStyle();
		ofSetLineWidth(1.0);

		switch (plane) {
		case XY:
			ofSetColor(255, 255, 255);
			ofDrawBitmapString("X",x0-4,y0-size-4);
			ofSetColor(255, 0, 0);
			ofDrawLine(x0, y0, x0, y0 - size);
			ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);
			ofSetColor(255, 255, 255);
			ofDrawBitmapString("Y", x0 - size - 8, y0 + 4);
			ofSetColor(0, 255, 0);
			ofDrawLine(x0, y0, x0 - size, y0);
			ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 - size + 5, y0 + 3);
			break;
		case ZY:
			ofSetColor(255, 255, 255);
			ofDrawBitmapString("Z", x0 - 4, y0 - size - 4);
			ofSetColor(0, 255, 0);
			ofDrawLine(x0, y0, x0 - size, y0);
			ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 - size + 5, y0 + 3);
			ofSetColor(255, 255, 255);
			ofDrawBitmapString("Y", x0 - size - 8, y0 + 4);
			ofSetColor(0, 0, 255);
			ofDrawLine(x0, y0, x0, y0 - size);
			ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);
			break;
		case ZX:
			ofSetColor(255, 255, 255);
			ofDrawBitmapString("Z", x0 - 4, y0 - size - 4);
			ofSetColor(255, 0, 0);
			ofDrawLine(x0, y0, x0 - size, y0);
			ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 - size + 5, y0 + 3);
			ofSetColor(255, 255, 255);
			ofDrawBitmapString("X", x0 - size - 8, y0 + 4);
			ofSetColor(0, 0, 255);
			ofDrawLine(x0, y0, x0, y0 - size);
			ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);
			break;
		}

		ofPopStyle();
	}

	Common::CVector3 CartesianToSpherical(const Common::CVector3 & cartesian) {
		float x = cartesian.x;
		float y = cartesian.y;
		float z = cartesian.z;

		float radius = sqrt(x * x + y * y + z * z);

		float azim = atan2(y, x);
		float elev = atan2(z, sqrt(x * x + y * y));

		float azimDeg = ofRadToDeg(azim);
		float elevDeg = ofRadToDeg(elev);

		return Common::CVector3(azimDeg, elevDeg, radius);
	}
};

#endif
