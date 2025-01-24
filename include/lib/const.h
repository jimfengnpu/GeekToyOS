#pragma once

#define div_round_up(x, d) (((x) + (d) - 1)/(d))
#define size_mask(__size) ((__size) - 1)
#define bit_size(___x) (1 << (___x))
#define bit_mask(__size) (size_mask(bit_size(__size)))
#define bit_part(_x, _pos, _size) (((_x) >> (_pos)) & bit_mask(_size))
#define bit_build(_x, _pos, _size) (((_x) & bit_mask(_size)) << (_pos))
// _a must 2**n
#define align(_x, _a) (((_x) + size_mask(_a)) & (~size_mask(_a)))
#define align_down(_x, _a) (((_x)) & (~size_mask(_a)))

#define max(_a, _b) (((_a) > (_b))? (_a) : (_b))
#define min(_a, _b) (((_a) < (_b))? (_a) : (_b))
