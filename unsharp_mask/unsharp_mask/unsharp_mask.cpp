#include <chrono>
#include <CL\cl.h>
#include "unsharp_mask.hpp"

// Apply an unsharp mask to the 24-bit PPM loaded from the file path of
// the first input argument; then write the sharpened output to the file path
// of the second argument. The third argument provides the blur radius.

static const char* images[] = {
  "ghost-town-8k.ppm",
  "lena.ppm"
};
static const unsigned CURRENT_IMAGE = 0;
static const char *outputImage = "out.ppm";

inline void checkErr(cl_int err, const char * name)
{
  if (err != CL_SUCCESS)
  {
    std::cerr << "OpenCL Error: " << name << " (" << err << ")\n";
    exit(EXIT_FAILURE);
  }
}

cl_context createContext()
{
  cl_uint platformIdCount = 0;
  clGetPlatformIDs(0, nullptr, &platformIdCount);

  std::vector<cl_platform_id> platformIds(platformIdCount);
  clGetPlatformIDs(platformIdCount, platformIds.data(), nullptr);

  cl_uint deviceIdCount = 0;
  clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_ALL, 0, nullptr,
    &deviceIdCount);
  std::vector<cl_device_id> deviceIds(deviceIdCount);
  clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_ALL, deviceIdCount,
    deviceIds.data(), nullptr);

  const cl_context_properties contextProperties[] =
  {
    CL_CONTEXT_PLATFORM,
    reinterpret_cast<cl_context_properties> (platformIds[0]),
    0, 0
  };

  cl_int error = 0;
  cl_context context = clCreateContext(
    contextProperties, deviceIdCount,
    deviceIds.data(), nullptr,
    nullptr, &error);
  checkErr(error, "createContext()");

  return context;
}

int main(int argc, char *argv[])
{
  const char *ifilename = argc > 1 ?           argv[1] : images[CURRENT_IMAGE];
  const char *ofilename = argc > 2 ?           argv[2] : outputImage;
  const int blur_radius = argc > 3 ? std::atoi(argv[3]) : 5;

  ppm img;
  std::vector<unsigned char> data_in, data_sharp;

  std::cout << "Reading in file: " << ifilename << "...\n";
  img.read(ifilename, data_in);
  std::cout << "Resizing data...\n";
  data_sharp.resize(img.w * img.h * img.nchannels);

  std::cout << "Applying unsharp mask...\n";
  auto t1 = std::chrono::steady_clock::now();

  unsharp_mask(data_sharp.data(), data_in.data(), blur_radius,
               img.w, img.h, img.nchannels);

  auto t2 = std::chrono::steady_clock::now();
  std::cout << std::chrono::duration<double>(t2-t1).count() << " seconds.\n";

  std::cout << "Saving result to " << ofilename << "...\n";
  img.write(ofilename, data_sharp);
  
  return 0;
}

