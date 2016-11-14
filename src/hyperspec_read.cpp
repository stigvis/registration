//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "readimage.h"
#include "registration.h"
#include "tiffio.h"
#include "matio.h"
#include <iostream>
#include <sstream>
#include <string>
#include "string.h"
#include "hyperspec_read.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

/* TODO:

Read from config file
-> http://www.hyperrealm.com/libconfig/
-> https://en.wikipedia.org/wiki/Configuration_file

Apply gradient of image (DONE)
-> ITK/Examples/Filtering/GradientMagnitudeRecursiveGaussianImageFilter.cxx (With sigma=[1 2 3])



Apply registration
-> ITK/Examples/Registration/ImageRegistration6.cxx
-> ITK/Examples/Registration/ImageRegistration7.cxx
-> ITK/Examples/Registration/ImageRegistration9.cxx
-> ITK/Examples/Registration/ImageRegistration12.cxx (With gradient)

*/

void hyperspec_read_img(const char *filename){
  // Function for handling .img
  // Read hyperspectral header file
	struct hyspex_header header;
	hyperspectral_err_t errcode = hyperspectral_read_header(filename, &header); //see readimage.h for possible error codes.

  //std::cout << header.datatype << std::endl;

	// Read hyperspectral image

	if ( header.datatype != 4 ){ // TODO: Support multiple data types
		std::cout << "Unsupported datatype. Must be float." << std::endl;
	}

	float *img = new float[header.samples*header.lines*header.bands]();
	errcode = hyperspectral_read_image(filename, &header, img);

  typedef float         PixelType;
  const   unsigned int  Dimension = 2;

	// Variable `img` now contains the full hyperspectral image. See also readimage.h for a version of hyperspectral_read_image which reads only a specified subset of the image (using struct image_subset for specifying image subset)

	// Setup types
  typedef itk::Image< PixelType, Dimension > ImageType;
	typedef itk::Image< PixelType, Dimension > GradientImageType;
	typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< GradientImageType, ImageType >  GradientFilterType;

	// Initiate image container
  ImageType::RegionType region;
  ImageType::IndexType start;

  start[0] = 0;
  start[1] = 0;

  ImageType::SizeType size;
  size[0] = header.samples;
  size[1] = header.lines;

  region.SetSize(size);
  region.SetIndex(start);

  // Fixed image
  ImageType::Pointer fixed = ImageType::New();
  fixed->SetRegions(region);
  fixed->Allocate();

  // Moving image
  ImageType::Pointer moving = ImageType::New();

  // Read center image for registration
  int centerband = header.bands/2;
  for (int i=0; i < header.lines; i++){
    for (int j=0; j < header.samples; j++){
      ImageType::IndexType pixelIndex;
      pixelIndex[0] = j;
      pixelIndex[1] = i;
      fixed->SetPixel(pixelIndex, img[i*header.samples*header.bands + centerband*header.samples + j]);
		}
	}

	// Create and setup a gradient filter
  int sigma = 1; // TODO: Take as input
	GradientFilterType::Pointer gradientFilter = GradientFilterType::New();
  gradientFilter->SetSigma ( sigma );
	gradientFilter->SetInput( fixed );

  // Read other images for processing
  for (int i=0; i < header.bands; i++){
    // Skip fixed image
    if ( i == centerband ){
      continue;
    }

    moving->SetRegions(region);
    moving->Allocate();

    for (int j=0; j < header.lines; j++){
      for (int k=0; k < header.samples; k++){
        ImageType::IndexType pixelIndex;
        pixelIndex[0] = k;
        pixelIndex[1] = j;
        moving->SetPixel(pixelIndex, img[j*header.samples*header.bands + i*header.samples + k]);
      }
    }

    // Throw to registration handler
    char buffer[32];                                        // Filename buffer
    snprintf(buffer, sizeof(char) * 32, "output%i.tif", i); // Recursive filenames
    registration1( fixed, moving, buffer );
  }


	// Write to file
	/*
  typedef  itk::ImageFileWriter< ImageType  > WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName("test.tif");
	writer->SetInput( gradientFilter->GetOutput());
	writer->Update();
	*/



/*
  float* linebuffer;
  for (int im=0; im < header.bands; im++) {
    char buffer[32]; // The filename buffer.
    snprintf(buffer, sizeof(char) * 32, "file%i.tif", im); // recursive filenames
    TIFF *out = TIFFOpen(buffer, "w"); // open file for writing
    TIFFSetField(out, TIFFTAG_IMAGEWIDTH, header.lines);
    TIFFSetField(out, TIFFTAG_IMAGELENGTH, header.samples);
    TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);   // set number of channels per pixel
    TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, 1);    // set the size of the channels
    TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
    TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP); // Float point image
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 32);
    TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    // Allocate memory
    float* tmp=(float*)_TIFFmalloc(header.samples*header.lines);

    if (tmp != NULL) {
      linebuffer = tmp;
    } else {
      std::cout << "Error allocating memory." << std::endl ;
    }

    linebuffer[0]=0;
	  for (int i=0; i < header.lines; i++) {
      for (int j=0; j < header.samples; j++) {
        linebuffer[j] = image[i*header.samples*header.bands + im*header.samples + j];
      }
      TIFFWriteScanline(out, linebuffer, i);
    }
    _TIFFfree(linebuffer);
    TIFFClose(out);
  }
*/
  delete [] img;

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

  //Mat_VarPrint(HSIi,1);
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
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT); // Not float point images
    TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 16);
    TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

    // Allocate memory
    uint16_t* tmp=(uint16_t*)_TIFFmalloc(xSize*ySize);

    if (tmp != NULL) {
      linebuffer = tmp;
    } else {
      std::cout << "Error allocating memory." << std::endl ;
    }

    linebuffer[0]=0;
    for (int i=0; i < ySize; i++) {
      for (int j=0; j < xSize; j++) {
        linebuffer[j] = hData[j + xSize*i + xSize*ySize*im];
      }
      TIFFWriteScanline(out, linebuffer, i);
    }
    _TIFFfree(linebuffer);
    TIFFClose(out);
  }

  // Cleanup
	Mat_VarFree(wavelengthsi);
  Mat_VarFree(wavelengthsd);
  Mat_VarFree(HSIi);
	Mat_VarFree(HSId);
  Mat_Close(matfp);
}
