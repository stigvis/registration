//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"

#ifndef HYPERSPEC_H_DEFINED
#define HYPERSPEC_H_DEFINED



// Function that reads an .img file and splits into .tif files
void hyperspec_img( const char *filename );
// Function that reads a .mat file and splits into .tif files
void hyperspec_mat( const char *filename );

ImageType::Pointer imageContainer( struct hyspex_header header );

#endif // HYPERSPEC_READ_H_DEFINED
