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

		/* ofPushStyle();
		ofNoFill();
		ofSetColor(255);
		ofSetLineWidth(1.0);
		ofDrawRectangle(cx, cy, width, height);
		ofPopStyle();*/

		ofTranslate(cx, cy);

		// dibujamos el lóbulo en coordenadas locales (0,0)
		DrawDirectionalLobeProjected(energies, 0, 0, maxRadius, plane);

		ofPopMatrix();
	}

private:
	void DrawDirectionalLobeProjected(
		const std::vector<float> & energies,
		float cx,
		float cy,
		float maxRadius,
		Plane plane) {
		if (energies.empty()) return;

		ofPushStyle();
		ofNoFill();
		ofSetLineWidth(2);
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
					px = cx + x3D * maxRadius;
					py = cy - y3D * maxRadius;
					break;
				case YZ:
					px = cx + y3D * maxRadius;
					py = cy - z3D * maxRadius;
					break;
				case XZ:
					px = cx + x3D * maxRadius;
					py = cy - z3D * maxRadius;
					break;
				}

				ofVertex(px, py);
			}
		}

		ofEndShape(true);
		ofPopStyle();
	}


	int numAz = 72; // resolución angular
	int numEl = 36;
};

#endif
