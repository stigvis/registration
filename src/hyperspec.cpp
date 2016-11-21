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
#include "hyperspec.h"
using namespace std;

/* TODO:

Read from config file
//> http://www.hyperrealm.com/libconfig/
//> https://en.wikipedia.org/wiki/Configuration_file

*/

void hyperspec_img(const char *filename){
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
  int regmethod = 3; // TODO: Take from config

  // Create and choose diffoutput
  int diff_conf = 1; // TODO: Take as input

  // Create image containers
  ImageType::Pointer fixed     = imageContainer(header);
  ImageType::Pointer moving    = imageContainer(header);
  ImageType::Pointer ffixed    = imageContainer(header);
  ImageType::Pointer fmoving   = imageContainer(header);
  ImageType::Pointer output    = imageContainer(header);
  ImageType::Pointer outdiff   = imageContainer(header);

  // Create and setup a gradient filter
  int sigma = 1; // TODO: Take as input

  GradientFilterType::Pointer gradient = gradientFilter( fixed, sigma );
  CastFilterType::Pointer castFixed     = castImage( fixed );
  CastFilterType::Pointer castGradient  = castImage( gradient->GetOutput() );

  ResampleFilterType::Pointer registration;
  TransformRigidType::Pointer transform;

  // Get fixed image
  for (int j=0; j < header.lines; j++){
    for (int k=0; k < header.samples; k++){
      ImageType::IndexType pixelIndex;
      pixelIndex[0] = k;
      pixelIndex[1] = j;
      fixed->SetPixel(pixelIndex, img[j*header.samples*header.bands + ( header.bands / 2 )*header.samples + k]);
    }
  }

  ffixed = medianFilter( fixed, 1 );
  ffixed->Update();


  // Read images for processing
  // Image i=0 is fixed
  for (int i=0; i < header.bands; i++){
    for (int j=0; j < header.lines; j++){
      for (int k=0; k < header.samples; k++){
        ImageType::IndexType pixelIndex;
        pixelIndex[0] = k;
        pixelIndex[1] = j;
        //if (i == 1){
          //fixed->SetPixel(pixelIndex, img[j*header.samples*header.bands + (i-1)*header.samples + k]);
          //out[j*header.samples*header.bands + (i-1)*header.samples + k] =
          //                            img[j*header.samples*header.bands + (i-1)*header.samples + k];
        //}
        moving->SetPixel(pixelIndex, img[j*header.samples*header.bands + i*header.samples + k]);
      }
    }

    fmoving = medianFilter( moving, 1 );
    fmoving->Update();

    //if ( i > 1 ){
    //  fixed = registration->GetOutput();
    //  fixed->Update();
    //}
    //fixed = medianFilter( fixed, 1 );
    //fixed->Update();

    // Throw to registration handler
    if (regmethod == 1){                              // Rigid transform
      transform = registration1( ffixed, fmoving );
      registration = resampleRigidPointer(
                                  fixed,
                                  moving,
                                  transform );
    } else if (regmethod == 2){                       // Similarity transform
      registration = registration2( fixed, moving );
    } else if (regmethod == 3){                       // Affine transform
      registration = registration3( fixed, moving );
    } else if (regmethod == 4){                       // Rigid transform + mask
      CastFilterType::Pointer castMoving = castImage ( moving );
      registration = registration4( fixed,
                                    moving,
                                    castGradient->GetOutput() ); // See registration.cpp
    } else {
      cout << "Specify a method from 1-4. Falling back to method 3, Affine" << endl;
      registration = registration3( fixed, moving );
    }

    cout << "Done with " << i + 1 << " of " << header.bands << endl;
    output = registration->GetOutput();
    output->Update();

    DifferenceFilterType::Pointer difference = diffFilter(
                                           moving,
                                           registration ); // See registration.cpp
    //outdiff = difference->GetOutput();
    //outdiff->Update();

    // Add to output
    for (int j=0; j < header.lines; j++){
      for (int k=0; k < header.samples; k++){
        ImageType::IndexType pixelIndex;
        pixelIndex[0] = k;
        pixelIndex[1] = j;
        out[j*header.samples*header.bands + i*header.samples + k] = output->GetPixel(pixelIndex);

        // Compute the difference between the images before and after registration
        // (If we want a visible diff-image)
        if ( diff_conf == 1 ){ // TODO: Take from config
          diff[j*header.samples*header.bands + i*header.samples + k] = outdiff->GetPixel(pixelIndex);
        }
      }
    }
  }

  hyperspectral_write_header( "test_out", header.bands, header.samples, header.lines, header.wlens );
  hyperspectral_write_image( "test_out", header.bands, header.samples, header.lines, out );

  if ( diff_conf == 1){
    hyperspectral_write_header( "test_diff", header.bands, header.samples, header.lines, header.wlens );
    hyperspectral_write_image( "test_diff", header.bands, header.samples, header.lines, diff );
  }

  delete [] out;
  delete [] diff;
  delete [] img;

}

void hyperspec_mat(const char *filename){
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
  cout << "Number of images: " << nSize << ", Image dimensions: " << xSize << "x" << ySize << ", Wavelengths: " ;
  for (int i=0; i<nWave-1; i++){
    cout << wData[i] << ", ";
  }
  cout << wData[nWave-1] << " Array size: " << HSId->nbytes/HSId->data_size << " Compression: " << HSId->compression << " Data type: " << HSId->data_type << endl;

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
      cout << "Error allocating memory." << endl ;
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
