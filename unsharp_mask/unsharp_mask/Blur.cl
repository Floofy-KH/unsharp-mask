const char* blurSource = "__kernel void blurKernel(__global unsigned char *out, __global unsigned char *in, int blurRadius, uint nChannels) \
{ \
  size_t x = get_global_id(0); \
  size_t w = get_global_size(0); \
  size_t y = get_global_id(1); \
  size_t h = get_global_size(1); \
  float red_total = 0, green_total = 0, blue_total = 0; \
  uint nsamples = (blurRadius * 2 - 1) * (blurRadius * 2 - 1); \
 \
  for (int j = y - blurRadius + 1; j < y + blurRadius; ++j) \
  { \
    for (int i = x - blurRadius + 1; i < x + blurRadius; ++i) \
    { \
      uint r_i = i < 0 ? 0 : i >= w ? w - 1 : i; \
      uint r_j = j < 0 ? 0 : j >= h ? h - 1 : j; \
      uint byte_offset = (r_j*w + r_i)*nChannels; \
      red_total += in[byte_offset + 0]; \
      green_total += in[byte_offset + 1]; \
      blue_total += in[byte_offset + 2]; \
    } \
  } \
 \
  uint byte_offset = (y*w + x)*nChannels; \
  out[byte_offset + 0] = red_total / nsamples; \
  out[byte_offset + 1] = green_total / nsamples; \
  out[byte_offset + 2] = blue_total / nsamples; \
}\0";