#ifndef OF_APPLE_DDD_H__
#define OF_APPLE_DDD_H__
#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);

	void setThreshold(int threshold);
	int getThreshold() const;

private:
	ofVideoGrabber grabber;

	ofxCvColorImage image;
	ofxCvGrayscaleImage imageGray;
	ofxCvGrayscaleImage imageBg;
	ofxCvGrayscaleImage imageDiff;

	ofxCvContourFinder contourFinder;

	int threshold;
	bool learn;
};

#endif
