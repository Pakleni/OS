#ifndef _pcb_h_
#define _pcb_h_

typedef unsigned long StackSize;
typedef unsigned int Time;

class Thread;
class KernelSem;

class PCB{
public:
	static int idCounter;
	static volatile PCB * running;
	static volatile Thread * idle;
	static int blockCS;

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


	KernelSem * mySem;
	Thread * myThread;

	static void wrapper();

};

void lockCS();
void unlockCS();
void dispatch();

#endif
