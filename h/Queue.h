#ifndef _queue_h_
#define _queue_h_

#include <stdio.h>

void lockCS();
void unlockCS();

template<class T>
class Queue{
	struct Node {
		T info;
		Node * next;
		Node(T t): info(t), next(0){}
		Node(const Node& n): info(n.info), next((n.next) ? (new Node(*(n.next))) : 0){}
	};

	Node *front;
	Node *back;
public:

	class iterator{
		iterator(Node *_i, Queue* _q): i(_i), q(_q){}
		Node * i;
		Queue * q;
	public:
		//prefix
		Queue<T>::iterator& operator++();
		//postfix
		Queue<T>::iterator operator++(int);
		//dereference
		T& operator*(){
			if (!i) {
				lockCS();
				printf("<QUEUE> dereferencing null\n");
				unlockCS();
			}
			return i->info;
		}

		int operator==(Queue<T>::iterator& it){
			return (i == it.i);
		}

		int operator!=(Queue<T>::iterator& it){
			return (i != it.i);
		}
		friend Queue;
	};

	Queue();
	Queue(const Queue<T>&);

	~Queue();
	Queue<T>& operator=(const Queue<T>&);
	void push(T);
	T pop();

	Queue<T>::iterator end(){
		return iterator(0, this);
	}

	Queue<T>::iterator begin(){
		return iterator(front, this);
	}

	T erase(Queue<T>::iterator& it){
		T info;
		if (!it.i){
			lockCS();
			printf("<QUEUE> Fatal no current to erase\n");
			unlockCS();
		}
		else if (it.i == front){
			it.i = 0;
			info = this->pop();
		}
		else {
			info = it.i->info;

			Node * temp = front;

			while (temp->next != it.i) temp = temp->next;

			temp->next = it.i->next;

			if (back == it.i) back = temp;
			delete it.i;

			it.i = temp;
		}

		return info;
	}

	friend Queue<T>::iterator& iterator::operator++(){
		if (!i) {i = q->front;}
		else{i = i->next;}
		return *this;
	}
	friend Queue<T>::iterator iterator::operator++(int){
		Queue<T>::iterator it = *this;
		if (!i) {i = q->front;}
		else{i = i->next;}
		return it;
	}
};

//Basic
template<class T>
Queue<T>::Queue(): front(0), back(0){}

template<class T>
Queue<T>::Queue(const Queue<T>& q): front(0), back(0){
	if(q.front){
		front = new Node(*q.front);

		Node * curr = front;
		while (curr->next){
			curr = curr->next;
		}
		back = curr;
	}
}

template<class T>
Queue<T>& Queue<T>::operator=(const Queue<T> & q){
	if(q.front){
		front = new Node(*q.front);

		Node * curr = front;
		while (curr->next){
			curr = curr->next;
		}
		back = curr;
	}
	else {
		front = 0;
		back = 0;
	}

	return *this;
}

template<class T>
Queue<T>::~Queue(){
	Node * temp;
    while (front){
        temp = front->next;
        delete front;
        front = temp;
    }
}

template<class T>
void Queue<T>::push(T t){

	Node * temp = new Node(t);

	if (!temp){
		lockCS();
		printf("<QUEUE> Fatal new Node\n");
		unlockCS();
	}

	if (!front){
		front = temp;
		back = temp;
	}
	else {
		back->next = temp;
		back = temp;
	}
}

template<class T>
T Queue<T>::pop(){
	T info;

	if (!front) {
		lockCS();
		printf("<QUEUE> Fatal Empty Queue\n");
		unlockCS();
	}
	else if (front == back){
		info = front->info;
		delete front;
		back = 0;
		front = 0;
	}
	else {
		info = front->info;
		Node * temp = front;
		front = front->next;
		delete temp;
	}

	return info;
}
#endif
