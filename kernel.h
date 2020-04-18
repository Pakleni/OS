#ifndef _kernel_h_
#define _kernel_h_
#include "Queue.h"
#include "List.h"

class Semaphore;
class PCB;

class IVTEntry;

typedef unsigned char IVTNo;

typedef unsigned int Time;

class KernelSem{
public:
	static List<KernelSem> semaphoreList;

	int val;
	Queue<PCB> blocked;

	KernelSem(int);
	~KernelSem();

	void block();
	void deblock();

	void operator--();

	int wait(Time);
	int signal(int);
};

class KernelEv{
public:

	int tId;

	int val;

	PCB * blocked;

	KernelEv(IVTNo);
	void block();
	void deblock();

	void wait();
	void signal();

};

#endif
