
#include "HardwareBreakpoint.h"
#include <thread>

int main()
{
	// Check on single bytes
	{
		// Check on eight bytes
		char bytes[32] = {};

		HardwareBreakpoint hbp;
		hbp.Set((void*)&bytes[1], 1, HardwareBreakpoint::Condition::Write);

		HardwareBreakpoint hbp2;
		hbp2.Set((void*)&bytes[3], 1, HardwareBreakpoint::Condition::Write);

		for (int i = 0; i < 10; ++i)
		{
			// Expect to break when i in [1, 3]
			bytes[i] = i + 2;
		}
	}

	// Check on acces from another thread
	{
		uint32_t value = 1;

		HANDLE wait = CreateEventA(NULL, FALSE, FALSE, "");

		auto function = [&value, &wait]()
		{
			WaitForSingleObject(wait, INFINITE);

			// Expect to break here
			value = 2;
			return;
		};

		std::thread thread(function);

		HardwareBreakpoint hbp;
		hbp.Set(&value, 4, HardwareBreakpoint::Condition::Write);

		SetEvent(wait);

		thread.join();
	}

	return 0;
}
