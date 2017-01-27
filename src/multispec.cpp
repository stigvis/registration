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
  UintImageType::Pointer fixed_raw    = rawContainer( xsize, ysize );
  UintImageType::Pointer moving_raw   = rawContainer( xsize, ysize );
  ImageType::Pointer fixed            = imgContainer( xsize, ysize );
  ImageType::Pointer moving           = imgContainer( xsize, ysize );
  // Filtered images
  ImageType::Pointer ffixed           = imgContainer( xsize, ysize );
  ImageType::Pointer fmoving          = imgContainer( xsize, ysize );
  // Output images
  ImageType::Pointer output           = imgContainer( xsize, ysize );
  ImageType::Pointer outdiff          = imgContainer( xsize, ysize );
  UintImageType::Pointer output_raw   = rawContainer( xsize, ysize );
  UintImageType::Pointer outdiff_raw  = rawContainer( xsize, ysize );

  // Difference image
  DifferenceFilterType::Pointer difference = DifferenceFilterType::New();

  // Initiate pointers
  ResampleFilterType::Pointer       registration;

  // Read fixed image
  fixed_raw = readRaw(fixed_raw, 1, xsize, ysize, argv[1]);
  //Write out with specified naming scheme
  writeRaw( fixed_raw, 1, xsize, ysize, params.reg_name );


  // Cast to float
  CastFilterFloatType::Pointer fixed_cast_in = castFloatImage( fixed_raw );
  fixed = fixed_cast_in->GetOutput();
  fixed->Update();

  /* Uncomment for writing to .tif
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( "input1.tif" );
  writer->SetInput( fixed_raw );
  writer->Update();
  */

  // Filter images
  ffixed = fixed;
  if ( params.median == 1){
    ffixed = medianFilter( ffixed, params.radius );
    ffixed->Update();
  }
  if ( params.gradient == 1){
    ffixed = gradientFilter( ffixed, params.sigma );
    ffixed->Update();
  }

  for ( int i=2; i<argc; i++){

    char buffer[32];
    snprintf(buffer, sizeof(char) * 32, "1%i.tif", i);
    // Read moving images
    moving_raw = readRaw(moving_raw, i, xsize, ysize, argv[i] );

    // Cast to float
    CastFilterFloatType::Pointer moving_cast_in = castFloatImage( moving_raw );
    moving = moving_cast_in->GetOutput();

    /* Uncomment for writing to .tif
    WriterType::Pointer writer2 = WriterType::New();
    string name = "input";
    name += to_string(i);
    name += ".tif";
    writer2->SetFileName( name );
    writer2->SetInput( moving );
    writer2->Update();
    */

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

    WarperType::Pointer warper = WarperType::New();
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
    } else if (params.regmethod == 5){
      CompositeTransformType::Pointer translation_transform;
      translation_transform = translation(
                                  ffixed,
                                  fmoving,
                                  params );

      ResampleFilterType::Pointer resample = ResampleFilterType::New();
      resample->SetTransform(          translation_transform          );
      resample->SetInput(                     moving                  );
      resample->SetSize(  fixed->GetLargestPossibleRegion().GetSize() );
      resample->SetOutputOrigin(         fixed->GetOrigin()           );
      resample->SetOutputSpacing(        fixed->GetSpacing()          );
      resample->SetDefaultPixelValue(               0.0               );
      registration = resample;

      difference = diffFilter(
                                  moving,
                                  registration );
    } else if (params.regmethod == 6){
      warper = registration5(
                                  fixed,
                                  moving,
                                  params );
    }

    // Add to output containers
    if (params.regmethod == 6){
      output = warper->GetOutput();
      output->Update();
    } else {
      output = registration->GetOutput();
      output->Update();

      outdiff = difference->GetOutput();
      outdiff->Update();
    }

    // Write images

    /* Uncomment for writing to .tif
    WriterType::Pointer writer3 = WriterType::New();
    string name2 = params.reg_name;
    name2 += to_string(i);
    name2 += ".tif";
    writer3->SetFileName( name2 );
    writer3->SetInput( output );
    writer3->Update();
    */

    CastFilterUintType::Pointer moving_cast_out = castUintImage ( output );
    output_raw = moving_cast_out->GetOutput();
    writeRaw( output_raw, i, xsize, ysize, params.reg_name );

    // Write diff
    if ( params.diff_conf == 4 && params.regmethod != 6){
      CastFilterUintType::Pointer diff_cast_out = castUintImage ( outdiff );
      outdiff_raw = diff_cast_out->GetOutput();
      outdiff_raw->Update();
      writeRaw( outdiff_raw, i, xsize, ysize, params.diff_name );
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
UintImageType::Pointer readRaw( UintImageType* const itkraw,
                                int i,
                                int xsize,
                                int ysize,
                                char *argv ){

  cout << "In: " << argv << endl;

  // Open images
  FILE *fid = fopen(argv, "rb");
  if (fid == NULL){
    perror(argv);
    exit(EXIT_FAILURE);
  }

  unsigned short *in_data = new unsigned short[xsize*ysize]();
  int read_bytes = fread(in_data, sizeof(uint16_t), xsize*ysize, fid);

  // One pixel at a time
  for ( int j=0; j<ysize; j++){
    for (int k=0; k<xsize; k++){
      UintImageType::IndexType pixelIndex;
      pixelIndex[0] = k;
      pixelIndex[1] = j;
      itkraw->SetPixel(pixelIndex, in_data[xsize*j + k]);
    }
  }

  fclose( fid );
  delete [] in_data;
  return itkraw;
}

// Writing from itk image container
void writeRaw(  UintImageType* itkimg,
                int i,
                int xsize,
                int ysize,
                string name ){

  // Recursive names
  name += to_string(i);
  //string writername = name;
  //writername += ".png";
  name += ".raw";

  // Prepare
  fstream fid (name.c_str(), ios::out | ios::binary);
  unsigned short *out_data = new unsigned short[xsize*ysize]();
  size_t size = sizeof(unsigned short) * (xsize*ysize);

  // One pixel at a time
  for ( int j=0; j<ysize; j++){
    for ( int k=0; k<xsize; k++){
      UintImageType::IndexType pixelIndex;
      pixelIndex[0] = k;
      pixelIndex[1] = j;
      out_data[xsize*j + k] = itkimg->GetPixel(pixelIndex);
    }
  }

  cout << "Out: " << name << endl;
  // Write
  fid.write (reinterpret_cast<char*>(out_data), size);
  // Clean
  delete [] out_data;
  fid.close();

}
