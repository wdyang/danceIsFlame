#include "touchOsc.h"


void testApp::parseIpadOSCMessage(){
    ofxOscMessage m;
    ipadReceiver.getNextMessage(&m);
    
    if(!bSenderLive){
        ipadIP = m.getRemoteIp();
        ipadSender.setup(ipadIP, PORT_TO_IPAD);
    }
    
    string msg_string="";
    string raw_address;
    
    raw_address = m.getAddress();

    if(raw_address=="/1/connect"){
        int val = m.getArgAsInt32(0);
        cout<<"connect request received: "<<val<<endl;
        if(val==0){
            oscSendInitConfig();
        }
    }else if(raw_address=="/1/toggleFullScreen"){
        int val = m.getArgAsInt32(0);
        if(val==0) ofToggleFullscreen();
    }else if(raw_address=="/1/drawFluid"){
        drawFluid = m.getArgAsInt32(0);
    }else if(raw_address.find("/1/drawMode/1/")!=string::npos){  //   "/1/drawMode/1/1"

        char id_ch = raw_address[raw_address.length()-1];
        int id = id_ch - '0';
        switch (id) {
            case 1:
                fluidDrawer.setDrawMode(msa::fluid::kDrawColor);
                break;
            case 2:
                fluidDrawer.setDrawMode(msa::fluid::kDrawMotion);
                break;
            case 3:
                fluidDrawer.setDrawMode(msa::fluid::kDrawSpeed);
                break;
            case 4:
                fluidDrawer.setDrawMode(msa::fluid::kDrawVectors);
                break;
            default:
                break;
        }
        oscSendInt("/1/drawMode/1/1", 0);
        oscSendInt("/1/drawMode/1/2", 0);
        oscSendInt("/1/drawMode/1/3", 0);
        oscSendInt("/1/drawMode/1/4", 0);
        oscSendInt(raw_address, true);
    }else if(raw_address=="/1/doRGB"){
        fluidSolver.doRGB = m.getArgAsInt32(0);
    }else if(raw_address=="/1/doVorticityConfine"){
        fluidSolver.doVorticityConfinement = m.getArgAsInt32(0);
    }else if(raw_address=="/1/drawParticles"){
        drawParticles = m.getArgAsInt32(0);
    }else if(raw_address=="/1/wrapX"){
        fluidSolver.wrap_x = m.getArgAsInt32(0);
    }else if(raw_address=="/1/wrapY"){
        fluidSolver.wrap_y = m.getArgAsInt32(0);
    }else if(raw_address=="/1/videoColorMult"){
        myVideo->videoColorMult = m.getArgAsFloat(0);
    }else if(raw_address=="/1/videoVelocityMult"){
        myVideo->velocityMult = m.getArgAsFloat(0);
    }else if(raw_address=="/1/vMax"){
        particleSystem.setVMax(m.getArgAsFloat(0));
        oscSendFloat("/1/labelVMax", particleSystem.getVMax());
    }else if(raw_address=="/1/bTuioOn"){
        bTuioOn = m.getArgAsInt32(0);
        oscSendInt("/1/bTuioOn", bTuioOn);
    }else if(raw_address=="/1/viscocity"){
        fluidSolver.viscocity = m.getArgAsFloat(0);
        oscSendFormatedFloat("/1/labelViscocity", fluidSolver.viscocity, 5);
// Music player
// Add musicPosition, music msToTime() in testApp.h
    }else if(raw_address=="/1/musicVolume"){
        music.setVolume(m.getArgAsFloat(0));
        oscSendFloat("/1/labelVolume", music.getVolume());
    }else if(raw_address=="/1/musicPlaying"){
        bool playing = m.getArgAsInt32(0);
        if(playing){
            music.play();
            music.setPosition(musicPosition);
        }else{
            musicPosition = music.getPosition();
            music.stop();
        }
    }else if(raw_address=="/1/musicReset"){
        if(m.getArgAsInt32(0)==0){
            musicPosition = 0.f;
            music.setPosition(musicPosition);
            oscSendFloat("/1/musicPosition", 0.f);
            oscSendFloat("/1/labelMusicPositionMs", 0.f);
        }
    }else if(raw_address=="/1/musicPosition"){
        musicPosition = m.getArgAsFloat(0);
        music.setPosition(musicPosition);
        oscSendFloat("/1/MusicPosition", musicPosition);
        oscSendString("/1/labelMusicPositionMs", msToTime(music.getPositionMS()));
// End Music Player
    }else if(raw_address=="/1/saveState"){
        int val = m.getArgAsInt32(0);
        if(val==0) {
            if(bSavePresetVisible){
                oscSendInt("/1/savePreset/visible", false);
                oscSendString("/1/labelSaveState", "Show Save");
            }else{
                oscSendInt("/1/savePreset/visible", true);
                oscSendString("/1/labelSaveState", "Hide Save");
            }
            bSavePresetVisible = !bSavePresetVisible;
        }
    }else if(raw_address.find("/1/savePreset/1/")!=string::npos){
        char id_ch = raw_address[raw_address.length()-1];
        int id = id_ch - '0';
        id--; //touchOsc is from 1 up, C++ is from 0 up
        cout<<"saving to present with id "<< id<<endl;
        saveToXml(id);
    }else if(raw_address.find("/1/loadPreset/1/")!=string::npos){
        char id_ch = raw_address[raw_address.length()-1];
        int id = id_ch - '0';
        id--; //touchOsc is from 1 up, C++ is from 0 up
        cout<<"Load from  present with id "<< id<<endl;
        loadFromXml(id);
        oscSendInitConfig();
        for(int i=0; i<NumPreset; i++){
            char address[24];
            sprintf(address, "/1/labelPreset%i/color", i+1);
            if(i==id){
                oscSendString(address, "green");
            }else{
                oscSendString(address, "yellow");
            }
        }
    }else{
        cout<<"not handled: "<<raw_address<<endl;
    }

}

void testApp::oscSendInitConfig(){
    oscSendString("/1/connect/color", "green");
    oscSendInt("/1/drawFluid", drawFluid);
    oscSendInt("/1/drawMode/1/1", (fluidDrawer.getDrawMode() == msa::fluid::kDrawColor));
    oscSendInt("/1/drawMode/1/2", (fluidDrawer.getDrawMode() == msa::fluid::kDrawMotion));
    oscSendInt("/1/drawMode/1/3", (fluidDrawer.getDrawMode() == msa::fluid::kDrawSpeed));
    oscSendInt("/1/drawMode/1/4", (fluidDrawer.getDrawMode() == msa::fluid::kDrawVectors));
    oscSendInt("/1/doRGB", fluidSolver.doRGB);
    oscSendInt("/1/doVorticityConfine", fluidSolver.doVorticityConfinement);
    oscSendInt("/1/drawParticles", drawParticles);
    oscSendInt("/1/wrapX", fluidSolver.wrap_x);
    oscSendInt("/1/warpY", fluidSolver.wrap_y);
    
    oscSendFloat("/1/videoColorMult", myVideo->videoColorMult);
    oscSendFloat("/1/videoVelocityMult", myVideo->velocityMult);
    
    oscSendFloat("/1/vMax", particleSystem.getVMax());
    oscSendFloat("/1/labelVMax", particleSystem.getVMax());
    oscSendFloat("/1/viscocity", fluidSolver.viscocity);
    oscSendFormatedFloat("/1/labelViscocity", fluidSolver.viscocity, 5);
    
    oscSendFloat("/1/musicVolume", music.getVolume());
    oscSendFloat("/1/labelVolume", music.getVolume());
    
    oscSendInt("/1/bTuioOn", bTuioOn);
    
    for(int i=0; i<NumPreset; i++){
        char address[24];
        sprintf(address, "/1/labelPreset%i", i+1);
        oscSendString(address, presetName[i]);
    }
    oscSendInt("/1/savePreset/visible", false);
    oscSendString("/1/labelSaveState", "Show Save");
    
}

void testApp::oscSendString(const string &address, const string &msg){
    ofxOscMessage m;
    m.setAddress(address);
    m.addStringArg(msg);
    ipadSender.sendMessage(m);
}

void testApp::oscSendFloat(const string &address, float msg){
    ofxOscMessage m;
    m.setAddress(address);
    m.addFloatArg(msg);
    ipadSender.sendMessage(m);
}

void testApp::oscSendFormatedFloat(const string &address, float msg, int precision){
    char st[24], formatst[20];
    sprintf(formatst, "%%.%df", precision);
    sprintf(st, formatst, msg);
    cout<<formatst<<" "<<st<<endl;
    ofxOscMessage m;
    m.setAddress(address);
    m.addStringArg(st);
    ipadSender.sendMessage(m);
}


void testApp::oscSendInt(const string &address, int msg){
    ofxOscMessage m;
    m.setAddress(address);
    m.addIntArg(msg);
    ipadSender.sendMessage(m);
}

void testApp::saveToXml(int i){
    string fname = presetName[i]+".xml";
    gui.setAutoSave(false);
    gui.currentPage().setXMLName(fname);
    gui.saveToXML();
    gui.currentPage().setXMLName(defaultPresetName);
    gui.setAutoSave(true);
}

void testApp::loadFromXml(int i){
    string fname = presetName[i]+".xml";
    gui.setAutoSave(false);
    gui.currentPage().setXMLName(fname);
    gui.loadFromXML();
    gui.currentPage().setXMLName(defaultPresetName);
    gui.setAutoSave(true);
}


string testApp::msToTime(int ms){
    
    int sec = ms/1000;
    std::ostringstream s;
    s<<sec/60<<":"<<sec%60;
    
    return s.str();
}
