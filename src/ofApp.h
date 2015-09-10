#ifndef OF_APPLE_DDD_H__
#define OF_APPLE_DDD_H__
#pragma once

#include <vector>

#include "ofMain.h"
#include "ofxOpenCv.h"

#define BUMP_BANDS 18

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

	void dragEvent(ofDragInfo info);

	void keyPressed(int key);
private:
	void updateContours();
	void updateBump();
	void bump();

	void drawBumpDebug();

	void processPath(std::vector<ofPoint> &blob, ofPath &path);

	ofVideoGrabber grabber;

	ofxCvColorImage image;
	ofxCvGrayscaleImage imageGray;
	ofxCvGrayscaleImage imageBg;
	ofxCvGrayscaleImage imageDiff;

	ofxCvContourFinder contourFinder;

	ofSoundPlayer player;
	int bumpCooldown;
	float bumpThreshold;
	bool bands[BUMP_BANDS];
	float bumpDebug;
	float *levelsDebug;
	bool debugSound;

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
