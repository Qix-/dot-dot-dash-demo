#include "ofApp.h"

static const int IMG_SIZE_W = 320;
static const int IMG_SIZE_H = 240;
static const int CONTOUR_SIZE = 20;

void ofApp::setup() {
	this->grabber.setVerbose(true);
	this->grabber.setup(IMG_SIZE_W, IMG_SIZE_H);

	this->image.allocate(IMG_SIZE_W, IMG_SIZE_H);
	this->imageGray.allocate(IMG_SIZE_W, IMG_SIZE_H);
	this->imageBg.allocate(IMG_SIZE_W, IMG_SIZE_H);
	this->imageDiff.allocate(IMG_SIZE_W, IMG_SIZE_H);

	this->learn = true;
	this->threshold = 85;

	this->bgR = 124;
	this->bgG = 188;
	this->bgB = 6;
}

void ofApp::update() {
	ofBackground(this->bgR, this->bgG, this->bgB);

	bool newFrame = this->grabber.isFrameNew();

	if (newFrame) {
		this->image.setFromPixels(this->grabber.getPixels());
		this->imageGray = this->image;

		if (this->learn) {
			this->learn = false;
			this->imageBg = this->imageGray;
		}

		this->imageDiff.absDiff(this->imageBg, this->imageGray);
		this->imageDiff.threshold(this->threshold);

		this->contourFinder.findContours(this->imageDiff, CONTOUR_SIZE,
			(IMG_SIZE_W * IMG_SIZE_H) / 3, 10, true);
	}
}

void ofApp::draw() {
	ofSetHexColor(0xFFFFFF);
	this->image.draw(20,20);
	this->imageGray.draw(360,20);
	this->imageBg.draw(20,280);
	this->imageDiff.draw(360,280);

	ofFill();
	ofSetHexColor(0x333333);
	ofDrawRectangle(360, 540, IMG_SIZE_W, IMG_SIZE_H);
	ofSetHexColor(0xFFFFFF);

	this->contourFinder.draw(360, 540);
}

void ofApp::keyPressed(int key) {

}

void setThreshold(int threshold) {
	this->threshold = threshold;
}

int getThreshold() const {
	return this->threshold;
}
