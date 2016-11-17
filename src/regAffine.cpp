// =========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"

// ===================================
// Image registration method 3
// ===================================
ResampleFilterType::Pointer registration3(
                                        ImageType* const fixed,
                                        ImageType* const moving ){
  // Initialize parameters (see reg1.cpp for description)
  // TODO: Read from config
  float angle     = 0.0;
  float scale     = 1.0;
  float lrate     = 1.0;
  float slength   = 0.0001;
  int   niter     = 300;

  const unsigned int numberOfLevels = 1;
  const double translationScale = 1.0 / 1000.0;

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
  transform->SetScale( scale );
  transform->SetAngle( angle );

  registration->SetInitialTransform( transform );
  registration->InPlaceOn();

  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );

  optimizerScales[0] =  1.0;
  optimizerScales[1] =  1.0;
  optimizerScales[2] =  1.0;
  optimizerScales[3] =  1.0;
  optimizerScales[4] =  translationScale;
  optimizerScales[5] =  translationScale;

  optimizer->SetScales(   optimizerScales   );
  optimizer->SetLearningRate(     lrate     );
  optimizer->SetMinimumStepLength( slength  );
  optimizer->SetNumberOfIterations( niter   );

  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  RegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
  shrinkFactorsPerLevel.SetSize( 1 );
  shrinkFactorsPerLevel[0] = 1;

  RegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
  smoothingSigmasPerLevel.SetSize( 1 );
  smoothingSigmasPerLevel[0] = 0;

  registration->SetNumberOfLevels(          numberOfLevels          );
  registration->SetSmoothingSigmasPerLevel( smoothingSigmasPerLevel );
  registration->SetShrinkFactorsPerLevel(   shrinkFactorsPerLevel   );

  // Start registration process
  try {
    registration->Update();
    std::cout << "Optimizer stop condition: "
              << registration->GetOptimizer()->GetStopConditionDescription()
              << std::endl;
  }
  catch( itk::ExceptionObject & err ){
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    exit(1);
  }

  // Resample new image
  ResampleFilterType::Pointer resample = resampleAffinePointer(
                                        fixed,
                                        moving,
                                        transform );

  // Print results
  finalAffineParameters(transform, optimizer );

  return resample;

}
