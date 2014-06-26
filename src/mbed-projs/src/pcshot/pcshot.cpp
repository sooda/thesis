// led means focus triggered

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

const int MASK = 0x1ff; // 9 bits

Serial ser(SERIAL_TX, SERIAL_RX);

#define STATE_NONE 0
#define STATE_FOCUS 1
#define STATE_SHOOT 2
#define STATE_RESET 3

volatile int recvstate; // start at none
volatile int recvmask;
int workmask;
int workstate;


void recvfunc() {
	int c = ser.getc();
	ser.putc(c);
	switch (c) {
	case '1':
		workmask <<= 1;
		workmask |= 1;
		break;
	case '0':
		workmask <<= 1;
		break;
	case 'F':
		workstate = STATE_FOCUS;
		break;
	case 'S':
		workstate = STATE_SHOOT;
		break;
	case 'R':
		workmask = 0;
		recvmask = 0;
		workstate = STATE_RESET;
		recvstate = workstate;
		break;
	case ' ': /* fallthrough */
	case '\n':
		recvmask = workmask;
		recvstate = workstate;
		workmask = 0;
		break;
	}
}

void butnfunc() {
	if (focus == 0) {
		focus = MASK;
	} else {
		shutter = MASK;
		wait(5.2);
		shutter = 0;
	}
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
		int st, rmask;
		while ((st = recvstate) == STATE_NONE) {
			// wait
		}
		rmask = recvmask;
		recvstate = STATE_NONE;

		switch (st) {
		case STATE_FOCUS:
			if (rmask)
				led = 1;
			focus = rmask & MASK;
			break;
		case STATE_SHOOT:
			shutter = rmask & MASK;
			break;
		case STATE_RESET:
			focus = 0;
			shutter = 0;
			led = 0;
			break;
		}
		//wait(0.5);
	}
}
