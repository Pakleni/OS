#include "List.h"
#include <dos.h>
#include <stdio.h>

#include "Kernel.h"
#include "PCB.h"
#include "SCHEDULE.H"
#include "Semaphor.h"
#include "Thread.h"

#define lock asm cli
#define unlock asm sti

int PCB::idCounter = 0;


PCB::PCB(StackSize size, Time ts, Thread * th): id(idCounter++), stack(0), timeSlice(ts), finished(0), myThread(th), started(0), ss(0), sp(0), blocked(0), blockedTime(0), mySem(0){

	if (th == 0){
		return;
	}

	mySem = new KernelSem(0);

	stack = new unsigned [size];


	stack[size - 1] = 0x200;

	void (*body)();

	body = PCB::wrapper;

#ifndef BCC_BLOCK_IGNORE

	stack[size - 2] = FP_SEG(body);
	stack[size - 3] = FP_OFF(body);


	sp = FP_OFF(stack + size - 12);
	ss = FP_SEG(stack + size - 12);
#endif
}

PCB::~PCB(){
	delete mySem;
	delete[] stack;
}

void PCB::wrapper(){
	PCB::running->myThread->run();
	PCB::running->finished = 1;

	PCB::running->mySem->signal(-(PCB::running->mySem->val));

	dispatch();
}




//CONTEXT SWITCH

//CONTEXT SWITCH CONTROL


volatile PCB * PCB::running = 0;

int PCB::blockCS = 0;
void lockCS(){
	PCB::blockCS = 1;
}

void unlockCS(){
	PCB::blockCS = 0;
}

volatile Thread * PCB::idle = 0;

class idleThread : public Thread{
public:
	idleThread();
	void run();
};


idleThread::idleThread() : Thread(4096, 1){}

void idleThread::run(){
	while(1);
}

volatile Time cntr = 1;
volatile int context_switch_on_demand = 0;

unsigned tsp;
unsigned tss;

//sync
void interrupt timer(...){
	if (!context_switch_on_demand && cntr) --cntr;
	if (!PCB::blockCS && (context_switch_on_demand || (PCB::running->timeSlice != 0 && cntr == 0))) {

		if (!PCB::running->finished){
		#ifndef BCC_BLOCK_IGNORE
			asm {
				mov tsp, sp
				mov tss, ss
			}
		#endif

			PCB::running->sp = tsp;
			PCB::running->ss = tss;

			if (!PCB::running->blocked && !PCB::idle)
				Scheduler::put( (PCB *) PCB::running );
		}

		PCB::running = Scheduler::get();
		if (!PCB::running){
			if (!PCB::idle){
				PCB::idle = new idleThread();
			}

			PCB::running = (volatile PCB*) PCB::idle->myPCB;
		}
		else {
			if (PCB::idle){
				delete PCB::idle;
				PCB::idle = 0;
			}
		}

		tsp = PCB::running->sp;
		tss = PCB::running->ss;

		cntr = PCB::running->timeSlice;

		#ifndef BCC_BLOCK_IGNORE
		asm {
			mov sp, tsp
			mov ss, tss
		}
		#endif

	}


	#ifndef BCC_BLOCK_IGNORE
	if(!context_switch_on_demand) asm int 0x60
	else context_switch_on_demand = 0;
	#endif
}





//async

extern void tick();

void interrupt userTimer(...){

	if(!context_switch_on_demand) for (KernelSem::semaphoreList.begin();KernelSem::semaphoreList.get();KernelSem::semaphoreList.next()){
			(*(KernelSem::semaphoreList.get()))--;
	}

	tick();
}

void dispatch(){
	#ifndef BCC_BLOCK_IGNORE
	lock;
	context_switch_on_demand = 1;
	timer();
	unlock;
	#endif
}

typedef void interrupt (*pInterrupt)(...);

pInterrupt oldTimer, oldUserTimer;

void inic(){
#ifndef BCC_BLOCK_IGNORE
	lock

	oldUserTimer = getvect(0x1C);
	setvect(0x1C,userTimer);


	oldTimer = getvect(0x08);
	setvect(0x08, timer);

	setvect(0x60, oldTimer);

	unlock
#endif
}

void restore(){
#ifndef BCC_BLOCK_IGNORE
	lock

	setvect(0x1C,oldUserTimer);

	setvect(0x08,oldTimer);

	unlock
#endif
}

