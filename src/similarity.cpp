// =========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"

// =================================================
// Image registration method 2, similarity transform
// =================================================
TransformSimilarityType::Pointer registration2(
                                        ImageType* const fixed,
                                        ImageType* const moving,
                                        reg_params params ){

  // Optimizer and Registration containers
  OptimizerType::Pointer    optimizer     = OptimizerType::New();
  RegistrationSimilarityType::Pointer registration  = registrationSimilarityContainer(
                                        fixed,
                                        moving,
                                        optimizer );

  // Construction of the transform object
  TransformSimilarityType::Pointer    transform     = TransformSimilarityType::New();
  TransformSimilarityInitializerType::Pointer initializer = initializerSimilarityContainer(
                                        fixed,
                                        moving,
                                        transform );

  // Set parameters
  transform->SetScale( params.scale );
  transform->SetAngle( params.angle );

  registration->SetInitialTransform( transform );
  registration->InPlaceOn();

  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );

  optimizerScales[0] = 10.0;
  optimizerScales[1] =  1.0;
  optimizerScales[2] =  params.translationScale;
  optimizerScales[3] =  params.translationScale;
  optimizerScales[4] =  params.translationScale;
  optimizerScales[5] =  params.translationScale;

  optimizer->SetScales(       optimizerScales     );
  optimizer->SetLearningRate(     params.lrate    );
  optimizer->SetMinimumStepLength( params.slength );
  optimizer->SetNumberOfIterations( params.niter  );

  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  RegistrationSimilarityType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
  shrinkFactorsPerLevel.SetSize( 1 );
  shrinkFactorsPerLevel[0] = 1;

  RegistrationSimilarityType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
  smoothingSigmasPerLevel.SetSize( 1 );
  smoothingSigmasPerLevel[0] = 0;

  registration->SetNumberOfLevels(          params.numberOfLevels   );
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
  ResampleFilterType::Pointer resample = resampleSimilarityPointer(
                                        fixed,
                                        moving,
                                        transform );

  // Print results
  if ( params.output == 1 ){
    finalSimilarityParameters(transform, optimizer );
  }

  return transform;

}
