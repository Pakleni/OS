#include "List.h"
#include "SCHEDULE.H"

#include "stdio.h"
#include "Kernel.h"
#include "Thread.h"
#include "PCB.h"


static List<Thread *> _threadList;

Thread::Thread (StackSize stackSize, Time timeSlice){
	lockCS();
	myPCB = new PCB(stackSize,timeSlice, this);
	_threadList+=this;
	unlockCS();
}

Thread::~Thread(){
	lockCS();
	delete myPCB;
	_threadList-=this;
	unlockCS();
}

ID Thread::getId(){
	return myPCB->id;
}

ID Thread::getRunningId(){
	return PCB::running->id;
}

Thread * Thread::getThreadById(ID id){
	for (List<Thread*>::iterator i = _threadList.begin(); i != _threadList.end(); ++i){
		if ((*i)->myPCB->id == id){
			return *i;
		}
	}


	return 0;
}

void Thread::waitToComplete(){
	if(myPCB != PCB::running && myPCB->finished == 0) myPCB->mySem->wait(0);

}

void Thread::start(){
	if (!myPCB->started){
		Scheduler::put(myPCB);
		myPCB->started = 1;
	}
}

//signals

void Thread::signal(SignalId signal){
	myPCB->signal(signal);
}

void Thread::registerHandler(SignalId signal, SignalHandler handler){
	myPCB->registerHandler(signal, handler);
}

void Thread::unregisterAllHandlers(SignalId id){
	myPCB->unregisterHandler(id);
}

void Thread::swap(SignalId id, SignalHandler hand1, SignalHandler hand2){
	myPCB->swap(id, hand1, hand2);
}

void Thread::blockSignal(SignalId signal){
	myPCB->blockSignal |= 1 << signal;
}

void Thread::blockSignalGlobally(SignalId signal){
	PCB::blockSignalGlobal |= 1 << signal;
}

void Thread::unblockSignal(SignalId signal){
	myPCB->blockSignal &= -1 - 1 << signal;
}

void Thread::unblockSignalGlobally(SignalId signal){
	PCB::blockSignalGlobal &= -1 - 1 << signal;
}
