#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cmath>
#include <ApplicationServices/ApplicationServices.h>

CGImageRef getFullscreen() {
	return CGWindowListCreateImage(CGRectInfinite, 0, 0, 1);
}

CGImageRef getImage(int window_id) {
	int** window_id_arr = (int**)(&window_id); 
	CFArrayRef anArray = CFArrayCreate(NULL, (const void **)window_id_arr, 1 + 16, NULL);
	return CGWindowListCreateImageFromArray(CGRectNull, anArray, 1);
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
	//imageToFile(getImage(466));
	/*
	CGImageRef img = getImage(0x5e6e);
	imageToFile(img);
	CFDataRef rawData = CGDataProviderCopyData(CGImageGetDataProvider(img));
	UInt8* buf = (UInt8 *) CFDataGetBytePtr(rawData);
	CFIndex length = CFDataGetLength(rawData);
	CFRelease(rawData);
	*/
	int cmd;
	int windowId;
	fread(&windowId, 4, 1, stdin);
	char *c_buf = (char*)malloc(2900 * 1500 * 4);
	while (!feof(stdin)) {
		fread(&cmd, 4, 1, stdin);
		if (cmd == 0) {  // Get image
			CGImageRef img = getImage(windowId);
			// imageToFile(img);
			CFDataRef rawData = CGDataProviderCopyData(CGImageGetDataProvider(img));
			UInt8* buf = (UInt8 *) CFDataGetBytePtr(rawData);
			CFIndex length = CFDataGetLength(rawData);
			int w = (int)CGImageGetWidth(img);
			int h = (int)CGImageGetHeight(img);
			int r = (int)CGImageGetBytesPerRow(img) / 4;
			int l = length;
			for (int j = 0; j < h / 2; j++) {
				for (int i = 0; i < w / 2; i++) {
					int p = i + j * w / 2;
					int q = i * 2 + j * 2 * r;
					c_buf[4 * p] = buf[4 * q];
					c_buf[4 * p + 1] = buf[4 * q + 1];
					c_buf[4 * p + 2] = buf[4 * q + 2];
					c_buf[4 * p + 3] = buf[4 * q + 3];
				}
			}
			w = w / 2;
			h = h / 2;
			fwrite(&w, 4, 1, stdout);
			fwrite(&h, 4, 1, stdout);
			fwrite(c_buf, 1, w * h * 4, stdout);
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
	move(1000, 500);
	// sleep(2000);
	// findEnemy();
	imageToFile(getImage(0x5e6e));
	for (int i = 0; i < 100; i++) {

	}
	/*
	for(unsigned long i = 0; i < length; i += 4) {
		int r = buf[i];
		int g = buf[i + 1];
		int b = buf[i + 2];
		std::cout << r << " " << g << " " << b << std::endl;
	}
	*/
	for (int i = 0; i < 1000; i++) {
		// std::cout << image[i] << std::endl;
	}
}
