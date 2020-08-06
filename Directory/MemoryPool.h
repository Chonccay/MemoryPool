#pragma once
#include<cstddef>
#include<cstdint>
#include<utility>

template <typename T, size_t BlockSize = 8192> class MemoryPool {
public:
	template <typename U> struct rebind { typedef MemoryPool<U> other; };
	MemoryPool() noexcept;//���캯����noexcept˵��������������쳣
	MemoryPool(const MemoryPool &mp)noexcept;//�������캯��
	MemoryPool(MemoryPool &&mp)noexcept;//�ƶ����캯��
	template <class U>MemoryPool(const MemoryPool<U> &mp)noexcept;//�������캯����ģ��
	~MemoryPool() noexcept; //��������
	MemoryPool &operator=(const MemoryPool &mp) = delete;//���ÿ�����ֵ
	MemoryPool &operator=(MemoryPool &&mp)noexcept;//�ƶ���ֵ

	/*��ȡԪ�����ڵ�ַ*/
	inline T *address(T &element) const noexcept { return &element; }
	inline const T *address(const T &element)const noexcept { return &element; }

	//������Allocator��Ҫʵ�ֵ�һЩ�ӿ�
	/*
	* ���ڴ��Ϊ��������ڴ� *
	*/
	inline T *allocate(size_t n = 1, const T *hint = nullptr) {
		if (freeSlots_ != nullptr) {
			//����freeSlots_�������һ�����еĶ����
			T *result = reinterpret_cast<T *>(freeSlots_);
			freeSlots_ = freeSlots_->Next;
			return result;
		}
		else {
			//freeSlots_��δ��ʼ�������Ѿ��ľ�������һ���µ��ڴ�����
			if (currentSlot_ >= lastSlot_) {
				allocateBlock();
			}
			return reinterpret_cast<T *>(currentSlot_++);//��ǰslot����
		}
	}
	/*
	* ���ڴ��Ϊ�����ͷ��ڴ� *
	*/
	inline void deallocate(T *p, size_t = 1) {
		if (p != nullptr) {
			//��Ҫ��p����һ���ڵ�ָ��freeSlots_������ͷ�巨��
			//Ҫ����Next���뽫pת����Slot_ *
			reinterpret_cast<Slot_ *>(p)->Next = freeSlots_;
			freeSlots_ = reinterpret_cast<Slot_ *>(p);
		}
	}

	/*
	* �������������� *
	*/
	template <typename U, typename... Args> //�ɱ�ģ�����
	inline void construct(U *p, Args &&... args) {
		new (p) U(std::forward<Args>(args)...); //ʹ��std::forwardת�����ģ��
	}
	template <typename U> inline void destroy(U *p) { p->~U(); }

	/*
	* new �� delete *
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
	* ��������ɶ��ٸ����� *
	*/
	inline size_t max_size() const noexcept {
		//-1��ʾ��ǰ��Ѱַ������ڴ��ַ�� -1/BlockSize�������Block��
		size_t maxBlocks = -1 / BlockSize;
		//ÿ��Block�����ɵ�T�ĸ��������Block��
		//cout << maxBlocks << endl;
		return (BlockSize - sizeof(char *)) / sizeof(Slot_)*maxBlocks;
	}


private:
	// ���ڴ洢�ڴ���еĶ���ۣ�Ҫô��ʵ����Ϊһ����Ŷ���Ĳۣ�Ҫô��ʵ����Ϊһ��ָ���Ŷ���۵Ĳ�ָ��
	union Slot_
	{
		T element;
		Slot_ *Next;
	};

	Slot_ *currentBlock_;//ָ��ǰ�ڴ�����
	Slot_ *currentSlot_;//ָ��ǰ�ڴ������һ�������
	Slot_ *lastSlot_;  //ָ��ǰ�ڴ���������һ�������
	Slot_ *freeSlots_; //ָ��ǰ�ڴ�����Ŀ��ж����

	void allocateBlock();//Ϊ�ڴ�ط����ڴ溯��
	/*�ڱ����ڼ�ִ�м�飬�鿴��ǰ������ڴ���Ƿ�̫С*/
	static_assert(BlockSize >= 2 * sizeof(Slot_), "BlockSize is too small.");
};

/*��ʼ�����еĲ�ָ��*/
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
		Slot_ *nextBlock = cur->Next;//ѭ�������ڴ���е��ڴ�����
		::operator delete(reinterpret_cast<void *>(cur));
		cur = nextBlock;
	}
}

/*
* �ƶ���ֵ *
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
* Ϊ�ڴ�ط����ڴ� *
*/
template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateBlock() {
	//�µ�Slotͨ��ͷ�巨����
	char *newBlock = reinterpret_cast<char *>(::operator new(BlockSize));
	reinterpret_cast<Slot_ *>(newBlock)->Next = currentBlock_;
	currentBlock_ = reinterpret_cast<Slot_ *>(newBlock);
	//��Ҫ�ڴ���룬������Ϊ��һ��Slot�����ŵ�����һ��Block��ָ�롣
	char *body = newBlock + sizeof(Slot_ *);//bodyָ��һ�����Ѿ�ռ�õ��ڴ��С
	uintptr_t addr = reinterpret_cast<uintptr_t>(body);//ʹ��reinterpret_castǿ��ת��bodyΪ�ܹ��洢ָ����޷�������
	size_t align = alignof(Slot_);//alignof����Slot_�Ķ���ֵ
	size_t bodyPadding = (align - addr) % align;//bodyPaddingָ��ռ�õ��ڴ�ռ��С
	currentSlot_ = reinterpret_cast<Slot_ *>(body + bodyPadding);//�ڴ�����ǰָ��ָ��Ķ����
	lastSlot_ = reinterpret_cast<Slot_ *>(newBlock + BlockSize - sizeof(Slot_) + 1);
}