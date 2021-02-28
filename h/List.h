#ifndef _list_h_
#define _list_h_

#include<stdio.h>

void lockCS();
void unlockCS();

template<class T>
class List{
	struct Node{
		T info;
		Node * next;
		Node(T t): info(t), next(0){}
		Node(const Node& n): info(n.info), next((n.next) ? (new Node(*(n.next))) : 0){}
	};
	Node * head;

public:
	class iterator{
		iterator(Node *n): i(n){}
		Node * i;
	public:
		//prefix
		List<T>::iterator& operator++(){
			i = i->next;
			return *this;
		}
		//postfix
		List<T>::iterator operator++(int){
			List<T>::iterator it = *this;
			i = i->next;
			return it;
		}
		//dereference
		T& operator*(){
			if (!i){
				lockCS();
				printf("<LIST> dereferencing null\n");
				unlockCS();
			}
			return i->info;
		}

		int operator==(List<T>::iterator& it){
			return (i == it.i);
		}
		int operator!=(List<T>::iterator& it){
			return !(i == it.i);
		}
		friend List;
	};

	List();
	List(const List<T> &);
	~List();
	List<T>& operator=(const List<T> &);
	void operator+=(T);
	void operator-=(T);

	List<T>::iterator end(){
		return iterator(0);
	}

	List<T>::iterator begin(){
		return iterator(head);
	}

	T pop();
};

template<class T>
List<T>::List() : head(0){}
template<class T>
List<T>::List(const List<T>& li) : head(0){
	if(li.head){
		head = new Node(*li.head);
	}
}
template<class T>
List<T>& List<T>::operator=(const List<T> & li){
	if(li.head){
		head = new Node(*li.head);
	}
	else {
		head = 0;
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


	if (!temp) {
		lockCS();
		printf("<LIST> Fatal new Node\n");
		unlockCS();
	}

	temp->next = head;
	head = temp;

}
//Called when deleting a Thread
template<class T>
void List<T>::operator-=(T t){

	if (!head){
		lockCS();
		printf("<LIST> Empty List\n");
		unlockCS();
		return;
	}

	Node * curr = head;
	Node * temp;

	if (curr->info != t){
		while(curr->next && curr->next->info != t){
			curr = curr->next;
		}
		if(!curr->next) {
			lockCS();
			printf("<LIST> Not in list\n");
			unlockCS();
			return;
		}
		temp = curr->next;
		curr->next = curr->next->next;
	}
	else {
		temp = head;
		head = head->next;
	}

	delete temp;

}

template<class T>
T List<T>::pop(){
	if (!head){
		lockCS();
		printf("<LIST> Empty List\n");
		unlockCS();
		return 0;
	}

	T info = head->info;
	Node * temp = head;

	head = head->next;
	delete temp;

	return info;

}

#endif
