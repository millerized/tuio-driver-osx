/*
 TUIO Demo - part of the reacTIVision project
 http://reactivision.sourceforge.net/
 
 Copyright (c) 2005-2009 Martin Kaltenbrunner <mkalten@iua.upf.edu>
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef INCLUDED_SimpleSimulator_H
#define INCLUDED_SimpleSimulator_H

#include "TuioServer.h"
#include "TuioCursor.h"
#include <list>
#include <math.h>

#include <SDL.h>
#include <SDL_thread.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

/* General touch state definitions */
#define TPM_RELEASED	0
#define TPM_PRESSED	1

/* Event enumerations */
typedef enum {
	TPM_NOEVENT = 0,		/* Unused (do not remove) */
	TPM_TOUCHMOTION,		/* Touch moved */
	TPM_TOUCHBUTTONDOWN,		/* Touch button pressed */
	TPM_TOUCHBUTTONUP		/* Touch button released */
} TPM_EventType;

typedef struct TPM_TouchButtonEvent {
	uint8_t type;	/* TPM_TOUCHBUTTONDOWN or TPM_TOUCHBUTTONUP */
	uint8_t which;	/* The touch device index */
	uint8_t button;	/* The touch button index */
	uint8_t state;	/* TOUCH_PRESSED or TOUCH_RELEASED */
	uint16_t x, y;	/* The X/Y coordinates of the touch at press time */
} TPM_TouchButtonEvent;

typedef union TPM_Event {
	uint8_t type;
	TPM_TouchButtonEvent button;
} TPM_Event;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

#define TPM_NUM		4

typedef struct TPM_PACKET {
	unsigned char report_id;
	unsigned char tid_pressed;
	unsigned short int x;
	unsigned short int y;
} TPM_PACKET;

#pragma pack(pop)   /* restore original alignment from stack */

using namespace TUIO;

class SimpleSimulator { 
	
public:
	SimpleSimulator(const char *host, int port, bool v);
	~SimpleSimulator() {
		delete tuioServer;
	};
	
	void run();
	TuioServer *tuioServer;
	TuioCursor *cursor;
	TuioCursor *cursor_tpm[TPM_NUM];
	std::list<TuioCursor*> stickyCursorList;
	
private:
	void processEventsForTPM();
	int TPM_PollEvent(void *data);
	SDL_Surface *window;
	bool verbose, fullscreen, running;
	
	int width, height;
	int screen_width, screen_height;
	int window_width, window_height;
	TuioTime currentTime;
	
	void touchPressed(float x, float y, int p);
	void touchReleased(float x, float y, int p);
	void touchDragged(float x, float y, int p);

	int s_id;

	TPM_PACKET tpm_packet;
};

int hidapi_main(int argc, char* argv[]);
int hidapi_read_data(char* data, int size);

#endif /* INCLUDED_SimpleSimulator_H */
