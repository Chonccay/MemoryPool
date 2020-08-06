#pragma once
/*
* 实现简单堆栈结构的模板类 *
/*使用一个栈结构来测试内存池提供的分配性能*/
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
	typedef typename Alloc::template rebind<Node>::other allocator;//得到和Alloc分配策略相同的类型Node的分配器
	StackAlloc() { head_ = 0; }//构造函数
	~StackAlloc() { clear(); }//析构函数
	
	bool empty() { return (head_ == 0); }

	/*释放栈中元素的所有内存*/
	void clear() {
		Node* curr = head_;
		while (curr != nullptr) { //依次出栈
			Node* temp = curr->prev;
			/*先析构，再回收内存*/
			allocator_.destroy(curr);
			allocator_.deallocate(curr, 1);
			curr = temp;
		}
		//head_ = nullptr;
		head_ = 0;
	}

	/*把一个元素放到栈顶*/
	void push(T element) {
		Node* newNode = allocator_.allocate(1);
		allocator_.construct(newNode, Node());
		newNode->data = element;
		newNode->prev = head_;
		head_ = newNode;
	}

	/*删除并返回栈顶元素*/
	T pop() {
		T result = head_->data;
		Node* temp = head_->prev;
		allocator_.destroy(head_);
		allocator_.deallocate(head_, 1);
		head_ = temp;
		return result;
	}

	/*返回栈顶元素*/
	T top() { return head_->data; }
private:
	allocator allocator_;
	Node* head_;
};
#endif // !STACK_ALLOC_H
