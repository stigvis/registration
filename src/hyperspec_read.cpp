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

/* TODO:

Read from config file
//> http://www.hyperrealm.com/libconfig/
//> https://en.wikipedia.org/wiki/Configuration_file

Apply gradient of image (DONE)
//> ITK/Examples/Filtering/GradientMagnitudeRecursiveGaussianImageFilter.cxx (With sigma=(1 2 3))

Apply registration
//> ITK/Examples/Registration/ImageRegistration6.cxx (DONE)
//> ITK/Examples/Registration/ImageRegistration7.cxx
//> ITK/Examples/Registration/ImageRegistration9.cxx
//> ITK/Examples/Registration/ImageRegistration12.cxx (With gradient)

*/


// Initiate image container
ImageType::Pointer imageContainer( struct hyspex_header header ){
  ImageType::RegionType region;
  ImageType::IndexType start;

  start[0] = 0;
  start[1] = 0;

  ImageType::SizeType size;
  size[0] = header.samples;
  size[1] = header.lines;

  region.SetSize(size);
  region.SetIndex(start);

  ImageType::Pointer container = ImageType::New();
  container->SetRegions(region);
  container->Allocate();
  return container;
}

// Initiate gradient filter
GradientFilterType::Pointer gradientFilter( ImageType* const fixed, int sigma ){
  GradientFilterType::Pointer gradient = GradientFilterType::New();
  gradient->SetSigma( sigma );
  gradient->SetInput( fixed );
  return gradient;
}

void hyperspec_read_img(const char *filename){
  // Function for handling .img
  // Read hyperspectral header file
	struct hyspex_header header;
	hyperspectral_err_t errcode = hyperspectral_read_header(filename, &header); //see readimage.h for possible error codes.

	// Read hyperspectral image
	float *img  = new float[header.samples*header.lines*header.bands]();
	errcode = hyperspectral_read_image(filename, &header, img);
	// Variable `img` now contains the full hyperspectral image. See also readimage.h for a version of hyperspectral_read_image which reads only a specified subset of the image (using struct image_subset for specifying image subset)

  // Float container for output images
  float *out  = new float[header.samples*header.lines*header.bands]();
  // Float container for output images that show diff between input and output
  float *diff = new float[header.samples*header.lines*header.bands]();

  // Choose registration method
  int regmethod = 1; // TODO: Take from config

  // Create image containers
  ImageType::Pointer fixed = imageContainer(header);
  ImageType::Pointer moving = imageContainer(header);

  // Read center image into fixed for registration
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

  GradientFilterType::Pointer gradient = gradientFilter( fixed, sigma );
  ResampleFilterType::Pointer registration;
  // Read other images for processing
  for (int i=0; i < header.bands; i++){
    // Skip fixed image
    if ( i == centerband ){
      continue;
    }

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
    if (regmethod == 1){
      registration = registration1( fixed, moving );
    } else if (regmethod == 2){
    } else if (regmethod == 3){
    } else if (regmethod == 4){
    } else {
      std::cout << "Specify a method from 1-4. Falling back to method 4" << std::endl;
      registration = registration1( fixed, moving );
    }
 
    itk::ImageRegionIterator<ImageType> imageIterator(
                                      registration->GetOutput(),
                                      registration->GetOutput()->GetLargestPossibleRegion());
    while( !imageIterator.IsAtEnd() ){
      // Get the value of the current pixel
      for (int j=0; j < header.lines; j++){
        for (int k=0; k < header.samples; k++){
          out[j*header.samples*header.bands + i*header.samples + k] = imageIterator.Get();
          ++imageIterator;
        }
      }
    }
    /*
    // Add to output
    //ImageType::Pointer temp = registration->GetOutput();
    for (int j=0; j < header.lines; j++){
      for (int k=0; k < header.samples; k++){
        ImageType::IndexType pixelIndex;
        pixelIndex[0] = k;
        pixelIndex[1] = j;
        out[j*header.samples*header.bands + i*header.samples + k] = (registration->GetOutput())->GetPixel(pixelIndex);
      }
    }
    */	
    // Add to diff

    // Write to file
/*	
	  WriterType::Pointer writer = WriterType::New();
	  writer->SetFileName( buffer );
	  writer->SetInput( registration->GetOutput() );
	  writer->Update();
*/
  }

  hyperspectral_write_header( "test", header.bands, header.samples, header.lines, header.wlens );
  hyperspectral_write_image( "test", header.bands, header.samples, header.lines, out );

  delete [] out;
  delete [] diff;
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
