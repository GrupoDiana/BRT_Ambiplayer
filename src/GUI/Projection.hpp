#ifndef _PROJECTION_HPP_
#define _PROJECTION_HPP_

#include "ofMain.h"
#include <cmath>
#include <vector>

enum Plane { XY,
	ZY,
	ZX };

class CProjection {
public:
	CProjection(int numAzimuth = 72, int numElevation = 36)
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

		//DrawDirectionalLobeProjectedPoints(energies, maxRadius, plane);
		//DrawGeodesicVertices(vertices, maxRadius);
		DrawDirectionalLobeProjectedPointsGeodesic(energies, maxRadius, plane, vertices);

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
			for (int ie = 0; ie < numEl; ++ie) {
				float val = energies[ia * numEl + ie];
				float r = std::sqrt(val / maxVal);

				float az = TWO_PI * ia / numAz;
				float el = -HALF_PI + ie * (PI / (numEl - 1));

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
				} else { // XZ
					px = -x3D * maxRadius;
					py = -z3D * maxRadius;
				}

				ofDrawCircle(px, py, 1.5f);
			}
		}

		ofPopStyle();
	}

	void DrawDirectionalLobeProjectedPointsGeodesic(
		const std::vector<float> & energies,
		float maxRadius,
		Plane plane, const std::vector<Common::CVector3> & vertices) {
		if (energies.empty()) return;

		float maxVal = 0.0f;
		for (float e : energies)
			if (e > maxVal) maxVal = e;
		if (maxVal == 0.0f) maxVal = 1.0f;

		ofPushStyle();
		ofNoFill();
		ofSetLineWidth(1.0);

		for (int i = 0; i < vertices.size(); ++i) {
				float val = energies[i];
				float r = std::sqrt(val / maxVal);

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
				} else { // XZ
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
