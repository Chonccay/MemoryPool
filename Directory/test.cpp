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

	/* ʹ��C++Ĭ�ϵ�allocator*/
	/*std::allocator ��C++��׼�����ṩ��Ĭ�Ϸ����� */
	/*��ʹ�� new �������ڴ湹���¶����ʱ���Ʊ�Ҫ����������Ĭ�Ϲ��캯��*/
	/*ʹ�� std::allocator ����Խ��ڴ����Ͷ���Ĺ������������߼������뿪����ʹ�÷�����ڴ���ԭʼ��δ����ġ�*/
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
	std::cout << "Ĭ�ϵ�allocator������ʱ��: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

	/* ʹ��MemoryPool */
	/*StackAlloc ��һ������ջ����������ģ���������һ��������ջ�е�Ԫ�����ͣ��ڶ�����������ջʹ�õ��ڴ��������*/
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
	std::cout << "MemoryPool������ʱ��: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

	/* ��MemoryPool��STL vector�Ƚ� */
	/*std::vector ��ʵ�ַ�ʽ��ʵ���ڴ�ؽ�Ϊ���ƣ��� std::vector �ռ䲻����ʱ��
	  ���������ڵ��ڴ�������������һ���������򣬲��������ڴ������е��������忽��һ�ݵ��������С�*/
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
	std::cout << "Vectorʱ��: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

	//std::cout << "MemoryPool������ҪԶ����std::vector��std::allocator\n";
	//std::cout << "MemoryPool��Ȼ�кܶ���;���κ����͵����Լ������ж������ʱ��������(���ǿ��Թ���ͬһ���ڴ��)\n";
	system("pause");
	return 0;
}