#include "stdio.h"
#include "Kernel.h"
#include "Semaphor.h"
#include "PCB.h"

typedef unsigned int Time;

Semaphore::Semaphore(int init): myImpl(new KernelSem(init)){}

Semaphore::~Semaphore(){delete myImpl;}

int Semaphore::wait(Time maxTimeToWait){return myImpl->wait(maxTimeToWait);}

int Semaphore::signal(int n){return myImpl->signal(n);}

int Semaphore::val() const{return myImpl->val;}
