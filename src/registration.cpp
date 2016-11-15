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

// Initialize initializer container
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

// Resample moving image with transform
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
  resample->SetDefaultPixelValue(               0.4               ); // ?
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

// Print results
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

//-------------------------------------------------------------------------------------------
// Bilderegistreringsmetode numero uno
//-------------------------------------------------------------------------------------------
ResampleFilterType::Pointer registration1( ImageType* const fixed, ImageType* const moving ){

  // Initialize parameters
  // TODO: Read parameters from config
  float angle   = 0.0;                          // Transform angle
  float lrate   = 3;                          // Learning rate
  float slength = 1;                        // Minimum step length
  int   niter   = 200;                          // Number of iterations

  const unsigned int numberOfLevels = 1;        // 1:1 transform
  const double translationScale = 1.0 / 1000.0;

  // Optimizer and Registration containers
  OptimizerType::Pointer      optimizer     = OptimizerType::New();
  RegistrationType::Pointer registration = registrationContainer(
                                        fixed,
                                        moving,
                                        optimizer );

  // Construction of the transform object
  TransformType::Pointer  transform = TransformType::New();
  TransformInitializerType::Pointer initializer = initializerContainer(
                                        fixed,
                                        moving,
                                        transform );

  // Set parameters
  transform->SetAngle( angle );

  registration->SetInitialTransform( transform );
  registration->InPlaceOn();

  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );

  optimizerScales[0] = 1.0;
  optimizerScales[1] = translationScale;
  optimizerScales[2] = translationScale;
  optimizerScales[3] = translationScale;
  optimizerScales[4] = translationScale;

  optimizer->SetScales(   optimizerScales  );
  optimizer->SetLearningRate(     lrate    );
  optimizer->SetMinimumStepLength( slength );
  optimizer->SetNumberOfIterations( niter  );

  // Create the command observer and register it with the optimizer
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  // Optional: Shrinking and/or smoothing, set to 0
  RegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
  shrinkFactorsPerLevel.SetSize( 1 );
  shrinkFactorsPerLevel[0] = 1;

  RegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
  smoothingSigmasPerLevel.SetSize ( 1 );
  smoothingSigmasPerLevel[0] = 0;

  registration->SetNumberOfLevels(          numberOfLevels          );
  registration->SetSmoothingSigmasPerLevel( smoothingSigmasPerLevel );
  registration->SetShrinkFactorsPerLevel(   shrinkFactorsPerLevel   );

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
  finalParameters( transform, optimizer );


  // Compute the difference between the images before and after registration
  // We want a visible diff-image
  RescalerType::Pointer difference = diffFilter(
                                        moving,
                                        resample );
/*
  WriterType::Pointer     writerdiff        =  WriterType::New();
  writerdiff->SetInput( difference->GetOutput()  );

  // Write the transform

  WriterType::Pointer     writer = WriterType::New();

  writer->SetFileName( &argv[3] );
  writer->SetInput( resample->GetOutput() );
  writer->Update();
*/
  return resample;
};
