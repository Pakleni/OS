#ifndef _list_h_
#define _list_h_
template<class T>
class List{
	struct Node{
		T info;
		Node * next;
		Node(T t): info(t), next(0){}
		Node(const Node& n): info(n.info), next(n.next){}
	};
	Node * head;
	Node * i;

public:
	List();
	List(const List<T> &);
	~List();
	List<T>& operator=(const List<T> &);
	void operator+=(T);
	void operator-=(T);
	void begin();
	void next();
	T& get();
	int end();
};
template<class T>
List<T>::List() : head(0), i(0) {}
template<class T>
List<T>::List(const List<T>& li) : head(0), i(0) {
	if(li.head){
		head = new Node(*li.head);
	}
}
template<class T>
List<T>& List<T>::operator=(const List<T> & li){
	if(li.head){
		head = new Node(*li.head);
	}

	return *this;
}

template<class T>
List<T>::~List(){
	Node * temp;
	while (head){
		temp = head;
		head = head->next;
		delete temp;
	}
}
//Called when creating a Thread
template<class T>
void List<T>::operator+=(T t){

	Node * temp = new Node(t);

	Node * curr = head;

	if (curr){
		while(curr->next){
			curr = curr->next;
		}

		curr->next = temp;
	}
	else {
		head = temp;
	}
}
//Called when deleting a Thread
template<class T>
void List<T>::operator-=(T t){

	Node * curr = head;
	Node * temp;

	if (curr->info != t){
		while(curr && curr->next->info != t){
			curr = curr->next;
		}
		if(!curr) return;
		curr->next = curr->next->next;
	}
	else {
		temp = head;
		head = head->next;
		temp->next = 0;
		delete temp;
	}
}
//sets iterator to head
template<class T>
void List<T>::begin(){
	i = head;
}
//moves iterator
template<class T>
void List<T>::next(){
	if(i) i = i->next;
}
//gets iterator
template<class T>
T& List<T>::get(){
	return i->info;
}

template<class T>
int List<T>::end(){
	if (i) return 1;

	return 0;
}
#endif
