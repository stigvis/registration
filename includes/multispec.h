//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#ifndef MULTISPEC_READ_H_DEFINED
#define MULTISPEC_READ_H_DEFINED

// Function that reads a .raw file
void                    multispec_raw(
                    int argc,
                    char *argv[] );

#include "registration.h"

// Create raw itk container
UintImageType::Pointer  rawContainer(
                    int xsize,
                    int ysize );

// Create float itk container
ImageType::Pointer      imgContainer(
                    int xsize,
                    int ysize );

// Read raw image to itk container
UintImageType::Pointer  readRaw(
                    UintImageType* const itkimg,
                    int i,
                    int xsize,
                    int ysize,
                    char *argv );

// Write images from itk container to raw format
void                    writeRaw(
                    UintImageType* const itkimg,
                    int i,
                    int xsize,
                    int ysize,
                    std::string name );



#endif // MULTISPEC_READ_H_DEFINED
