#include "IVTEntry.h"
#include <stdio.h>

#include "Event.h"
#include "PCB.h"
#include "Semaphor.h"
#include "Thread.h"

#include "Scheduler.h"

void inic();
void restore();

extern int userMain(int argc, char * argv[]);


class mainThread : public Thread{
	mainThread();
	void run();
	friend int main(int argc, char * argv[]);
};


mainThread::mainThread() : Thread(0, defaultTimeSlice){

};

void mainThread::run(){}


int main(int argc, char * argv[]){

	mainThread main;
	main.start();
	PCB::running = Scheduler::get();

	inic();

	int ret = userMain(argc, argv);

	restore();

	return ret;
}
