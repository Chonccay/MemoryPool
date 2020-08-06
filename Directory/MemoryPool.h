#pragma once
#include<cstddef>
#include<cstdint>
#include<utility>

template <typename T, size_t BlockSize = 8192> class MemoryPool {
public:
	template <typename U> struct rebind { typedef MemoryPool<U> other; };
	MemoryPool() noexcept;//构造函数，noexcept说明函数不会出现异常
	MemoryPool(const MemoryPool &mp)noexcept;//拷贝构造函数
	MemoryPool(MemoryPool &&mp)noexcept;//移动构造函数
	template <class U>MemoryPool(const MemoryPool<U> &mp)noexcept;//拷贝构造函数的模板
	~MemoryPool() noexcept; //析构函数
	MemoryPool &operator=(const MemoryPool &mp) = delete;//禁用拷贝赋值
	MemoryPool &operator=(MemoryPool &&mp)noexcept;//移动赋值

	/*获取元素所在地址*/
	inline T *address(T &element) const noexcept { return &element; }
	inline const T *address(const T &element)const noexcept { return &element; }

	//以下是Allocator需要实现的一些接口
	/*
	* 从内存池为对象分配内存 *
	*/
	inline T *allocate(size_t n = 1, const T *hint = nullptr) {
		if (freeSlots_ != nullptr) {
			//存在freeSlots_，则分配一个空闲的对象槽
			T *result = reinterpret_cast<T *>(freeSlots_);
			freeSlots_ = freeSlots_->Next;
			return result;
		}
		else {
			//freeSlots_还未初始化或者已经耗尽，分配一个新的内存区块
			if (currentSlot_ >= lastSlot_) {
				allocateBlock();
			}
			return reinterpret_cast<T *>(currentSlot_++);//当前slot后移
		}
	}
	/*
	* 从内存池为对象释放内存 *
	*/
	inline void deallocate(T *p, size_t = 1) {
		if (p != nullptr) {
			//需要将p的下一个节点指向freeSlots_，采用头插法。
			//要访问Next必须将p转换成Slot_ *
			reinterpret_cast<Slot_ *>(p)->Next = freeSlots_;
			freeSlots_ = reinterpret_cast<Slot_ *>(p);
		}
	}

	/*
	* 构造与析构对象 *
	*/
	template <typename U, typename... Args> //可变模板参数
	inline void construct(U *p, Args &&... args) {
		new (p) U(std::forward<Args>(args)...); //使用std::forward转发变参模板
	}
	template <typename U> inline void destroy(U *p) { p->~U(); }

	/*
	* new 与 delete *
	*/
	template <typename... Args>inline T *newElement(Args &&... args) {
		T *result = allocate();
		construct(result, std::forward<Args>(args)...);
		return result;
	}
	inline void deleteElement(T *p) {
		if (p != nullptr) {
			p->~T();
			deallocate(p);
		}
	}

	/*
	* 最多能容纳多少个对象 *
	*/
	inline size_t max_size() const noexcept {
		//-1表示当前能寻址的最大内存地址， -1/BlockSize就是最大Block数
		size_t maxBlocks = -1 / BlockSize;
		//每个Block能容纳的T的个数乘最大Block数
		//cout << maxBlocks << endl;
		return (BlockSize - sizeof(char *)) / sizeof(Slot_)*maxBlocks;
	}


private:
	// 用于存储内存池中的对象槽，要么被实例化为一个存放对象的槽，要么被实例化为一个指向存放对象槽的槽指针
	union Slot_
	{
		T element;
		Slot_ *Next;
	};

	Slot_ *currentBlock_;//指向当前内存区块
	Slot_ *currentSlot_;//指向当前内存区块的一个对象槽
	Slot_ *lastSlot_;  //指向当前内存区块的最后一个对象槽
	Slot_ *freeSlots_; //指向当前内存区块的空闲对象槽

	void allocateBlock();//为内存池分配内存函数
	/*在编译期间执行检查，查看当前定义的内存池是否太小*/
	static_assert(BlockSize >= 2 * sizeof(Slot_), "BlockSize is too small.");
};

/*初始化所有的槽指针*/
template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool() noexcept {
	currentBlock_ = nullptr;
	currentSlot_ = nullptr;
	lastSlot_ = nullptr;
	freeSlots_ = nullptr;
}

template <typename T, size_t BlockSize>
template <typename U>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U> &mp) noexcept
	:MemoryPool() {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() noexcept {
	Slot_ *cur = currentBlock_;
	while (cur != nullptr) {
		Slot_ *nextBlock = cur->Next;//循环销毁内存池中的内存区块
		::operator delete(reinterpret_cast<void *>(cur));
		cur = nextBlock;
	}
}

/*
* 移动赋值 *
*/
template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize> &MemoryPool<T, BlockSize>::
operator=(MemoryPool &&mp)noexcept {
	if (this != &mp) {
		std::swap(currentSlot_, mp.currentSlot_);
		currentSlot_ = mp.currentSlot_;
		lastSlot_ = mp.lastSlot_;
		freeSlots_ = mp.freeSlots_;
	}
	return *this;
}

/*
* 为内存池分配内存 *
*/
template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateBlock() {
	//新的Slot通过头插法插入
	char *newBlock = reinterpret_cast<char *>(::operator new(BlockSize));
	reinterpret_cast<Slot_ *>(newBlock)->Next = currentBlock_;
	currentBlock_ = reinterpret_cast<Slot_ *>(newBlock);
	//需要内存对齐，这是因为第一个Slot区域存放的是上一个Block的指针。
	char *body = newBlock + sizeof(Slot_ *);//body指上一部分已经占用的内存大小
	uintptr_t addr = reinterpret_cast<uintptr_t>(body);//使用reinterpret_cast强制转换body为能够存储指针的无符号整数
	size_t align = alignof(Slot_);//alignof返回Slot_的对齐值
	size_t bodyPadding = (align - addr) % align;//bodyPadding指新占用的内存空间大小
	currentSlot_ = reinterpret_cast<Slot_ *>(body + bodyPadding);//内存对齐后当前指针指向的对象槽
	lastSlot_ = reinterpret_cast<Slot_ *>(newBlock + BlockSize - sizeof(Slot_) + 1);
}