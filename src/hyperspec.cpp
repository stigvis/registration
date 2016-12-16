//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "readimage.h"
#include "registration.h"
#include "hyperspec.h"
using namespace std;

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
  // Resample image
  ResampleFilterType::Pointer       registration;

  // Read fixed image
  int i = header.bands / 2;
  fixed = readITK( fixed, img, i, header );

  // Filter image
  ffixed = fixed;
  if (params.median == 1){
    ffixed = medianFilter( ffixed, params.radius );
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

    // Skip center band (fixed)
    if ( i == header.bands/2 ){
      out = writeITK( moving, out, i, header );
      continue;
    }

    // Filter images
    fmoving = moving;
    if ( params.median == 1){
      fmoving = medianFilter( fmoving, params.radius );
      fmoving->Update();
    }
    if ( params.gradient == 1){
      fmoving = gradientFilter( fmoving, params.sigma );
      fmoving->Update();
    }

    // Throw to registration handler
    // Rigid transform
    if (params.regmethod == 1){
      TransformRigidType::Pointer       rigid_transform;
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
      TransformSimilarityType::Pointer  similarity_transform;
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
      TransformAffineType::Pointer      affine_transform;
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
      // BSpline transform
    } else if (params.regmethod == 4){
      TransformBSplineType::Pointer      bspline_transform;
      bspline_transform = registration4(
                                  ffixed,
                                  fmoving,
                                  params );
      registration = resampleBSplinePointer(
                                  fixed,
                                  moving,
                                  bspline_transform );
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

    // Need some images for the report
    WriterType::Pointer writer = WriterType::New();
    string name = params.reg_name;
    name += to_string(i);
    name += ".tif";
    writer->SetFileName( name );
    writer->SetInput( output );
    //writer->SetInput( moving );
    writer->Update();


    cout << "Done with " << i + 1 << " of " << header.bands << endl;

  }
  // Write to .img container
  // See readimage.h
  hyperspectral_write_header( params.reg_name.c_str(), header.bands,
    header.samples, header.lines, header.wlens );
  hyperspectral_write_image( params.reg_name.c_str(), header.bands,
    header.samples, header.lines, out );

  if ( params.diff_conf == 1){
    hyperspectral_write_header( params.diff_name.c_str(), header.bands,
      header.samples, header.lines, header.wlens );
    hyperspectral_write_image( params.diff_name.c_str(), header.bands,
      header.samples, header.lines, diff );
  }

  // Clear memory
  delete [] out;
  delete [] diff;
  delete [] img;

}

void hyperspec_mat(const char *filename){

  // Read parameters config
  struct reg_params params;
  conf_err_t reg_errcode = params_read( &params );

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
  float *wData = static_cast<float*>(wavelengthsd->data);
  float *hData = static_cast<float*>(HSId->data);
  cout  << "Number of images: "
        << nSize
        << ", Image dimensions: "
        << xSize
        << "x"
        << ySize
        << ", Wavelengths: ";
  for (int i=0; i<nWave-1; i++){
    cout
        << wData[i]
        << ", ";
  }
  cout  << wData[nWave-1]
        << " Data type, wavelengths: "
        << wavelengthsd->data_type
        << " Compression, wavelengths: "
        << wavelengthsd->compression
        << " Array size: "
        << HSId->nbytes/HSId->data_size
        << " Compression, images: "
        << HSId->compression
        << " Data type, images: "
        << HSId->data_type
        << endl;


  // Declare ITK pointers
  ImageType::Pointer fixed    = imageMatContainer( xSize, ySize );
  ImageType::Pointer ffixed   = imageMatContainer( xSize, ySize );
  ImageType::Pointer moving   = imageMatContainer( xSize, ySize );
  ImageType::Pointer fmoving  = imageMatContainer( xSize, ySize );
  ImageType::Pointer output   = imageMatContainer( xSize, ySize );
  ImageType::Pointer outdiff  = imageMatContainer( xSize, ySize );
  // Difference image
  DifferenceFilterType::Pointer difference = DifferenceFilterType::New();
  // Resample image
  ResampleFilterType::Pointer       registration;

  // Read fixed
  fixed = readMat(fixed, nSize/2, xSize, ySize, hData);

  // Filter image
  ffixed = fixed;
  if (params.median == 1){
    ffixed = medianFilter( ffixed, params.radius );
    ffixed->Update();
  }
  if (params.gradient == 1){
    ffixed = gradientFilter( ffixed, params.sigma );
    ffixed->Update();
  }


  float *out  = new float[xSize*ySize*nSize]();
  float *diff = new float[xSize*ySize*nSize]();
  for (int i=0; i<nSize; i++){

    // Read moving
    moving = readMat( moving, i, xSize, ySize, hData );

    // Skip center band (fixed)
    if ( i == nSize/2 ){
      out = writeMat( moving, out, i, xSize, ySize );
      continue;
    }

    // Filter images
    fmoving = moving;
    if ( params.median == 1){
      fmoving = medianFilter( fmoving, params.radius );
      fmoving->Update();
    }
    if ( params.gradient == 1){
      fmoving = gradientFilter( fmoving, params.sigma );
      fmoving->Update();
    }

    // Throw to registration handler
    // Rigid transform
    if (params.regmethod == 1){
      TransformRigidType::Pointer       rigid_transform;
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
      TransformSimilarityType::Pointer  similarity_transform;
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
      TransformAffineType::Pointer      affine_transform;
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
      // BSpline transform
    } else if (params.regmethod == 4){
      TransformBSplineType::Pointer      bspline_transform;
      bspline_transform = registration4(
                                  ffixed,
                                  fmoving,
                                  params );
      registration = resampleBSplinePointer(
                                  fixed,
                                  moving,
                                  bspline_transform );
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
    out = writeMat( output, out, i, xSize, ySize );
    if ( params.diff_conf == 1){
      diff = writeMat( output, diff, i, xSize, ySize );
    }

    cout << "Done with " << i + 1 << " of " << nSize << endl;

  }

  // Write to .mat container
  outMat( out, params.reg_name, wavelengthsd, HSId );
  if (params.diff_conf == 1 ){
    outMat( diff, params.diff_name, wavelengthsd, HSId );
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
    cout  << "Missing config file. Will use default values."
          << endl;
  //  return CONF_FILE_NOT_FOUND;
  }
  char confText[MAX_FILE_SIZE] = "";
  int sizeRead = 1;
  int offset = 0;
  while (sizeRead){
    sizeRead = fread(confText + offset, sizeof(char), MAX_CHAR, fp);
    offset += sizeRead/sizeof(char);
  }

  // Extract parameters from config
  string regmethod  = getParam(confText, "regmethod"    );
  string reg_name   = getParam(confText, "reg_name"     );
  string diff_conf  = getParam(confText, "diff_conf"    );
  string diff_name  = getParam(confText, "diff_name"    );
  string median     = getParam(confText, "median"       );
  string radius     = getParam(confText, "radius"       );
  string gradient   = getParam(confText, "gradient"     );
  string sigma      = getParam(confText, "sigma"        );
  string angle      = getParam(confText, "angle"        );
  string scale      = getParam(confText, "scale"        );
  string lrate      = getParam(confText, "lrate"        );
  string slength    = getParam(confText, "slength"      );
  string niter      = getParam(confText, "niter"        );
  string numberOfLevels
                    = getParam(confText, "numoflev"     );
  string translationScale
                    = getParam(confText, "tscale"       );
  string translation
                    = getParam(confText, "translation"  );
  string metric     = getParam(confText, "metric"       );
  string output     = getParam(confText, "output"       );

  cout << "Reading parameters from params.conf" << endl;

  // Convert strings to values
  // Set default values for missing strings
  if (regmethod.empty() || fp == NULL ){
    params->regmethod = 1;
    cout << "Missing regmethod, setting to default value: "
      << params->regmethod << endl;
  } else {
    params->regmethod = strtod(regmethod.c_str(), NULL);
  }
  if (reg_name.empty() || fp == NULL ){
    params->reg_name = "out";
    cout << "Missing reg_name, setting to default value: "
      << params->reg_name << endl;
  } else {
    params->reg_name = reg_name;
  }
  if (diff_conf.empty() || fp == NULL ){
    params->diff_conf = 1;
    cout << "Missing diff_conf, setting to default value: "
      << params->diff_conf << endl;
  } else {
    params->diff_conf = strtod(diff_conf.c_str(), NULL);
  }
  if (diff_name.empty() || fp == NULL ){
    params->diff_name = "diffout";
    cout << "Missing diff_name, setting to default value: "
      << params->regmethod << endl;
  } else {
    params->diff_name = diff_name;
  }
  if (median.empty() || fp == NULL ){
    params->median    = 1;
    cout << "Missing median, setting to default value: "
      << params->median << endl;
  } else {
    params->median    = strtod(median.c_str(),    NULL);
  }
  if (radius.empty() || fp == NULL ){
    params->radius    = 1;
    cout << "Missing radius, setting to default value: "
      << params->radius << endl;
  } else {
    params->radius    = strtod(radius.c_str(),    NULL);
  }
  if (gradient.empty() || fp == NULL ){
    params->gradient  = 0;
    cout << "Missing gradient, setting to default value: "
      << params->gradient << endl;
  } else {
    params->gradient  = strtod(gradient.c_str(),  NULL);
  }
  if (sigma.empty() || fp == NULL ){
    params->sigma     = 1;
    cout << "Missing sigma, setting to default value: "
      << params->sigma << endl;
  } else {
    params->sigma     = strtod(sigma.c_str(),     NULL);
  }
  if (angle.empty() || fp == NULL ){
    params->angle     = 0.0;
    cout << "Missing angle, setting to default value: "
      << params->angle << endl;
  } else {
    params->angle     = strtod(angle.c_str(),     NULL);
  }
  if (scale.empty() || fp == NULL ){
    params->scale     = 1.0;
    cout << "Missing scale, setting to default value: "
      << params->scale << endl;
  } else {
    params->scale     = strtod(scale.c_str(),     NULL);
  }
  if (lrate.empty() || fp == NULL ){
    params->lrate     = 1.0;
    cout << "Missing lrate, setting to default value: "
      << params->lrate << endl;
  } else {
    params->lrate     = strtod(lrate.c_str(),     NULL);
  }
  if (slength.empty() || fp == NULL ){
    params->slength   = 0.0001;
    cout << "Missing slength, setting to default value: "
      << params->slength << endl;
  } else {
    params->slength   = strtod(slength.c_str(),   NULL);
  }
  if (niter.empty() || fp == NULL ){
    params->niter     = 300;
    cout << "Missing niter, setting to default value: "
      << params->niter << endl;
  } else {
    params->niter     = strtod(niter.c_str(),     NULL);
  }
  if (numberOfLevels.empty() || fp == NULL ){
    params->numberOfLevels
                      = 1;
    cout << "Missing numoflev, setting to default value: "
      << params->numberOfLevels << endl;
  } else {
    params->numberOfLevels
                      = strtod(numberOfLevels.c_str(),
                                                  NULL);
  }
  if (translationScale.empty() || fp == NULL ){
    params->translationScale
                      = 0.001;
    cout << "Missing tscale, setting to default value: "
      << params->translationScale << endl;
  } else {
    params->translationScale
                      = strtod(translationScale.c_str(),
                                                  NULL);
  }
  if (translation.empty() || fp == NULL ){
    params->translation
                      = 0;
    cout << "Missing translation, setting to default value: "
      << params->translation << endl;
  } else {
    params->translation
                      = strtod(translation.c_str(),
                                                  NULL);
  }
  if (metric.empty() || fp == NULL ){
    params->metric    = 0;
    cout << "Missing translation, setting to default value: "
      << params->metric << endl;
  } else {
    params->metric    = strtod(translation.c_str(),
                                                  NULL);
  }
  if (output.empty() || fp == NULL ){
    params->output    = 1;
    cout << "Missing output, setting to default value: "
      << params->output << endl;
  } else {
    params->output    = strtod(output.c_str(),    NULL);
  }

  fclose(fp);
  cout  << "Parameters:"           << endl
        << endl
        << "Registration method: " << params->regmethod
        << endl
        << "Registration name: "   << params->reg_name
        << endl
        << "Difference image: "    << params->diff_conf
        << endl
        << "Difference name: "     << params->diff_name
        << endl
        << "Median filtering: "    << params->median
        << endl
        << "Gradient filtering: "  << params->gradient
        << endl
        << "Median radius: "       << params->radius
        << endl
        << "Gradient sigma: "      << params->sigma
        << endl
        << "Initial angle: "       << params->angle
        << endl
        << "Initial scale: "       << params->scale
        << endl
        << "Learning rate: "       << params->lrate
        << endl
        << "Minimum step length: " << params->slength
        << endl
        << "Number of iterations: "<< params->niter
        << endl
        << "numberOfLevels: "      << params->numberOfLevels
        << endl
        << "translationScale: "    << params->translationScale
        << endl
        << "Translation: "         << params->translation
        << endl
        << "Metric: "              << params->metric
        << endl
        << "Output: "              << params->output
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
  strcat(regexExpr, "\\s*=\\s*([0-9|.|a-z|\"|]+)");

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

// Initiate image container
ImageType::Pointer imageMatContainer(
                                unsigned xSize,
                                unsigned ySize ){
  ImageType::RegionType region;
  ImageType::IndexType start;

  start[0] = 0;
  start[1] = 0;

  ImageType::SizeType size;
  size[0] = xSize;
  size[1] = ySize;

  region.SetSize(size);
  region.SetIndex(start);

  ImageType::Pointer container = ImageType::New();
  container->SetRegions(region);
  container->Allocate();
  return container;
}

ImageType::Pointer readMat( ImageType* const itkmat,
                                int i,
                                unsigned xSize,
                                unsigned ySize,
                                float *hData ){
  for (int j=0; j < ySize; j++) {
    for (int k=0; k < xSize; k++) {
      //UintImageType::IndexType pixelIndex;
      ImageType::IndexType pixelIndex;
      pixelIndex[0] = k;
      pixelIndex[1] = j;
      itkmat->SetPixel(pixelIndex, hData[j + xSize*i + xSize*ySize*i]);
    }
  }
  return itkmat;
}

float* writeMat(            ImageType* const itkmat,
                            float *hData,
                            int i,
                            unsigned xSize,
                            unsigned ySize ){
  for ( int j=0; j < ySize; j++ ){
    for ( int k=0; k < xSize; k++ ){
      ImageType::IndexType pixelIndex;
      pixelIndex[0] = k;
      pixelIndex[1] = j;
      hData[j + xSize*i + xSize*ySize*i]
        = itkmat->GetPixel(pixelIndex);
    }
  }
  return hData;
}

void outMat(                float *hData,
                            string outname,
                            matvar_t *wavelengthsd,
                            matvar_t *HSId ){

  mat_t *matout;

  matout = Mat_Open(outname.c_str(),MAT_ACC_RDWR);
  Mat_VarWrite( matout, wavelengthsd, MAT_COMPRESSION_ZLIB );

  matvar_t *HSI = Mat_VarCreate("HSI",MAT_C_SINGLE,MAT_T_SINGLE,
    HSId->rank,HSId->dims,static_cast<void*>(hData),0);
  Mat_VarWrite( matout, HSI, MAT_COMPRESSION_ZLIB );
  Mat_VarFree(HSI);

}
