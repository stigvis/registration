// =========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"
using namespace std;

template <typename TRegistration>
class RegistrationInterfaceCommand : public itk::Command{
public:
  typedef  RegistrationInterfaceCommand   Self;
  typedef  itk::Command                   Superclass;
  typedef  itk::SmartPointer<Self>        Pointer;
  itkNewMacro( Self );

protected:
  RegistrationInterfaceCommand() {};

public:
  typedef   TRegistration                          RegistrationType;

  void Execute( itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE{
    Execute( (const itk::Object *) object , event );
  }

  void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE{
    if( !(itk::MultiResolutionIterationEvent().CheckEvent( &event ) ) ){
      return;
    }

    cout << "\nObserving from class " << object->GetNameOfClass();
    if (!object->GetObjectName().empty()){
      cout << " \"" << object->GetObjectName() << "\"" << endl;
    }

    const RegistrationType * registration = static_cast<const RegistrationType *>( object );

    unsigned int currentLevel = registration->GetCurrentLevel();
    typename RegistrationType::ShrinkFactorsPerDimensionContainerType shrinkFactors =
                                              registration->GetShrinkFactorsPerDimension( currentLevel );
    typename RegistrationType::SmoothingSigmasArrayType smoothingSigmas =
                                                            registration->GetSmoothingSigmasPerLevel();

    cout << "-------------------------------------" << endl;
    cout << " Current multi-resolution level = " << currentLevel << endl;
    cout << "    shrink factor = " << shrinkFactors << endl;
    cout << "    smoothing sigma = " << smoothingSigmas[currentLevel] << endl;
  }
};

CompositeTransformType::Pointer translation(
                                ImageType* const fixed,
                                ImageType* const moving,
                                reg_params params ){

  TOptimizerType::Pointer       transOptimizer     = TOptimizerType::New();
  TRegistrationType::Pointer    transRegistration  = TRegistrationType::New();

  transRegistration->SetOptimizer(     transOptimizer     );

  if ( params.metric == 1 ){
    TMetricType::Pointer                  transMetric       =
                                          TMetricType::New();
    transRegistration->SetMetric(         transMetric       );
    transMetric->SetNumberOfHistogramBins( 24 );
  } else {
    MetricType::Pointer                   transMetric       =
                                          MetricType::New();
    transRegistration->SetMetric(         transMetric       );
  }

  TTransformType::Pointer   movingInitTx  = TTransformType::New();

  TParametersType initialParameters( movingInitTx->GetNumberOfParameters() );

  initialParameters[0] = 0.0;
  initialParameters[1] = 0.0;

  movingInitTx->SetParameters( initialParameters );

  transRegistration->SetMovingInitialTransform( movingInitTx );
  CompositeTransformType::Pointer  compositeTransform  =
                                          CompositeTransformType::New();
  compositeTransform->AddTransform( movingInitTx );

  transRegistration->SetFixedImage(         fixed           );
  transRegistration->SetMovingImage(        moving          );

  const unsigned int numberOfLevels1 = 1;
  TRegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel1;
  shrinkFactorsPerLevel1.SetSize( numberOfLevels1 );
  shrinkFactorsPerLevel1[0] = 1;

  TRegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel1;
  smoothingSigmasPerLevel1.SetSize( numberOfLevels1 );
  smoothingSigmasPerLevel1[0] = 1;

  transRegistration->SetNumberOfLevels ( numberOfLevels1 );
  transRegistration->SetShrinkFactorsPerLevel( shrinkFactorsPerLevel1 );
  transRegistration->SetSmoothingSigmasPerLevel( smoothingSigmasPerLevel1 );

  transOptimizer->SetNumberOfIterations( params.niter );
  // Relaxation, for speed and coarse pre-registration
  transOptimizer->SetRelaxationFactor( 0.1 );

  transOptimizer->SetLearningRate( params.lrate );
  transOptimizer->SetMinimumStepLength( params.slength );

  typedef RegistrationInterfaceCommand<TRegistrationType> TranslationCommandType;
  CommandIterationUpdate::Pointer observer1 = CommandIterationUpdate::New();
  transOptimizer->AddObserver( itk::IterationEvent(), observer1 );

  TranslationCommandType::Pointer command1 = TranslationCommandType::New();
  transRegistration->AddObserver( itk::MultiResolutionIterationEvent(), command1 );

  try{
    transRegistration->Update();
    cout  << "Optimizer stop condition: "
          << transRegistration->GetOptimizer()->GetStopConditionDescription()
          << endl;
  } catch( itk::ExceptionObject & err ){
    cout << "ExceptionObject caught !" << endl;
    cout << err << endl;
    exit(1);
  }

  compositeTransform->AddTransform(
            transRegistration->GetModifiableTransform() );

  cout << "\nInitial parameters of the registration process:"   << endl
       << movingInitTx->GetParameters() << endl;

  cout << "\nTranslation parameters after registration:"        << endl
       << transOptimizer->GetCurrentPosition()                  << endl
       << " Last LearningRate: "
       << transOptimizer->GetCurrentStepLength()                << endl;

  return compositeTransform;

}
