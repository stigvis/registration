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


void hyperspec_read_img(const char *filename){
  // Function for handling .img
  // Read hyperspectral header file
	struct hyspex_header header;
	hyperspectral_err_t errcode = hyperspectral_read_header(filename, &header); //see readimage.h for possible error codes.

	// Read hyperspectral image
	float *image = new float[header.samples*header.lines*header.bands]();
  errcode = hyperspectral_read_image(filename, &header, image);

	// Variable `image` now contains the full hyperspectral image. See also readimage.h for a version of hyperspectral_read_image which reads only a specified subset of the image (using struct image_subset for specifying image subset)

  float* linebuffer;
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
  matvar_t *HSIi = Mat_VarReadInfo(matfp, "HSI");
	matvar_t *HSId = Mat_VarRead(matfp, "HSI");
	matvar_t *wavelengthsi = Mat_VarReadInfo(matfp, "wavelengths");
  matvar_t *wavelengthsd = Mat_VarRead(matfp, "wavelengths");

  Mat_VarPrint(HSIi,1);
  //Mat_VarPrint(wavelengthsd,1);

  // Get information from file
  // Image size
  unsigned xSize = HSId->dims[0];
  unsigned ySize = HSId->dims[1];
  // Number of images
  unsigned nSize = HSId->dims[2];
  //Wavelengths
  unsigned nWave = wavelengthsd->dims[1];
  static double *wData = static_cast<double*>(wavelengthsd->data);
	short unsigned *hData = static_cast<uint16_t*>(HSId->data);
  std::cout << "Number of images: " << nSize << ", Image dimensions: " << xSize << "x" << ySize << ", Wavelengths: " ;
  for (int i=0; i<nWave-1; i++){
    std::cout << wData[i] << ", ";
  }
	std::cout << wData[nWave-1] << " Array size: " << HSId->nbytes/HSId->data_size << " Compression: " << HSId->compression << " Data type: " << HSId->data_type << std::endl;
  /*
  for (int i=0; i<xSize*ySize; i++){
    std::cout << hData[i];
  }
  std::cout << " " << std::endl;
	*/
  // Write to tiff

  uint16_t* linebuffer;

  for (int im=0; im<HSId->dims[2]; im++) {
    char buffer[32]; // The filename buffer.
    snprintf(buffer, sizeof(char) * 32, "file%i.tif", im);
    TIFF *out = TIFFOpen(buffer, "w");
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, xSize);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, ySize);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);   // set number of channels per pixel
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, 1);    // set the size of the channels
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);
    TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    short unsigned* tmp=(uint16_t*)_TIFFmalloc(xSize*ySize);

    if (tmp != NULL) {
      linebuffer = tmp;
    } else {
      std::cout << "Error allocating memory." << std::endl ;
    }

    //int center_band = im;
    // Uncomment to displace image k pixels to the right
    //for (int k=0; k < 199; k++) {
    //  linebuffer[k] = 0;
    //}
    linebuffer[0]=0;
    for (int i=0; i < ySize; i++) {
      for (int j=0; j < xSize; j++) {
        linebuffer[j] = hData[j + xSize*i + xSize*ySize*im];
      }

    TIFFWriteScanline(out, linebuffer, i);
    }
    std::cout << "Written one image" << std::endl;

    _TIFFfree(linebuffer);
    TIFFClose(out);
  }

  // Cleanup
	Mat_VarFree(wavelengthsi);
  Mat_VarFree(wavelengthsd);
  Mat_VarFree(HSIi);
	Mat_VarFree(HSId);
  Mat_Close(matfp);

  //char buffer[32];
  //TIFF *out = TIFFOpen("filename.tif", "w");
}
