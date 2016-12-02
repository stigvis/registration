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

// ========================================
// Container for registration parameters
// ========================================

struct reg_params {
  // Registration method
  int regmethod;
  // Registration output name
  std::string reg_name;
  //const char* reg_name;
  // Output from diff
  int diff_conf;
  // Diff output name
  std::string diff_name;
  //const char* diff_name;
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
};

// ======================================
// Errors
// ======================================

enum conf_err_t {
  // Successful
  CONF_NO_ERR,
  // File not found
  CONF_FILE_NOT_FOUND,
  // Read error
  CONF_FILE_READING_ERROR
};

// Initialize parameters
conf_err_t params_read( struct reg_params *params);

// Read config
std::string getParam(std::string confText, std::string property);

// Function that reads an .img file and splits into .tif files
void hyperspec_img( const char *filename );
// Function that reads a .mat file and splits into .tif files
void hyperspec_mat( const char *filename );

#include "registration.h"

ImageType::Pointer imageContainer( struct hyspex_header header );
ImageType::Pointer readITK( ImageType* const itkimg,
                            float *img,
                            int i,
                            struct hyspex_header header );

float* writeITK(            ImageType* const itkimg,
                            float *image,
                            int i,
                            struct hyspex_header header );
ImageType::Pointer readMat( ImageType* const itkmat,
                            int i,
                            unsigned xSize,
                            unsigned ySize,
                            float *hData );
#endif // HYPERSPEC_READ_H_DEFINED
