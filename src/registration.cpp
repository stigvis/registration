
#include "itkImageRegistrationMethodv4.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"

#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredTransformInitializer.h"

//#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkSubtractImageFilter.h"


//  The following section of code implements a command observer
//  that will monitor the evolution of the registration process.
//
#include "itkCommand.h"
class CommandIterationUpdate : public itk::Command {
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef  itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );

protected:
  CommandIterationUpdate() {};

public:
  typedef itk::RegularStepGradientDescentOptimizerv4<double>  OptimizerType;
  typedef const OptimizerType *                               OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE
    {
    Execute( (const itk::Object *)caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE
    {
    OptimizerPointer optimizer = static_cast< OptimizerPointer >( object );
    if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }
    std::cout << optimizer->GetCurrentIteration() << "   ";
    std::cout << optimizer->GetValue() << "   ";
    std::cout << optimizer->GetCurrentPosition() << std::endl;
    }
};

// Instantiation of input images
const   unsigned int  Dimension = 2;
typedef float         PixelType;

typedef itk::Image< PixelType, Dimension >  FixedImageType;
typedef itk::Image< PixelType, Dimension >  MovingImageType;

void registration1( FixedImageType* const fixed, MovingImageType* const moving, char argv[] ){

  // Initialize parameters
  // TODO: Read parameters from config
  float angle   = 0.0;                          // Transform angle
  float lrate   = 0.1;                          // Learning rate
  float slength = 0.001;                        // Minimum step length
  int   niter   = 200;                          // Number of iterations

  const unsigned int numberOfLevels = 1;        // 1:1 transform
  const double translationScale = 1.0 / 1000.0; 


  // Instantiation of transform types
  typedef itk::CenteredRigid2DTransform< double >             TransformType;
  typedef itk::RegularStepGradientDescentOptimizerv4<double>  OptimizerType;
  typedef itk::MeanSquaresImageToImageMetricv4<
                              FixedImageType,
                              MovingImageType >               MetricType;
  typedef itk::ImageRegistrationMethodv4<
                              FixedImageType,
                              MovingImageType >               RegistrationType;

  MetricType::Pointer         metric        = MetricType::New();
  OptimizerType::Pointer      optimizer     = OptimizerType::New();
  RegistrationType::Pointer   registration  = RegistrationType::New();

  registration->SetMetric(    metric    );
  registration->SetOptimizer( optimizer );

  // Construction of the transform object
  TransformType::Pointer  transform = TransformType::New();

  //typedef itk::ImageFileReader< FixedImageType  > FixedImageReaderType;
  //typedef itk::ImageFileReader< MovingImageType > MovingImageReaderType;

  //FixedImageReaderType::Pointer   fixedImageReader  = FixedImageReaderType::New();
  //MovingImageReaderType::Pointer  movingImageReader = MovingImageReaderType::New();

  registration->SetFixedImage(  fixed  );
  registration->SetMovingImage( moving );

  // Instantiation of the initializer
  typedef itk::CenteredTransformInitializer<
                              TransformType,
                              FixedImageType,
                              MovingImageType >               TransformInitializerType;

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

  // Set parameters
  transform->SetAngle( angle );

  registration->SetInitialTransform( transform );
  registration->InPlaceOn();

  typedef OptimizerType::ScalesType                           OptimizerScalesType;
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

  typedef itk::ResampleImageFilter<
                              MovingImageType,
                              FixedImageType >      ResampleFilterType;
  ResampleFilterType::Pointer resample = ResampleFilterType::New();

  resample->SetTransform(               transform                 );
  resample->SetInput(                     moving                  );
  resample->SetSize( fixed->GetLargestPossibleRegion().GetSize()  );
  resample->SetOutputOrigin(        fixed->GetOrigin()            );
  resample->SetOutputSpacing(       fixed->GetSpacing()           );
  resample->SetDefaultPixelValue(               0.4               ); // ?

  typedef itk::Image< PixelType, Dimension >        OutputImageType;
  /*
  typedef itk::CastImageFilter<
                              FixedImageType,
                              OutputImageType >     CastFilterType;

  CastFilterType::Pointer     caster =              CastFilterType::New();
  caster->SetInput( resample->GetOutput() );
  */

  // Grafting transform object to the output
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

  // Compute the difference between the images before and after registration
  // We want a visible diff-image
  typedef itk::Image< PixelType, Dimension >        DifferenceImageType;
  typedef itk::SubtractImageFilter<
                              FixedImageType,
                              FixedImageType,
                              DifferenceImageType > DifferenceFilterType;
  typedef itk::Image< PixelType, Dimension >        OutputImageType;
  typedef itk::RescaleIntensityImageFilter<
                              DifferenceImageType,
                              OutputImageType >     RescalerType;
  typedef itk::ImageFileWriter< OutputImageType >   WriterType;

  DifferenceFilterType::Pointer difference  =  DifferenceFilterType::New();
  RescalerType::Pointer intensityRescaler   =  RescalerType::New();

  intensityRescaler->SetOutputMinimum( 0 );
  intensityRescaler->SetOutputMaximum( 1 );

  difference->SetInput1(    fixed   );
  difference->SetInput2(    resample->GetOutput() );

  intensityRescaler->SetInput( difference->GetOutput()  );

  WriterType::Pointer     writerdiff        =  WriterType::New();
  writerdiff->SetInput( intensityRescaler->GetOutput()  );

  // Write the transform

  WriterType::Pointer     writer = WriterType::New();

  writer->SetFileName( &argv[3] );
  writer->SetInput( resample->GetOutput() );
  writer->Update();
};
