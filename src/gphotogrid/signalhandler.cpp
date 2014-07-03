#include <signal.h>

void trap_ctrlc(void (*handfunc)(int)) {
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = handfunc;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, nullptr);
}

