#include "mbed.h"
//#include "serial_api.h"

DigitalOut led(LED1); // PA_5
InterruptIn button(USER_BUTTON); // PC_13
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

Serial ser(SERIAL_TX, SERIAL_RX);

volatile int recvmask;
int workmask;

void recvfunc() {
	workmask <<= 1;
	int c = ser.getc();
	ser.putc(c);
	if (c == '1')
		workmask |= 1;
	else if (c == ' ' || c == '\r' || c == '\n') {
		recvmask = workmask;
		workmask = 0;
	}
}

void butnfunc() {
	recvmask = mask;
}

int main() {
	// bootup signal
	for (int i = 0; i < 3; i++) {
		led = 1;
		wait(0.1);
		led = 0;
		wait(0.1);
	}

	focus = 0;
	shutter = 0;
	led = 0;

	ser.attach(recvfunc, Serial::RxIrq);
	button.fall(butnfunc);

	for (;;) {
		while (recvmask == 0)
			;
		int m = recvmask;
		recvmask = 0;
		led = 1;

		focus = m & mask;
		wait(0.5);
		shutter = m & mask;
		wait(0.1);
		shutter = 0;
		focus = 0;
		led = 0;
	}
}
