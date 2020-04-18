#ifndef _list_h_
#define _list_h_
template<class T>
class List{
	struct Node{
		T * info;
		Node * next;
		Node(T * t): info(t), next(0){}
	};
	Node * head;
	Node * i;

public:
	List();
	~List();
	void operator+=(T *);
	void operator-=(T *);
	void begin();
	void next();
	T * get();
};
template<class T>
List<T>::List() : head(0), i(0) {}
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
void List<T>::operator+=(T * t){

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
void List<T>::operator-=(T * t){

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
T * List<T>::get(){
	if (i) return i->info;
	else return 0;
}
#endif
