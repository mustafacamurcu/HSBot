#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cmath>
#include <ApplicationServices/ApplicationServices.h>
#include <AudioToolbox/AudioToolbox.h>

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

int getMuted() {
	AudioObjectPropertyAddress propertyAddress = {
		kAudioHardwarePropertyDefaultOutputDevice,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};

	AudioDeviceID deviceID;
	UInt32 dataSize = sizeof(deviceID);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &dataSize, &deviceID);

	AudioObjectPropertyAddress propertyAddress2 = {
		kAudioDevicePropertyMute,
		kAudioDevicePropertyScopeOutput,
		1
	};

	UInt32 muted;
	dataSize = sizeof(muted);
	AudioObjectGetPropertyData(deviceID, &propertyAddress2, 0, NULL, &dataSize, &muted);
	return muted;
}

void move(int x, int y) {
	if (getMuted()) return;
	CGEventRef mouseDownEv = CGEventCreateMouseEvent(NULL,
			kCGEventMouseMoved,
			CGPointMake(x, y),
			kCGMouseButtonRight);
	CGEventPost(kCGHIDEventTap, mouseDownEv);
}

void rclick(int x, int y) {
	if (getMuted()) return;
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

void key(int k) {
	if (getMuted()) return;
	CGEventRef cmdd = CGEventCreateKeyboardEvent(NULL, k, true);
	CGEventRef cmdu = CGEventCreateKeyboardEvent(NULL, k, false);
	CGEventPost(kCGHIDEventTap, cmdd);
	CGEventPost(kCGHIDEventTap, cmdu);
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
	std::cin >> windowId;
	while (!feof(stdin)) {
		std::cin >> cmd;
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
			std::cin >> x >> y;
			move(x, y);
		} else if (cmd == 2) {
			int x, y;
			std::cin >> x >> y;
			rclick(x, y);
		} else if (cmd == 3) {
			int k;
			std::cin >> k;
			key(k);
		} else if (cmd == 4) {
		} else if (cmd == 5) {
		} else if (cmd == 6) {
		}
	}
}
