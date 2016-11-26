//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "multispec.h"
#include "hyperspec.h"
#include "registration.h"
#include "fstream"
#include "iostream"
#include "inttypes.h"
using namespace std;

void multispec_raw( int argc, char *argv[] ){

  if ( argc < 2){
    cerr << "Missing raw files." << endl;
    exit(1);
  }

  // Read parameters config
  struct reg_params params;
  conf_err_t reg_errcode = params_read( &params );

  // Known size of input files
  int xsize = 1024;
  int ysize = 768;

  // Input images
  ImageType::Pointer fixed            = imgContainer( xsize, ysize );
  UintImageType::Pointer fixed_raw    = rawContainer( xsize, ysize );
  ImageType::Pointer moving           = imgContainer( xsize, ysize );
  UintImageType::Pointer moving_raw   = rawContainer( xsize, ysize );
  // Filtered images
  ImageType::Pointer ffixed           = imgContainer( xsize, ysize );
  ImageType::Pointer fmoving          = imgContainer( xsize, ysize );
  // Output images
  ImageType::Pointer output           = imgContainer( xsize, ysize );
  UintImageType::Pointer output_raw   = rawContainer( xsize, ysize );
  ImageType::Pointer outdiff          = imgContainer( xsize, ysize );
  UintImageType::Pointer outdiff_raw  = rawContainer( xsize, ysize );

  // Difference image
  DifferenceFilterType::Pointer difference = DifferenceFilterType::New();

  // Initiate pointers
  ResampleFilterType::Pointer       registration;
  TransformRigidType::Pointer       rigid_transform;
  TransformSimilarityType::Pointer  similarity_transform;
  TransformAffineType::Pointer      affine_transform;

  // Read fixed image
  // Open images
  fixed_raw = readRaw(fixed_raw, argc/2, xsize, ysize, argv );

  UintWriterType::Pointer writer1 = UintWriterType::New();
  writer1->SetFileName("3.tif");
  writer1->SetInput(fixed_raw);
  writer1->Update();

  // Cast to float
  CastFilterFloatType::Pointer fixed_cast_in = castFloatImage( fixed_raw );
  fixed = fixed_cast_in->GetOutput();
  fixed->Update();

  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName("2.tif");
  writer->SetInput(fixed);
  writer->Update();

  // Filter images
  ffixed = fixed;
  if ( params.median == 1){
    fmoving = medianFilter( ffixed, params.radius );
    fmoving->Update();
  }
  if ( params.gradient == 1){
    ffixed = gradientFilter( ffixed, params.sigma );
    ffixed->Update();
  }

  for ( int i=1; i<argc; i++){

    // Read moving images
    moving_raw = readRaw(moving_raw, i, xsize, ysize, argv );

    // Cast to float
    CastFilterFloatType::Pointer moving_cast_in = castFloatImage( moving_raw );
    moving = moving_cast_in->GetOutput();
    //moving->Update();

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
    //output = registration->GetOutput();
    //output->Update();

    outdiff = difference->GetOutput();
    outdiff->Update();

    // Write images
    CastFilterUintType::Pointer moving_cast_out = castUintImage ( registration->GetOutput() );
    output_raw = moving_cast_out->GetOutput();
    /*try {
      output_raw->Update();
    }
    catch( itk::ExceptionObject & err ){
      cerr << "ExceptionObject caught !" << endl;
      cerr << err << endl;
      exit(1);
    }
    output_raw->Update();
*/
    writeRaw( output_raw, i, xsize, ysize, params );

    // Write diff
    if ( params.diff_conf == 4 ){
      CastFilterUintType::Pointer diff_cast_out = castUintImage ( outdiff );
      outdiff_raw = diff_cast_out->GetOutput();
      outdiff_raw->Update();
      writeRaw( outdiff_raw, i, xsize, ysize, params );
    }

    cout << "Done with " << i << " of " << argc-1 << endl;
  }
}

// Creating itk image container
// Unsigned short
UintImageType::Pointer rawContainer(
                                int xsize,
                                int ysize ){

  UintImageType::RegionType region;
  UintImageType::IndexType  start;

  start[0] = 0;
  start[1] = 0;

  UintImageType::SizeType   size;
  size[0] = xsize;
  size[1] = ysize;

  region.SetSize(size);
  region.SetIndex(start);

  UintImageType::Pointer uintType = UintImageType::New();
  uintType->SetRegions(region);
  uintType->Allocate();

  return uintType;
}

// Float
ImageType::Pointer imgContainer(
                                int xsize,
                                int ysize ){
  ImageType::RegionType region;
  ImageType::IndexType start;

  start[0] = 0;
  start[1] = 0;

  ImageType::SizeType size;
  size[0] = xsize;
  size[1] = ysize;

  region.SetSize(size);
  region.SetIndex(start);

  ImageType::Pointer container = ImageType::New();
  container->SetRegions(region);
  container->Allocate();
  return container;
}

// Reading to itk image container
UintImageType::Pointer readRaw( UintImageType* itkraw,
                                int i,
                                int xsize,
                                int ysize,
                                char *argv[] ){
  // Open images
  FILE *fid = fopen(argv[i], "rb");
  uint16_t *img_data = new uint16_t[xsize*ysize]();
  int read_bytes = fread(img_data, sizeof(uint16_t), xsize*ysize, fid);
  fclose( fid );

  // One pixel at a time
  for ( int j=0; j<ysize; j++){
    for (int k=0; k<xsize; k++){
      UintImageType::IndexType pixelIndex;
      pixelIndex[0] = k;
      pixelIndex[1] = j;
      itkraw->SetPixel(pixelIndex, img_data[xsize*ysize*(i-1) + xsize*j + k]);
      //cout << img_data[xsize*ysize*(i-1) + xsize*j + k] << endl;
    }
  }

  cout << argv[i] << endl;
  cout << itkraw << endl;
  return itkraw;
}

// Writing from itk image container
void writeRaw(  UintImageType* itkimg,
                int i,
                int xsize,
                int ysize,
                reg_params params ){
/*
  // Prepare
  char buffer[32];
  snprintf(buffer, sizeof(char) * 32, "%d%d.raw", params.reg_name, i-1);
  FILE *fid = fopen(buffer, "wb");
  uint16_t *out_data = new uint16_t[xsize*ysize]();
*/
  // One pixel at a time
  for ( int j=0; j<ysize; j++){
    for ( int k=0; k<xsize; k++){
      UintImageType::IndexType pixelIndex;
      pixelIndex[0] = k;
      pixelIndex[0] = j;
      cout << j << endl;
      cout << k << endl;
      //out_data[xsize*ysize*(i-1) + xsize*j + k] =
      cout << itkimg->GetPixel(pixelIndex) << endl;
    }
  }

  // Write
 // fclose(fid);
  cout << "1" << endl;

}
