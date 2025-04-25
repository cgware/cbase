#ifndef CERR_H
#define CERR_H

typedef enum cerr_e {
	CERR_OK,
	CERR_MEM,
	CERR_VAL,
	CERR_EXIST,
	CERR_NOT_FOUND,
	CERR_DESC,
	CERR_NOT_EMPTY,
	CERR_TYPE,
	CERR_END,
	CERR_UNKNOWN,
} cerr_t;

const char *cerr_str(cerr_t err);

#endif
