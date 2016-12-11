// =========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"
using namespace std;

// ===================================
// Image registration method 3
// ===================================
TransformAffineType::Pointer registration3(
                                        ImageType* const fixed,
                                        ImageType* const moving,
                                        reg_params params ){

  // Optimizer and Registration containers
  OptimizerType::Pointer          optimizer     = OptimizerType::New();
  RegistrationAffineType::Pointer registration  = registrationAffineContainer(
                                        fixed,
                                        moving,
                                        optimizer );

  // Construction of the transform object
  TransformAffineType::Pointer    transform     = TransformAffineType::New();
  TransformAffineInitializerType::Pointer initializer = initializerAffineContainer(
                                        fixed,
                                        moving,
                                        transform );

  // Set parameters
  if (params.translation == 1 ){
    CompositeTransformType::Pointer ttransform = translation(
                                        fixed,
                                        moving,
                                        params );
    registration->SetMovingInitialTransform( ttransform );
  } else {
    registration->SetInitialTransform( transform );
  }
  registration->InPlaceOn();

  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );

  optimizerScales[0] =  1.0;
  optimizerScales[1] =  1.0;
  optimizerScales[2] =  1.0;
  optimizerScales[3] =  1.0;
  optimizerScales[4] =  params.translationScale;
  optimizerScales[5] =  params.translationScale;

  optimizer->SetScales(         optimizerScales     );
  optimizer->SetLearningRate(     params.lrate      );
  optimizer->SetMinimumStepLength( params.slength   );
  optimizer->SetNumberOfIterations( params.niter    );

  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  RegistrationAffineType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
  shrinkFactorsPerLevel.SetSize( 1 );
  shrinkFactorsPerLevel[0] = 1;

  RegistrationAffineType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
  smoothingSigmasPerLevel.SetSize( 1 );
  smoothingSigmasPerLevel[0] = 0;

  registration->SetNumberOfLevels(      params.numberOfLevels       );
  registration->SetSmoothingSigmasPerLevel( smoothingSigmasPerLevel );
  registration->SetShrinkFactorsPerLevel(   shrinkFactorsPerLevel   );

  // Start registration process
  try {
    registration->Update();
    cout << "Optimizer stop condition: "
              << registration->GetOptimizer()->GetStopConditionDescription()
              << endl;
  }
  catch( itk::ExceptionObject & err ){
    cerr << "ExceptionObject caught !" << endl;
    cerr << err << endl;
    exit(1);
  }

  // Resample new image
  ResampleFilterType::Pointer resample = resampleAffinePointer(
                                        fixed,
                                        moving,
                                        transform );

  // Print results
  if ( params.output == 1 ){
    finalAffineParameters(transform, optimizer );
  }

  return transform;

}
