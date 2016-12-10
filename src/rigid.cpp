// =========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"
using namespace std;

// ===================================
// Image registration method 1
// ===================================
TransformRigidType::Pointer registration1(
                                        ImageType* const fixed,
                                        ImageType* const moving,
                                        reg_params params ){

  // Optimizer and Registration containers
  OptimizerType::Pointer          optimizer     = OptimizerType::New();
  RegistrationRigidType::Pointer  registration  = registrationRigidContainer(
                                        fixed,
                                        moving,
                                        optimizer );

  // Construction of the transform object
  TransformRigidType::Pointer     transform     = TransformRigidType::New();
  TransformRigidInitializerType::Pointer initializer = initializerRigidContainer(
                                        fixed,
                                        moving,
                                        transform );

  // Set parameters
  transform->SetAngle( params.angle );

  if ( params.translation == 1) {
    CompositeTransformType::Pointer ttransform = translation(
                                        fixed,
                                        moving );
    registration->SetInitialTransform( ttransform );
  } else {
    registration->SetInitialTransform( transform );
  }
  registration->InPlaceOn();

  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );

  optimizerScales[0] = 1.0;
  optimizerScales[1] = params.translationScale;
  optimizerScales[2] = params.translationScale;
  optimizerScales[3] = params.translationScale;
  optimizerScales[4] = params.translationScale;

  optimizer->SetScales(       optimizerScales     );
  optimizer->SetLearningRate(     params.lrate    );
  optimizer->SetMinimumStepLength( params.slength );
  optimizer->SetNumberOfIterations( params.niter  );

  // Create the command observer and register it with the optimizer
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  // Optional: Shrinking and/or smoothing, set to 0
  RegistrationRigidType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
  shrinkFactorsPerLevel.SetSize( 1 );
  shrinkFactorsPerLevel[0] = 1;

  RegistrationRigidType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
  smoothingSigmasPerLevel.SetSize ( 1 );
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

  // Get final transform
  TransformRigidType::Pointer finalTransform = TransformRigidType::New();
  finalTransform->SetParameters( transform->GetParameters() );
  finalTransform->SetFixedParameters( transform->GetFixedParameters() );

  // Print results
  if ( params.output == 1 ){
    finalRigidParameters( transform, optimizer );
  }

  return finalTransform;
};
