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
int PCB::blockCS = 0;
volatile PCB * PCB::running = 0;
int PCB::blockSignalGlobal = 0;


PCB::PCB(StackSize size, Time ts, Thread * th): id(idCounter++), stack(0), timeSlice(ts),
		finished(0), myThread(th), started(0), ss(0), sp(0), blocked(0), blockedTime(0), mySem(0), blockSignal(0), myFather(0){

	if (id == 0){
		return;
	}

	myFather = running->myThread;

	for(int i = 0; i < 16; ++i){
		handlers[i] = running->handlers[i];
	}

	blockSignal = running->blockSignal;

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

	PCB::running->myFather->signal(1);

	PCB::running->myThread->signal(2);

	PCB::running->mySem->signal(-(PCB::running->mySem->val));

	PCB::running->finished = 1;

	dispatch();
}




//CONTEXT SWITCH

//CONTEXT SWITCH CONTROL



void PCB::lockCS(){
	PCB::blockCS = 1;
}

void PCB::unlockCS(){
	PCB::blockCS = 0;
}

volatile Time cntr = 1;
volatile int context_switch_on_demand = 0;

unsigned tsp;
unsigned tss;

//sync

Idle * Idle::idle = 0;
const int Idle::STACK_SIZE = defaultStackSize;

void Idle::f(){
	while(1){asm hlt;}
}

Idle::Idle(): stack(new unsigned[STACK_SIZE]), ss(0), sp(0){
	stack[STACK_SIZE - 1] = 0x200;

	void (*body)();

	body = Idle::f;

#ifndef BCC_BLOCK_IGNORE

	stack[STACK_SIZE - 2] = FP_SEG(body);
	stack[STACK_SIZE - 3] = FP_OFF(body);


	sp = FP_OFF(stack + STACK_SIZE - 12);
	ss = FP_SEG(stack + STACK_SIZE - 12);
#endif
}

Idle::~Idle(){
	delete[] stack;
}

int Idle::init(){
	if (idle) return 0;
	idle = new Idle();
	return 1;
}

void Idle::end(){
	if (!idle) return;
	delete idle;
	idle = 0;
}

void interrupt timer(...){

	if (!context_switch_on_demand && cntr) --cntr;
	if (!PCB::blockCS && (context_switch_on_demand || (PCB::running->timeSlice != 0 && cntr == 0))) {

		if (!Idle::idle){
			if (!PCB::running->finished){
				#ifndef BCC_BLOCK_IGNORE
				asm {
					mov tsp, sp
					mov tss, ss
				}
				#endif
				PCB::running->sp = tsp;
				PCB::running->ss = tss;

				if (!PCB::running->blocked)
					Scheduler::put( (PCB *) PCB::running );
			}
			else {
				delete PCB::running->stack;
				PCB::running->stack = 0;
			}
		}


		PCB::running = Scheduler::get();
		if (!PCB::running){
			if (Idle::init()){
				tsp = Idle::idle->sp;
				tss = Idle::idle->ss;

				cntr = 1;

				#ifndef BCC_BLOCK_IGNORE
				asm {
					mov sp, tsp
					mov ss, tss
				}
				#endif
			}

		}
		else {
			Idle::end();

			tsp = PCB::running->sp;
			tss = PCB::running->ss;

			cntr = PCB::running->timeSlice;


			#ifndef BCC_BLOCK_IGNORE
			asm {
				mov sp, tsp
				mov ss, tss
			}
			#endif

			((PCB *)PCB::running)->performSignals();
		}



	}


	if(!context_switch_on_demand) for (KernelSem::semaphoreList.begin();KernelSem::semaphoreList.end();KernelSem::semaphoreList.next()){
			(*(KernelSem::semaphoreList.get()))--;
	}


	#ifndef BCC_BLOCK_IGNORE
	if(!context_switch_on_demand) asm int 0x60
	else context_switch_on_demand = 0;
	#endif

}





//async

extern void tick();

void interrupt userTimer(...){
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

void PCB::registerHandler(SignalId signal, SignalHandler handler){
	handlers[signal]+=(handler);
}

void PCB::unregisterHandler(SignalId signal){
	handlers[signal] = List<SignalHandler>();
}

void PCB::swap(SignalId id, SignalHandler hand1, SignalHandler hand2){
	SignalHandler *h1 = 0;
	SignalHandler *h2 = 0;

	for(handlers[id].begin();handlers[id].end();handlers[id].next()){
		if (handlers[id].get() == hand1){
			h1 = &handlers[id].get();
		}
		if (handlers[id].get() == hand2){
			h2 = &handlers[id].get();
		}
	}

	if (!h1 && !h2) return;

	*h1 = hand2;
	*h2 = hand1;

}

void PCB::signal(SignalId signal){
	if (signal > 15) return;

	if (PCB::running == this && (!((1 << onHold.get()) & blockSignal) && !((1 << onHold.get()) & blockSignalGlobal))){
		performSignal(signal);
	}
	else {
		onHold.insert(signal);
	}
}

void PCB::performSignals(){
	for(onHold.begin(); onHold.end(); onHold.next()){
		if (!((1 << onHold.get()) & blockSignal) && !((1 << onHold.get()) & blockSignalGlobal)){
			performSignal(onHold.del());
		}
	}
}

void signal0(){

	if (PCB::running->myFather) PCB::running->myFather->signal(1);

	Thread *current = PCB::running->myThread;

	current->signal(2);

	PCB::running->mySem->signal(-(PCB::running->mySem->val));

	PCB::running->finished = 1;

	PCB::unlockCS();

	dispatch();
}

void PCB::performSignal(SignalId signal){
	lockCS();

	for(handlers[signal].begin();handlers[signal].end();handlers[signal].next()){
		(*handlers[signal].get())();
	}

	if (signal == 0) signal0();

	unlockCS();
}
