#include "event.h"
#include "kernel.h"
#include "Queue.h"
#include "pcb.h"
#include "List.h"
#include "SCHEDULE.H"

#include "IVTEntry.h"

#include "dos.h"

#include "stdio.h"

extern volatile int context_switch_on_demand;

extern void lockCS();
extern void unlockCS();

extern void dispatch();

//KERNEL SEM

List<KernelSem> KernelSem::semaphoreList;

KernelSem::KernelSem(int init): val(init){
	semaphoreList+=this;
}

KernelSem::~KernelSem(){
	semaphoreList-=this;
}

void KernelSem::block(){

	blocked.insert((PCB *)PCB::running);

	PCB::running->blocked = 1;
}

void KernelSem::deblock(){
	PCB * temp = blocked.remove();

	temp->blocked = 0;

	Scheduler::put(temp);
}

void KernelSem::operator--(){
	for (blocked.begin();blocked.get();blocked.next()){
		if (blocked.get()->blockedTime > 0){
			if (--(blocked.get()->blockedTime) == 0){


				PCB * temp = blocked.del();
				temp->blocked = 0;
				++val;
				Scheduler::put(temp);

			}
		}
	}
}

int KernelSem::wait(Time maxTimeToWait){

	int ret = 1;
	if (--val<0){
		lockCS();
		PCB::running->blockedTime = maxTimeToWait;

		block();

		if (maxTimeToWait && PCB::running->blockedTime == 0){
			val++;
			ret = 0;
		}

		unlockCS();

		context_switch_on_demand = 1;
		dispatch();

		PCB::running->blockedTime = 0;
	}
	return ret;
}

int KernelSem::signal(int n){
	lockCS();
	int ret;

	if (n > 0){

		for (int i = 0; i < (n > - val ? - val : n); i ++){
			deblock();
		}

		val+=n;

		ret = (val > 0 ? n - val : n);
	}
	else if (n == 0){
		if (val++<0) deblock();
		ret = 0;
	}
	else ret = n;

	unlockCS();
	return ret;
}


//KERNELEV



KernelEv::KernelEv(IVTNo ivtNo): val(0), blocked(0), tId(PCB::running->id){
	IVTEntry::getIVTE(ivtNo)->setEvent(this);
}

void KernelEv::block(){
	blocked = (PCB *) PCB::running;
	PCB::running->blocked = 1;
}

void KernelEv::deblock(){
	blocked->blocked = 0;
	Scheduler::put(blocked);
	blocked = 0;
}

void KernelEv::wait(){

	if (PCB::running->id != tId) return;

	if (val == 0){
		lockCS();
		--val;

		block();
		unlockCS();

		context_switch_on_demand = 1;
		dispatch();
	}
}

void KernelEv::signal(){
	if (val == -1){
		lockCS();
		++val;

		deblock();
		unlockCS();
	}
}
