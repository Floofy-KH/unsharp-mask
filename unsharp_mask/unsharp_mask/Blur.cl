__kernel void blurKernel(__global unsigned char *out, __global unsigned char *in, int blurRadius, uint nChannels)
{
  size_t x = get_global_id(0);
  size_t w = get_global_size(0);
  size_t y = get_global_id(1);
  size_t h = get_global_size(1);
  int offset_x = (((int)get_global_id(2) % (blurRadius*2)) - blurRadius) + 1;
  int offset_y = (floor((float)get_global_id(2) / (blurRadius*2.0f)) - blurRadius) + 1;
  int pixel_x = x + offset_x;
  int pixel_y = y + offset_y;
  __local float red_total, green_total, blue_total;
  red_total = 0;
  green_total = 0;
  blue_total = 0;
  uint nsamples = (blurRadius*2 - 1) * (blurRadius*2 - 1);

 /* for (int j = y - blurRadius + 1; j < y + blurRadius; ++j)
  {
    for (int i = x - blurRadius + 1; i < x + blurRadius; ++i)
    {*/
      uint r_x = pixel_x < 0 ? 0 : pixel_x >= w ? w - 1 : pixel_x;
      uint r_y = pixel_y < 0 ? 0 : pixel_y >= h ? h - 1 : pixel_y;
      uint in_offset = (r_y*w + r_x)*nChannels;
      red_total += in[in_offset + 0];
      green_total += in[in_offset + 1];
      blue_total += in[in_offset + 2];
    /*}
  }*/

  barrier(CLK_LOCAL_MEM_FENCE);
  uint out_offset = (y*w + x)*nChannels;
  out[out_offset + 0] = red_total / nsamples;
  out[out_offset + 1] = green_total / nsamples;
  out[out_offset + 2] = blue_total / nsamples;
}