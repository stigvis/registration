#include "readimage.h"
#include "tiffio.h"
#include <iostream>
#include <sstream>
#include <string>

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


  for (int im=0; im < header.bands; im++) {
    //char* name;
    //sprintf(name, "%d.tif", im);
    //const char* name = (char)im;
    //name << (char)im << ".tif";
    char buffer[32]; // The filename buffer.
    // Put "file" then im then ".tif" in to filename.
    snprintf(buffer, sizeof(char) * 32, "file%i.tif", im);
    TIFF *out = TIFFOpen(buffer, "w");
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, header.lines);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, header.samples);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);   // set number of channels per pixel
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, 1);    // set the size of the channels
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 32);
    TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    float* linebuffer;
    float* tmp=(float*)_TIFFmalloc(header.samples*header.lines);
    //float* linebuffer= (float*)malloc(sizeof (float*)*header.lines);

    if (tmp != NULL) {
      linebuffer = tmp;
    } else {
      std::cout << "Error allocating memory." << std::endl ;
    }

	  //example for printing the center band of the image to standard output:
    int center_band = im;
	  for (int i=0; i < header.lines; i++) {
	    for (int j=0; j < header.samples; j++) {
        //		std::cout << image[i*header.samples*header.bands + center_band*header.samples + j] << " ";
        //std::cout << "Done with " << j << std::endl ;
        //std::cout << "Done with buffer " << std::endl ;
    	  //float buffer = image[i*header.samples*header.bands + center_band*header.samples + j];
    	  linebuffer[j] = image[i*header.samples*header.bands + center_band*header.samples + j];
        //buf[j] = buffer;
        //std::cout << "Done with row " << j << "." << std::endl ;
      }

      //TIFFWriteEncodedStrip (out, i, linebuffer, header.samples);
      TIFFWriteScanline(out, linebuffer, i);
      //{
      //  std::cout << "Unable to write a row." << std::endl ;
      //  break ;
      //}


    }

    //_TIFFfree(out);
    _TIFFfree(linebuffer);
    TIFFClose(out);
  }
	delete [] image;
}
