//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#ifndef MULTISPEC_READ_H_DEFINED
#define MULTISPEC_READ_H_DEFINED

// Read .raw files
void                    multispec_raw(
                    // Number of inputs
                    int argc,
                    // Inputs
                    char *argv[] );

#include "registration.h"
// Create raw itk container
UintImageType::Pointer  rawContainer(
                    // Image width
                    int xsize,
                    // Image height
                    int ysize );

// Create float itk container
ImageType::Pointer      imgContainer(
                    // Image width
                    int xsize,
                    // Image height
                    int ysize );

// Read raw image to itk container
UintImageType::Pointer  readRaw(
                    // Pointer to write to
                    UintImageType* const itkimg,
                    // Image number
                    int i,
                    // Image width
                    int xsize,
                    // Image height
                    int ysize,
                    // File to read from
                    char *argv );

// Write images from itk container to raw format
void                    writeRaw(
                    // Pointer to read from
                    UintImageType* const itkimg,
                    // Image number
                    int i,
                    // Image width
                    int xsize,
                    // Image height
                    int ysize,
                    // Output name
                    std::string name );

#endif // MULTISPEC_READ_H_DEFINED
