#ifndef _queue_h_
#define _queue_h_
template<class T>
class Queue{
	struct Node {
		T * info;
		Node * next;
		Node(T * t): info(t), next(0){}
	};

	Node *front;
	Node *back;
	Node *i;
public:
	Queue();
	~Queue();
	void insert(T *);
	T * remove();

	void begin();
	void next();
	T * get();

	T * del();
};

template<class T>
Queue<T>::Queue(): front(0), back(0), i(0){

}

template<class T>
Queue<T>::~Queue(){
	Node * temp;
	while (front){
		temp = front;
		front = front->next;
		delete temp;
	}
}

template<class T>
void Queue<T>::insert(T * t){

	Node * temp = new Node(t);
	if (!front){
		front = temp;
		back = temp;
	}
	else {
		back->next = temp;
		back = back->next;
	}
}

template<class T>
T * Queue<T>::remove(){
	T * info;
	if (!front) info = 0;
	else if (front == back){
		info = front->info;
		delete front;
		back = front = 0;
	}
	else {
		info = front->info;
		Node * temp = front;
		front = front->next;
		delete temp;
	}

	return info;
}
//sets iterator to head
template<class T>
void Queue<T>::begin(){
	i = front;
}
//moves iterator
template<class T>
void Queue<T>::next(){
	if(i) i = i->next;
	else i = front;
}
//gets iterator
template<class T>
T * Queue<T>::get(){
	if (i) return i->info;
	else return 0;
}
template<class T>
T * Queue<T>::del(){
	T * info;
	if (!i) info = 0;
	else if (i == front){
		info = this->remove();
	}
	else {
		info = i->info;

		Node * temp = front;

		while (temp->next != i) temp = temp-> next;

		i = temp;
		temp = temp->next;

		i->next = temp->next;

		delete temp;
	}

	return info;
}
#endif
