#include "unsharp_mask.hpp"
#include "blur.cl"
#include "add_weighted_float.cl"
#include <chrono>
#include <fstream>

// Apply an unsharp mask to the 24-bit PPM loaded from the file path of
// the first input argument; then write the sharpened output to the file path
// of the second argument. The third argument provides the blur radius.

static const char* images[] = {
  "ghost-town-8k.ppm",
  "lena.ppm"
};
static const unsigned CURRENT_IMAGE = 1;
static const char *outputImage = "out.ppm";

cl_device_id deviceId; 
cl_uint deviceMaxComputeUnits = 0;
cl_ulong deviceMaxAllocSize = 0;
cl_ulong deviceMaxGlobalMemSize = 0;


cl_context createContext()
{
	cl_int error = 0;
  cl_uint platformIdCount = 0;
  clGetPlatformIDs(0, nullptr, &platformIdCount);

  std::vector<cl_platform_id> platformIds(platformIdCount);
  clGetPlatformIDs(platformIdCount, platformIds.data(), nullptr);
  std::cout << "The following platforms are available:\n";
  for (int i = 0; i < platformIdCount; ++i)
  {

    size_t size;
    error = clGetPlatformInfo(platformIds[i], CL_PLATFORM_VERSION, NULL, NULL, &size);
    checkErr(error, "Get platform Version size");
    char *version = new char[size];
    error = clGetPlatformInfo(platformIds[i], CL_PLATFORM_VERSION, size, version, NULL);
    checkErr(error, "Get platform version");

    error = clGetPlatformInfo(platformIds[i], CL_PLATFORM_NAME, NULL, NULL, &size);
    checkErr(error, "Get platform name size");
    char *name = new char[size];
    error = clGetPlatformInfo(platformIds[i], CL_PLATFORM_NAME, size, name, NULL);
    checkErr(error, "Get platform name");

    error = clGetPlatformInfo(platformIds[i], CL_PLATFORM_VENDOR, NULL, NULL, &size);
    checkErr(error, "Get platform vendor size");
    char *vendor = new char[size];
    error = clGetPlatformInfo(platformIds[i], CL_PLATFORM_VENDOR, size, vendor, NULL);
    checkErr(error, "Get platform vendor");

    std::cout << "Version: " << version << "\nName: " << name << "\nVendor: " << vendor << "\n\n";
  }
  std::cout << "Using the first platform.\n\n";

  cl_uint deviceIdCount = 0;
  clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_ALL, 0, nullptr,
    &deviceIdCount);
  std::vector<cl_device_id> deviceIds(deviceIdCount);
  clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_ALL, deviceIdCount,
    deviceIds.data(), nullptr); 
  std::cout << "The following devices are available:\n";
  for (int i = 0; i < deviceIdCount; ++i)
  {
    size_t size;
    error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_VERSION, NULL, NULL, &size);
    checkErr(error, "Get device Version size");
    char *version = new char[size];
    error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_VERSION, size, version, NULL);
    checkErr(error, "Get device version");

    error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_NAME, NULL, NULL, &size);
    checkErr(error, "Get device name size");
    char *name = new char[size];
    error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_NAME, size, name, NULL);
    checkErr(error, "Get device name");

    error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_VENDOR, NULL, NULL, &size);
    checkErr(error, "Get device vendor size");
    char *vendor = new char[size];
    error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_VENDOR, size, vendor, NULL);
    checkErr(error, "Get device vendor");

    cl_uint maxComputeUnits;
    error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, NULL);
    checkErr(error, "Get device max compute units");

    cl_ulong maxAllocSize;
    error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxAllocSize), &maxAllocSize, NULL);
    checkErr(error, "Get device max alloc size");

    cl_ulong maxGlobalMemSize;
    error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(maxGlobalMemSize), &maxGlobalMemSize, NULL);
    checkErr(error, "Get device max global memory size");

    std::cout << "ID: " << deviceIds[i] << "\nVersion: " << version << "\nName: " << name << "\nVendor: " << vendor << 
      "\nMax compute units: " << maxComputeUnits << "\nMax alloc size: " << maxAllocSize << 
      "\nGlobal memory size: " << maxGlobalMemSize << "\n\n";

    if (maxComputeUnits > deviceMaxComputeUnits)
    {
      deviceId = deviceIds[i];
      deviceMaxAllocSize = maxAllocSize;
      deviceMaxComputeUnits = maxComputeUnits;
      deviceMaxGlobalMemSize = maxGlobalMemSize;
    }
  }
  std::cout << "Using device: " << deviceId << std::endl <<std::endl;

  const cl_context_properties contextProperties[] =
  {
    CL_CONTEXT_PLATFORM,
    reinterpret_cast<cl_context_properties> (platformIds[0]),
    0, 0
  };

  cl_context context = clCreateContext(
    contextProperties, deviceIdCount,
    deviceIds.data(), nullptr,
    nullptr, &error);
  checkErr(error, "createContext()");

  return context;
}

cl_program createProgram(cl_context context)
{
  const int numFiles = 2;
  const char *source[numFiles];
  size_t lengths[numFiles];
  /*const char *files[numFiles] = 
  {
    "blur.cl",
    "add_weighted_float.cl"
  };


  for (int i = 0; i < numFiles; ++i)
  {
    std::ifstream file(files[i], std::ios::in | std::ios::ate);
    if (file.is_open())
    {
      std::streampos length = file.tellg();
      file.seekg(0, file.beg);

      source[i] = new char[length];
      file.read(source[i], length);
      lengths[i] = length;
      file.close();
    }
    else
    {
      std::cerr << "Failed to open file: " << files[i] << "... Aborting." << std::endl;
      exit(EXIT_FAILURE);
    }
  }*/
  
  std::string blurSourceStr(blurSource);
  std::string addWeightedFloatSourceStr(addWeightedFloatSource);
  source[0] = blurSource;
  source[1] = addWeightedFloatSource;
  lengths[0] = blurSourceStr.length();
  lengths[1] = addWeightedFloatSourceStr.length();
  cl_int error = 0;
  cl_program program = clCreateProgramWithSource(context, numFiles, (const char**)source, lengths, &error);
  checkErr(error, "Create program");

  error = clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);
  if (error != CL_SUCCESS)
  {
    // Determine the size of the log
    size_t log_size;
    clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

    // Allocate memory for the log
    char *log = new char[log_size];

    // Get the log
    clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

    // Print the log
    std::cout << log << std::endl;

    exit(EXIT_FAILURE);
  }

  return program;
}

int main(int argc, char *argv[])
{
  const char *ifilename = argc > 1 ?           argv[1] : images[CURRENT_IMAGE];
  const char *ofilename = argc > 2 ?           argv[2] : outputImage;
  const int blur_radius = argc > 3 ?		   std::atoi(argv[3]) : 5;
  const bool on_gpu = argc > 4 ? std::strcmp(argv[4],"cpu") != 0 : true;

  cl_context clContext = nullptr;
  cl_program program = nullptr;
  cl_command_queue clQueue = nullptr;

  ppm img;
  std::vector<unsigned char> data_in, data_sharp;

  std::cout << "Reading in file: " << ifilename << "...\n";
  img.read(ifilename, data_in);
  if (on_gpu)
  {
    cl_int error = 0;
    clContext = createContext();
    clQueue = clCreateCommandQueue(clContext, deviceId, NULL, &error);
    checkErr(error, "Initialise command queue");
    program = createProgram(clContext);
  }
  std::cout << "Resizing data...\n";
  data_sharp.resize(img.w * img.h * img.nchannels);

  std::cout << "Applying unsharp mask...\n";
  auto t1 = std::chrono::steady_clock::now();

  if (on_gpu)
  {
    unsharp_mask(data_sharp.data(), data_in.data(), blur_radius,
      img.w, img.h, img.nchannels, clContext, clQueue, program);
  }
  else
  {
    unsharp_mask(data_sharp.data(), data_in.data(), blur_radius,
      img.w, img.h, img.nchannels);
  }

  auto t2 = std::chrono::steady_clock::now();
  std::cout << std::chrono::duration<double>(t2-t1).count() << " seconds.\n";

  std::cout << "Saving result to " << ofilename << "...\n";
  img.write(ofilename, data_sharp);

  /*if (on_gpu)
  {
    if(clQueue != nullptr)
      clReleaseCommandQueue(clQueue);
    if (program != nullptr)
      clReleaseProgram(program);
    if (clContext != nullptr)
      clReleaseContext(clContext);
  }*/
  
  return 0;
}

