#include "readimage.h"
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " hyperspectral_image_path" << std::endl;
		exit(1);
	}

	char *filename = argv[1];

	//read hyperspectral header file
	struct hyspex_header header;
	hyperspectral_err_t errcode = hyperspectral_read_header(filename, &header); //see readimage.h for possible error codes.

	//read hyperspectral image
	float *image = new float[header.samples*header.lines*header.bands]();
	errcode = hyperspectral_read_image(filename, &header, image);

	//variable `image` now contains the full hyperspectral image. See also readimage.h for a version of hyperspectral_read_image which reads only a specified subset of the image (using struct image_subset for specifying image subset)

	//example for printing the center band of the image to standard output:
	int center_band = header.bands/2;
	for (int i=0; i < header.lines; i++) {
		for (int j=0; j < header.samples; j++) {
			std::cout << image[i*header.samples*header.bands + center_band*header.samples + j] << " ";
		}
		std::cout << std::endl;
	}

	delete [] image;
}
