#include "readimage.h"
#include "tiffio.h"
#include "matio.h"
#include <iostream>
#include <sstream>
#include <string>
#include "string.h"
//#include <stdio>

void hyperspec_read_img(const char *filename);
void hyperspec_read_mat(const char *filename);


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


void hyperspec_read_img(const char *filename){
  // Function for handling .img
  // Read hyperspectral header file
	struct hyspex_header header;
	hyperspectral_err_t errcode = hyperspectral_read_header(filename, &header); //see readimage.h for possible error codes.

	// Read hyperspectral image
	float *image = new float[header.samples*header.lines*header.bands]();
  errcode = hyperspectral_read_image(filename, &header, image);

	// Variable `image` now contains the full hyperspectral image. See also readimage.h for a version of hyperspectral_read_image which reads only a specified subset of the image (using struct image_subset for specifying image subset)


  for (int im=0; im < header.bands; im++) {
    char buffer[32]; // The filename buffer.
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

    if (tmp != NULL) {
      linebuffer = tmp;
    } else {
      std::cout << "Error allocating memory." << std::endl ;
    }

    int center_band = im;
    /* Uncomment to displace image k pixels to the right */
    //for (int k=0; k < 199; k++) {
    //  linebuffer[k] = 0;
    //}
    linebuffer[0]=0;
	  for (int i=0; i < header.lines; i++) {
	    for (int j=0; j < header.samples; j++) {
  	    linebuffer[j] = image[i*header.samples*header.bands + center_band*header.samples + j];
      }

    TIFFWriteScanline(out, linebuffer, i);

    }

    _TIFFfree(linebuffer);
    TIFFClose(out);
  }
	delete [] image;
}

void hyperspec_read_mat(const char *filename){
  // Function for handling .mat
  // Read mat pointer
  mat_t *matfp;

  matfp = Mat_Open(filename,MAT_ACC_RDONLY);
  if ( NULL == matfp ) {
    fprintf(stderr,"Error opening MAT file %s\n",filename);
    exit(1);
  }

  // Read mat information
  matvar_t *HSI = Mat_VarReadInfo(matfp, "HSI");
  matvar_t *wavelengths = Mat_VarRead(matfp, "wavelengths");

  Mat_VarPrint(HSI,1);
  Mat_VarPrint(wavelengths,1);

  // Cleanup
  Mat_VarFree(wavelengths);
  Mat_VarFree(HSI);
  Mat_Close(matfp);

  //char buffer[32];
  //TIFF *out = TIFFOpen("filename.tif", "w");
}
