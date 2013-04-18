/*
 *  VideoSource.cpp
 *  MSAFluid
 *
 *  Created by Weidong Yang on 10/29/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "VideoSource.h"

void videoSource::setupVideo(int NX, int NY, msa::fluid::Solver  * fluidSolver0)
{
	
	fluidSolver = fluidSolver0;
	fluidWidth = NX;
	fluidHeight = NY;
	
	if (4*fluidHeight>3*fluidWidth){
		camHeight=fluidHeight;
		camWidth=(4*camHeight)/3;
		
		camMapX0=(camWidth-fluidWidth)/2;
		camMapY0=0;
		camMapXend=camMapX0+fluidWidth-1; 
		camMapYend=camHeight-1;
	}else{
		camWidth=fluidWidth;
		camHeight=(3*camWidth)/4;
		
		camMapX0=0;
		camMapY0=(camHeight-fluidHeight)/2;
		camMapXend=camWidth-1; 
		camMapYend=camMapY0+fluidHeight-1;
	}
	
	numPixels = (fluidWidth) * (fluidHeight);
	
//	destroy();
	videoFrameCnt=1; videoCaptured=1;
	vidGrabber.setVerbose(true);
    vidGrabber.setDeviceID(0);
	vidGrabber.initGrabber(camWidth,camHeight);

	videoTexture.allocate(fluidWidth, fluidHeight, GL_RGB);
	videoMapped = new unsigned char[numPixels*3];
	videoMirrored = new unsigned char[numPixels*3];
	previousFrame = new unsigned char[numPixels*3];
	differenceFrame = new unsigned char[numPixels*3];
	
	pixelChange = new int[numPixels];
	prevPixelChange = new int[numPixels];
//	videoColorMult = 0.1;
//    hue = 0;
//	velocityMult = 0.0001;
//	videoAlpha = 127;
//    doMirror = false;
	
	
	
//	ofxMSAFluidSolver *	fluidSolver;
}

void videoSource::destroy(){
	
	if(videoMapped){
		videoTexture.clear();
		delete [] videoMapped;
		delete [] videoMirrored;
		delete [] previousFrame;
		delete [] differenceFrame;
		delete [] pixelChange;
		delete [] prevPixelChange;
		
		vidGrabber.close();
	}
}

void videoSource::update(){
	doRGB=fluidSolver->doRGB;
	
	videoFrameCnt++;
	if(videoFrameCnt>600){
		videoFrameCnt=1; videoCaptured=0;
	}
//	vidGrabber.grabFrame();
	vidGrabber.update();
	if (vidGrabber.isFrameNew()){
		videoCaptured++;
        if(dynamicHue){
            hue+=0.01;
            if (hue>1) hue=0;
        }
		
		unsigned char * pixels = vidGrabber.getPixels();
		int iy=0;
		int ix=0;
		for (int iy0=camMapY0; iy0<camMapYend; iy0++){
			ix=0;
			for (int ix0=camMapX0; ix0<camMapXend; ix0++){
				videoMapped[Fluid_IX(ix, iy)*3]=pixels[Cam_IX(ix0, iy0)*3];
				videoMapped[Fluid_IX(ix, iy)*3+1]=pixels[Cam_IX(ix0, iy0)*3+1];
				videoMapped[Fluid_IX(ix, iy)*3+2]=pixels[Cam_IX(ix0, iy0)*3+2];
				ix++;
			}
			iy++;
		}
//flip video horizontally
        for (int iy=0; iy<fluidHeight; iy++){
            int idxin=3*iy*fluidWidth;
            int idxmirror = 3*(iy+1)*fluidWidth -3;
            for (int ix=0; ix<fluidWidth; ix++){
                for(int icolor=0; icolor<3; icolor++)
                    videoMirrored[idxmirror+icolor]=pixels[idxin+icolor];
                idxin+=3; idxmirror-=3;
            }
        }
//get video change
		float totalChange=0;

        if(doMirror) {
            for (int i = 0; i < numPixels*3; i++){
                differenceFrame[i]=abs(videoMirrored[i]-previousFrame[i]);
                totalChange+=differenceFrame[i];
                previousFrame[i]=videoMirrored[i];
            }
        }else{
            for (int i = 0; i < numPixels*3; i++){
                differenceFrame[i]=abs(pixels[i]-previousFrame[i]);
                totalChange+=differenceFrame[i];
                previousFrame[i]=pixels[i];
            }
		}
		for (int i=0; i<numPixels; i++){
				pixelChange[i] = 0;
				for(int icolor=0; icolor<3; icolor++)
					pixelChange[i]+=differenceFrame[i*3+icolor];
		}
		

	}
	
}

void videoSource::draw(){
	videoToColor();
	

	
	if(doDraw){
	//	vidGrabber.draw(200+fluidWidth,200, camWidth, camHeight);
		
		char tempStr[255];
		sprintf(tempStr, "cam %d %d fluid %d %d %d %d FrameCapture rate is %f with Frame rate %f",camWidth, camHeight, fluidWidth, fluidHeight, camMapY0, camMapYend, videoCaptured/(1.0*videoFrameCnt), ofGetFrameRate());
		ofDrawBitmapString(tempStr, 50,ofGetHeight()-50);
	}
	ofEnableAlphaBlending();
	ofSetColor(255, 255, 255, int(videoAlpha));
	videoTexture.loadData(videoMirrored, fluidWidth, fluidHeight, GL_RGB);
//	videoTexture.loadData(differenceFrame, fluidWidth, fluidHeight, GL_RGB);
	//videoTexture.draw(200,200, fluidWidth, fluidHeight);
//	videoTexture.draw(0,0, ofGetWidth(), ofGetHeight());
	videoTexture.draw(0,0, camWidth, camHeight);
}

void videoSource::videoToColor(){
    //	msaColor drawColor;
    msa::Color drawColor;
//	float hue = 0.0;
	int *tempPixelChange;
	tempPixelChange = new int[numPixels];
	
	//	 hue = ofGetFrameNum() % 360;
    //	drawColor.setHSV(hue, 1, 1);
	drawColor.setHsb(hue, 1, 1);
	float colorMult=0;
	
	int threshold = 20;
	float vx=0;
	float vy=0;
	
	//smooth
	for (int iy=1; iy<(fluidHeight-1); iy++){
		for(int ix=1; ix<(fluidWidth-1); ix++){
			tempPixelChange[Fluid_IX(ix, iy)]=pixelChange[Fluid_IX(ix, iy)]+0.3*((pixelChange[Fluid_IX(ix, iy+1)]+
															  pixelChange[Fluid_IX(ix, iy-1)]+pixelChange[Fluid_IX(ix+1, iy)]+pixelChange[Fluid_IX(ix-1, iy)])-
															 4*pixelChange[Fluid_IX(ix, iy)]);
		}
	}
	
	if(videoCaptured>15){
		if(doRGB){
			for (int iy=1; iy<(fluidHeight-1); iy++){
				for(int ix=1; ix<(fluidWidth-1); ix++){
					int idx=Fluid_IX(ix, iy);
					colorMult=max(0, tempPixelChange[idx]-threshold)*videoColorMult;
					if(colorMult>255.) colorMult=255;
                    //					fluidSolver->r[idx]  += drawColor.r * colorMult;
                    //					fluidSolver->g[idx]  += drawColor.g * colorMult;
                    //					fluidSolver->b[idx]  += drawColor.b * colorMult;
					fluidSolver->color[idx].x  += drawColor.r * colorMult/256.0*2;
					fluidSolver->color[idx].y  += drawColor.g * colorMult/256.0*2;
					fluidSolver->color[idx].z  += drawColor.b * colorMult/256.0*2;
			
					if(tempPixelChange[idx]>prevPixelChange[idx]){
						vx = pixelChange[Fluid_IX(ix-1, iy)]-pixelChange[Fluid_IX(ix+1, iy)];
						vy = pixelChange[Fluid_IX(ix, iy-1)]-pixelChange[Fluid_IX(ix, iy+1)];
//						fluidSolver->u[idx] += vx * velocityMult;
//						fluidSolver->v[idx] += vy * velocityMult;
						fluidSolver->uv[2*idx] += vx * velocityMult;
						fluidSolver->uv[2*idx+1] += vy * velocityMult;
				
					}
				}
			}
		}else{
			for (int iy=1; iy<(fluidHeight-1); iy++){
				for(int ix=1; ix<(fluidWidth-1); ix++){
					int idx=Fluid_IX(ix, iy);
					colorMult=max(0, tempPixelChange[idx]-threshold)*videoColorMult;
					if(colorMult>255.) colorMult=255;
//					fluidSolver->r[idx]  += drawColor.r * colorMult/100.;
					fluidSolver->color[idx].x  += drawColor.r * colorMult/100.;
					
					if(tempPixelChange[idx]>prevPixelChange[idx]){
						vx = pixelChange[Fluid_IX(ix-1, iy)]-pixelChange[Fluid_IX(ix+1, iy)];
						vy = pixelChange[Fluid_IX(ix, iy-1)]-pixelChange[Fluid_IX(ix, iy+1)];
//						fluidSolver->u[idx] += vx * velocityMult;
//						fluidSolver->v[idx] += vy * velocityMult;
						fluidSolver->uv[idx].x += vx * velocityMult;
						fluidSolver->uv[idx].y += vy * velocityMult;
						
					}
				}
			}
		}
		
	}
	
	
	for (int iy=0; iy<fluidHeight; iy++){
		for(int ix=0; ix<fluidWidth; ix++){
			prevPixelChange[Fluid_IX(ix, iy)]=tempPixelChange[Fluid_IX(ix, iy)];
		}
	}
	
	delete[] tempPixelChange;
	
}


