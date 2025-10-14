#include "./../include/map.h"

static inline uint32_t
idx_scramble(uint32_t i)
{
	return (i * 0x003779B1u) & 0xFFFFFFu;
}

static inline uint32_t
idx_unscramble(uint32_t s)
{
	return (s * 0x008B2F51u) & 0xFFFFFFu;
}

static inline uint8_t
unpack_chan_msb(uint32_t idx, unsigned c)
{
	uint32_t v =
		((idx >> (0 * 3 + c)) & 1u) << 7 |
		((idx >> (1 * 3 + c)) & 1u) << 6 |
		((idx >> (2 * 3 + c)) & 1u) << 5 |
		((idx >> (3 * 3 + c)) & 1u) << 4 |
		((idx >> (4 * 3 + c)) & 1u) << 3 |
		((idx >> (5 * 3 + c)) & 1u) << 2 |
		((idx >> (6 * 3 + c)) & 1u) << 1 |
		((idx >> (7 * 3 + c)) & 1u) << 0;

	return (uint8_t)v;
}

static inline uint32_t
pack_from_chan_msb(uint8_t v, unsigned c)
{
	return
		((uint32_t)((v >> 7) & 1u)) << (0 * 3 + c) |
		((uint32_t)((v >> 6) & 1u)) << (1 * 3 + c) |
		((uint32_t)((v >> 5) & 1u)) << (2 * 3 + c) |
		((uint32_t)((v >> 4) & 1u)) << (3 * 3 + c) |
		((uint32_t)((v >> 3) & 1u)) << (4 * 3 + c) |
		((uint32_t)((v >> 2) & 1u)) << (5 * 3 + c) |
		((uint32_t)((v >> 1) & 1u)) << (6 * 3 + c) |
		((uint32_t)((v >> 0) & 1u)) << (7 * 3 + c);
}

uint32_t
map_color(unsigned idx)
{
	idx &= 0xFFFFFFu;
	idx = idx_scramble(idx);

	uint8_t r = unpack_chan_msb(idx, 0);
	uint8_t g = unpack_chan_msb(idx, 1);
	uint8_t b = unpack_chan_msb(idx, 2);

	return 0xFF000000u
		| ((uint32_t) r << 16)
		| ((uint32_t) g << 8)
		| b;
}

unsigned
map_idx(uint32_t col)
{
	uint8_t r = (col >> 16) & 0xFF;
	uint8_t g = (col >>  8) & 0xFF;
	uint8_t b =  col        & 0xFF;

	uint32_t idx = 0;
	idx |= pack_from_chan_msb(r, 0);
	idx |= pack_from_chan_msb(g, 1);
	idx |= pack_from_chan_msb(b, 2);

	idx = idx_unscramble(idx);

	return idx & 0xFFFFFFu;
}
