#include <string>
#include <iostream>
#include "ofApp.h"

using namespace std;

static const int IMG_SIZE_W = 320;
static const int IMG_SIZE_H = 240;
static const int THRESHOLD_INC = 1;
static const int MAX_BLOBS = 4; // Keep it around (number of people * 2)
static const float SIMPLIFICATION_INC = 0.1f;
static const float BASS_INC = 0.05f;
static const float BASS_INC_FINE = 0.01f;
static const int BUMP_DEBUG_W = 500;
static const int BUMP_DEBUG_H = 300;
static const float BUMP_DITHER_INC = 0.01f;

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

	this->findMin = 20;
	this->findMax = (IMG_SIZE_W * IMG_SIZE_H) / 3;

	this->learn = true;
	this->threshold = 75;
	this->simplification = 1.8f;

	this->silhouettes.setFillColor(ofColor::fromHex(0));
	this->silhouettes.setFilled(true);
	this->holes.setFilled(true);

	this->debug = false;
	this->debugSound = false;

	this->player.setLoop(true);
	this->bumpThreshold = 0.4;
	memset(&this->bands[0], 0, sizeof(this->bands));
	this->bump = 0.0f;
	this->dither = 0.14f;

	this->cooldownCount = 5;
	this->cooldown = 0;

	this->bg.setHsb(0, 161, 255);
	this->onBump(); // Initialize the background color.

	this->showHoles = false;

	this->frameDelay = 0;
	this->frame = 0;
}

void ofApp::update() {
	ofBackground(this->bg);

	this->grabber.update();
	bool newFrame = this->grabber.isFrameNew();

	if (newFrame && this->frame-- == 0) {
		this->frame = this->frameDelay;
		this->image.setFromPixels(this->grabber.getPixels());
		this->image.mirror(false, true);
		this->imageGray = this->image;

		if (this->learn) {
			cout << "re-learning background" << endl;
			this->learn = false;
			this->imageBg = this->imageGray;
		}

		this->imageDiff.absDiff(this->imageBg, this->imageGray);
		this->imageDiff.threshold(this->threshold);

		this->contourFinder.findContours(this->imageDiff, this->findMin,
			this->findMax, MAX_BLOBS, true);

		this->updateContours();
	}

	this->updateBump();
}

void ofApp::onBump() {
	this->bg.setHue(rand() % 360);
}

// make sure 0 <= x <= 1 or you'll get weird results.
float interpolate(float x, float y) {
	return (1.0f - pow(x, 3)) * y;
}

void ofApp::influenceBands(float *levels) {
	for (int i = 0; i < BUMP_BANDS; i++) {
		if (levels[i] > this->bands[i].level) {
			this->bands[i].level = levels[i];
			this->bands[i].rawLevel = levels[i];
			this->bands[i].cooldown = 0.0f;
		} else {
			if (this->bands[i].cooldown < 1.0f) {
				this->bands[i].cooldown += this->dither;
				this->bands[i].cooldown = min(1.0f, this->bands[i].cooldown);
				this->bands[i].level = interpolate(
					this->bands[i].cooldown,
					this->bands[i].rawLevel);
			}
		}
	}
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
			++enabled;
		}
	}

	if (!enabled) {
		return;
	}

	this->bump /= (float) enabled;

	if (this->bump >= this->bumpThreshold) {
		if (this->cooldown <= 0) {
			this->onBump();
			this->cooldown = this->cooldownCount;
		} else {
			--this->cooldown;
		}
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
	this->holes.setFillColor(this->bg);

	this->silhouettes.clear();
	this->holes.clear();

	vector<ofxCvBlob>::iterator bit = this->contourFinder.blobs.begin();
	while (bit != this->contourFinder.blobs.end()) {
		ofxCvBlob blob = *(bit++);

		if (blob.hole && !this->showHoles) {
			continue;
		}

		ofPath &path = blob.hole
			? this->holes
			: this->silhouettes;

		this->processPath(blob.pts, path);
		path.close();
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
	if (this->showHoles) {
		this->holes.draw(0, 0);
	}

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
		case 'h':
			this->showHoles = !this->showHoles;
			cout << "holes: " << this->showHoles << endl;
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
		case 'w':
			this->dither += BUMP_DITHER_INC;
			cout << "band dither: " << this->dither << endl;
			break;
		case 'q':
			this->dither -= BUMP_DITHER_INC;
			cout << "band dither: " << this->dither << endl;
			break;
		case '=': // +
			++this->cooldownCount;
			cout << "cooldown count: " << this->cooldownCount << endl;
			break;
		case '-':
			--this->cooldownCount;
			cout << "cooldown count: " << this->cooldownCount << endl;
			break;
		case 'v':
			++this->frameDelay;
			cout << "frame delay: " << this->frameDelay << endl;
			break;
		case 'c':
			--this->frameDelay;
			this->frameDelay = max(this->frameDelay, 0);
			cout << "frame delay: " << this->frameDelay << endl;
			break;
		case 'm':
			++this->findMin;
			cout << "this min contour size: " << this->findMin << endl;
			break;
		case 'n':
			--this->findMin;
			cout << "this min contour size: " << this->findMin << endl;
			break;
		case 'M':
			++this->findMax;
			cout << "this max contour size: " << this->findMax << endl;
			break;
		case 'N':
			--this->findMax;
			cout << "this max contour size: " << this->findMax << endl;
			break;
		case '1':
		case '2':  // and I'm free ...
		case '3':
		case '4':
		case '5':
		case '6':
		case '7': // ... free fallin'...
		case '8': // (yes, I understand this could be two conditionals.)
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
