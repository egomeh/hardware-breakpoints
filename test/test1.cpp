
#include "HardwareBreakpoint.h"

int main()
{
	// Lets make 8 bytes and set our bp on the whole range
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
	

	return 0;
}
