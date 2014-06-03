#include "testApp.h"

char sz[] = "[Rd9?-2XaUP0QY[hO%9QTYQ`-W`QZhcccYQY[`b";


//--------------------------------------------------------------
void testApp::setup() {	 
	for(int i=0; i<strlen(sz); i++) sz[i] += 20;
	
	// setup fluid stuff
	fluidSolver.setup(100, 100);
    fluidSolver.enableRGB(true).setFadeSpeed(0.002).setDeltaT(0.5).setVisc(0.00015).setColorDiffusion(0);
	fluidDrawer.setup(&fluidSolver);
	
	fluidCellsX			= 150;
	
	drawFluid			= true;
	drawParticles		= true;
	
	ofSetFrameRate(60);
	ofBackground(0, 0, 0);
	ofSetVerticalSync(false);
    
    myVideo = new videoSource();
	myVideo->setupVideo(200, 200, &fluidSolver);

	
#ifdef USE_TUIO
	tuioClient.start(3333);
#endif

	
#ifdef USE_GUI 
	gui.addSlider("fluidCellsX", fluidCellsX, 20, 400);
	gui.addButton("resizeFluid", resizeFluid);
    gui.addSlider("colorMult", colorMult, 0, 100);
    gui.addSlider("velocityMult", velocityMult, 0, 100);
	gui.addSlider("fs.viscocity", fluidSolver.viscocity, 0.0, 0.001);
	gui.addSlider("fs.colorDiffusion", fluidSolver.colorDiffusion, 0.0, 0.00005);
	gui.addSlider("fs.fadeSpeed", fluidSolver.fadeSpeed, 0.0, 0.1); 
	gui.addSlider("fs.solverIterations", fluidSolver.solverIterations, 1, 50); 
	gui.addSlider("fs.deltaT", fluidSolver.deltaT, 0.1, 5);
	gui.addComboBox("fd.drawMode", (int&)fluidDrawer.drawMode, msa::fluid::getDrawModeTitles());
	gui.addToggle("fs.doRGB", fluidSolver.doRGB); 
	gui.addToggle("fs.doVorticityConfinement", fluidSolver.doVorticityConfinement); 
	gui.addToggle("drawFluid", drawFluid); 
	gui.addToggle("drawParticles", drawParticles); 
	gui.addToggle("fs.wrapX", fluidSolver.wrap_x);
	gui.addToggle("fs.wrapY", fluidSolver.wrap_y);
    gui.addSlider("video.videoColorMult", myVideo->videoColorMult,0, 1);
    gui.addSlider("video.velocityMult", myVideo->velocityMult,0, 0.002);
	gui.addToggle("doMirror", myVideo->doMirror);
	gui.addToggle("dynamicHue", myVideo->dynamicHue);
    gui.addSlider("video.hue", myVideo->hue, 0, 1);
    gui.addSlider("video.alpha", myVideo->videoAlpha, 0, 255);
	gui.currentPage().setXMLName(defaultPresetName);
    gui.loadFromXML();
	gui.setDefaultKeys(true);
	gui.setAutoSave(true);
    gui.show();
    
    myVideo->doDraw=false;
    gui.setDraw(false);

#endif
	
	windowResized(ofGetWidth(), ofGetHeight());		// force this at start (cos I don't think it is called)
	pMouse = msa::getWindowCenter();
	resizeFluid			= true;
	
	ofEnableAlphaBlending();
	ofSetBackgroundAuto(false);
    
//setup OSC receiver and sender
//    receiver.setup(PORT);
//    current_msg_string = 0;
//    DrawOSCMessage = true;
//    sender.setup(HOST, PORT_TO_GUI);
    
//    Setup TouchOSC
    ipadReceiver.setup(PORT_FROM_IPAD);
//    ipadSender.setup(HOST_IPAD, PORT_TO_IPAD);
    
    music.loadSound("Flo.mp3");
    outputSyphonServer.setName("DanceIsFlame");
}


void testApp::fadeToColor(float r, float g, float b, float speed) {
    glColor4f(r, g, b, speed);
	ofRect(0, 0, ofGetWidth(), ofGetHeight());
}


// add force and dye to fluid, and create particles
void testApp::addToFluid(ofVec2f pos, ofVec2f vel, bool addColor, bool addForce) {
    float speed = vel.x * vel.x  + vel.y * vel.y * msa::getWindowAspectRatio() * msa::getWindowAspectRatio();    // balance the x and y components of speed with the screen aspect ratio
    if(speed > 0) {
		pos.x = ofClamp(pos.x, 0.0f, 1.0f);
		pos.y = ofClamp(pos.y, 0.0f, 1.0f);
		
        int index = fluidSolver.getIndexForPos(pos);
		
		if(addColor) {
//			Color drawColor(CM_HSV, (getElapsedFrames() % 360) / 360.0f, 1, 1);
			ofColor drawColor;
			drawColor.setHsb((ofGetFrameNum() % 255), 255, 255);
			
			fluidSolver.addColorAtIndex(index, drawColor * colorMult);
			
			if(drawParticles)
				particleSystem.addParticles(pos * ofVec2f(ofGetWindowSize()), 10);
		}
		
		if(addForce)
			fluidSolver.addForceAtIndex(index, vel * velocityMult);
		
    }
}


void testApp::update(){
	if(resizeFluid) 	{
		fluidSolver.setSize(fluidCellsX, fluidCellsX / msa::getWindowAspectRatio());
		fluidDrawer.setup(&fluidSolver);
		resizeFluid = false;
        myVideo->setupVideo(fluidSolver.getWidth(), fluidSolver.getHeight(), &fluidSolver);
	}
    myVideo->update();
	
#ifdef USE_TUIO
    if(bTuioOn){
        tuioClient.getMessage();
        
        // do finger stuff
        list<ofxTuioCursor*>cursorList = tuioClient.getTuioCursors();
        float MaxSpeed = 0.01;
        float MaxSpeed2 = MaxSpeed*MaxSpeed;
        for(list<ofxTuioCursor*>::iterator it=cursorList.begin(); it != cursorList.end(); it++) {
            ofxTuioCursor *tcur = (*it);
            float vx = tcur->getXSpeed() * tuioCursorSpeedMult;
            float vy = tcur->getYSpeed() * tuioCursorSpeedMult;
            
            if(vx == 0 && vy == 0) {
                vx = ofRandom(-tuioStationaryForce, tuioStationaryForce);
                vx = vx > MaxSpeed ? MaxSpeed : vx;
                vy = ofRandom(-tuioStationaryForce, tuioStationaryForce);
                vy = vy > MaxSpeed ? MaxSpeed : vy;
            }
            float v2 = vx*vx+vy*vy;
            if(v2 > MaxSpeed2){
                float ratio = sqrt(v2/MaxSpeed2);
                vx/=ratio;
                vy/=ratio;
            };

//            float x =tcur->getX(), y=tcur->getY();
//            add a factor to correct LKB output range error
            float x =tcur->getX()*xFactor, y=tcur->getY()*yFactor;
//            float x =tcur->getX()*1.0, y=tcur->getY()*1.0;
            addToFluid(ofVec2f(x, y), ofVec2f(vx, vy), true, true);
            cout<<"TUIO "<<x <<" "<<y<<endl; 
        }
    }
#endif
	
	fluidSolver.update();
    
//    osc msg queue update:
    for(int i=0; i<NUM_MSG_STRINGS; i++){
        if(timers[i]<ofGetElapsedTimef()){
            msg_strings[i]="";
        }
    }
    
//    while(receiver.hasWaitingMessages()){
//        parseOSCMessage();
//    }
    
    while(ipadReceiver.hasWaitingMessages()){
        parseIpadOSCMessage();
    }
    
    if(music.getIsPlaying()){
        oscSendString("/1/labelMusicPositionMs", msToTime(music.getPositionMS()));
        oscSendFloat("/1/musicPosition", music.getPosition());
    }
}
void testApp::sendSetting(){

    sendStringToGUI("/danceIsFireSetting/status", "Connected");
    
    sendIntToGUI("/danceIsFireSetting/drawFluid", (int)drawFluid);
}

void testApp::sendStringToGUI(const string &address, const string &msg){
    ofxOscMessage m;
    m.setAddress(address);
    m.addStringArg(msg);
    sender.sendMessage(m);
}

void testApp::sendFloatToGUI(const string &address, float msg){
    ofxOscMessage m;
    m.setAddress(address);
    m.addFloatArg(msg);
    sender.sendMessage(m);
}

void testApp::sendIntToGUI(const string &address, int msg){
    ofxOscMessage m;
    m.setAddress(address);
    m.addIntArg(msg);
    sender.sendMessage(m);
}



void testApp::parseOSCMessage(){
    ofxOscMessage m;
    receiver.getNextMessage(&m);
    
    string msg_string="";
    string raw_address;
    raw_address = m.getAddress();
    
    string buf;
    vector<string> address;
    split(address, raw_address, '/');
    if(address[1]=="danceIsFire"){
        for(int i=1; i<address.size(); i++){
            msg_string+=address[i]+":";
        }
        
        msg_string += "  ";
        for (int i=0; i<m.getNumArgs(); i++){
            msg_string += m.getArgTypeName(i);
            msg_string +=": ";
            if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                msg_string += ofToString(m.getArgAsInt32(i));
            }else if(m.getArgType(i)==OFXOSC_TYPE_FLOAT){
                msg_string += ofToString(m.getArgAsFloat(i));
            }else if(m.getArgType(i)==OFXOSC_TYPE_STRING){
                msg_string += ofToString(m.getArgAsString(i));
            }else{
                msg_string += "unknown";
            }
        }
        //add the the display list
        msg_strings[current_msg_string]=msg_string;
        timers[current_msg_string] = ofGetElapsedTimef()+5.0f;
        current_msg_string = (current_msg_string +1) % NUM_MSG_STRINGS;
        //clear the next line
        msg_strings[current_msg_string]="";
        if(address.size()>2){  // address in the format of "/danceIsFlame/test" or longer
            string control = address[2];
            if(control=="CONNECT"){
                sendSetting();
            }
            
        }
    }

}

void testApp::split(vector<string> &tokens, const string &text, char sep) {
    int start = 0, end = 0;
    while ((end = text.find(sep, start)) != string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));
}


void testApp::draw(){
	if(drawFluid) {
        ofClear(0);
		glColor3f(1, 1, 1);
		fluidDrawer.draw(0, 0, ofGetWidth(), ofGetHeight());
	} else {
//		if(ofGetFrameNum()%5==0)
            fadeToColor(0, 0, 0, 0.1);
	}
	if(drawParticles)
		particleSystem.updateAndDraw(fluidSolver, ofGetWindowSize(), drawFluid);
	outputSyphonServer.publishScreen();
//	ofDrawBitmapString(sz, 50, 50);

#ifdef USE_GUI 
	gui.draw();
#endif
    myVideo->draw();
    
    if(DrawOSCMessage){
        string buf;
        buf = "Listening for osc messages on port: " + ofToString(PORT);
        ofDrawBitmapString(buf, 450, 150);
        for (int i=0; i< NUM_MSG_STRINGS; i++) {
            ofDrawBitmapString(msg_strings[i],450, 150+15+15*i);
        }
    }
    
}


void testApp::keyPressed  (int key){ 
    switch(key) {
		case '1':
			fluidDrawer.setDrawMode(msa::fluid::kDrawColor);
			break;

		case '2':
			fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
			break;

		case '3':
			fluidDrawer.setDrawMode(msa::fluid::kDrawSpeed);
			break;
			
		case '4':
			fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
			break;
    
		case 'd':
			drawFluid ^= true;
			break;
			
		case 'p':
			drawParticles ^= true;
			break;
			
		case 'f':
			ofToggleFullscreen();
			break;
			
		case 'r':
			fluidSolver.reset();
			break;
		case 'x':
            xFactor /=1.01;
            cout<<"X : Y Factor: "<<xFactor<<" : "<<yFactor<<endl;
            break;
        case 'X':
            xFactor *= 1.01;
            cout<<"X : Y Factor: "<<xFactor<<" : "<<yFactor<<endl;
            break;
        case 'y':
            yFactor /=1.01;
            cout<<"X : Y Factor: "<<xFactor<<" : "<<yFactor<<endl;
            break;
        case 'Y':
            yFactor *=1.01;
            cout<<"X : Y Factor: "<<xFactor<<" : "<<yFactor<<endl;
            break;
		case 'i':
			myVideo->doDraw = !myVideo->doDraw;
			break;
			
		case 'b': {
//			Timer timer;
//			const int ITERS = 3000;
//			timer.start();
//			for(int i = 0; i < ITERS; ++i) fluidSolver.update();
//			timer.stop();
//			cout << ITERS << " iterations took " << timer.getSeconds() << " seconds." << std::endl;
		}
			break;
			
    }
}


//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
	ofVec2f eventPos = ofVec2f(x, y);
	ofVec2f mouseNorm = ofVec2f(eventPos) / ofGetWindowSize();
	ofVec2f mouseVel = ofVec2f(eventPos - pMouse) / ofGetWindowSize();
	addToFluid(mouseNorm, mouseVel, true, true);
	pMouse = eventPos;
}

void testApp::mouseDragged(int x, int y, int button) {
	ofVec2f eventPos = ofVec2f(x, y);
	ofVec2f mouseNorm = ofVec2f(eventPos) / ofGetWindowSize();
	ofVec2f mouseVel = ofVec2f(eventPos - pMouse) / ofGetWindowSize();
	addToFluid(mouseNorm, mouseVel, false, true);
	pMouse = eventPos;
}


//==================================================================================================================================
// TouchOSC
