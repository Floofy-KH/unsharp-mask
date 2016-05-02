const char* addWeightedFloatSource = "__kernel void add_weighted_float(__global unsigned char *out, \
  __global unsigned char *in1, float alpha, \
  __global unsigned char *in2, float  beta, float gamma, \
  unsigned nchannels) \
{ \
  size_t x = get_global_id(0); \
  size_t y = get_global_id(1); \
  size_t w = get_global_size(0); \
  size_t h = get_global_size(1); \
 \
  unsigned byte_offset = (y*w + x)*nchannels; \
 \
  float tmp = in1[byte_offset + 0] * alpha + in2[byte_offset + 0] * beta + gamma; \
  out[byte_offset + 0] = tmp < 0 ? 0 : tmp > UCHAR_MAX ? UCHAR_MAX : tmp; \
 \
  tmp = in1[byte_offset + 1] * alpha + in2[byte_offset + 1] * beta + gamma; \
  out[byte_offset + 1] = tmp < 0 ? 0 : tmp > UCHAR_MAX ? UCHAR_MAX : tmp; \
 \
  tmp = in1[byte_offset + 2] * alpha + in2[byte_offset + 2] * beta + gamma; \
  out[byte_offset + 2] = tmp < 0 ? 0 : tmp > UCHAR_MAX ? UCHAR_MAX : tmp; \
 \
}\0";