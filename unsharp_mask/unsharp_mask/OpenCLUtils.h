#pragma once

#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <CL\cl.h>
#include <iostream>

inline void checkErr(cl_int err, const char * name)
{
  if (err != CL_SUCCESS)
  {
    std::cerr << "OpenCL Error: " << name << " (" << err << ")\n";
    exit(EXIT_FAILURE);
  }
}