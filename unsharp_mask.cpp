#include <chrono>
#include "unsharp_mask.hpp"

// Apply an unsharp mask to the 24-bit PPM loaded from the file path of
// the first input argument; then write the sharpened output to the file path
// of the second argument. The third argument provides the blur radius.

int main(int argc, char *argv[])
{
  const char *ifilename = argc > 1 ?           argv[1] : "../ghost-town-8k.ppm";
  const char *ofilename = argc > 2 ?           argv[2] : "../out.ppm";
  const int blur_radius = argc > 3 ? std::atoi(argv[3]) : 5;

  ppm img;
  std::vector<unsigned char> data_in, data_sharp;

  img.read(ifilename, data_in);
  data_sharp.resize(img.w * img.h * img.nchannels);

  auto t1 = std::chrono::steady_clock::now();

  unsharp_mask(data_sharp.data(), data_in.data(), blur_radius,
               img.w, img.h, img.nchannels);

  auto t2 = std::chrono::steady_clock::now();
  std::cout << std::chrono::duration<double>(t2-t1).count() << " seconds.\n";

  img.write(ofilename, data_sharp);
  
  return 0;
}

