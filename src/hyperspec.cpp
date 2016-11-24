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
#include "hyperspec.h"
using namespace std;

/* TODO:

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

  // Read parameters config
  struct reg_params params;
  conf_err_t reg_errcode = params_read( &params );

  // Read hyperspectral header file
  // See readimage.h for possible error codes.
  struct hyspex_header header;
  hyperspectral_err_t hyp_errcode
    = hyperspectral_read_header(filename, &header);

  // Read hyperspectral image
  float *img  = new float[header.samples*header.lines*header.bands]();
  hyp_errcode = hyperspectral_read_image(filename, &header, img);

  // Float container for output images
  float *out  = new float[header.samples*header.lines*header.bands]();
  // Float container for output images that show diff between input and output
  float *diff = new float[header.samples*header.lines*header.bands]();

  // Create itk image pointers
  // Input images
  ImageType::Pointer fixed     = imageContainer(header);
  ImageType::Pointer moving    = imageContainer(header);
  // Filtered images
  ImageType::Pointer ffixed    = imageContainer(header);
  ImageType::Pointer fmoving   = imageContainer(header);
  // Output images
  ImageType::Pointer output    = imageContainer(header);
  ImageType::Pointer outdiff   = imageContainer(header);
  // Difference image
  DifferenceFilterType::Pointer difference = DifferenceFilterType::New();

  // Initiate pointers
  ResampleFilterType::Pointer       registration;
  TransformRigidType::Pointer       rigid_transform;
  TransformSimilarityType::Pointer  similarity_transform;
  TransformAffineType::Pointer      affine_transform;

  // Read fixed image
  int i = header.bands / 2;
  fixed = readITK( fixed, img, i, header );

  // Filter image
  ffixed = fixed;
  if (params.median == 1){
    ffixed = medianFilter( ffixed, params.sigma );
    ffixed->Update();
  }
  if (params.gradient == 1){
    ffixed = gradientFilter( ffixed, params.sigma );
    ffixed->Update();
  }

  // Read images for processing
  // Image i=0 is fixed
  for (int i=0; i < header.bands; i++){

    // Read moving image
    moving = readITK( moving, img, i, header );

    // Filter images
    fmoving = moving;
    if ( params.median == 1){
      fmoving = medianFilter( fmoving, params.sigma );
      fmoving->Update();
    }
    if ( params.gradient == 1){
      fmoving = gradientFilter( fmoving, params.sigma );
      fmoving->Update();
    }

    // Throw to registration handler
    // Rigid transform
    if (params.regmethod == 1){
      rigid_transform = registration1(
                                  ffixed,
                                  fmoving,
                                  params );
      registration = resampleRigidPointer(
                                  fixed,
                                  moving,
                                  rigid_transform );
      difference = diffFilter(
                                  moving,
                                  registration );
      // Similarity transform
    } else if (params.regmethod == 2){
      similarity_transform = registration2(
                                  ffixed,
                                  fmoving,
                                  params );
      registration = resampleSimilarityPointer(
                                  fixed,
                                  moving,
                                  similarity_transform );
      difference = diffFilter(
                                  moving,
                                  registration );
      // Affine transform
    } else if (params.regmethod == 3){
      affine_transform = registration3(
                                  ffixed,
                                  fmoving,
                                  params );
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

    // Update output array(s)
    out = writeITK( output, out, i, header );
    if ( params.diff_conf == 1){
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

  if ( params.diff_conf == 1){
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

const int MAX_CHAR = 512;
const int MAX_FILE_SIZE = 4000;

// Reading parameters from config
conf_err_t params_read( struct reg_params *params ){

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

  // Extract parameters from config
  string regmethod  = getParam(confText, "regmethod");
  string diff_conf  = getParam(confText, "diff_conf");
  string median     = getParam(confText, "median"   );
  string radius     = getParam(confText, "radius"   );
  string gradient   = getParam(confText, "gradient" );
  string sigma      = getParam(confText, "sigma"    );
  string angle      = getParam(confText, "angle"    );
  string scale      = getParam(confText, "scale"    );
  string lrate      = getParam(confText, "lrate"    );
  string slength    = getParam(confText, "slength"  );
  string niter      = getParam(confText, "niter"    );
  string numberOfLevels
                    = getParam(confText, "numoflev" );
  string translationScale
                    = getParam(confText, "tscale"   );

  cout << "Reading parameters from params.conf" << endl;

  // Convert strings to values
  // Set default values for missing strings
  if (regmethod == "" ){
    params->regmethod = 1;
    fprintf(stderr, "Missing regmethod, setting to default value: %d\n", params->regmethod);
    cout << "Missing regmethod, setting to default value: "
      << params->regmethod << endl;
  } else {
    params->regmethod = strtod(regmethod.c_str(), NULL);
  }
  if (diff_conf == ""){
    params->diff_conf = 1;
    cout << "Missing diff_conf, setting to default value: "
      << params->diff_conf << endl;
  } else {
    params->diff_conf = strtod(diff_conf.c_str(), NULL);
  }
  if (median.empty()){
    params->median    = 1;
    cout << "Missing median, setting to default value: "
      << params->median << endl;
  } else {
    params->median    = strtod(median.c_str(),    NULL);
  }
  if (radius.empty()){
    params->radius    = 1;
    cout << "Missing radius, setting to default value: "
      << params->radius << endl;
  } else {
    params->radius    = strtod(radius.c_str(),    NULL);
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
                      = 0.001;
    cout << "Missing tscale, setting to default value: "
      << params->translationScale << endl;
  } else {
    params->translationScale
                      = strtod(translationScale.c_str(),
                                                  NULL);
  }
  fclose(fp);
  cout  << "Parameters: "           << endl;
  cout  << "Registration method: "  << params->regmethod
        << " Difference image: "    << params->diff_conf
        << " Median filtering: "    << params->median
        << " Gradient filtering: "  << params->gradient
        << " Median radius: "       << params->radius
        << " Gradient sigma: "      << params->sigma
        << " Initial angle: "       << params->angle
        << " Initial scale: "       << params->scale
        << " Learning rate: "       << params->lrate
        << " Minimum step length: " << params->slength
        << " Number of iterations: "<< params->niter
        << " numberOfLevels: "      << params->numberOfLevels
        << " translationScale: "    << params->translationScale
        << endl;

  return CONF_NO_ERR;
}

// Read config file with regex
string getParam(string confText, string property){
  regex_t propertyMatch;
  int numMatch = 2;
  regmatch_t *matchArray = (regmatch_t*)malloc(sizeof(regmatch_t)*numMatch);

  char regexExpr[MAX_CHAR] = "";
  strcat(regexExpr, property.c_str());
  //property followed by = and a set of number or dots
	strcat(regexExpr, "\\s*=\\s*([0-9|.]+)");

	int retcode = regcomp(&propertyMatch, regexExpr, REG_EXTENDED | REG_NEWLINE | REG_PERL);
  int match = regexec(&propertyMatch, confText.c_str(), numMatch, matchArray, 0);

  string retVal;
  if (match != 0){
    retVal = "";
  } else {
    retVal = confText.substr(matchArray[1].rm_so, matchArray[1].rm_eo - matchArray[1].rm_so);
  }

	//cleanup
	regfree(&propertyMatch);
	free(matchArray);

	return retVal;
}

