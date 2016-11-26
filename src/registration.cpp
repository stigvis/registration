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

// ==========================
// Functions for float images
// ==========================

// Median filter
ImageType::Pointer medianFilter( ImageType* const fixed, int radius ){
  MedianFilterType::Pointer median = MedianFilterType::New();
  ImageType::SizeType rad;

  rad[0] = radius;
  rad[1] = radius;

  median->SetRadius( 	rad );
  median->SetInput( fixed );
  median->Update();

  return median->GetOutput();
}

// Gradient filter
ImageType::Pointer gradientFilter( ImageType* const fixed, int sigma ){
  GradientFilterType::Pointer gradient = GradientFilterType::New();

  gradient->SetSigma( sigma );
  gradient->SetInput( fixed );
  gradient->Update();

  return gradient->GetOutput();
}

// Initialize registration container
RegistrationRigidType::Pointer registrationRigidContainer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      OptimizerType::Pointer optimizer  ){

  MetricType::Pointer               metric        = MetricType::New();
  RegistrationRigidType::Pointer    registration  = RegistrationRigidType::New();

  registration->SetMetric(      metric    );
  registration->SetOptimizer(   optimizer );
  registration->SetFixedImage(    fixed   );
  registration->SetMovingImage(   moving  );
  return registration;
}

// Initialize registration container with Transform2Type
RegistrationSimilarityType::Pointer registrationSimilarityContainer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      OptimizerType::Pointer optimizer  ){

  MetricType::Pointer                   metric        =
                                      MetricType::New();
  RegistrationSimilarityType::Pointer   registration  =
                                      RegistrationSimilarityType::New();

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
TransformRigidInitializerType::Pointer initializerRigidContainer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      TransformRigidType::Pointer transform ){

  TransformRigidInitializerType::Pointer initializer =
                                      TransformRigidInitializerType::New();

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
TransformSimilarityInitializerType::Pointer initializerSimilarityContainer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      TransformSimilarityType::Pointer transform ){

  TransformSimilarityInitializerType::Pointer initializer =
                                      TransformSimilarityInitializerType::New();

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
ResampleFilterType::Pointer resampleRigidPointer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      TransformRigidType::Pointer transform ){
  ResampleFilterType::Pointer resample = ResampleFilterType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize(  fixed->GetLargestPossibleRegion().GetSize() );
  resample->SetOutputOrigin(         fixed->GetOrigin()           );
  resample->SetOutputSpacing(        fixed->GetSpacing()          );
  resample->SetDefaultPixelValue(               0.0               );
  return resample;
}

// Resample moving image with similarity transform
ResampleFilterType::Pointer resampleSimilarityPointer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      TransformSimilarityType::Pointer transform ){
  ResampleFilterType::Pointer resample = ResampleFilterType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize( fixed->GetLargestPossibleRegion().GetSize()  );
  resample->SetOutputOrigin(        fixed->GetOrigin()            );
  resample->SetOutputSpacing(       fixed->GetSpacing()           );
  resample->SetOutputDirection(     fixed->GetDirection()         );
  resample->SetDefaultPixelValue(               0.0               );
  resample->Update();
  return resample;
}

// Resample moving image with similarity transform
ResampleFilterType::Pointer resampleAffinePointer(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      TransformAffineType::Pointer transform ){
  ResampleFilterType::Pointer resample = ResampleFilterType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize( fixed->GetLargestPossibleRegion().GetSize()  );
  resample->SetOutputOrigin(        fixed->GetOrigin()            );
  resample->SetOutputSpacing(       fixed->GetSpacing()           );
  resample->SetOutputDirection(     fixed->GetDirection()         );
  resample->SetDefaultPixelValue(               0.0               );
  resample->Update();
  return resample;
}

// Calculate diff between image before and after registration
DifferenceFilterType::Pointer diffFilter(
                                      ImageType* const moving,
                                      ResampleFilterType::Pointer resample ){

  DifferenceFilterType::Pointer difference  =  DifferenceFilterType::New();

  difference->SetInput1(        moving         );
  difference->SetInput2( resample->GetOutput() );

  return difference;
}

// Cast unsigned short to float
CastFilterFloatType::Pointer castFloatImage( UintImageType* const img ){
/*
  RescalerUintType::Pointer rescale = RescalerUintType::New();

  rescale->SetInput( img );
  rescale->SetOutputMinimum( 0 );
  rescale->SetOutputMaximum( 65535 );
*/

  CastFilterFloatType::Pointer castFilter = CastFilterFloatType::New();
  castFilter->SetInput( img );
	castFilter->Update();

  return castFilter;
}

// =========================
// Functions for uint images
// =========================

// Cast float to unsigned short
CastFilterUintType::Pointer castUintImage( ImageType* const img ){
/*
  RescalerFloatType::Pointer rescale = RescalerFloatType::New();

  rescale->SetInput( img );
  rescale->SetOutputMinimum( 0 );
  rescale->SetOutputMaximum( 1 );
*/
  CastFilterUintType::Pointer castFilter = CastFilterUintType::New();
  castFilter->SetInput( img );
  castFilter->Update();

  return castFilter;
}

/*
// Median filter
UintImageType::Pointer medianUintFilter(  UintImageType* const fixed,
                                          int radius ){
  MedianFilterUintType::Pointer median = MedianFilterUintType::New();
  UintImageType::SizeType rad;

  rad[0] = radius;
  rad[1] = radius;

  median->SetRadius( 	rad );
  median->SetInput( fixed );

  return median->GetOutput();
}

// Gradient filter
UintImageType::Pointer gradientUintFilter(  UintImageType* const fixed,
                                        int sigma ){
  GradientFilterUintType::Pointer gradient = GradientFilterUintType::New();

  gradient->SetSigma( sigma );
  gradient->SetInput( fixed );
  gradient->Update();

  return gradient->GetOutput();
}

// Initialize registration container
RegistrationRigidUintType::Pointer registrationRigidUintContainer(
                                      UintImageType* const fixed,
                                      UintImageType* const moving,
                                      OptimizerType::Pointer optimizer  ){

  MetricUintType::Pointer             metric        = MetricUintType::New();
  RegistrationRigidUintType::Pointer  registration  = RegistrationRigidUintType::New();

  registration->SetMetric(      metric    );
  registration->SetOptimizer(   optimizer );
  registration->SetFixedImage(    fixed   );
  registration->SetMovingImage(   moving  );
  return registration;
}

// Initialize registration container with Transform2Type
RegistrationSimilarityUintType::Pointer registrationSimilarityUintContainer(
                                      UintImageType* const fixed,
                                      UintImageType* const moving,
                                      OptimizerType::Pointer optimizer  ){

  MetricUintType::Pointer                   metric        = MetricUintType::New();
  RegistrationSimilarityUintType::Pointer   registration  = RegistrationSimilarityUintType::New();

  registration->SetMetric(      metric    );
  registration->SetOptimizer(   optimizer );
  registration->SetFixedImage(    fixed   );
  registration->SetMovingImage(   moving  );
  return registration;
}

// Initialize registration container with TransformAffineType
RegistrationAffineUintType::Pointer registrationAffineUintContainer(
                                      UintImageType* const fixed,
                                      UintImageType* const moving,
                                      OptimizerType::Pointer optimizer  ){

  MetricUintType::Pointer               metric        = MetricUintType::New();
  RegistrationAffineUintType::Pointer   registration  = RegistrationAffineUintType::New();

  registration->SetMetric(      metric    );
  registration->SetOptimizer(   optimizer );
  registration->SetFixedImage(    fixed   );
  registration->SetMovingImage(   moving  );
  return registration;
}

// Initialize initializer container with rigid transform
TransformRigidInitializerUintType::Pointer initializerRigidUintContainer(
                                      UintImageType* const fixed,
                                      UintImageType* const moving,
                                      TransformRigidType::Pointer transform ){

  TransformRigidInitializerUintType::Pointer initializer =
                                      TransformRigidInitializerUintType::New();

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
TransformSimilarityInitializerUintType::Pointer initializerSimilarityUintContainer(
                                      UintImageType* const fixed,
                                      UintImageType* const moving,
                                      TransformSimilarityType::Pointer transform ){

  TransformSimilarityInitializerUintType::Pointer initializer =
                                      TransformSimilarityInitializerUintType::New();

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
TransformAffineInitializerUintType::Pointer initializerAffineUintContainer(
                                      UintImageType* const fixed,
                                      UintImageType* const moving,
                                      TransformAffineType::Pointer transform ){

  TransformAffineInitializerUintType::Pointer initializer =
                                      TransformAffineInitializerUintType::New();

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
ResampleFilterUintType::Pointer resampleRigidUintPointer(
                                      UintImageType* const fixed,
                                      UintImageType* const moving,
                                      TransformRigidType::Pointer transform ){
  ResampleFilterUintType::Pointer resample = ResampleFilterUintType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize(  fixed->GetLargestPossibleRegion().GetSize() );
  resample->SetOutputOrigin(         fixed->GetOrigin()           );
  resample->SetOutputSpacing(        fixed->GetSpacing()          );
  resample->SetDefaultPixelValue(               0.0               );
  return resample;
}

// Resample moving image with similarity transform
ResampleFilterUintType::Pointer resampleSimilarityUintPointer(
                                      UintImageType* const fixed,
                                      UintImageType* const moving,
                                      TransformSimilarityType::Pointer transform ){
  ResampleFilterUintType::Pointer resample = ResampleFilterUintType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize( fixed->GetLargestPossibleRegion().GetSize()  );
  resample->SetOutputOrigin(        fixed->GetOrigin()            );
  resample->SetOutputSpacing(       fixed->GetSpacing()           );
  resample->SetOutputDirection(     fixed->GetDirection()         );
  resample->SetDefaultPixelValue(               0.0               );
  resample->Update();
  return resample;
}

// Resample moving image with similarity transform
ResampleFilterUintType::Pointer resampleAffineUintPointer(
                                      UintImageType* const fixed,
                                      UintImageType* const moving,
                                      TransformAffineType::Pointer transform ){
  ResampleFilterUintType::Pointer resample = ResampleFilterUintType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize( fixed->GetLargestPossibleRegion().GetSize()  );
  resample->SetOutputOrigin(        fixed->GetOrigin()            );
  resample->SetOutputSpacing(       fixed->GetSpacing()           );
  resample->SetOutputDirection(     fixed->GetDirection()         );
  resample->SetDefaultPixelValue(               0.0               );
  resample->Update();
  return resample;
}

// Calculate diff between image before and after registration
DifferenceFilterUintType::Pointer diffUintFilter(
                                      UintImageType* const moving,
                                      ResampleFilterUintType::Pointer resample ){

  DifferenceFilterUintType::Pointer difference  =  DifferenceFilterUintType::New();

  difference->SetInput1(        moving         );
  difference->SetInput2( resample->GetOutput() );

  return difference;
}
*/
// =============================
// Print outputs from transforms
// =============================


// Print results from rigid transform
void finalRigidParameters(
                                      TransformRigidType::Pointer transform,
                                      OptimizerType::Pointer optimizer ){
  TransformRigidType::ParametersType finalParameters = transform->GetParameters();

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
void finalSimilarityParameters( TransformSimilarityType::Pointer transform,
                      OptimizerType::Pointer optimizer ){
  TransformSimilarityType::ParametersType finalParameters = transform->GetParameters();

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

  const TransformAffineType::ParametersType finalParameters = transform->GetParameters();

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
}
