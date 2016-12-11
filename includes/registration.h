//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#ifndef REGISTRATION_H_DEFINED
#define REGISTRATION_H_DEFINED

// Insight Toolkit
#include "itkImage.h"

// Image registration
#include "itkImageRegistrationMethodv4.h"
#include "itkMattesMutualInformationImageToImageMetricv4.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"

// Deformable image registration
#include "itkBSplineTransform.h"
#include "itkBSplineTransformParametersAdaptor.h"
#include "itkCorrelationImageToImageMetricv4.h"
#include "itkLBFGSOptimizerv4.h"
#include "itkMemoryProbesCollectorBase.h"
#include "itkTimeProbesCollectorBase.h"

// Filtering
#include "itkBinaryThresholdImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

// Transform
#include "itkAffineTransform.h"
#include "itkBSplineTransformInitializer.h"
#include "itkCenteredSimilarity2DTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkCompositeTransform.h"
#include "itkIdentityTransform.h"
#include "itkTransformToDisplacementFieldFilter.h"
#include "itkTranslationTransform.h"

// Image I/O
#include "itkCastImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkImageMaskSpatialObject.h"
#include "itkResampleImageFilter.h"

// Image operations
#include "itkRescaleIntensityImageFilter.h"
#include "itkSquaredDifferenceImageFilter.h"
#include "itkSubtractImageFilter.h"
/*
// Introduce a class that will keep track of the Translation registration
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

  void Execute( itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE;
  void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE;
};

*/

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

  void Execute(itk::Object *caller, const itk::EventObject & event ) ITK_OVERRIDE;
  void Execute(const itk::Object * object, const itk::EventObject & event ) ITK_OVERRIDE;
};

// Instantiation of input images
const   unsigned int    Dimension = 2;
typedef float           PixelType;
typedef unsigned char   CharPixelType;
typedef unsigned short  UintPixelType;

typedef itk::Image< PixelType, Dimension >                  ImageType;
typedef itk::Image< UintPixelType, Dimension >              UintImageType;

// Filters
typedef itk::MedianImageFilter<
                            ImageType,
                            ImageType >                     MedianFilterType;
typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<
                            ImageType,
                            ImageType >                     GradientFilterType;
typedef itk::ShrinkImageFilter<
                            ImageType,
                            ImageType >                     ShrinkFilterType;

// Registration
typedef itk::RegularStepGradientDescentOptimizerv4<
                            double>                         OptimizerType;
typedef itk::LBFGSOptimizerv4                               OptimizerBSplineType;
typedef itk::MeanSquaresImageToImageMetricv4<
                            ImageType,
                            ImageType >                     MetricType;

// Instantiation of transform types

// Translation
typedef itk::RegularStepGradientDescentOptimizerv4<
                            double >                        TOptimizerType;
typedef itk::TranslationTransform<
                            double,
                            Dimension >                     TTransformType;
typedef itk::MattesMutualInformationImageToImageMetricv4<
                            ImageType,
                            ImageType >                     TMetricType;
typedef itk::ImageRegistrationMethodv4<
                            ImageType,
                            ImageType,
                            TTransformType >                TRegistrationType;
typedef OptimizerType::ParametersType                       TParametersType;
typedef itk::CompositeTransform<
                            double,
                            Dimension >                     CompositeTransformType;
//typedef RegistrationInterfaceCommand<
//                            TRegistrationType >             TranslationCommandType;


// Rigid
typedef itk::CenteredRigid2DTransform<
                            double >                        TransformRigidType;
typedef itk::CenteredTransformInitializer<
                            TransformRigidType,
                            ImageType,
                            ImageType >                     TransformRigidInitializerType;
typedef itk::ImageRegistrationMethodv4<
                            ImageType,
                            ImageType,
                            TransformRigidType >            RegistrationRigidType;

// Similarity
typedef itk::CenteredSimilarity2DTransform<
                            double >                        TransformSimilarityType;
typedef itk::CenteredTransformInitializer<
                            TransformSimilarityType,
                            ImageType,
                            ImageType >                     TransformSimilarityInitializerType;
typedef itk::ImageRegistrationMethodv4<
                            ImageType,
                            ImageType,
                            TransformSimilarityType >       RegistrationSimilarityType;

// Affine
typedef itk::AffineTransform<
                            double,
                            Dimension >                     TransformAffineType;
typedef itk::CenteredTransformInitializer<
                            TransformAffineType,
                            ImageType,
                            ImageType >                     TransformAffineInitializerType;
typedef itk::ImageRegistrationMethodv4<
                            ImageType,
                            ImageType,
                            TransformAffineType >           RegistrationAffineType;

// BSpline
const unsigned int SplineOrder = 3;
typedef double CoordinateRepType;
typedef itk::BSplineTransform<
                            CoordinateRepType,
                            Dimension,
                            SplineOrder >                   TransformBSplineType;
typedef itk::ImageRegistrationMethodv4<
                            ImageType,
                            ImageType >                     RegistrationBSplineType;
typedef itk::BSplineTransformInitializer<
                            TransformBSplineType,
                            ImageType >                     InitializerBSplineType;
typedef TransformBSplineType::ParametersType                ParametersBSplineType;
typedef itk::BSplineTransformParametersAdaptor<
                            TransformBSplineType >          BSplineAdaptorType;
typedef itk::RegistrationParameterScalesFromPhysicalShift<
                            MetricType >                    ScalesEstimatorType;


// Image casting, because registrations only supports float
typedef itk::CastImageFilter<
                            UintImageType,
                            ImageType >                     CastFilterFloatType;
typedef itk::CastImageFilter<
                            ImageType,
                            UintImageType >                 CastFilterUintType;
typedef itk::RescaleIntensityImageFilter<
                            UintImageType,
                            UintImageType >                 RescalerUintType;
typedef itk::RescaleIntensityImageFilter<
                            ImageType,
                            ImageType >                     RescalerFloatType;



// Set up outputs and writers
typedef itk::SubtractImageFilter<
                            ImageType,
                            ImageType,
                            ImageType >                     DifferenceFilterType;
typedef itk::ResampleImageFilter<
                            ImageType,
                            ImageType >                     ResampleFilterType;
typedef itk::ImageFileWriter<
                            ImageType >                     WriterType;
typedef itk::ImageFileWriter<
                            UintImageType >                 UintWriterType;

// Set up optimizer
typedef OptimizerType::ScalesType                           OptimizerScalesType;

// Generic handlers, float
RegistrationRigidType::Pointer registrationRigidContainer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            OptimizerType::Pointer optimizer );
RegistrationSimilarityType::Pointer registrationSimilarityContainer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            OptimizerType::Pointer optimizer );
RegistrationAffineType::Pointer registrationAffineContainer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            OptimizerType::Pointer optimizer );
TransformRigidInitializerType::Pointer initializerRigidContainer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            TransformRigidType::Pointer transform );
TransformSimilarityInitializerType::Pointer initializerSimilarityContainer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            TransformSimilarityType::Pointer transform );
TransformAffineInitializerType::Pointer initializerAffineContainer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            TransformAffineType::Pointer transform );
ResampleFilterType::Pointer resampleRigidPointer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            TransformRigidType::Pointer transform );
ResampleFilterType::Pointer resampleSimilarityPointer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            TransformSimilarityType::Pointer transform );
ResampleFilterType::Pointer resampleAffinePointer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            TransformAffineType::Pointer transform );
ResampleFilterType::Pointer resampleBSplinePointer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            TransformBSplineType::Pointer transform );
DifferenceFilterType::Pointer diffFilter(
                            ImageType* const moving,
                            ResampleFilterType::Pointer resample );

// Image filtering
ImageType::Pointer            gradientFilter(
                              ImageType* const fixed,
                              int sigma );
ImageType::Pointer            medianFilter(
                              ImageType* const fixed,
                              int radius );

// Image I/O
CastFilterFloatType::Pointer  castFloatImage(
                              UintImageType* const img );
CastFilterUintType::Pointer   castUintImage(
                              ImageType* const img );

// Printing parameters
void finalRigidParameters( TransformRigidType::Pointer transform,
                      OptimizerType::Pointer optimizer);
void finalSimilarityParameters( TransformSimilarityType::Pointer transform,
                      OptimizerType::Pointer optimizer);
void finalAffineParameters( TransformAffineType::Pointer transform,
                      OptimizerType::Pointer optimizer);

// Image registrations
#include "hyperspec.h"
TransformRigidType::Pointer registration1(
                            ImageType* const fixed,
                            ImageType* const moving,
                            reg_params params );
TransformSimilarityType::Pointer registration2(
                            ImageType* const fixed,
                            ImageType* const moving,
                            reg_params params );
TransformAffineType::Pointer registration3(
                            ImageType* const fixed,
                            ImageType* const moving,
                            reg_params params );
TransformBSplineType::Pointer registration4(
                            ImageType* const fixed,
                            ImageType* const moving,
                            reg_params params );
CompositeTransformType::Pointer translation(
                            ImageType* const fixed,
                            ImageType* const moving,
                            reg_params params );

#endif // REGISTRATION_H_DEFINED
