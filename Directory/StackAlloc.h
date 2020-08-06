#pragma once
/*
* ʵ�ּ򵥶�ջ�ṹ��ģ���� *
/*ʹ��һ��ջ�ṹ�������ڴ���ṩ�ķ�������*/
#ifndef STACK_ALLOC_H
#define STACK_ALLOC_H
#include<memory>
template <typename T>
struct  StackNode_
{
	T data;
	StackNode_* prev;
};
template <class T,class Alloc=std::allocator<T>>
class StackAlloc
{
public:
	typedef StackNode_<T> Node;
	typedef typename Alloc::template rebind<Node>::other allocator;//�õ���Alloc���������ͬ������Node�ķ�����
	StackAlloc() { head_ = 0; }//���캯��
	~StackAlloc() { clear(); }//��������
	
	bool empty() { return (head_ == 0); }

	/*�ͷ�ջ��Ԫ�ص������ڴ�*/
	void clear() {
		Node* curr = head_;
		while (curr != nullptr) { //���γ�ջ
			Node* temp = curr->prev;
			/*���������ٻ����ڴ�*/
			allocator_.destroy(curr);
			allocator_.deallocate(curr, 1);
			curr = temp;
		}
		//head_ = nullptr;
		head_ = 0;
	}

	/*��һ��Ԫ�طŵ�ջ��*/
	void push(T element) {
		Node* newNode = allocator_.allocate(1);
		allocator_.construct(newNode, Node());
		newNode->data = element;
		newNode->prev = head_;
		head_ = newNode;
	}

	/*ɾ��������ջ��Ԫ��*/
	T pop() {
		T result = head_->data;
		Node* temp = head_->prev;
		allocator_.destroy(head_);
		allocator_.deallocate(head_, 1);
		head_ = temp;
		return result;
	}

	/*����ջ��Ԫ��*/
	T top() { return head_->data; }
private:
	allocator allocator_;
	Node* head_;
};
#endif // !STACK_ALLOC_H
