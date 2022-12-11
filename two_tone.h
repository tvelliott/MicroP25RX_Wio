#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
  uint8_t tone_idx;
  float tone_freq_hz;
} two_tone_table;

int two_tone_get_idx( int zero_cross );


#ifdef __cplusplus
}
#endif
