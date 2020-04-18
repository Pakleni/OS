/*
 * IVTEntry.h
 *
 *  Created on: Apr 8, 2020
 *      Author: OS1
 */

#ifndef IVTENTRY_H_
#define IVTENTRY_H_

#include "dos.h"
#include "kernel.h"

typedef unsigned char IVTNo;

typedef void interrupt (*pInterrupt)(...);

#define PREPAREENTRY(x,y)									\
	void interrupt inter##x(...){               			\
		IVTEntry::getIVTE(x)->event->signal();				\
		if(y) {IVTEntry::getIVTE(x)->oldInter();}}  	 	\
	IVTEntry * ___ivt##x = new IVTEntry(x, inter##x);

class KernelEv;

class IVTEntry{
public:

	static IVTEntry * IVTEList[];
	static IVTEntry * getIVTE(IVTNo);
	static void restore();

	void setEvent(KernelEv *);

	KernelEv * event;
	int n;
	pInterrupt oldInter, inter;

	IVTEntry(IVTNo, pInterrupt);
	~IVTEntry();
};

#endif /* IVTENTRY_H_ */

