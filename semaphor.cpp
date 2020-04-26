#include "stdio.h"
#include "Kernel.h"
#include "Semaphor.h"

void lockCS();
void unlockCS();

typedef unsigned int Time;

Semaphore::Semaphore(int init): myImpl(){
	lockCS();
	myImpl = new KernelSem(init);
	unlockCS();
}

Semaphore::~Semaphore(){
	lockCS();
	delete myImpl;
	unlockCS();
}

int Semaphore::wait(Time maxTimeToWait){return myImpl->wait(maxTimeToWait);}

int Semaphore::signal(int n){return myImpl->signal(n);}

int Semaphore::val() const{return myImpl->val;}
