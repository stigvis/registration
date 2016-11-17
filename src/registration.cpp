//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"

// Keeping track of the iterations
void CommandIterationUpdate::Execute(itk::Object *caller, const itk::EventObject & event){ // ITK_OVERRIDE{
  Execute( (const itk::Object *)caller, event);
}

void CommandIterationUpdate::Execute(const itk::Object * object, const itk::EventObject & event){ // ITK_OVERRIDE{
  OptimizerPointer optimizer = static_cast< OptimizerPointer >( object );
  if( ! itk::IterationEvent().CheckEvent( &event ) ){
    return;
  }
  std::cout << optimizer->GetCurrentIteration() << "   ";
  std::cout << optimizer->GetValue() << "   ";
  std::cout << optimizer->GetCurrentPosition() << std::endl;
}

// Initialize registration container
RegistrationType::Pointer registrationContainer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      OptimizerType::Pointer optimizer  ){

  MetricType::Pointer         metric        = MetricType::New();
  RegistrationType::Pointer   registration  = RegistrationType::New();

  registration->SetMetric(      metric    );
  registration->SetOptimizer(   optimizer );
  registration->SetFixedImage(    fixed   );
  registration->SetMovingImage(   moving  );
  return registration;
}

// Initialize registration container with Transform2Type
Registration2Type::Pointer registration2Container(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      OptimizerType::Pointer optimizer  ){

  MetricType::Pointer         metric        = MetricType::New();
  Registration2Type::Pointer  registration  = Registration2Type::New();

  registration->SetMetric(      metric    );
  registration->SetOptimizer(   optimizer );
  registration->SetFixedImage(    fixed   );
  registration->SetMovingImage(   moving  );
  return registration;
}

// Initialize registration container with TransformAffineType
RegistrationAffineType::Pointer registrationAffineContainer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      OptimizerType::Pointer optimizer  ){

  MetricType::Pointer         metric        = MetricType::New();
  RegistrationAffineType::Pointer  registration  = RegistrationAffineType::New();

  registration->SetMetric(      metric    );
  registration->SetOptimizer(   optimizer );
  registration->SetFixedImage(    fixed   );
  registration->SetMovingImage(   moving  );
  return registration;
}

// Initialize initializer container with rigid transform
TransformInitializerType::Pointer initializerContainer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      TransformType::Pointer transform ){

  TransformInitializerType::Pointer initializer =
                                      TransformInitializerType::New();

  // Initializer is now connected to the transform and to the fixed and moving images
  initializer->SetTransform(   transform  );
  initializer->SetFixedImage(   fixed     );
  initializer->SetMovingImage(  moving    );

  // Select center of mass mode
  initializer->MomentsOn();

  // Compute the center and translation
  initializer->InitializeTransform();
  return initializer;
}

// Initialize initializer container with similarity transform
Transform2InitializerType::Pointer initializer2Container(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      Transform2Type::Pointer transform ){

  Transform2InitializerType::Pointer initializer =
                                      Transform2InitializerType::New();

  // Initializer is now connected to the transform and to the fixed and moving images
  initializer->SetTransform(   transform  );
  initializer->SetFixedImage(   fixed     );
  initializer->SetMovingImage(  moving    );

  // Select center of mass mode
  initializer->MomentsOn();

  // Compute the center and translation
  initializer->InitializeTransform();
  return initializer;
}

// Initialize initializer container with similarity transform
TransformAffineInitializerType::Pointer initializerAffineContainer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      TransformAffineType::Pointer transform ){

  TransformAffineInitializerType::Pointer initializer =
                                      TransformAffineInitializerType::New();

  // Initializer is now connected to the transform and to the fixed and moving images
  initializer->SetTransform(   transform  );
  initializer->SetFixedImage(   fixed     );
  initializer->SetMovingImage(  moving    );

  // Select center of mass mode
  initializer->MomentsOn();

  // Compute the center and translation
  initializer->InitializeTransform();
  return initializer;
}

// Resample moving image with rigid transform
ResampleFilterType::Pointer resamplePointer(
                                      ImageType* const moving,
                                      ImageType* const fixed,
                                      TransformType::Pointer transform ){
  ResampleFilterType::Pointer resample = ResampleFilterType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize( fixed->GetLargestPossibleRegion().GetSize()  );
  resample->SetOutputOrigin(        fixed->GetOrigin()            );
  resample->SetOutputSpacing(       fixed->GetSpacing()           );
  resample->SetDefaultPixelValue(               0.0               ); // ?
  return resample;
}

// Resample moving image with similarity transform
ResampleFilterType::Pointer resample2Pointer(
                                      ImageType* const moving,
                                      ImageType* const fixed,
                                      Transform2Type::Pointer transform ){
  ResampleFilterType::Pointer resample = ResampleFilterType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize( fixed->GetLargestPossibleRegion().GetSize()  );
  resample->SetOutputOrigin(        fixed->GetOrigin()            );
  resample->SetOutputSpacing(       fixed->GetSpacing()           );
  resample->SetOutputDirection(     fixed->GetDirection()         );
  resample->SetDefaultPixelValue(               0.0               ); // ?
  return resample;
}

// Resample moving image with similarity transform
ResampleFilterType::Pointer resampleAffinePointer(
                                      ImageType* const moving,
                                      ImageType* const fixed,
                                      TransformAffineType::Pointer transform ){
  ResampleFilterType::Pointer resample = ResampleFilterType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize( fixed->GetLargestPossibleRegion().GetSize()  );
  resample->SetOutputOrigin(        fixed->GetOrigin()            );
  resample->SetOutputSpacing(       fixed->GetSpacing()           );
  resample->SetOutputDirection(     fixed->GetDirection()         );
  resample->SetDefaultPixelValue(               0.0               ); // ?
  return resample;
}

// Calculate diff between image before and after registration
RescalerType::Pointer diffFilter(
                                      ImageType* const moving,
                                      ResampleFilterType::Pointer resample ){

  DifferenceFilterType::Pointer difference  =  DifferenceFilterType::New();
  RescalerType::Pointer intensityRescaler   =  RescalerType::New();

  intensityRescaler->SetOutputMinimum( 0 );
  intensityRescaler->SetOutputMaximum( 1 );

  difference->SetInput1(        moving         );
  difference->SetInput2( resample->GetOutput() );

  intensityRescaler->SetInput( difference->GetOutput()  );
  return intensityRescaler;
}

// Print results from rigid transform
void finalParameters( TransformType::Pointer transform,
                      OptimizerType::Pointer optimizer ){
  TransformType::ParametersType finalParameters = transform->GetParameters();

  const double finalAngle           = finalParameters[0];
  const double finalRotationCenterX = finalParameters[1];
  const double finalRotationCenterY = finalParameters[2];
  const double finalTranslationX    = finalParameters[3];
  const double finalTranslationY    = finalParameters[4];

  const unsigned int  numberOfIterations = optimizer->GetCurrentIteration();
  const double        bestValue          = optimizer->GetValue();

  // Print results
  const double finalAngleInDegrees = finalAngle * 180.0 / itk::Math::pi;

  std::cout << "Result = " << std::endl;
  std::cout << " Angle (radians) " << finalAngle  << std::endl;
  std::cout << " Angle (degrees) " << finalAngleInDegrees  << std::endl;
  std::cout << " Center X      = " << finalRotationCenterX  << std::endl;
  std::cout << " Center Y      = " << finalRotationCenterY  << std::endl;
  std::cout << " Translation X = " << finalTranslationX  << std::endl;
  std::cout << " Translation Y = " << finalTranslationY  << std::endl;
  std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Metric value  = " << bestValue          << std::endl;

}

// Print results from similarity transform
void final2Parameters( Transform2Type::Pointer transform,
                      OptimizerType::Pointer optimizer ){
  Transform2Type::ParametersType finalParameters = transform->GetParameters();

  const double finalAngle           = finalParameters[0];
  const double finalRotationCenterX = finalParameters[1];
  const double finalRotationCenterY = finalParameters[2];
  const double finalTranslationX    = finalParameters[3];
  const double finalTranslationY    = finalParameters[4];

  const unsigned int  numberOfIterations = optimizer->GetCurrentIteration();
  const double        bestValue          = optimizer->GetValue();

  // Print results
  const double finalAngleInDegrees = finalAngle * 180.0 / itk::Math::pi;

  std::cout << "Result = " << std::endl;
  std::cout << " Angle (radians) " << finalAngle  << std::endl;
  std::cout << " Angle (degrees) " << finalAngleInDegrees  << std::endl;
  std::cout << " Center X      = " << finalRotationCenterX  << std::endl;
  std::cout << " Center Y      = " << finalRotationCenterY  << std::endl;
  std::cout << " Translation X = " << finalTranslationX  << std::endl;
  std::cout << " Translation Y = " << finalTranslationY  << std::endl;
  std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Metric value  = " << bestValue          << std::endl;

}

// Print results from affine transform
void finalAffineParameters( TransformAffineType::Pointer transform,
                            OptimizerType::Pointer optimizer ){

  const TransformAffineType::ParametersType finalParameters =
              registration->GetOutput()->Get()->GetParameters();

  const double finalRotationCenterX = transform->GetCenter()[0];
  const double finalRotationCenterY = transform->GetCenter()[1];
  const double finalTranslationX    = finalParameters[4];
  const double finalTranslationY    = finalParameters[5];

  const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
  const double bestValue = optimizer->GetValue();

  std::cout << "Result = " << std::endl;
  std::cout << " Center X      = " << finalRotationCenterX  << std::endl;
  std::cout << " Center Y      = " << finalRotationCenterY  << std::endl;
  std::cout << " Translation X = " << finalTranslationX  << std::endl;
  std::cout << " Translation Y = " << finalTranslationY  << std::endl;
  std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Metric value  = " << bestValue          << std::endl;

  //Compute the rotation angle and scaling from SVD of the matrix
  // \todo Find a way to figure out if the scales are along X or along Y.
  // VNL returns the eigenvalues ordered from largest to smallest.

  vnl_matrix<double> p(2, 2);
  p[0][0] = (double) finalParameters[0];
  p[0][1] = (double) finalParameters[1];
  p[1][0] = (double) finalParameters[2];
  p[1][1] = (double) finalParameters[3];
  vnl_svd<double> svd(p);
  vnl_matrix<double> r(2, 2);
  r = svd.U() * vnl_transpose(svd.V());
  double angle = std::asin(r[1][0]);

  const double angleInDegrees = angle * 180.0 / itk::Math::pi;

  std::cout << " Scale 1         = " << svd.W(0)        << std::endl;
  std::cout << " Scale 2         = " << svd.W(1)        << std::endl;
  std::cout << " Angle (degrees) = " << angleInDegrees  << std::endl;

