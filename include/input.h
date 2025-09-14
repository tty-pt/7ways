#ifndef INPUT_H
#define INPUT_H

typedef void input_cb_t(unsigned short type, int value);

void input_init(int flags);
void input_poll(void);
void input_deinit(void);

void input_gen_init(void);
void input_reg(unsigned short key, input_cb_t *cb);
input_cb_t * input_cb(unsigned short key);

#endif
