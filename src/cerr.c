#include "cerr.h"

const char *cerrs[] = {
	[CERR_OK]	 = "OK",
	[CERR_MEM]	 = "Out of memory",
	[CERR_VAL]	 = "Invalid argument",
	[CERR_EXIST]	 = "Already exists",
	[CERR_NOT_FOUND] = "Not found",
	[CERR_DESC]	 = "Bad descriptor",
	[CERR_NOT_EMPTY] = "Not empty",
	[CERR_TYPE]	 = "Wrong type",
	[CERR_END]	 = "End",
	[CERR_UNKNOWN]	 = "Unknown error",
};

const char *cerr_str(cerr_t err)
{
	if (err < 0 || err > CERR_UNKNOWN) {
		err = CERR_UNKNOWN;
	}

	return cerrs[err];
}
