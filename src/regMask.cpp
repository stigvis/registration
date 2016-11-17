// =========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"

// ===================================
// Image registration method 4
// ===================================
ResampleFilterType::Pointer registration4(
                                        ImageType* const fixed,
                                        ImageType* const moving,
                                        GradientFilterType::Pointer gradient ){
  // Initialize parameters (see reg1.cpp for description)
  // TODO: Read from config
  float angle     = 0.0;
  float scale     = 1.0;
  float lrate     = 1.0;
  float slength   = 0.0001;
  int   niter     = 200;

  const unsigned int numberOfLevels = 1;
  const double translationScale = 1.0 / 1000.0;

  // Optimizer and Registration containers
  MetricType::Pointer       metric        = MetricType::New();
  OptimizerType::Pointer    optimizer     = OptimizerType::New();
  RegistrationType::Pointer registration  = registrationMaskContainer(
                                        fixed,
                                        moving,
                                        metric,
                                        optimizer );

  // Construction of the transform object
  TransformType::Pointer    transform     = TransformType::New();
  TransformInitializerType::Pointer initializer = initializerContainer(
                                        fixed,
                                        moving,
                                        transform );

  // Set parameters
  //transform->SetScale( scale );
  transform->SetAngle( angle );

  registration->SetMetric(        metric        );
  registration->SetInitialTransform( transform  );
  registration->InPlaceOn();

  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );

  optimizerScales[0] =  1.0;
  optimizerScales[1] =  translationScale;
  optimizerScales[2] =  translationScale;
  optimizerScales[3] =  translationScale;
  optimizerScales[4] =  translationScale;
  optimizerScales[5] =  translationScale;

  optimizer->SetScales(   optimizerScales   );
  optimizer->SetLearningRate(     lrate     );
  optimizer->SetMinimumStepLength( slength  );
  optimizer->SetNumberOfIterations( niter   );

  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  // Apply mask
  MaskType::Pointer    spatialObjectMask  = MaskType::New();

  spatialObjectMask->SetImage( gradient->GetOutput() );
  metric->SetFixedImageMask( spatialObjectMask );

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
  ResampleFilterType::Pointer resample = resamplePointer(
                                        fixed,
                                        moving,
                                        transform );

  // Print results
  finalParameters(transform, optimizer );

  return resample;

}
