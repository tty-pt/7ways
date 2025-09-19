#ifndef INPUT_H
#define INPUT_H

#include <linux/input-event-codes.h>

typedef int input_cb_t(unsigned short code,
		unsigned short type,
		int value);

void input_init(int flags);
void input_poll(void);
void input_deinit(void);

void input_reg(unsigned short key,
		input_cb_t *cb);

void input_call(unsigned short code,
		unsigned short value,
		int type);

extern int input_default(unsigned short code,
		unsigned short value,
		int type);

unsigned short key_value(unsigned short code);

#endif
