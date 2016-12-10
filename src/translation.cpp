// =========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"
using namespace std;

//  The following section of code implements a Command observer
//  that will monitor the configurations of the registration process
//  at every change of stage and resolution level.
//
template <typename TRegistration>
class RegistrationInterfaceCommand : public itk::Command
{
public:
  typedef  RegistrationInterfaceCommand   Self;
  typedef  itk::Command                   Superclass;
  typedef  itk::SmartPointer<Self>        Pointer;
  itkNewMacro( Self );

protected:
  RegistrationInterfaceCommand() {};

public:
  typedef   TRegistration                          RegistrationType;

  // The Execute function simply calls another version of the \code{Execute()}
  // method accepting a \code{const} input object
  void Execute( itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE
    {
    Execute( (const itk::Object *) object , event );
    }

  void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE
    {
    if( !(itk::MultiResolutionIterationEvent().CheckEvent( &event ) ) )
      {
      return;
      }

    std::cout << "\nObserving from class " << object->GetNameOfClass();
    if (!object->GetObjectName().empty())
      {
      std::cout << " \"" << object->GetObjectName() << "\"" << std::endl;
      }

    const RegistrationType * registration = static_cast<const RegistrationType *>( object );

    unsigned int currentLevel = registration->GetCurrentLevel();
    typename RegistrationType::ShrinkFactorsPerDimensionContainerType shrinkFactors =
                                              registration->GetShrinkFactorsPerDimension( currentLevel );
    typename RegistrationType::SmoothingSigmasArrayType smoothingSigmas =
                                                            registration->GetSmoothingSigmasPerLevel();

    std::cout << "-------------------------------------" << std::endl;
    std::cout << " Current multi-resolution level = " << currentLevel << std::endl;
    std::cout << "    shrink factor = " << shrinkFactors << std::endl;
    std::cout << "    smoothing sigma = " << smoothingSigmas[currentLevel] << std::endl;
    std::cout << std::endl;
    }
};

CompositeTransformType::Pointer translation(
                                ImageType* const fixed,
                                ImageType* const moving ){

  OptimizerType::Pointer        transOptimizer     = OptimizerType::New();
  TMetricType::Pointer          transMetric        = TMetricType::New();
  TRegistrationType::Pointer    transRegistration  = TRegistrationType::New();

  transRegistration->SetOptimizer(     transOptimizer     );
  transRegistration->SetMetric(        transMetric        );

  TTransformType::Pointer   movingInitTx  = TTransformType::New();

  TParametersType initialParameters( movingInitTx->GetNumberOfParameters() );

  // Initial offset in mm along X
  initialParameters[0] = 3.0;
  // Initial offset in mm along Y
  initialParameters[1] = 5.0;

  movingInitTx->SetParameters( initialParameters );

  // Software Guide : BeginCodeSnippet
  transRegistration->SetMovingInitialTransform( movingInitTx );
  CompositeTransformType::Pointer  compositeTransform  =
                                          CompositeTransformType::New();
  compositeTransform->AddTransform( movingInitTx );

  transRegistration->SetFixedImage(         fixed           );
  transRegistration->SetMovingImage(        moving          );
  //transRegistration->SetObjectName("TranslationRegistration");

  const unsigned int numberOfLevels1 = 1;
  TRegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel1;
  shrinkFactorsPerLevel1.SetSize( numberOfLevels1 );
  shrinkFactorsPerLevel1[0] = 3;

  TRegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel1;
  smoothingSigmasPerLevel1.SetSize( numberOfLevels1 );
  smoothingSigmasPerLevel1[0] = 2;

  transRegistration->SetNumberOfLevels ( numberOfLevels1 );
  transRegistration->SetShrinkFactorsPerLevel( shrinkFactorsPerLevel1 );
  transRegistration->SetSmoothingSigmasPerLevel( smoothingSigmasPerLevel1 );

  transMetric->SetNumberOfHistogramBins( 24 );
  transOptimizer->SetNumberOfIterations( 500 );
  transOptimizer->SetRelaxationFactor( 0.5 );

  transOptimizer->SetLearningRate( 8 );
  transOptimizer->SetMinimumStepLength( 1.0 );

  typedef RegistrationInterfaceCommand<TRegistrationType> TranslationCommandType;
  CommandIterationUpdate::Pointer observer1 = CommandIterationUpdate::New();
  transOptimizer->AddObserver( itk::IterationEvent(), observer1 );

  TranslationCommandType::Pointer command1 = TranslationCommandType::New();
  transRegistration->AddObserver( itk::MultiResolutionIterationEvent(), command1 );

  try
    {
    transRegistration->Update();
    cout << "Optimizer stop condition: "
      << transRegistration->GetOptimizer()->GetStopConditionDescription()
      << endl;
    }
  catch( itk::ExceptionObject & err )
    {
    cout << "ExceptionObject caught !" << endl;
    cout << err << endl;
    exit(1);
    }

  compositeTransform->AddTransform(
    transRegistration->GetModifiableTransform() );

  return compositeTransform;

}