#include "IVTEntry.h"
#include <stdio.h>

#include "Event.h"
#include "PCB.h"
#include "Semaphor.h"
#include "Thread.h"

void inic();
void restore();

extern int userMain(int argc, char * argv[]);

int main(int argc, char * argv[]){

	inic();

	PCB::running = new PCB(0, defaultTimeSlice, 0);

	int ret = userMain(argc, argv);

	delete PCB::running;

	restore();

	return ret;
}
