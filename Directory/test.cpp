#include"stdafx.h"
#include <iostream>
#include <cassert>
#include <time.h>
#include <vector>

#include "MemoryPool.h"
#include "StackAlloc.h"

/*  */
#define ELEMS 10000000
#define REPS 10

int main()
{
	typedef int Element;
	clock_t start;

	/* 使用C++默认的allocator*/
	/*std::allocator 是C++标准库中提供的默认分配器 */
	/*在使用 new 来申请内存构造新对象的时候，势必要调用类对象的默认构造函数*/
	/*使用 std::allocator 则可以将内存分配和对象的构造这两部分逻辑给分离开来，使得分配的内存是原始、未构造的。*/
	StackAlloc<Element, std::allocator<Element> > stackDefault;
	start = clock();
	for (int j = 0; j < REPS; j++)
	{
		assert(stackDefault.empty());
		for (Element i = 0; i < ELEMS; i++) {
			// Unroll to time the actual code and not the loop
			stackDefault.push(i);
			stackDefault.push(i);
			stackDefault.push(i);
			stackDefault.push(i);
		}
		for (Element i = 0; i < ELEMS; i++) {
			// Unroll to time the actual code and not the loop
			stackDefault.pop();
			stackDefault.pop();
			stackDefault.pop();
			stackDefault.pop();
		}
	}
	std::cout << "默认的allocator分配器时间: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

	/* 使用MemoryPool */
	/*StackAlloc 是一个链表栈，接受两个模板参数，第一个参数是栈中的元素类型，第二个参数就是栈使用的内存分配器。*/
	StackAlloc<Element, MemoryPool<Element> > stackPool;
	start = clock();
	for (Element j = 0; j < REPS; j++)
	{
		assert(stackPool.empty());
		for (Element i = 0; i < ELEMS; i++) {
			// Unroll to time the actual code and not the loop
			stackPool.push(i);
			stackPool.push(i);
			stackPool.push(i);
			stackPool.push(i);
		}
		for (Element i = 0; i < ELEMS; i++) {
			// Unroll to time the actual code and not the loop
			stackPool.pop();
			stackPool.pop();
			stackPool.pop();
			stackPool.pop();
		}
	}
	std::cout << "MemoryPool分配器时间: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

	/* 将MemoryPool与STL vector比较 */
	/*std::vector 的实现方式其实和内存池较为类似，在 std::vector 空间不够用时，
	  会抛弃现在的内存区域重新申请一块更大的区域，并将现在内存区域中的数据整体拷贝一份到新区域中。*/
	std::vector<Element> stackVector;
	start = clock();
	for (Element j = 0; j < REPS; j++)
	{
		assert(stackVector.empty());
		for (Element i = 0; i < ELEMS; i++) {
			// Unroll to time the actual code and not the loop
			stackVector.push_back(i);
			stackVector.push_back(i);
			stackVector.push_back(i);
			stackVector.push_back(i);
		}
		for (Element i = 0; i < ELEMS; i++) {
			// Unroll to time the actual code and not the loop
			stackVector.pop_back();
			stackVector.pop_back();
			stackVector.pop_back();
			stackVector.pop_back();
		}
	}
	std::cout << "Vector时间: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

	//std::cout << "MemoryPool的性能要远高于std::vector和std::allocator\n";
	//std::cout << "MemoryPool仍然有很多用途。任何类型的树以及当你有多个链表时都是例子(它们可以共享同一个内存池)\n";
	system("pause");
	return 0;
}