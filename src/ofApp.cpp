#include <iostream>
#include <vector>
#include "ofApp.h"

using namespace std;

static const int IMG_SIZE_W = 320;
static const int IMG_SIZE_H = 240;
static const int CONTOUR_SIZE = 20;
static const int THRESHOLD_INC = 1;
static const float SIMPLIFICATION_INC = 0.1f;

void ofApp::setup() {
	this->grabber.setVerbose(true);
	this->grabber.initGrabber(IMG_SIZE_W, IMG_SIZE_H);

	cout << "asked for grabber size of "
		<< IMG_SIZE_W << "x" << IMG_SIZE_H
		<< ", got " << this->grabber.getWidth() << "x"
		<< this->grabber.getHeight()
		<< endl;

	this->image.allocate(IMG_SIZE_W, IMG_SIZE_H);
	this->imageGray.allocate(IMG_SIZE_W, IMG_SIZE_H);
	this->imageBg.allocate(IMG_SIZE_W, IMG_SIZE_H);
	this->imageDiff.allocate(IMG_SIZE_W, IMG_SIZE_H);

	this->learn = true;
	this->threshold = 85;

	this->bgR = 124;
	this->bgG = 188;
	this->bgB = 6;

	this->silhouettes.setFillColor(ofColor::fromHex(0));
	this->silhouettes.setFilled(true);
	this->holes.setFilled(true);
}

void ofApp::update() {
	ofBackground(this->bgR, this->bgG, this->bgB);

	this->grabber.update();
	bool newFrame = this->grabber.isFrameNew();

	if (newFrame) {
		this->image.setFromPixels(this->grabber.getPixels());
		this->imageGray = this->image;

		if (this->learn) {
			cout << "re-learning background" << endl;
			this->learn = false;
			this->imageBg = this->imageGray;
		}

		this->imageDiff.absDiff(this->imageBg, this->imageGray);
		this->imageDiff.threshold(this->threshold);

		this->contourFinder.findContours(this->imageDiff, CONTOUR_SIZE,
			(IMG_SIZE_W * IMG_SIZE_H) / 3, 10, true);

		this->updateContours();
	}
}

void ofApp::updateContours() {
	this->holes.setFillColor(ofColor(this->bgR, this->bgG, this->bgB));

	this->silhouettes.clear();
	this->holes.clear();

	vector<ofxCvBlob>::iterator bit = this->contourFinder.blobs.begin();
	while (bit != this->contourFinder.blobs.end()) {
		ofxCvBlob blob = *bit;

		ofPath &path = blob.hole ? this->holes : this->silhouettes;

		vector<ofPoint>::iterator pit = blob.pts.begin();
		bool first = true;
		while (pit != blob.pts.end()) {
			ofPoint pt = *pit;

			if (first) {
				first = false;
				path.moveTo(pt);
			} else {
				path.curveTo(pt);
			}

			++pit;
		}

		path.close();

		++bit;
	}

	this->silhouettes.simplify(this->simplification);
	this->holes.simplify(this->simplification);
}

void ofApp::draw() {
	ofSetHexColor(0xFFFFFF);
	this->image.draw(0, IMG_SIZE_H);
	this->imageDiff.draw(0, 0);
	this->contourFinder.draw(0, 0);

	this->silhouettes.draw(IMG_SIZE_W, 0);
	//this->holes.draw(IMG_SIZE_W, 0);
}

void ofApp::keyPressed(int key) {
	switch (key) {
		case ' ':
			this->learn = true;
			break;
		case '.':
			this->simplification += SIMPLIFICATION_INC;
			cout << "simplification: " << this->simplification << endl;
			break;
		case ',':
			this->simplification -= SIMPLIFICATION_INC;
			cout << "simplification: " << this->simplification << endl;
			break;
		case ']':
			this->threshold += THRESHOLD_INC;
			cout << "threshold: " << this->threshold << endl;
			break;
		case '[':
			this->threshold -= THRESHOLD_INC;
			cout << "threshold: " << this->threshold << endl;
			break;
	}
}
