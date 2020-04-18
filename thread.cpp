#include "thread.h";
#include "pcb.h";
#include "kernel.h"
#include "List.h"
#include "SCHEDULE.H"

#include "stdio.h"

//const StackSize maxStackSize = 65536/sizeof(unsigned);

List<Thread> threadList;

Thread::Thread (StackSize stackSize, Time timeSlice){
	myPCB = new PCB(stackSize,timeSlice, this);
	threadList+=this;
}

Thread::~Thread(){
	delete myPCB;
	threadList-=this;
}

ID Thread::getId(){
	return myPCB->id;
}

ID Thread::getRunningId(){
	return PCB::running->id;
}

Thread * Thread::getThreadById(ID id){
	for (threadList.begin(); threadList.get(); threadList.next()){
		if (threadList.get()->myPCB->id == id){
			return threadList.get();
		}
	}


	return 0;
}

void Thread::waitToComplete(){
#ifndef BCC_BLOCK_IGNORE

	if(myPCB->finished == 0) myPCB->mySem->wait(0);

#endif
}

void Thread::start(){
	if (!myPCB->started){
		Scheduler::put(myPCB);
		myPCB->started = 1;
	}
}
