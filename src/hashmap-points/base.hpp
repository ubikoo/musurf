/*
 * base.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef BASE_H_
#define BASE_H_

#include "atto/opencl/opencl.hpp"

namespace Params {
/* Model parameters */
static const cl_uint empty_state = 0xffffffff;
static const cl_uint n_points = 16384;
static const cl_uint n_cells = 5;
static const cl_uint load_factor = 4;
static const cl_uint capacity = load_factor * n_points;

static const cl_float3 domain_lo = {-1.0f, -1.0f, -1.0f};
static const cl_float3 domain_hi = { 1.0f,  1.0f,  1.0f};

/* OpenGL parameters */
static const int window_width = 1024;
static const int window_height = 1024;
static const char window_title[] = "hashmap-points";
static const double poll_timeout = 0.01;

/* OpenCL parameters */
static const cl_ulong device_index = 2;
static const cl_ulong work_group_size = 256;
} /* Params */

#endif /* BASE_H_ */
