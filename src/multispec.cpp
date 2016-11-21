//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"
#include "multispec.h"
#include "libraw.h"
using namespace std;


void multispec_raw( int argc, char *argv[] ){

  if ( argc < 2){
    cerr << "Missing raw files." << endl;
    exit(1);
  }

  LibRaw ImageProcessor(unsigned int flags=0);


  for ( int i=1; i<argc; i++){
    // TODO: http://www.libraw.org/docs/Samples-LibRaw-eng.html
    //  cout << argv << endl;
  }

}
