#include <stdint.h>

typedef struct {
  uint8_t tone_idx;
  float tone_freq_hz;
} two_tone_table;


int two_tone_get_idx(float measured_freq_hz);
void process_tone_pair_qcii( int TONE_A, int TONE_B );
float two_tone_get_freq(int idx);
