/* ---------------------------------------------------------------------------------------
* MIT License
*
* Copyright (c) 2023 Davut Coþkun.
* All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* ---------------------------------------------------------------------------------------
*/

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
