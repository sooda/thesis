// led means focus triggered

#include "mbed.h"
//#include "serial_api.h"

DigitalOut led(LED1); // PA_5
InterruptIn button(USER_BUTTON); // PC_13
// PC_5 misbehaves, or the camera (OLD?)J
BusOut focus(
		PB_8, // A
		PA_6, // B
		PB_6, // C
		PA_9, // D
		PB_10,// E
		PC_6, // F
		PA_12,// G
		PB_12,// H
		PB_1  // I
);
BusOut shutter(
		PC_9, // A
		PB_9, // B
		PA_7, // C
		PC_7, // D
		PA_8, // E
		PC_8, // F
		PC_5, // G
		PA_11,// H
		PB_2  // I
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
		wait(0.2);
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
