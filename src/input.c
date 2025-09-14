#include "../include/input.h"

#include <qmap.h>

typedef struct {
	input_cb_t *cb;
} input_t;

unsigned input_hd;

input_cb_t *
input_cb(unsigned short key)
{
	const input_t *inp = qmap_get(input_hd, &key);

	if (!inp)
		return NULL;

	return inp->cb;
}

void
input_reg(unsigned short key, input_cb_t *cb)
{
	input_t inp = { .cb = cb };
	qmap_put(input_hd, &key, &inp);
}

void input_gen_init(void) {
	unsigned qm_us
		= qmap_reg(sizeof(unsigned short));
	unsigned qm_icb
		= qmap_reg(sizeof(input_t));
	input_hd = qmap_open(qm_us, qm_icb, 0xFF, 0);
}
