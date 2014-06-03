#include "mbed.h"

DigitalOut led(LED1); // PA_5
DigitalIn button(USER_BUTTON); // PC_13

Serial ser(SERIAL_TX, SERIAL_RX);

volatile int recvval;
int workval;

int period, duration;
int speed = 1;

// us
#define INITDURATION 100
#define INITPERIOD 40000 // 25 fps
#define PERIODSTEP 1
#define DURATIONSTEP 1

#define SPEED1 1
#define SPEED2 10
#define SPEED3 1000

void lel() {
	int c = ser.getc();
	ser.putc(c);
	switch (c) {
	case 'q':
		period -= SPEED1 * PERIODSTEP;
		recvval = -1;
		break;
	case 'w':
		period += SPEED1 * PERIODSTEP;
		recvval = -1;
		break;
	case 'a':
		period -= SPEED2 * PERIODSTEP;
		recvval = -1;
		break;
	case 's':
		period += SPEED2 * PERIODSTEP;
		recvval = -1;
		break;
	case 'z':
		period -= SPEED3 * PERIODSTEP;
		recvval = -1;
		break;
	case 'x':
		period += SPEED3 * PERIODSTEP;
		recvval = -1;
		break;

	case 'e':
		duration -= SPEED1 * DURATIONSTEP;
		recvval = -1;
		break;
	case 'r':
		duration += SPEED1 * DURATIONSTEP;
		recvval = -1;
		break;
	case 'd':
		duration -= SPEED2 * DURATIONSTEP;
		recvval = -1;
		break;
	case 'f':
		duration += SPEED2 * DURATIONSTEP;
		recvval = -1;
		break;
	case 'c':
		duration -= SPEED3 * DURATIONSTEP;
		recvval = -1;
		break;
	case 'v':
		duration += SPEED3 * DURATIONSTEP;
		recvval = -1;
		break;
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
	ser.attach(&lel, Serial::RxIrq);

	PwmOut p(PA_15);

	period = INITPERIOD;
	duration = INITDURATION;
	p.period_us(period);
	p.pulsewidth_us(duration);
	for (;;) {
		while (recvval == 0) {
			// wait
		}
		//prevval = recvval;
		p.period_us(period);
		p.pulsewidth_us(duration);
		printf("per %d us = freq %d Hz, dur %d us (%d %%)\r\n", period, 1000000 / period, duration, 100 * duration / period);
		recvval = 0;
	}
}
