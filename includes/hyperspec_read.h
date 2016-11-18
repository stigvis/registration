//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"

#ifndef HYPERSPEC_READ_H_DEFINED
#define HYPERSPEC_READ_H_DEFINED



// Function that reads an .img file and splits into .tif files
void hyperspec_read_img( const char *filename );
// Function that reads a .mat file and splits into .tif files
void hyperspec_read_mat( const char *filename );

ImageType::Pointer imageContainer( struct hyspex_header header );

GradientFilterType::Pointer gradientFilter( ImageType* const fixed, int sigma );
CastFilterType::Pointer castImage( ImageType* const img );

#endif // HYPERSPEC_READ_H_DEFINED
