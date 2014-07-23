#include "mbed.h"

DigitalOut myled(LED1);
DigitalIn button(USER_BUTTON);

// topmost one
DigitalOut focus(PB_8);
DigitalOut shutter(PC_9);

// lowest in column 2
InterruptIn hotshoe(PC_4);

// "button" down, button up
// up = release
int downtime = 0;
int uptime = 0;

Timer timer;

void feedback() {
	uptime = timer.read_us();
}

int main() {
	printf("o hai\r\n");
	focus = 0;
	shutter = 0;
	myled = 0;
	hotshoe.fall(&feedback);
	timer.start();

	for (;;) {
		while (button != 0)
			;
		printf("start\r\n");
		myled = 1;
		focus = 1;
		wait(0.2);
		downtime = timer.read_us();
		shutter = 1;

		wait(0.2);
		shutter = 0;
		focus = 0;

		int totaltime = uptime - downtime;
		printf("end %d\r\n", totaltime);
	}
}
