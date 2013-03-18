/*
 *  VideoSource.h
 *  MSAFluid
 *
 *  Created by Weidong Yang on 10/29/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#include "ofMain.h"
#include "MSACore.h"
#include "MSAFluid.h"
//#include "msaColor.h"

#ifndef	WEIDAAASEED
//CAM_IX(x, y)
#define		Cam_IX(i, j)		((i) + (camWidth)  *(j))	
#define		Fluid_IX(i, j)		((i) + (fluidWidth)  *(j))	

#define	WEIDAAASEED
#endif


class videoSource{
public:
	videoSource(){
	}

	void setupVideo(int NX, int NY, msa::fluid::Solver  * fluidSolver);
	void destroy();
	void update();
	void draw();
	void videoToColor();
	
	ofVideoGrabber      vidGrabber;

	int		videoFrameCnt;
	int		videoCaptured;
	
	int					camWidth;
	int					camHeight;
	int					numPixels;
	
	int					fluidWidth;
	int					fluidHeight;
	int		camMapX0, camMapY0, camMapXend, camMapYend;
	
	
	ofTexture			videoTexture;
	unsigned char *		videoMapped;
	unsigned char *		videoMirrored;
	unsigned char *     previousFrame;
	unsigned char *     differenceFrame;
	
	int *			pixelChange;
	int *			prevPixelChange;
	float			videoColorMult;
	float			velocityMult;
    float           hue;
	int				videoAlpha;

	bool			doRGB;
	bool			doDraw;
	
	msa::fluid::Solver  *	fluidSolver;
	
	
};
