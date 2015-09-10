#include "ofMain.h"
#include "ofApp.h"

int main() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
	return 0;
}
