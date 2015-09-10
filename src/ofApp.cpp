#include <string>
#include <iostream>
#include "ofApp.h"

using namespace std;

static const int IMG_SIZE_W = 320;
static const int IMG_SIZE_H = 240;
static const int CONTOUR_SIZE = 20;
static const int THRESHOLD_INC = 1;
static const float SIMPLIFICATION_INC = 0.1f;
static const float BASS_INC = 0.05f;
static const float BASS_INC_FINE = 0.01f;
static const int BUMP_COOLDOWN = 15;
static const int BUMP_DEBUG_W = 500;
static const int BUMP_DEBUG_H = 300;

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

	this->debug = false;
	this->debugSound = false;

	this->player.setLoop(true);
	this->bumpThreshold = 0.7;
	memset(&this->bands[0], 0, sizeof(this->bands));
	this->bump = 0.0f;
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

	this->updateBump();
}

static int c = 0; /* XXX DEBUG */
void ofApp::onBump() {
	cout << "\x1b[36;1mBUMP\x1b[0m" << c++ << endl;
}

int ofApp::influenceBands(float *levels) {

}

void ofApp::updateBump() {
	float *levels = ofSoundGetSpectrum(BUMP_BANDS);
	if (!levels) {
		// OpenFrameworks has already warned that it's not implemented
		//  at this point.
		//  see: ofSoundPlayer.cpp
		return;
	}

	this->influenceBands(levels);

	int enabled = 0;
	this->bump = 0.0f;
	for (int i = 0; i < BUMP_BANDS; i++) {
		if (this->bands[i].enabled) {
			this->bump += this->bands[i].level;
		}
	}

	if (!enabled) {
		return;
	}

	this->bump /= (float) enabled;

	if (this->bump >= this->bumpThreshold) {
		this->onBump();
	}
}

void ofApp::processPath(std::vector<ofPoint> &blob, ofPath &path) {
	// first, we use a PolyLine to simplify the blob points
	this->simplifier.clear();
	this->simplifier.addVertices(blob);
	this->simplifier.close();
	this->simplifier.simplify(this->simplification);

	// then, we re-curve them so they don't look like origami
	std::vector<ofPoint> &vertices = this->simplifier.getVertices();
	std::vector<ofPoint>::iterator itr = vertices.begin();
	bool first = true;
	while (itr != vertices.end()) {
		ofPoint p = *itr;
		if (first) {
			first = false;
			path.moveTo(p);
		} else {
			path.curveTo(p);
		}
		++itr;
	}
}

void ofApp::updateContours() {
	this->holes.setFillColor(ofColor(this->bgR, this->bgG, this->bgB));

	this->silhouettes.clear();
	this->holes.clear();

	vector<ofxCvBlob>::iterator bit = this->contourFinder.blobs.begin();
	while (bit != this->contourFinder.blobs.end()) {
		ofxCvBlob blob = *bit;

		ofPath &path = blob.hole
			? this->holes
			: this->silhouettes;

		this->processPath(blob.pts, path);
		path.close();

		++bit;
	}

	float scaleFactorX = ofGetWidth() / IMG_SIZE_W;
	float scaleFactorY = ofGetHeight() / IMG_SIZE_H;

	this->silhouettes.scale(scaleFactorX, scaleFactorY);
	this->holes.scale(scaleFactorX, scaleFactorY);
}

void ofApp::drawBumpDebug() {
	int height = ofGetHeight();
	int barWidth = BUMP_DEBUG_W / BUMP_BANDS;

	if (this->bump >= this->bumpThreshold) {
		ofSetHexColor(0x00FFFF);
	} else {
		ofSetHexColor(0xFF0000);
	}

	ofDrawRectangle(
		0,
		height - (BUMP_DEBUG_H * this->bump),
		BUMP_DEBUG_W,
		BUMP_DEBUG_H);

	for (int i = 0; i < BUMP_BANDS; i++) {
		if (this->bands[i].enabled) {
			ofSetHexColor(0xFFFFFF);
		} else {
			ofSetHexColor(0);
		}

		ofDrawRectangle(
			i * barWidth,
			height - (BUMP_DEBUG_H * this->bands[i].level),
			barWidth,
			BUMP_DEBUG_H);
	}

	ofSetHexColor(0xFF00FF);
	ofDrawRectangle(
		0,
		height - (BUMP_DEBUG_H * this->bumpThreshold),
		BUMP_DEBUG_W,
		1);
}

void ofApp::draw() {
	this->silhouettes.draw(0, 0);
	this->holes.draw(0, 0);

	if (this->debug) {
		ofSetHexColor(0xFFFFFF);
		this->image.draw(0, IMG_SIZE_H);
		this->imageDiff.draw(0, 0);
		this->contourFinder.draw(0, 0);
	}

	if (this->debugSound) {
		this->drawBumpDebug();
	}
}

void ofApp::keyPressed(int key) {
	// yes, there is definitely a better way to do this.
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
		case 'd':
			this->debug = !this->debug;
			cout << "debug: " << this->debug << endl;
			break;
		case 's':
			this->debugSound = !this->debugSound;
			cout << "debug sound: " << this->debugSound << endl;
			break;
		case 'f':
			ofToggleFullscreen();
			break;
		case 'x':
			this->bumpThreshold += BASS_INC;
			cout << "bass threshold: " << this->bumpThreshold << endl;
			break;
		case 'z':
			this->bumpThreshold -= BASS_INC;
			cout << "bass threshold: " << this->bumpThreshold << endl;
			break;
		case 'X':
			this->bumpThreshold += BASS_INC_FINE;
			cout << "bass threshold: " << this->bumpThreshold << endl;
			break;
		case 'Z':
			this->bumpThreshold -= BASS_INC_FINE;
			cout << "bass threshold: " << this->bumpThreshold << endl;
			break;
		case '1':
		case '2':  // and I'm free ...
		case '3':
		case '4':
		case '5':
		case '6':
		case '7': // ... free fallin'...
		case '8':
		case '9':
			this->bands[key - '1'].enabled = !this->bands[key - '1'].enabled;
			break;
		case '!':
			this->bands[9].enabled = !this->bands[9].enabled;
			break;
		case '@':
			this->bands[10].enabled = !this->bands[10].enabled;
			break;
		case '#':
			this->bands[11].enabled = !this->bands[11].enabled;
			break;
		case '$':
			this->bands[12].enabled = !this->bands[12].enabled;
			break;
		case '%':
			this->bands[13].enabled = !this->bands[13].enabled;
			break;
		case '^':
			this->bands[14].enabled = !this->bands[14].enabled;
			break;
		case '&':
			this->bands[15].enabled = !this->bands[15].enabled;
			break;
		case '*':
			this->bands[16].enabled = !this->bands[16].enabled;
			break;
		case '(':
			this->bands[17].enabled = !this->bands[17].enabled;
			break;
	}
}

void ofApp::dragEvent(ofDragInfo info) {
	// currently only support one file at a time.
	if (info.files.size() != 1) {
		cerr << "only one file is allowed at a time!" << endl;
		return;
	}

	string filename = info.files[0];
	cout << "starting to play: " << filename << endl;

	this->player.stop();
	this->player.loadSound(filename, true);
	this->player.play();
}
