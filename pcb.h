#ifndef _pcb_h_
#define _pcb_h_

#include "Queue.h"

typedef unsigned long StackSize;
typedef unsigned int Time;

class Thread;
class KernelSem;

typedef void (*SignalHandler)();
typedef unsigned SignalId;

void lockCS();
void unlockCS();

class PCB{
	static int idCounter;
public:
	static volatile PCB * running;
	static int blockSignalGlobal;

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

	Queue<SignalHandler> handlers[16];
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
	static void f();
public:
	Idle();
	~Idle();
	static Idle idle;
	static int isIdle;
	unsigned ss;
	unsigned sp;
};

#endif
