#pragma once

#define div_round_up(x, d) (((x) + (d) - 1)/(d))
#define bit_mask(__size) ((1 << (__size)) - 1)
#define bit_part(_x, _pos, _size) (((_x) >> (_pos)) & bit_mask(_size))
#define bit_build(_x, _pos, _size) (((_x) & bit_mask(_size)) << (_pos))
