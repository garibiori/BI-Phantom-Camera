/**
 * DoublyLinkedList Template file
 * 
 * @author Devin Keller and Ori G
 * @version 1.0
 * @see DoublyLinkedList.h
 * 
 */

#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H
#include <iostream>
#include <exception>
using namespace std;
template <class T> class ListNode{
public:
  ListNode();
  ListNode(T d);
  ~ListNode();
  ListNode *next;
  ListNode *prev;
  T data;
};
//implementation
template <class T>
ListNode<T>::ListNode(){
}
template <class T>
ListNode<T>::ListNode(T d){
  data = d;
  next = NULL; //0 null pointer
  prev = NULL;
}
template <class T>
ListNode<T>::~ListNode(){
}
//doublylinked list class
template <class T> class DoublyLinkedList{
private:
  ListNode<T> *front;
  ListNode<T> *back;
  unsigned int size;
public:
  DoublyLinkedList();
  ~DoublyLinkedList();
  void insertFront(T d);
  void insertBack(T d);
  T removeFront();
  T removeBack();
  T removeNode(T value);
  T find(T value);
  bool isEmpty();
  unsigned int getSize();
};
template <class T>
DoublyLinkedList<T>::DoublyLinkedList(){
  front = NULL;
  back = NULL;
  size = 0;
}
template <class T>
DoublyLinkedList<T>::~DoublyLinkedList(){
  //guess what??? build some character and research
}
template <class T>
void DoublyLinkedList<T>::insertFront(T d){
  ListNode<T> *node = new ListNode<T>(d);
  if(isEmpty()) {
    back = node;
  }
  else{
    //it is not empty
    node->next = front;
    front->prev = node;
  }
  front = node;
  ++size;
}
template <class T>
void DoublyLinkedList<T>::insertBack(T d){
  ListNode<T> *node = new ListNode<T>(d);
  if(isEmpty()){
    front = node;
  }
  else{
    //it is not empty
    node->prev = back;
    back->next = node;
  }
  back = node;
  ++size;
}
template <class T>
T DoublyLinkedList<T>::removeFront(){
  if(isEmpty()) {
    throw runtime_error("list is empty!");
  }
  ListNode<T> *temp = front;
  if(front->next==NULL) {
    //only one node in the list and it's the front node
    back=NULL;
  }
  else{
    //more than one node in the list
    front->next->prev=NULL; //previous pointer of next node after front is NULL
  }
  front = front->next;
  temp->next=NULL;
  T data = temp->data;
  --size;
  delete temp;
  return data;
}
template <class T>
T DoublyLinkedList<T>::removeBack(){
  if(isEmpty()) {
    throw runtime_error("list is empty!");
  }
  ListNode<T> *temp = back;
  if(back->prev == NULL){
    //only one node in the list
    front = NULL;
  }
  else{
    //more than one node in the list
    back->prev->next=NULL; //next pointer of previous node before back is NULL
  }
  back = back->prev;
  temp->prev = NULL;
  T data = temp->data;
  --size;
  delete temp;
  return data;
}
template <class T>
T DoublyLinkedList<T>::find(T value){
  int pos = -1;
  ListNode<T> *curr = front;
  while(curr != NULL){
    ++pos;
    if(curr-> data == value){
      break;
    }
    curr = curr->next;
  }
  if(curr == NULL){
    pos = -1;
  }
  return pos;
}
template <class T>
T DoublyLinkedList<T>::removeNode(T value){
  if(isEmpty()) {
    throw runtime_error("list is empty!");
  }
  //we can leverage the find method to decide whether we can continue or not
  ListNode<T> *curr = front;
  while(curr->data!=value) {
    curr = curr->next;
    if(curr==NULL) {
      //in the end
      return -1;
    }
  }
  //but if we make it here, we found the value
  //we found it, let's proceed to remove it
  if(curr == front){
    front = curr->next;
    front->prev = NULL;
  }
  if(curr == back){ //Two if statements because front can be equal to back (front==back)
    back = curr->prev;
    back->next = NULL;
  }
  if(curr != front && curr != back) {
    //listnode is between front and back
    curr->prev->next = curr->next; //Next node's previous pointer now points to the node before current
    curr->next->prev = curr->prev; //Previous node's next pointer now points to the node after current
  }
  //garbage collect
  curr->next = NULL;
  curr->prev = NULL;
  T data = curr->data;
  --size;
  delete curr;
  return data;
}
template <class T>
unsigned int DoublyLinkedList<T>::getSize(){
  return size;
}
template <class T>
bool DoublyLinkedList<T>::isEmpty(){
  return (size == 0);
}
#endif