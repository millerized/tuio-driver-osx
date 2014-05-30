/*
	TUIO C++ Server Demo - part of the reacTIVision project
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

#include "SimpleSimulator.h"

int SimpleSimulator::TPM_PollEvent(void *data)
{
	TPM_Event *event = (TPM_Event *)data;
	static TPM_Event prev_event[TPM_NUM];
	int p;

	if( 0 == hidapi_read_data((char *)&tpm_packet, sizeof(TPM_PACKET)))
		return 0;

	event->button.which = 0;
	event->button.button = (tpm_packet.tid_pressed>>2)&0x1f;
	event->button.state = ((tpm_packet.tid_pressed&0x01) == 0x01) ? TPM_PRESSED : TPM_RELEASED;
	event->button.x = tpm_packet.x;
	event->button.y = tpm_packet.y;

	p = event->button.button;

	if (event->button.state == TPM_PRESSED) {
		event->type = TPM_TOUCHBUTTONDOWN;
		if (prev_event[p].button.state == TPM_PRESSED) {
			event->type = TPM_TOUCHMOTION;
		}
	} else {
		event->type = TPM_TOUCHBUTTONUP;
	}

	memcpy(&prev_event[p], event, sizeof(TPM_Event));

	return 1;
}

void SimpleSimulator::processEventsForTPM()
{
	TPM_Event event;

	int p;
	float fx = 0.0, fy = 0.0;

	if(!TPM_PollEvent( &event ))
	{
    		return;	
	}

	fx = (float)event.button.x/32767;
	fy = (float)event.button.y/32767;

	p = event.button.button;

//	printf("p = %d, fx = %f, fy = %f, event.type = 0x%x\n", p, fx, fy, event.type);

	switch( event.type ) {
	case TPM_TOUCHMOTION:
		touchDragged(fx, fy, p);
		break;
	case TPM_TOUCHBUTTONDOWN:
		touchPressed(fx, fy, p);
		break;
	case TPM_TOUCHBUTTONUP:
		touchReleased(fx, fy, p);
		break;
	}
}

void SimpleSimulator::touchPressed(float x, float y, int p) {
	if (verbose)
		printf("point %d clicked %f %f\n",p,x,y);

	if (cursor_tpm[p]!=NULL) return;
	cursor_tpm[p] = tuioServer->addTuioCursor(x,y);
}

void SimpleSimulator::touchDragged(float x, float y, int p) {
	if (verbose)
		printf("point %d dragged %f %f\n",p,x,y);

	if (cursor_tpm[p]==NULL) return;
	if (cursor_tpm[p]->getTuioTime()==currentTime) return;
	tuioServer->updateTuioCursor(cursor_tpm[p],x,y);
}

void SimpleSimulator::touchReleased(float x, float y, int p) {
	if (verbose)
		printf("point %d released\n",p);

	if (cursor_tpm[p]==NULL) return;
	tuioServer->removeTuioCursor(cursor_tpm[p]);
	cursor_tpm[p] = NULL;
}

SimpleSimulator::SimpleSimulator(const char* host, int port, bool v) {

	verbose = v;
	fullscreen = false;
	window_width = 640;
	window_height = 480;
	screen_width = 1024;
	screen_height = 768;
	
	cursor = NULL;
	if ((strcmp(host,"default")==0) && (port==0)) tuioServer = new TuioServer();
	else tuioServer = new TuioServer(host, port);
	currentTime = TuioTime::getSessionTime();
	
	//tuioServer->enablePeriodicMessages();
}

void SimpleSimulator::run() {
	running=true;
	while (running) {
		currentTime = TuioTime::getSessionTime();
		tuioServer->initFrame(currentTime);
		processEventsForTPM();
		tuioServer->stopUntouchedMovingCursors();
		tuioServer->commitFrame();
	} 
}

int main(int argc, char* argv[])
{
	SimpleSimulator *app;
	bool v = false;

        if( argc == 3 ) {
                v = true;
        }

	app = new SimpleSimulator("default",0,v);

	hidapi_main(argc, argv);
	
	app->run();
	delete(app);

	return 0;
}

/*******************************************************
 Windows HID simplification

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009, All Rights Reserved.
 
 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.
********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"

// Headers needed for sleeping.
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#define _NON_BLOCKING_	0

int res;
unsigned char buf[256];
#define MAX_STR 255
wchar_t wstr[MAX_STR];
hid_device *handle;
int i;

TPM_PACKET tpm_packet;

int hidapi_main(int argc, char* argv[])
{
#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif

	int screen_size = -1;

        if( argc >= 2 ) {
                screen_size = atoi(argv[1]);
        }

	struct hid_device_info *devs, *cur_dev;
	
	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;	
	while (cur_dev) {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n",  cur_dev->interface_number);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	switch (screen_size) {
	case 22:
		handle = hid_open(0x0eef, 0x7224, NULL);
		break;
	case 19:
	        handle = hid_open(0x0eef, 0x72c4, NULL);
		break;
        case 15:
                handle = hid_open(0x0eef, 0x72d0, NULL);
                break;
	default:
            handle = hid_open(0x0eef, 0x72d0, NULL);
		break;
	}

	if (!handle) {
        printf("unable to read device 0x72d0\n");
		printf("reading device 0x72c4\n");
        handle = hid_open(0x0eef, 0x72c4, NULL);
 		//return 1;
	}
    
    if (!handle) {
        printf("unable to read device 0x72c4\n");
		printf("reading device 0x7224\n");
        handle = hid_open(0x0eef, 0x7224, NULL);
 		//return 1;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, _NON_BLOCKING_);
	
	// Try to read from the device. There shoud be no
	// data here, but execution should not block.
#if _NON_BLOCKING_
	res = hid_read(handle, buf, 17);
#endif

#if 1
	// Send a Feature Report to the device
	buf[0] = 0x5;
	buf[1] = 0x02;
	buf[2] = 0x00;
	res = hid_send_feature_report(handle, buf, 17);
	if (res < 0) {
		printf("Unable to send a feature report.\n");
	}

	memset(buf,0,sizeof(buf));

	// Read a Feature Report from the device
	buf[0] = 0x5;
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res < 0) {
		printf("Unable to get a feature report.\n");
		printf("%ls", hid_error(handle));
	}
	else {
		// Print out the returned buffer.
		printf("Feature Report\n   ");
		for (i = 0; i < res; i++)
			printf("%02hhx ", buf[i]);
		printf("\n");
	}
#endif

	memset(buf,0,sizeof(buf));

	return 0;

_hid_read_loop_:
	// Read requested state. hid_read() has been set to be
	// non-blocking by the call to hid_set_nonblocking() above.
	// This loop demonstrates the non-blocking nature of hid_read().
	res = 0;

#if _NON_BLOCKING_
	while (res == 0) {
		res = hid_read(handle, buf, sizeof(buf));
		if (res == 0) {
//			printf("waiting...\n");
		}
		if (res < 0)
			printf("Unable to read()\n");
		#ifdef WIN32
		Sleep(500);
		#else
		usleep(5*1000);
		#endif
	}
#else
	res = hid_read(handle, buf, sizeof(buf));
#endif

	printf("Data read:\n   ");
	// Print out the returned buffer.
	for (i = 0; i < res; i++)
		printf("%02hhx ", buf[i]);
	printf("\n");

	goto _hid_read_loop_;

	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause");
#endif

	return 0;
}

int hidapi_read_data(char* data, int size)
{
        res = 0;
        res = hid_read(handle, buf, sizeof(buf));

#if 0
        printf("Data read:\n   ");
        // Print out the returned buffer.
        for (i = 0; i < res; i++)
                printf("%02hhx ", buf[i]);
        printf("\n");
#endif

	memcpy(&tpm_packet, buf, res);

#if 0
	if(size > sizeof(tpm_packet))
		size = sizeof(tpm_packet);
#endif
		
	memcpy(data, &tpm_packet, size);
	
	return size;
}

