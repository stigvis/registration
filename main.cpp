//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include <iostream>
#include "string.h"
#include "hyperspec_read.h"


int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " hyperspectral_image_path" << std::endl;
		exit(1);
	}

	char *filename = argv[1];

  // File format recognition and run correct function
  if (strstr(filename, "raw") ){
    // File is .raw
    std::cout << "Raw sucks" << std::endl;
    exit(1);
  } else if (strstr(filename, "img") ){
    // File is .img
    hyperspec_read_img(filename);
  } else if (strstr(filename, "mat") ){
    // File is .mat
    hyperspec_read_mat(filename);
  } else {
    // Unknown format
    std::cerr << "Currently supported file formats: img, mat, raw" << std::endl;
    exit(1);
  }
}
