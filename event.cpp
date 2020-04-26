#include "Event.h"
#include "Kernel.h"

void lockCS();
void unlockCS();

Event::Event(IVTNo ivtNo): myImpl(0){
	lockCS();
	myImpl = new KernelEv(ivtNo);
	unlockCS();
}

Event::~Event(){
	lockCS();
	delete myImpl;
	unlockCS();
}

void Event::wait(){myImpl->wait();}

void Event::signal(){myImpl->signal();}
