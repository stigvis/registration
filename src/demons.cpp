#include "registration.h"
using namespace std;

class CommandIterationUpdate2 : public itk::Command{
  public:
    typedef  CommandIterationUpdate2                     Self;
    typedef  itk::Command                               Superclass;
    typedef  itk::SmartPointer<CommandIterationUpdate2>  Pointer;
    itkNewMacro( CommandIterationUpdate2 );
  protected:
    CommandIterationUpdate2() {};
  public:
    void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE{
        Execute( (const itk::Object *)caller, event);
    }

    void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE{
         const DemonsFilterType * filter = static_cast< const DemonsFilterType * >( object );
        if( !(itk::IterationEvent().CheckEvent( &event )) ){
          return;
        }
        std::cout << filter->GetMetric() << std::endl;
      }
  };



WarperType::Pointer registration5(
                                      ImageType* const fixed,
                                      ImageType* const moving,
                                      reg_params params ){

  MatchingFilterType::Pointer matcher = MatchingFilterType::New();
  matcher->SetInput( moving );
  matcher->SetReferenceImage( fixed );
  matcher->SetNumberOfHistogramLevels( 2048 );
  matcher->SetNumberOfMatchPoints( 9 );
  matcher->ThresholdAtMeanIntensityOn();
  DemonsFilterType::Pointer filter = DemonsFilterType::New();
  CommandIterationUpdate2::Pointer observer = CommandIterationUpdate2::New();
  filter->AddObserver( itk::IterationEvent(), observer );
  filter->SetFixedImage( fixed );
  filter->SetMovingImage( matcher->GetOutput() );
  filter->SetNumberOfIterations( params.niter );
  filter->SetStandardDeviations( 1.0 );
  filter->Update();
  WarperType::Pointer warper = WarperType::New();
  LinInterpolatorType::Pointer interpolator = LinInterpolatorType::New();

  warper->SetInput( moving );
  warper->SetInterpolator( interpolator );
  warper->SetOutputSpacing( fixed->GetSpacing() );
  warper->SetOutputOrigin( fixed->GetOrigin() );
  warper->SetOutputDirection( fixed->GetDirection() );
  warper->SetDisplacementField( filter->GetOutput() );

  return warper;
}
