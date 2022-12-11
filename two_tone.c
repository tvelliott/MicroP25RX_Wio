#include "two_tone.h"

const two_tone_table two_tones[] = {
  1, 283.0f,
  2, 310.75f,
  3, 344.25f,
  4, 376.25f,
  5, 407.5f,
  6, 435.25f,
  7, 467.25f,
  8, 496.25f,
  9, 528.5f,
  10, 566.0f,
  11, 598.0f,
  12, 621.5f,
  13, 660.0f,
  14, 688.25f,
  15, 719.0f,
  16, 752.5f,
  17, 789.75f,
  18, 806.25f,
  19, 849.0f,
  20, 880.5f,
  21, 914.25f,
  22, 932.0f,
  23, 970.0f,
  24, 1011.0f,
  25, 1032.5f,
  26, 1055.0f,
  27, 1103.0f,
  28, 1128.75f,
  29, 1155.75f,
  30, 1184.5f,
  31, 1219.0f,
  32, 1242.75f,
  33, 1293.25f,
  34, 1320.0f,
  35, 1347.75f,
  36, 1376.75f,
  37, 1406.5f,
  38, 1437.75f,
  39, 1470.5f,
  40, 1505.0f,
  41, 1541.25f,
  42, 1579.25f,
  43, 1616.75f,
  44, 1650.0f,
  45, 1684.75f,
  46, 1721.0f,
  47, 1758.25f,
  48, 1797.25f,
  49, 1838.25f,
  50, 1881.25f,
  51, 1926.5f,
  52, 1974.0f,
  53, 2021.75f,
  54, 2065.0f,
  55, 2110.0f,
  56, 2156.75f,
  57, 2205.75f,
  58, 2257.5f,
  59, 2311.75f,
  60, 2369.0f,
  61, 2409.25f,
  62, 2461.5f,
  63, 2516.25f,
  64, 2573.5f,
  65, 2633.75f,
  66, 2697.0f,
  67, 2763.75f,
  68, 2813.25f,
  69, 2875.5f,
  70, 2941.25f,
  71, 3010.0f,
  72, 3082.25f
};



/////////////////////////////////////////////////////////////////
// find closest idx value for the measured frequency
/////////////////////////////////////////////////////////////////
int two_tone_get_idx( int zero_cross ) {

  if( zero_cross < 19 ) return -1;
  if( zero_cross > 250 ) return -1;

  float measured_freq_hz = (float) zero_cross * 12.5f;

  two_tone_table *rec;

  int idx=-1;
  float min_diff = 9999.0f;
  for(int i=0;i<72;i++) {
    rec = (two_tone_table *) &two_tones[i];
    if( fabs( rec->tone_freq_hz - measured_freq_hz ) < min_diff ) {
      min_diff = fabs( rec->tone_freq_hz - measured_freq_hz );
      idx = rec->tone_idx;
    }
  }

  return idx;
}