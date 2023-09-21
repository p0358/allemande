#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

class MemoryAddress
{
public:

	std::uintptr_t GetPtr() const
	{
		return ptr;
	}

	MemoryAddress() = default;
	MemoryAddress(std::uintptr_t ptr) : ptr(ptr) {}
	MemoryAddress(void* ptr) : ptr(std::uintptr_t(ptr)) {}
	
	void operator=(std::uintptr_t newptr) { ptr = newptr; }
	void operator=(void* newptr) { ptr = std::uintptr_t(newptr); }

	operator std::uintptr_t() const
	{
		return ptr;
	}

	operator void*() const
	{
		return reinterpret_cast<void*>(ptr);
	}

	template<typename T>
	operator T*() const
	{
		return reinterpret_cast<T*>(ptr);
	}

	operator bool() const
	{
		return ptr != 0;
	}

	bool operator!=(const MemoryAddress& addr) const
	{
		return ptr != addr.ptr;
	}

	bool operator==(const MemoryAddress& addr) const
	{
		return ptr == addr.ptr;
	}

	bool operator==(std::uintptr_t addr) const
	{
		return ptr == addr;
	}

	MemoryAddress& operator+=(std::ptrdiff_t diff)
	{
		ptr += diff;
		return *this;
	}

	friend uintptr_t operator+(const MemoryAddress& adr, std::ptrdiff_t diff)
	{
		return adr.ptr + diff;
	}

	friend uintptr_t operator+(const MemoryAddress& adr, int diff)
	{
		return adr.ptr + diff;
	}

	template<typename T>
	T Cast() const
	{
		return reinterpret_cast<T>(ptr);
	}

	template<class T>
	T GetValue() const
	{
		return *reinterpret_cast<T*>(ptr);
	}

	template<class T>
	T& GetValueRef() const
	{
		return *reinterpret_cast<T*>(ptr);
	}

	MemoryAddress Offset(std::ptrdiff_t offset) const
	{
		return MemoryAddress(ptr + offset);
	}

	MemoryAddress& OffsetSelf(std::ptrdiff_t offset)
	{
		ptr += offset;
		return *this;
	}

	MemoryAddress Deref(int deref = 1)
	{
		std::uintptr_t reference = ptr;

		while (deref--)
		{
			if (reference)
				reference = *reinterpret_cast<std::uintptr_t*>(reference);
		}

		return MemoryAddress(reference);
	}

	MemoryAddress& DerefSelf(int deref = 1)
	{
		while (deref--)
		{
			if (ptr)
				ptr = *reinterpret_cast<std::uintptr_t*>(ptr);
		}

		return *this;
	}

	template<std::size_t SIZE>
	bool CheckOpCodes(const uint8_t (&opcodeArray)[SIZE])
	{
		std::uintptr_t reference = ptr; // Create pointer reference.

		// Loop forward in the ptr class member.
		for (size_t i = 0; i < SIZE; i++, reference++)
		{
			auto byteAtCurrentAddress = *reinterpret_cast<std::uint8_t*>(reference); // Get byte at current address.

			if (byteAtCurrentAddress != opcodeArray[i]) // If byte at ptr doesn't equal in the byte array return false.
				return false;
		}

		return true;
	}

	template<std::size_t SIZE>
	void Patch(const uint8_t (&opcodes)[SIZE])
	{
		memcpy(reinterpret_cast<void*>(ptr), opcodes, SIZE); // Write opcodes to address.
	}

	template<typename T>
	void Patch(const T& data)
	{
		*(T*)(ptr) = data; // Write opcode to address.
	}

	void PatchByte(uint8_t byte)
	{
		if (*(uint8_t*)(ptr) != byte)
			return Patch<uint8_t>(byte);
	}

	void Memset(uint8_t value, size_t num)
	{
		memset((void*)ptr, value, num);
	}

protected:
	std::uintptr_t ptr = 0;
};
