#include "defs.h"

void operator-(TReturn ret)
{
	bool pass = TFAILED(ret);
	assert(!pass && TFAIL_MSG(ret));
}
