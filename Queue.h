#ifndef _queue_h_
#define _queue_h_
template<class T>
class Queue{
	struct Node {
		T info;
		Node * next;
		Node(T t): info(t), next(0){}
		Node(const Node& n): info(n.info), next(n.next){}
	};

	Node *front;
	Node *back;
	Node *i;
public:
	Queue();
	Queue(const Queue<T>&);

	~Queue();
	Queue<T>& operator=(const Queue<T>&);
	void insert(T);
	T remove();

	void begin();
	void next();
	int end();
	T& get();
	T del();
};

template<class T>
Queue<T>::Queue(): front(0), back(0), i(0){}

template<class T>
Queue<T>::Queue(const Queue<T>& q): front(0), back(0), i(0){
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
Queue<T>& Queue<T>::operator =(const Queue<T> & q){
	if(q.front){
			front = new Node(*q.front);

			Node * curr = front;
			while (curr->next){
				curr = curr->next;
			}
			back = curr;
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
void Queue<T>::insert(T t){

	Node * temp = new Node(t);
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
T Queue<T>::remove(){
	T info;

	if (!front) info = 0;
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
T& Queue<T>::get(){
	return i->info;
}
template<class T>
T Queue<T>::del(){
	T info;
	if (!i) info = 0;
	else if (i == front){
        i = 0;
		info = this->remove();
	}
	else {
		info = i->info;

		Node * temp = front;

		while (temp->next != i) temp = temp->next;

		temp->next = i->next;

        if (back == i) back = temp;
		delete i;

        i = temp;
	}

	return info;
}

template<class T>
int Queue<T>::end(){
	if (i) return 1;

	return 0;
}
#endif
