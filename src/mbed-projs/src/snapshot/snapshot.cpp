#include "mbed.h"

DigitalOut led(LED1); // PA_5
DigitalIn button(USER_BUTTON); // PC_13
// PC_5 misbehaves, or the camera
// PD8 undefined?
BusOut focus  (
		PC_9, // A column 1
		PC_8, // B column 2
		PB_9, // C column 1
		PA_12,// D column 2
		PA_7, // E column 1
		PC_7, // F ...
		PA_8, // G
		PB_4, // H
		PB_3  // I
);
BusOut shutter(
		PB_8, // A column 1
		PC_6, // B column 2
		PA_6, // C column 1
		PA_11,// D column 2
		PB_6, // E column 1
		PA_9, // F ...
		PB_10,// G
		PB_5, // H
		PA_10 // I
);

const int mask = 0x1ff; // 9 bits

int main() {
	// bootup signal
	for (int i = 0; i < 3; i++) {
		led = 1;
		wait(0.1);
		led = 0;
		wait(0.1);
	}

	// reset to set focus and shutter low
	focus = 0;
	shutter = 0;
	led = 0;

	// first keypress to focus
	while (button != 0)
		;
	focus = mask;
	wait(0.5);
	// led signs the next-keypress-is-shutter state
	led = 1;

	while(1) {
		while (button != 0)
			;
#if 1
		// all simultaneously
		shutter = mask;
		wait(0.1);
#else
		// strobo flash and sound effects mode
		for (int i = 0, x = 0; i < 9; i++) {
			x <<= 1;
			x |= 1;
			shutter = x;
			//wait(0.008);
			wait(0.25);
			//wait(1);
		}
#endif
		shutter = 0;
	}
}


