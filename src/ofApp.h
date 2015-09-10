#ifndef OF_APPLE_DDD_H__
#define OF_APPLE_DDD_H__
#pragma once

#include <vector>

#include "ofMain.h"
#include "ofxOpenCv.h"

#define BUMP_BANDS 18

struct Band {
	float level;
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
	int influenceBands(float *bands);
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
