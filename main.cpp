#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cmath>
#include <ApplicationServices/ApplicationServices.h>

CGImageRef getFullscreen() {
	return CGWindowListCreateImage(CGRectInfinite, 0, 0, 1);
}

CGImageRef getImage(int window_id) {
	int x = window_id;
	int** window_id_arr = (int**)(&x); 
	CFArrayRef anArray = CFArrayCreate(NULL, (const void **)window_id_arr, 1, NULL);
	return CGWindowListCreateImageFromArray(CGRectNull, anArray, 1 + 16);
}

void imageToFile(CGImageRef img) {
	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR("test.png"), kCFURLPOSIXPathStyle, false);
	CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
	CGImageDestinationAddImage(destination, img, NULL);
	CGImageDestinationFinalize(destination);
}

void move(int x, int y) {
	CGEventRef mouseDownEv = CGEventCreateMouseEvent(NULL,
			kCGEventMouseMoved,
			CGPointMake(x, y),
			kCGMouseButtonRight);
	CGEventPost (kCGHIDEventTap, mouseDownEv);
}

void rclick(int x, int y) {
	CGEventRef mouseDownEv = CGEventCreateMouseEvent(NULL,
			kCGEventRightMouseDown,
			CGPointMake(x, y),
			kCGMouseButtonRight);
	CGEventPost (kCGHIDEventTap, mouseDownEv);
	CGEventRef mouseUpEv = CGEventCreateMouseEvent(NULL,
			kCGEventRightMouseUp,
			CGPointMake(x, y),
			kCGMouseButtonRight);
	CGEventPost(kCGHIDEventTap, mouseUpEv);
}

void findEnemy() {
	int A = 0;
	for (int y = -A; y <= A; y += 20) {
		for (int x = -A; x <= A; x += 20) {
			move(1000 + x, 500 + y);
			CGImageRef img = getFullscreen();
			imageToFile(img);
			int w = CGImageGetWidth(img);
			CFDataRef rawData = CGDataProviderCopyData(CGImageGetDataProvider(img));
			UInt8* buf = (UInt8 *) CFDataGetBytePtr(rawData);
			CFIndex length = CFDataGetLength(rawData);
			for (int i = -50; i < 50; i++) {
				for (int j = -50; j < 50; j++) {
					int p = (1000 + j * 3) * 2 + (500 + i * 3) * w * 2;
					std::cout << ((int)buf[4 * p] + (int)buf[4 * p + 1] + (int)buf[4 * p + 2]) / 32 / 3;
				}
				std::cout << std::endl;
			}
			CFRelease(rawData);
		}
	}
}

void sleep(int micros) {
	usleep(micros * 1000);
}

int main() {
	int cmd;
	int windowId;
	fread(&windowId, 4, 1, stdin);
	while (!feof(stdin)) {
		fread(&cmd, 4, 1, stdin);
		if (cmd == 0) {  // Get image
			 CGImageRef img = getImage(windowId);
			//CGImageRef img = getFullscreen();
			CFDataRef rawData = CGDataProviderCopyData(CGImageGetDataProvider(img));
			UInt8* buf = (UInt8 *) CFDataGetBytePtr(rawData);
			CFIndex length = CFDataGetLength(rawData);
			int w = (int)CGImageGetWidth(img);
			int h = (int)CGImageGetHeight(img);
			int r = (int)CGImageGetBytesPerRow(img) / 4;
			int l = length;
			fwrite(&w, 4, 1, stdout);
			fwrite(&h, 4, 1, stdout);
			fwrite(&r, 4, 1, stdout);
			fwrite(&l, 4, 1, stdout);
			fwrite(buf, 1, r * h * 4, stdout);
			fflush(stdout);
			CFRelease(rawData);	
		} else if (cmd == 1) {  // Right click
			int x, y;
			fread(&x, 4, 1, stdin);
			fread(&y, 4, 1, stdin);
			move(x, y);
		} else if (cmd == 2) {
			int x, y;
			fread(&x, 4, 1, stdin);
			fread(&y, 4, 1, stdin);
			rclick(x, y);
		} else if (cmd == 3) {
		} else if (cmd == 4) {
		} else if (cmd == 5) {
		} else if (cmd == 6) {
		}
	}
}
