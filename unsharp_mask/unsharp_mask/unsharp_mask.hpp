#ifndef _UNSHARP_MASK_HPP_
#define _UNSHARP_MASK_HPP_


#pragma warning( push )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4244 )

#include "blur.hpp"
#include "add_weighted.hpp"
#include "ppm.hpp"
#include "OpenCLUtils.h"

void unsharp_mask(unsigned char *out, const unsigned char *in,
                  const int blur_radius,
                  const unsigned w, const unsigned h, const unsigned nchannels)
{
  std::vector<unsigned char> blur1, blur2, blur3;

  blur1.resize(w * h * nchannels);
  blur2.resize(w * h * nchannels);
  blur3.resize(w * h * nchannels);

  blur(blur1.data(),   in,           blur_radius, w, h, nchannels);
  blur(blur2.data(),   blur1.data(), blur_radius, w, h, nchannels);
  blur(blur3.data(),   blur2.data(), blur_radius, w, h, nchannels);
  add_weighted(out, in, 1.5f, blur3.data(), -0.5f, 0.0f, w, h, nchannels);
}

void unsharp_mask(unsigned char *out, const unsigned char *in,
  const int blur_radius,
  const unsigned w, const unsigned h, const unsigned nchannels, 
  cl_context context, cl_command_queue commandQueue, cl_program program)
{
  cl_int error = 0;
  size_t size = w*h*nchannels;
  unsigned char *buffer3 = new unsigned char[size];
  const cl_uint workDim = 2;
  size_t globalWorkSizes[workDim] = { w, h,};
  cl_kernel blurKernel = clCreateKernel(program, "blurKernel", &error);
  checkErr(error, "Create blur kernel");
  cl_mem buffer1 = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &error);
  checkErr(error, "Initialise buffer1");
  cl_mem buffer2 = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &error);
  checkErr(error, "Initialise buffer2");

  clEnqueueWriteBuffer(commandQueue, buffer1, true, 0, size, in, 0, NULL, NULL);

  checkErr(clSetKernelArg(blurKernel, 0, sizeof(buffer2), &buffer2), "Set out kernel arg");
  checkErr(clSetKernelArg(blurKernel, 1, sizeof(buffer1), &buffer1), "Set in kernel arg");
  checkErr(clSetKernelArg(blurKernel, 2, sizeof(int), &blur_radius), "Set blur radius kernel arg");
  checkErr(clSetKernelArg(blurKernel, 3, sizeof(unsigned), &nchannels), "Set num channels kernel arg");
  error = clEnqueueNDRangeKernel(commandQueue, blurKernel, workDim, NULL, globalWorkSizes, NULL, NULL, NULL, NULL);
  checkErr(error, "Blur 1");
  checkErr(clSetKernelArg(blurKernel, 0, sizeof(buffer1), &buffer1), "Set out kernel arg");
  checkErr(clSetKernelArg(blurKernel, 1, sizeof(buffer2), &buffer2), "Set in kernel arg");
  error = clEnqueueNDRangeKernel(commandQueue, blurKernel, workDim, NULL, globalWorkSizes, NULL, NULL, NULL, NULL);
  checkErr(error, "Blur 2");
  checkErr(clSetKernelArg(blurKernel, 0, sizeof(buffer2), &buffer2), "Set out kernel arg");
  checkErr(clSetKernelArg(blurKernel, 1, sizeof(buffer1), &buffer1), "Set in kernel arg");
  error = clEnqueueNDRangeKernel(commandQueue, blurKernel, workDim, NULL, globalWorkSizes, NULL, NULL, NULL, NULL);
  checkErr(error, "Blur 3");
  
  clEnqueueReadBuffer(commandQueue, buffer2, true, 0, size, buffer3, 0, NULL, NULL);
  add_weighted(out, in, 1.5f, buffer3, -0.5f, 0.0f, w, h, nchannels);

  error = clReleaseMemObject(buffer1);
  checkErr(error, "Releasing buffer1");
  error = clReleaseMemObject(buffer1);
  checkErr(error, "Releasing buffer2");
  clReleaseKernel(blurKernel);
}

#pragma warning( pop )

#endif // _UNSHARP_MASK_HPP_
