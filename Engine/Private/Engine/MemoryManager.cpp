
#include "pch.h"
#include "Engine/MemoryManager.h"

namespace MemoryManager
{

#if MemoryLeakDetector

	void* operator new(std::size_t sz)
	{
		std::printf("global op new called, size = %zu\n", sz);
		if (sz == 0)
			++sz; // avoid std::malloc(0) which may return nullptr on success

		if (void* ptr = std::malloc(sz))
		{
			return ptr;
		}

		throw std::bad_alloc{}; // required by [new.delete.single]/3
	}

	void* operator new (size_t size, const char* filename, int line)
	{
		void* ptr = ::operator new(size);
		return ptr;
	}

	//#define new new(__FILE__, __LINE__)

	void operator delete(void* ptr) noexcept
	{
		/*if (std::find(memory.begin(), memory.end(), ptr) != memory.end())
		{
			if (auto obj = static_cast<Object*>(ptr))
			{
				std::cout << "TypeID-obj = " << typeid(obj).name() << std::endl;
			}
			std::cout << "TypeID = " << typeid(ptr).name() << std::endl;
			memory.erase(std::remove(memory.begin(), memory.end(), ptr), memory.end());
		}*/
		//auto ind = std::distance(myAlloc.begin(), std::find(myAlloc.begin(), myAlloc.end(), ptr));
		//myAlloc[ind] = nullptr;
		std::free(ptr);
	}
#endif
}
