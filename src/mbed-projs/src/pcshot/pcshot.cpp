// led means focus triggered

#include "mbed.h"
//#include "serial_api.h"

DigitalOut led(LED1); // PA_5
InterruptIn button(USER_BUTTON); // PC_13

BusOut shutter(
		PC_9, // 1/A
		PB_9, // 2/B
		PA_7, // 3/C
		PC_7, // 4/D
		PA_8, // 5/E
		PC_8, // 6/F
		PC_5, // 7/G
		PA_11,// 8/H
		PB_2, // 9/I
		PB_15 //10/J
);
BusOut focus(
		PB_8, // 1/A
		PA_6, // 2/B
		PB_6, // 3/C
		PA_9, // 4/D
		PB_10,// 5/E
		PC_6, // 6/F
		PA_12,// 7/G
		PB_12,// 8/H
		PB_1, // 9/I
		PB_14 //10/J
);

const int MASK = 0x3ff; // 10 bits

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
	case '\r':
		recvmask = workmask;
		recvstate = workstate;
		workmask = 0;
		break;
	}
}

void butnfunc() {
	if (focus == 0) {
		focus = MASK;
		wait(0.2); // debounce
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
	}
}
