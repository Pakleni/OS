#include "PCB.h"
#include <dos.h>
#include "List.h"
#include "Kernel.h"
#include "Thread.h"
#include "SCHEDULE.H"

#define lock asm cli
#define unlock asm sti

int PCB::idCounter = 0;
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


	lockCS();
	mySem = new KernelSem(0);
	stack = new unsigned [size];
	unlockCS();

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


int blockCS = 0;
List<int> lockStack;
void lockCS(){
	lockStack+=(blockCS);
	blockCS = 1;
}

void unlockCS(){
	blockCS = lockStack.pop();
}

volatile Time cntr = 1;
volatile int context_switch_on_demand = 0;

unsigned tsp;
unsigned tss;

//sync

const int Idle::STACK_SIZE = defaultStackSize;

Idle Idle::idle;

void Idle::f(){
	while(1){asm hlt;}
}

Idle::Idle(): stack(0), ss(0), sp(0){

	lockCS();
	stack = new unsigned[STACK_SIZE];
	unlockCS();

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
	lockCS();
	delete[] stack;
	unlockCS();
}

int Idle::isIdle = 0;

void interrupt timer(...){
	if (!context_switch_on_demand && cntr) --cntr;
	if (!blockCS && (context_switch_on_demand || (PCB::running->timeSlice != 0 && cntr == 0))) {


		#ifndef BCC_BLOCK_IGNORE
		asm {
			mov tsp, sp
			mov tss, ss
		}
		#endif

		if (!Idle::isIdle){
			if (!PCB::running->finished){

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
		else {
			Idle::idle.sp = tsp;
			Idle::idle.ss = tss;
		}


		PCB::running = Scheduler::get();
		if (!PCB::running){

			Idle::isIdle = 1;

			tsp = Idle::idle.sp;
			tss = Idle::idle.ss;

			cntr = 1;
		}
		else {
			Idle::isIdle = 0;

			tsp = PCB::running->sp;
			tss = PCB::running->ss;

			cntr = PCB::running->timeSlice;

		}

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

	if (PCB::running) ((PCB *)PCB::running)->performSignals();

}





//async

extern void tick();

void interrupt userTimer(...){

	for (List<KernelSem *>::iterator i = KernelSem::semaphoreList.begin(); i != KernelSem::semaphoreList.end();++i){
		(*(*i))--;
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

void PCB::registerHandler(SignalId signal, SignalHandler handler){
	handlers[signal].push(handler);
}

void PCB::unregisterHandler(SignalId signal){
	handlers[signal] = Queue<SignalHandler>();
}

void PCB::swap(SignalId id, SignalHandler hand1, SignalHandler hand2){
	SignalHandler *h1 = 0;
	SignalHandler *h2 = 0;

	lockCS();
	for(Queue<SignalHandler>::iterator i =  handlers[id].begin(); i != handlers[id].end();++i){
		if (*i == hand1){
			h1 = &*i;
		}
		if (*i == hand2){
			h2 = &*i;
		}
	}


	if (!h1 && !h2) return;

	*h1 = hand2;
	*h2 = hand1;

	unlockCS();

}

void PCB::signal(SignalId signal){
	if (signal > 15) return;

	if (PCB::running == this && (!((1 << signal) & blockSignal) && !((1 << signal) & blockSignalGlobal))){
		performSignal(signal);
	}
	else {
		onHold.push(signal);
	}
}

void PCB::performSignals(){
	for(Queue<SignalId>::iterator i = onHold.begin(); i != onHold.end(); ++i){
		if (!((1 << *i) & blockSignal) && !((1 << *i) & blockSignalGlobal)){
			performSignal(onHold.erase(i));
		}
	}
}

void signal0(){

	if (!PCB::running){
		lockCS();
		printf("<SIGNAL0> Fatal error not running\n");
		unlockCS();
		return;
	}

	if (PCB::running->myFather) PCB::running->myFather->signal(1);

	Thread *current = PCB::running->myThread;

	current->signal(2);

	PCB::running->mySem->signal(-(PCB::running->mySem->val));

	PCB::running->finished = 1;

	unlockCS();
	dispatch();
}

void PCB::performSignal(SignalId signal){
	lockCS();

	for(Queue<SignalHandler>::iterator i = handlers[signal].begin(); i != handlers[signal].end();++i){
		(*(*i))();
	}

	if (signal == 0) signal0();

	unlockCS();
}
