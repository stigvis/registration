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

Testing:
Lise_arm_before_occlusion_mnf_inversetransformed.img
-> Reduced to 1601x1401x40 -> Fixed = 20
-> 2: 100, -100
-> 4: 200, -100
-> 6: 200, -100
-> 8: 200, -200
-> 10: 20 deg
-> 12: 40 deg
-> 14: -20 deg
-> 16: -40 deg


*/

void hyperspec_img(const char *filename){

  // Read hyperspectral header file
  // See readimage.h for possible error codes.
  struct hyspex_header header;
  hyperspectral_err_t hyp_errcode
    = hyperspectral_read_header(filename, &header);

  // Read hyperspectral image
  float *img  = new float[header.samples*header.lines*header.bands]();
  hyp_errcode = hyperspectral_read_image(filename, &header, img);

  // Read parameters config
  //struct reg_params params;
  //conf_err_t reg_errcode = params_read( &params );

  //cout << "Regmethod: " << params.regmethod << " scale " << params.scale << " translationScale " << params.translationScale << endl;

  // Float container for output images
  float *out  = new float[header.samples*header.lines*header.bands]();
  // Float container for output images that show diff between input and output
  float *diff = new float[header.samples*header.lines*header.bands]();

  // Choose registration method
  int regmethod = 1; // TODO: Take from config

  // Create and choose diffoutput
  int diff_conf = 1; // TODO: Take as input

  // Create itk image pointers
  ImageType::Pointer fixed     = imageContainer(header);
  ImageType::Pointer moving    = imageContainer(header);
  ImageType::Pointer ffixed    = imageContainer(header);
  ImageType::Pointer fmoving   = imageContainer(header);
  ImageType::Pointer output    = imageContainer(header);
  ImageType::Pointer outdiff   = imageContainer(header);
  DifferenceFilterType::Pointer difference = DifferenceFilterType::New();


  // Create and setup a gradient filter
  int sigma = 1; // TODO: Take as input
/*
  GradientFilterType::Pointer gradient
    = gradientFilter( fixed, sigma );
  CastFilterType::Pointer     castFixed
    = castImage( fixed );
  CastFilterType::Pointer     castGradient
    = castImage( gradient->GetOutput() );
*/
  // Initiate pointers
  ResampleFilterType::Pointer       registration;
  TransformRigidType::Pointer       rigid_transform;
  TransformSimilarityType::Pointer  similarity_transform;
  TransformAffineType::Pointer      affine_transform;

  // Read fixed image
  int i = header.bands / 2;
  fixed = readITK( fixed, img, i, header );

  ffixed = medianFilter( fixed, 1 );
  ffixed->Update();

  // Read images for processing
  // Image i=0 is fixed
  for (int i=0; i < header.bands; i++){

    // Read moving image
    moving = readITK( moving, img, i, header );

    // Apply filter
    fmoving = medianFilter( moving, sigma );
    fmoving->Update();

    // Throw to registration handler
    // Rigid transform
    if (regmethod == 1){
      rigid_transform = registration1(
                                  ffixed,
                                  fmoving );
      registration = resampleRigidPointer(
                                  fixed,
                                  moving,
                                  rigid_transform );
      difference = diffFilter(
                                  moving,
                                  registration );
      // Similarity transform
    } else if (regmethod == 2){
      similarity_transform = registration2(
                                  ffixed,
                                  fmoving );
      registration = resampleSimilarityPointer(
                                  fixed,
                                  moving,
                                  similarity_transform );
      difference = diffFilter(
                                  moving,
                                  registration );
      // Affine transform
    } else if (regmethod == 3){
      affine_transform = registration3(
                                  ffixed,
                                  fmoving );
      registration = resampleAffinePointer(
                                  fixed,
                                  moving,
                                  affine_transform );
      difference = diffFilter(
                                  moving,
                                  registration );
    } else {
      cout << "Specify a method from 1-3. Falling back to method 3, Affine" << endl;
      affine_transform = registration3( ffixed, fmoving );
      registration = resampleAffinePointer(
                                  fixed,
                                  moving,
                                  affine_transform );
      difference = diffFilter(
                                  moving,
                                  registration );
    }


    // Add to output containers
    output = registration->GetOutput();
    output->Update();

    outdiff = difference->GetOutput();
    outdiff->Update();

/*
    for (int j=0; j < header.lines; j++){
      for (int k=0; k < header.samples; k++){
        ImageType::IndexType pixelIndex;
        pixelIndex[0] = k;
        pixelIndex[1] = j;
        out[j*header.samples*header.bands + i*header.samples + k] = registration->GetOutput()->GetPixel(pixelIndex);

        // Compute the difference between the images before and after registration
        // (If we want a visible diff-image)
        if ( diff_conf == 1 ){ // TODO: Take from config
          diff[j*header.samples*header.bands + i*header.samples + k] = difference->GetOutput()->GetPixel(pixelIndex);
        }
      }
    }
*/
    // Update output array(s)
    out = writeITK( output, out, i, header );
    if ( diff_conf == 1){
      diff = writeITK( outdiff, diff, i, header );
    }

    cout << "Done with " << i + 1 << " of " << header.bands << endl;

  }
  // Write to .img container
  // See readimage.h
  hyperspectral_write_header( "test_out", header.bands,
    header.samples, header.lines, header.wlens );
  hyperspectral_write_image( "test_out", header.bands,
    header.samples, header.lines, out );

  if ( diff_conf == 1){
    hyperspectral_write_header( "test_diff", header.bands,
      header.samples, header.lines, header.wlens );
    hyperspectral_write_image( "test_diff", header.bands,
      header.samples, header.lines, diff );
  }

  // Clear memory
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

// Reading to image container
ImageType::Pointer readITK( ImageType* const itkimg,
                            float *img,
                            int i,
                            struct hyspex_header header ){

  for (int j=0; j < header.lines; j++){
    for (int k=0; k < header.samples; k++){
      ImageType::IndexType pixelIndex;
      pixelIndex[0] = k;
      pixelIndex[1] = j;
      itkimg->SetPixel(pixelIndex, img[j*header.samples*header.bands
        + i*header.samples + k]);
    }
  }
  return itkimg;
}

// Writing from image container
float* writeITK(            ImageType* const itkimg,
                            float *image,
                            int i,
                            struct hyspex_header header ){

  for (int j=0; j < header.lines; j++){
    for (int k=0; k < header.samples; k++){
      ImageType::IndexType pixelIndex;
      pixelIndex[0] = k;
      pixelIndex[1] = j;
      image[j*header.samples*header.bands + i*header.samples + k]
        = itkimg->GetPixel(pixelIndex);
    }
  }
  return image;
}

// Reading parameters from config
conf_err_t params_read( struct reg_params *params ){

  const int MAX_CHAR = 512;
  const int MAX_FILE_SIZE = 4000;
  string confName = "params.conf";

  // Open for reading
  FILE *fp = fopen("params.conf", "rt");
  if (fp == NULL){
    return CONF_FILE_NOT_FOUND;
  }
  char confText[MAX_FILE_SIZE] = "";
  int sizeRead = 1;
  int offset = 0;
  while (sizeRead){
    sizeRead = fread(confText + offset, sizeof(char), MAX_CHAR, fp);
    offset += sizeRead/sizeof(char);
  }
  fclose(fp);

  // Extract parameters from config
  string regmethod  = getValue(confText, "regmethod");
  string diff_conf  = getValue(confText, "diff_conf");
  string median     = getValue(confText, "median"   );
  string gradient   = getValue(confText, "gradient" );
  string sigma      = getValue(confText, "sigma"    );
  string angle      = getValue(confText, "angle"    );
  string scale      = getValue(confText, "scale"    );
  string lrate      = getValue(confText, "lrate"    );
  string slength    = getValue(confText, "slength"  );
  string niter      = getValue(confText, "niter"    );
  string numberOfLevels
                    = getValue(confText, "numoflev" );
  string translationScale
                    = getValue(confText, "tscale"   );

  // Convert strings to values
  // Set default values for missing strings
  if (regmethod.empty()){
    params->regmethod = 1;
    fprintf(stderr, "Missing regmethod, setting to default value: %d\n", params->regmethod);
  //  cout << "Missing regmethod, setting to default value: "
  //    << params->regmethod << endl;
  } else {
    params->regmethod = strtod(regmethod.c_str(), NULL);
  }
  if (diff_conf.empty()){
    params->diff_conf = 1;
    cout << "Missing diff_conf, setting to default value: "
      << params->diff_conf << endl;
  } else {
    params->diff_conf = strtod(diff_conf.c_str(), NULL);
  }
  if (median.empty()){
    params->median    = 0;
    cout << "Missing median, setting to default value: "
      << params->median << endl;
  } else {
    params->median    = strtod(median.c_str(),    NULL);
  }
  if (gradient.empty()){
    params->gradient  = 0;
    cout << "Missing gradient, setting to default value: "
      << params->gradient << endl;
  } else {
    params->gradient  = strtod(gradient.c_str(),  NULL);
  }
  if (sigma.empty()){
    params->sigma     = 1;
    cout << "Missing sigma, setting to default value: "
      << params->sigma << endl;
  } else {
    params->sigma     = strtod(sigma.c_str(),     NULL);
  }
  if (angle.empty()){
    params->angle     = 0.0;
    cout << "Missing angle, setting to default value: "
      << params->angle << endl;
  } else {
    params->angle     = strtod(angle.c_str(),     NULL);
  }
  if (scale.empty()){
    params->scale     = 1.0;
    cout << "Missing scale, setting to default value: "
      << params->scale << endl;
  } else {
    params->scale     = strtod(scale.c_str(),     NULL);
  }
  if (lrate.empty()){
    params->lrate     = 1.0;
    cout << "Missing lrate, setting to default value: "
      << params->lrate << endl;
  } else {
    params->lrate     = strtod(lrate.c_str(),     NULL);
  }
  if (slength.empty()){
    params->slength   = 0.0001;
    cout << "Missing slength, setting to default value: "
      << params->slength << endl;
  } else {
    params->slength   = strtod(slength.c_str(),   NULL);
  }
  if (niter.empty()){
    params->niter     = 300;
    cout << "Missing niter, setting to default value: "
      << params->niter << endl;
  } else {
    params->niter     = strtod(niter.c_str(),     NULL);
  }
  if (numberOfLevels.empty()){
    params->numberOfLevels
                      = 1;
    cout << "Missing numoflev, setting to default value: "
      << params->numberOfLevels << endl;
  } else {
    params->numberOfLevels
                      = strtod(numberOfLevels.c_str(),
                                                  NULL);
  }
  if (translationScale.empty()){
    params->translationScale
                      = 1.0 / 1000.0;
    cout << "Missing tscale, setting to default value: "
      << params->translationScale << endl;
  } else {
    params->translationScale
                      = strtod(translationScale.c_str(),
                                                  NULL);
  }
  fprintf(stderr, "hurr durr%d\n ", params->translationScale );
  cout << "hurr durr test" << endl;
  return CONF_NO_ERR;
}
