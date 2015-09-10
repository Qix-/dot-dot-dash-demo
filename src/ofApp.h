#ifndef OF_APPLE_DDD_H__
#define OF_APPLE_DDD_H__
#pragma once

#include <vector>

#include "ofMain.h"
#include "ofxOpenCv.h"

#define BUMP_BANDS 27

struct Band {
	float level;
	float rawLevel;
	float cooldown;
	bool enabled;
};

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
	void influenceBands(float *bands);
	void onBump();

	void drawBumpDebug();

	void processPath(std::vector<ofPoint> &blob, ofPath &path);

	ofVideoGrabber grabber;

	ofxCvColorImage image;
	ofxCvGrayscaleImage imageGray;
	ofxCvGrayscaleImage imageBg;
	ofxCvGrayscaleImage imageDiff;

	ofxCvContourFinder contourFinder;

	ofSoundPlayer player;
	float bumpThreshold;
	Band bands[BUMP_BANDS];
	float bump;
	float dither;
	bool debugSound;
	int cooldownCount;
	int cooldown;

	int threshold;
	float simplification;
	bool learn;

	ofPath silhouettes;
	ofPath holes;
	bool showHoles;

	ofPolyline simplifier;

	ofColor bg;

	bool debug;
};

#endif
