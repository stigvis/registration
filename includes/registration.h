//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#ifndef REGISTRATION_H_DEFINED
#define REGISTRATION_H_DEFINED

#include "itkImage.h"


// Instantiation of input images
const   unsigned int  Dimension = 2;
typedef float         PixelType;

typedef itk::Image< PixelType, Dimension >  FixedImageType;
typedef itk::Image< PixelType, Dimension >  MovingImageType;

// Image registration, type 1
void registration1( FixedImageType* const fixed, MovingImageType* const moving, char argv[] );

#endif // REGISTRATION_H_DEFINED
