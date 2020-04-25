#ifndef _pcb_h_
#define _pcb_h_

#include "List.h"
#include "Queue.h"

typedef unsigned long StackSize;
typedef unsigned int Time;

class Thread;
class KernelSem;

typedef void (*SignalHandler)();
typedef unsigned SignalId;

class PCB{
	static int idCounter;
public:
	static volatile PCB * running;
	static int blockCS;
	static int blockSignalGlobal;
	static void PCB::lockCS();
	static void PCB::unlockCS();

	PCB(StackSize, Time, Thread *);
	~PCB();

	int id;
	unsigned * stack;
	unsigned sp;
	unsigned ss;

	int timeSlice;
	int finished;
	int started;
	int blocked;
	int blockedTime;

	int blockSignal;

	List<SignalHandler> handlers[16];

	Queue<SignalId> onHold;

	void registerHandler(SignalId signal, SignalHandler handler);
	void unregisterHandler(SignalId signal);
	void swap(SignalId id, SignalHandler hand1, SignalHandler hand2);
	void signal(SignalId signal);
	void performSignal(SignalId signal);
	void performSignals();

	KernelSem * mySem;
	Thread * myFather;
	Thread * myThread;

	static void wrapper();

};

void dispatch();

class Idle{
private:
	static const int STACK_SIZE;
	unsigned * stack;
	Idle();
	~Idle();
	static void f();
public:
	static Idle* idle;
	static int init();
	static void end();

	unsigned ss;
	unsigned sp;
};

#endif
