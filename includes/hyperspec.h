//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include <iostream>
#include <sstream>
#include <string>
#include "string.h"

#ifndef HYPERSPEC_H_DEFINED
#define HYPERSPEC_H_DEFINED

// =====================================
// Container for registration parameters
// =====================================

struct reg_params {
  // Registration method
  int regmethod;
  // Registration output name
  std::string reg_name;
  // Output from diff
  int diff_conf;
  // Diff output name
  std::string diff_name;
  // Median filter
  int median;
  // Level of median filtering
  int radius;
  // Gradient filter
  int gradient;
  // Level of gradient filtering
  int sigma;
  // Initial transform angle
  float angle;
  // Initial transform scale
  float scale;
  // Learning rate of registration
  float lrate;
  // Minimum step length before completion
  float slength;
  // Maximum number of iterations
  int niter;
  // Resizing
  unsigned int numberOfLevels;
  // Translation scale
  double translationScale;
  // Intial Translation transform
  int translation;
  // Choose between mutual information and mean squares
  int metric;
  // Option for suppressing iteration outputs
  int output;
};

// ======
// Errors
// ======

enum conf_err_t {
  // Successful
  CONF_NO_ERR,
  // File not found
  CONF_FILE_NOT_FOUND,
  // Read error
  CONF_FILE_READING_ERROR
};


// =========
// Functions
// =========

// Read config (params.conf)
conf_err_t params_read( struct reg_params *params);

// Retrieve variable from config
std::string         getParam(
                            // Variable name
                            std::string confText,
                            // Variable value
                            std::string property );

// Read a hyperspectral .img file and
// output a registrated .img file
void                hyperspec_img(
                            const char *filename );

// Read a hyperspectral .mat file and
// output a registrated .mat file
void                hyperspec_mat(
                            const char *filename );

#include "registration.h"
// Create an image pointer for .img
ImageType::Pointer  imageContainer(
                            // Get size of image from .hdr
                            struct hyspex_header header );

// Read an image into an image pointer from .img
ImageType::Pointer  readITK(
                            // Pointer to write to
                            ImageType* const itkimg,
                            // Float to read from
                            float *img,
                            // Image band
                            int i,
                            // Image storage format
                            struct hyspex_header header );

// Write an image from an image pointer to a float*
float*              writeITK(
                            // Pointer to read from
                            ImageType* const itkimg,
                            // Float to write to
                            float *image,
                            // Image band
                            int i,
                            // Image storage format
                            struct hyspex_header header );

// Create an image pointer for .mat
ImageType::Pointer  imageMatContainer(
                            // Image width
                            unsigned xSize,
                            // Image height
                            unsigned ysize );

// Read an image into an image pointer from .mat
ImageType::Pointer  readMat(
                            // Pointer to write to
                            ImageType* const itkmat,
                            // Image band
                            int i,
                            // Image width
                            unsigned xSize,
                            // Image height
                            unsigned ySize,
                            // Float to read from
                            float *hData );

// Write an image from an image pointer to a float*
float*              writeMat(
                            // Pointer to read from
                            ImageType* const itkmat,
                            // Float to write to
                            float *hData,
                            // Image band
                            int i,
                            // Image width
                            unsigned xSize,
                            // Image height
                            unsigned ySize );

#endif // HYPERSPEC_READ_H_DEFINED
