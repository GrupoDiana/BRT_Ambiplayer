#ifndef _LOUDSPEAKERS_HPP_
#define _LOUDSPEAKERS_HPP_

#include "ofMain.h"

#include "ofApp.h"
#include <string>

class CLoudspeakers {
public:
	CLoudspeakers() { }

	void draw(float cx, float cy, float width, float height) {

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

		ofTranslate(cx + width / 2.0f, cy + height / 2.0f);

		ofPopMatrix();
	}

private:
	int margen = 10;
};

#endif
