#include "Queue.h"
#include "List.h"
#include "SCHEDULE.H"

#include "IVTEntry.h"

#include "dos.h"

#include "stdio.h"

#include "Event.h"
#include "Kernel.h"
#include "PCB.h"

extern volatile int context_switch_on_demand;

extern void dispatch();

//KERNEL SEM

List<KernelSem *> KernelSem::semaphoreList;

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

void KernelSem::operator--(int){
	for (blocked.begin();blocked.end();blocked.next()){
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

	PCB::lockCS();
	if (--val<0){

		PCB::running->blockedTime = maxTimeToWait;

		block();

		PCB::unlockCS();

		dispatch();

		if (maxTimeToWait && PCB::running->blockedTime == 0){
			return 0;
		}

		PCB::running->blockedTime = 0;
	}
	else {
		PCB::unlockCS();
	}
	return 1;
}

int KernelSem::signal(int n){
	PCB::lockCS();
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

	PCB::unlockCS();
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
		PCB::lockCS();
		--val;

		block();
		PCB::unlockCS();

		dispatch();
	}
}

void KernelEv::signal(){
	if (val == -1){
		PCB::lockCS();
		++val;

		deblock();
		PCB::unlockCS();
	}
}
