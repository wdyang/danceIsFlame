#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "MSAFluid.h"

//#include "MSATimer.h"
#include "ParticleSystem.h"

#include "ofMain.h"
#include "VideoSource.h"

#include "ofxOsc.h"
#define HOST "localhost"
#define PORT 12345
#define PORT_TO_GUI 12346
#define NUM_MSG_STRINGS 20

// comment this line out if you don't wanna use TUIO
// you will need ofxTUIO & ofxOsc
#define USE_TUIO		

// comment this line out if you don't wanna use the GUI
// you will need ofxSimpleGuiToo, ofxMSAInteractiveObject & ofxXmlSettings
// if you don't use the GUI, you won't be able to see the fluid parameters
#define USE_GUI		


#ifdef USE_TUIO
#include "ofxTuio.h"
#define tuioCursorSpeedMult				0.5	// the iphone screen is so small, easy to rack up huge velocities! need to scale down 
#define tuioStationaryForce				0.001f	// force exerted when cursor is stationary
#endif


#ifdef USE_GUI 
#include "ofxSimpleGuiToo.h"
#endif

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);

	void fadeToColor(float r, float g, float b, float speed);
	void addToFluid(ofVec2f pos, ofVec2f vel, bool addColor, bool addForce);

    
    
    float                   colorMult;
    float                   velocityMult;
	int                     fluidCellsX;
	bool                    resizeFluid;
	bool                    drawFluid;
	bool                    drawParticles;
	
	msa::fluid::Solver      fluidSolver;
	msa::fluid::DrawerGl	fluidDrawer;
	
	ParticleSystem          particleSystem;
	
	ofVec2f                 pMouse;
    
	// video section//
    videoSource		*	myVideo;
	int		videoFrameCnt;
	int		videoCaptured;
	int					camWidth;
	int					camHeight;
	ofVideoGrabber      vidGrabber;
	unsigned char * 	videoInverted;
	ofTexture			videoTexture;
	unsigned char *		videoMirrored;
	unsigned char *     previousFrame;
	unsigned char *     differenceFrame;
	ofTexture           differenceTexture;
	int pixelChange[240][320];
	int prevPixelChange[240][320];
	float videoColorMult;
	//video section end

	
#ifdef USE_TUIO
	ofxTuioClient tuioClient;
#endif	
    
    bool DrawOSCMessage;
    ofxOscReceiver receiver;
    ofxOscSender sender;
    int current_msg_string;
    string msg_strings[NUM_MSG_STRINGS];
    float timers[NUM_MSG_STRINGS];
	
    void split(vector<string> &tokens, const string &text, char sep);
    void sendSetting();
    void parseOSCMessage();
    void sendStringToGUI(const string &address, const string &msg);
    void sendFloatToGUI(const string &address, float msg);
    void sendIntToGUI(const string &address, int msg);
};



