//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"
#include "multispec.h"
#include "fstream"
#include "iostream"
#include "inttypes.h"
using namespace std;

void multispec_raw( int argc, char *argv[] ){

  if ( argc < 2){
    cerr << "Missing raw files." << endl;
    exit(1);
  }

  int xsize = 1024;
  int ysize = 768;

  UintImageType::Pointer moving = UintImageType::New();
  UintImageType::RegionType region;
  UintImageType::IndexType  start;

  start[0] = 0;
  start[1] = 0;

  UintImageType::SizeType   size;
  size[0] = xsize;
  size[1] = ysize;

  region.SetSize(size);
  region.SetIndex(start);

  moving->SetRegions(region);
  moving->Allocate();


  for ( int i=1; i<argc; i++){
    FILE *fid = fopen(argv[i], "rb");
    uint16_t *img_data = new uint16_t[xsize*ysize]();
    int read_bytes = fread(img_data, sizeof(uint16_t), xsize*ysize, fid);
    for ( int j=0; j<ysize; j++){
      for (int k=0; k<xsize; k++){
        UintImageType::IndexType pixelIndex;
        pixelIndex[0] = k;
        pixelIndex[1] = j;
        moving->SetPixel(pixelIndex, img_data[xsize*ysize*(i-1) + xsize*j + k]);
      }
    }
    UintWriterType::Pointer writer = UintWriterType::New();
    writer->SetInput( moving );
    char buffer[32];
    snprintf(buffer, sizeof(char) * 32, "file%i.tif", i);
    writer->SetFileName(buffer);
    writer->Update();
  }
}
