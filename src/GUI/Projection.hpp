#ifndef _PROJECTION_HPP_
#define _PROJECTION_HPP_

#include "ofMain.h"

#include "ofApp.h"
#include <string>

enum Plane { XY,
	YZ,
	XZ };

class CProjection {
public:
	CProjection() { }

	void draw(const std::vector<float> & energies,
		float cx, float cy, float width, float height, Plane plane) {

		float maxRadius = std::min(width, height) / 2.0f;

		ofPushMatrix();

		ofPushStyle();
		ofNoFill();
		ofSetColor(255);
		ofSetLineWidth(1.0);
		ofDrawRectangle(cx, cy , width - margen , height - margen);
		ofPopStyle();

		float axisSize = std::min(width, height) * 0.15f;
		float x0 = cx + width - margen - axisSize;
		float y0 = cy + height - margen - axisSize/2;
		DrawAxes(x0, y0, axisSize, plane);

		ofTranslate(cx + width / 2.0f, cy + height / 2.0f);

		// dibujamos el lóbulo en coordenadas locales (0,0)
		DrawDirectionalLobeProjected(energies, 0, 0, maxRadius, plane);

		ofPopMatrix();
	}

private:
	int margen = 10;

	void DrawDirectionalLobeProjected(
		const std::vector<float> & energies,
		float cx,
		float cy,
		float maxRadius,
		Plane plane) {
		 
		if (energies.empty()) return;

		ofPushStyle();
		ofNoFill();
		ofSetLineWidth(1.0);
		ofBeginShape();

		// recorremos todos los puntos del lóbulo
		for (int ia = 0; ia < numAz; ++ia) {
			for (int ie = 0; ie < numEl; ++ie) {
				float val = energies[ia * numEl + ie];
				float r = std::sqrt(val);

				float az = 2.0f * 3.1415 * ia / numAz;
				float el = 1.57 * (ie / float(numEl - 1) * 2.0f - 1.0f); // elevación [-pi/2, pi/2]

				// coordenadas 3D del lóbulo
				float x3D = r * cos(el) * cos(az);
				float y3D = r * cos(el) * sin(az);
				float z3D = r * sin(el);

				float px, py;

				// proyectamos según el plano
				switch (plane) {
				case XY:
					px = cx - y3D * maxRadius; 
					py = cy - x3D * maxRadius; 
					break;
				case YZ:
					px = cx - y3D * maxRadius; 
					py = cy - z3D * maxRadius; 
					break;
				case XZ:
					px = cx - x3D * maxRadius; 
					py = cy - z3D * maxRadius; 
					break;

				}

				ofVertex(px, py);
			}
		}

		ofEndShape(true);
		ofPopStyle();
	}

void DrawAxes(float x0, float y0, float size, Plane plane) {
		ofPushStyle();
		ofSetLineWidth(1.0);

		switch (plane) {

		case XY:
			ofSetColor(255, 0, 0);
			ofDrawLine(x0, y0, x0, y0 - size);
			ofDrawTriangle(x0, y0 - size,x0 - 3, y0 - size + 5,x0 + 3, y0 - size + 5);

			ofSetColor(0, 255, 0);
			ofDrawLine(x0, y0, x0 - size, y0); 
			ofDrawTriangle(x0 - size, y0,x0 - size + 5, y0 - 3,x0 - size + 5, y0 + 3);
			break;

		case YZ:
			ofSetColor(0, 255, 0);
			ofDrawLine(x0, y0, x0 - size, y0);
			ofDrawTriangle(x0 - size, y0,x0 - size + 5, y0 - 3,x0 - size + 5, y0 + 3);

			ofSetColor(0, 0, 255);
			ofDrawLine(x0, y0, x0, y0 - size); 
			ofDrawTriangle(x0, y0 - size,x0 - 3, y0 - size + 5,x0 + 3, y0 - size + 5);
			break;


		case XZ:
			ofSetColor(255, 0, 0);
			ofDrawLine(x0, y0, x0 - size, y0);
			ofDrawTriangle(x0 - size, y0, x0 - size + 5, y0 - 3, x0 + size + 5, y0 + 3);

			ofSetColor(0, 0, 255);
			ofDrawLine(x0, y0, x0, y0 - size);
			ofDrawTriangle(x0, y0 - size, x0 - 3, y0 - size + 5, x0 + 3, y0 - size + 5);
			break;
		}

		ofPopStyle();
	}



	int numAz = 72; // resolución angular
	int numEl = 36;
};

#endif
