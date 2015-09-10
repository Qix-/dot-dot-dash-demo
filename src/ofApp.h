#ifndef OF_APPLE_DDD_H__
#define OF_APPLE_DDD_H__
#pragma once

#include <vector>

#include "ofMain.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
private:
	void updateContours();

	void processPath(std::vector<ofPoint> &blob, ofPath &path);

	ofVideoGrabber grabber;

	ofxCvColorImage image;
	ofxCvGrayscaleImage imageGray;
	ofxCvGrayscaleImage imageBg;
	ofxCvGrayscaleImage imageDiff;

	ofxCvContourFinder contourFinder;

	int threshold;
	float simplification;
	bool learn;

	ofPath silhouettes;
	ofPath holes;

	ofPolyline simplifier;

	int bgR;
	int bgG;
	int bgB;

	bool debug;
};

#endif
