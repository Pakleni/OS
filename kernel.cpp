#include "Kernel.h"
#include "SCHEDULE.H"

#include "IVTEntry.h"

#include "dos.h"

#include "stdio.h"

#include "Event.h"
#include "PCB.h"

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
	blocked.push((PCB *)PCB::running);

	PCB::running->blocked = 1;
}

void KernelSem::deblock(){
	PCB * temp = blocked.pop();

	if (!temp){
		lockCS();
		printf("<SEM> Fatal Error, Nothing Blocked\n");
		unlockCS();
	}

	temp->blocked = 0;

	Scheduler::put(temp);
}

void KernelSem::operator--(int){
	for (Queue<PCB*>::iterator i = blocked.begin(); i != blocked.end();++i){
		if ((*i)->blockedTime > 0){
			if (--((*i)->blockedTime) == 0){
				PCB * temp = blocked.erase(i);
				if (!temp){
					lockCS();
					printf("<SEM> Fatal Error, can't decrease time on null\n");
					unlockCS();
					continue;
				}
				temp->blocked = 0;
				++val;
				Scheduler::put(temp);
			}
		}
	}
}

int KernelSem::wait(Time maxTimeToWait){

	lockCS();
	if (--val<0){

		PCB::running->blockedTime = maxTimeToWait;

		block();

		unlockCS();

		dispatch();

		if (maxTimeToWait && PCB::running->blockedTime == 0){
			return 0;
		}

		PCB::running->blockedTime = 0;
	}
	else {
		unlockCS();
	}

	return 1;
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
	if (!blocked){
		lockCS();
		printf("<EVENT> Nothing blocked\n");
		unlockCS();
	}
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
