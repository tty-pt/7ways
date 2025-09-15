#include "../include/input.h"

#include <stddef.h>

typedef struct {
	input_cb_t *cb;
} input_t;

static input_t cbs[256];

void
input_call(unsigned short code,
		unsigned short value,
		int type)
{
	input_t *inp = &cbs[code];

	if (!inp->cb)
		return;

	inp->cb(code, value, type);
}

void
input_reg(unsigned short key, input_cb_t *cb)
{
	cbs[key].cb = cb;
}
