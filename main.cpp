//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include <iostream>
#include "string.h"
#include "hyperspec.h"
#include "multispec.h"
#include "readimage.h"
using namespace std;

int main(int argc, char *argv[]){

  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " hyperspectral_image_path" << endl;
    exit(1);
  }

  char *filename = argv[1];

  // File format recognition and run correct function
  if (strstr(filename, "raw") ){
    // File is .raw, see src/multispec.cpp
    multispec_raw( argc, argv );
  } else if (strstr(filename, "img") ){
    // File is .img, see src/hyperspec.cpp
    hyperspec_img(filename);
  } else if (strstr(filename, "mat") ){
    // File is .mat, see src/hyperspec.cpp
    hyperspec_mat(filename);
  } else {
    // Unknown format
    cerr << "Currently supported file formats: img, mat, raw" << endl;
    exit(1);
  }
}
