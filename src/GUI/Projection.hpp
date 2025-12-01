#ifndef _PROJECTION_HPP_
#define _PROJECTION_HPP_

#include "ofMain.h"
#include <cmath>
#include <vector>

enum Plane { XY,
	YZ,
	XZ };

class CProjection {
public:
	CProjection(int numAzimuth = 72, int numElevation = 36)
		: numAz(numAzimuth)
		, numEl(numElevation) { }

	void draw(const std::vector<float> & energies,
		float cx, float cy, float width, float height,
		Plane plane) {
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

		DrawDirectionalLobeProjectedPoints(energies, maxRadius, plane);

		ofPopMatrix();
	}

private:
	int margen = 10;
	int numAz;
	int numEl;

	void DrawDirectionalLobeProjectedPoints(
		const std::vector<float> & energies,
		float maxRadius,
		Plane plane) {
		if (energies.empty()) return;

		float maxVal = 0.0f;
		for (float e : energies)
			if (e > maxVal) maxVal = e;
		if (maxVal == 0.0f) maxVal = 1.0f;

		ofPushStyle();
		ofNoFill();
		ofSetLineWidth(1.0);

		for (int ia = 0; ia < numAz; ++ia) {
			float az = TWO_PI * ia / numAz;

			for (int ie = 0; ie < numEl; ++ie) {
				float el = -HALF_PI + ie * (PI / (numEl - 1));
				float r = sqrtf(energies[ia * numEl + ie] / maxVal);

				// 1) dirección unitaria
				glm::vec3 dir = glm::vec3(cos(el) * cos(az),
					cos(el) * sin(az),
					sin(el)); // X+ dentro, Y+ izquierda, Z+ arriba

				// 2) aplicar escala r
				glm::vec3 pos3D = dir * r;

				float px = 0, py = 0;

				// 3) proyectar al plano
				switch (plane) {
				case XY:
					px = pos3D.y * maxRadius;
					py = -pos3D.x * maxRadius;
					break;
				case YZ:
					px = pos3D.y * maxRadius;
					py = -pos3D.z * maxRadius;
					break;
				case XZ:
					px = pos3D.x * maxRadius;
					py = -pos3D.z * maxRadius;
					break;
				}

				ofDrawCircle(px, py, 1.5f);
			}
		}

		ofPopStyle();
	}

	void DrawAxes(float x0, float y0, float size, Plane plane) {
		ofPushStyle();
		ofSetLineWidth(1.0);

		switch (plane) {
		case XY:
			ofSetColor(255, 0, 0);
			ofDrawLine(x0, y0, x0, y0 - size);
			ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);
			ofSetColor(0, 255, 0);
			ofDrawLine(x0, y0, x0 - size, y0);
			ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 - size + 5, y0 + 3);
			break;
		case YZ:
			ofSetColor(0, 255, 0);
			ofDrawLine(x0, y0, x0 - size, y0);
			ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 - size + 5, y0 + 3);
			ofSetColor(0, 0, 255);
			ofDrawLine(x0, y0, x0, y0 - size);
			ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);
			break;
		case XZ:
			ofSetColor(255, 0, 0);
			ofDrawLine(x0, y0, x0 - size, y0);
			ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 - size + 5, y0 + 3);
			ofSetColor(0, 0, 255);
			ofDrawLine(x0, y0, x0, y0 - size);
			ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);
			break;
		}

		ofPopStyle();
	}
};

#endif
