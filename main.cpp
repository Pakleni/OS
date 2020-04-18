#include "thread.h"
#include "Semaphor.h"
#include "IVTEntry.h"
#include "Event.h"

#include "pcb.h"
#include <stdio.h>

void inic();
void restore();

Semaphore * s = new Semaphore(0);

Semaphore * s2 = new Semaphore(0);

PREPAREENTRY(9,1);

class A : public Thread {
public:
	A(StackSize size = defaultStackSize,Time time = defaultTimeSlice) : Thread(size, time){}
	void run() {
		for (int i = 0; i < 30; ++i) {
			printf("a %d\n", i);
			for (int j = 0; j< 30000; ++j)
				for (int k = 0; k < 3000; ++k);
		}
		s->signal(0);
		s2->wait(0);
		printf("Zavrsio a\n");
	}
	~A(){waitToComplete();}
};

class B : public Thread {
public:
	B(StackSize size = defaultStackSize,Time time = defaultTimeSlice) : Thread(size, time){}
	void run() {
		for (int i = 0; i < 30; ++i) {
			printf("b %d\n", i);
			if (i == 20) s->signal(0);
			for (int j = 0; j< 30000; ++j)
				for (int k = 0; k < 3000; ++k);
		  }

		s2->wait(0);

		printf("Zavrsio b\n");
	}
	~B(){waitToComplete();}
};

int userMain(int argc, char * argv[]){

	Event e(9);

	e.wait();

	printf("PACE\n");

	s->wait(80);

	A * a = new A(1024, 10);
	B * b = new B(1024, 20);

	a->start();
	b->start();

	for (int i = 0; i < 60; ++i) {
		printf("main %d\n", i);
		if (i == 2) {
			s->wait(0);
			s->wait(1);
		}
		for (int j = 0; j< 30000; ++j)
			for (int k = 0; k < 3000; ++k);
	}

	s2->signal(2);

	delete a;
	delete b;
}

int main(int argc, char * argv[]){

	inic();

	PCB::running = new PCB(0, 1, 0);

	int ret = userMain(argc, argv);

	delete PCB::running;

	restore();

	return ret;
}
