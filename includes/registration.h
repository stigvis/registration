//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#ifndef REGISTRATION_H_DEFINED
#define REGISTRATION_H_DEFINED

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkImageRegistrationMethodv4.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"
#include "itkCenteredSimilarity2DTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredTransformInitializer.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkIdentityTransform.h"

// Introduce a class that will keep track of the iterations
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

typedef itk::Image< PixelType, Dimension >                  ImageType;
typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<
                            ImageType,
                            ImageType >                     GradientFilterType;


// Instantiation of transform types
typedef itk::CenteredRigid2DTransform< double >             TransformType;
typedef itk::CenteredTransformInitializer<
                            TransformType,
                            ImageType,
                            ImageType >                     TransformInitializerType;
typedef itk::RegularStepGradientDescentOptimizerv4<double>  OptimizerType;
typedef itk::MeanSquaresImageToImageMetricv4<
                            ImageType,
                            ImageType >                     MetricType;

typedef itk::ImageRegistrationMethodv4<
                            ImageType,
                            ImageType >                     RegistrationType;

typedef OptimizerType::ScalesType                           OptimizerScalesType;
typedef itk::SubtractImageFilter<
                            ImageType,
                            ImageType,
                            ImageType >                     DifferenceFilterType;
typedef itk::RescaleIntensityImageFilter<
                            ImageType,
                            ImageType >                     RescalerType;
typedef itk::ResampleImageFilter<
                            ImageType,
                            ImageType >                     ResampleFilterType;
// Set up writer
typedef itk::ImageFileWriter< ImageType >                   WriterType;

// Image registration, type 1
void registration1( ImageType* const fixed, ImageType* const moving, char argv[] );

#endif // REGISTRATION_H_DEFINED
