#include "../include/input.h"

#include <stddef.h>

#include <qsys.h>

typedef struct {
	input_cb_t *cb;
} input_t;

static input_t cbs[256];
unsigned keys[256];

void
input_call(unsigned short code,
		unsigned short value,
		int type)
{
	input_t *inp = &cbs[code];
	keys[code] = value;
	/* WARN("key: %hu %hu %d\n", code, value, type); */


	if (input_default(code, value, type) || !inp->cb)
		return;

	inp->cb(code, value, type);
}

void
input_reg(unsigned short key, input_cb_t *cb)
{
	cbs[key].cb = cb;
}

unsigned short
key_value(unsigned short code)
{
	return keys[code];
}
