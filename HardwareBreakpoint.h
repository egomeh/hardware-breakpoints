#pragma once

#include <windows.h>

class HardwareBreakpoint
{
public:
	enum class Condition : unsigned char
	{
		Write = 1,
		ReadWrite = 3
	};


	HardwareBreakpoint() : m_index(-1)
	{
	}

	~HardwareBreakpoint()
	{
		Clear();
	}

	bool Set(void* address, int size, Condition condition)
	{
		// This breakpoint is already set
		if (m_index != -1)
			return false;

		unsigned char sizeBytePattern = 0;
		switch (size)
		{
		case 1: sizeBytePattern = 0b00; break;
		case 2: sizeBytePattern = 0b01; break;
		case 4: sizeBytePattern = 0b11; break;
		case 8: sizeBytePattern = 0b10; break;
		default: return false; // Invalid range to watch
		}

		HANDLE thread = GetCurrentThread();
		CONTEXT context = {};
		context.ContextFlags |= CONTEXT_DEBUG_REGISTERS;

		if (!GetThreadContext(thread, &context))
			return false;

		// find an unused debug register
		for (int i = 0; i < 4; ++i)
		{
			int mask = 0b11 << (i * 2);

			if ((context.Dr7 & mask) > 0)
				continue;

			m_index = i;
			break;
		}
		
		// no available register :(
		if (m_index == -1)
			return false;

		// reset local and global bits
		context.Dr7 &= ~(((DWORD64)-1) & (0b11 << (m_index * 2)));

		// reset size and condition
		context.Dr7 &= ~(((DWORD64)-1) & (0b1111 << (16 + m_index * 4)));

		// set local bp bit
		context.Dr7 |= (DWORD64)1 << (m_index * 2);

		// set the condition
		context.Dr7 |= (DWORD64)((int)condition & 0b11) << (16 + m_index * 4);

		// set the size
		context.Dr7 |= (DWORD64)sizeBytePattern << (18 + m_index * 4);
		
		// set the address register
		*((DWORD64*)&context.Dr0 + m_index) = (DWORD64)address;

		if (!SetThreadContext(thread, &context))
			return false;

		return true;
	}

	void Clear()
	{
		if (m_index == -1)
			return;

		HANDLE thread = GetCurrentThread();
		CONTEXT context = {};
		context.ContextFlags |= CONTEXT_DEBUG_REGISTERS;

		if (!GetThreadContext(thread, &context))
			return;

		// reset local and global bits
		context.Dr7 &= ~(((DWORD64)-1) & (0b11 << (m_index * 2)));

		// reset size and condition
		context.Dr7 &= ~(((DWORD64)-1) & (0b1111 << (16 + m_index * 4)));

		SetThreadContext(thread, &context);
	}

private:

	int m_index;
};

