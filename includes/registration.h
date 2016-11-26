//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================


#ifndef REGISTRATION_H_DEFINED
#define REGISTRATION_H_DEFINED


#include "itkImage.h"

// Image registration
#include "itkImageRegistrationMethodv4.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"

// Edge detection
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

// Transform
#include "itkAffineTransform.h"
#include "itkCenteredSimilarity2DTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCenteredTransformInitializer.h"

// Image I/O
#include "itkResampleImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkImageMaskSpatialObject.h"

// Image operations
#include "itkMedianImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkSquaredDifferenceImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

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

  void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE;
  void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE;
};

// Instantiation of input images
const   unsigned int    Dimension = 2;
typedef float           PixelType;
typedef unsigned char   CharPixelType;
typedef unsigned short  UintPixelType;

typedef itk::Image< PixelType, Dimension >                  ImageType;
typedef itk::Image< UintPixelType, Dimension >              UintImageType;
typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<
                            ImageType,
                            ImageType >                     GradientFilterType;
typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<
                            UintImageType,
                            UintImageType >                 GradientFilterUintType;
typedef itk::MedianImageFilter<
                            ImageType,
                            ImageType >                     MedianFilterType;
typedef itk::MedianImageFilter<
                            UintImageType,
                            UintImageType >                 MedianFilterUintType;


// Instantiation of transform types
typedef itk::CenteredRigid2DTransform<
                            double >                        TransformRigidType;
typedef itk::CenteredTransformInitializer<
                            TransformRigidType,
                            UintImageType,
                            UintImageType >                 TransformRigidInitializerUintType;
typedef itk::CenteredTransformInitializer<
                            TransformRigidType,
                            ImageType,
                            ImageType >                     TransformRigidInitializerType;
typedef itk::CenteredSimilarity2DTransform<
                            double >                        TransformSimilarityType;
typedef itk::CenteredTransformInitializer<
                            TransformSimilarityType,
                            ImageType,
                            ImageType >                     TransformSimilarityInitializerType;
typedef itk::CenteredTransformInitializer<
                            TransformSimilarityType,
                            UintImageType,
                            UintImageType >                 TransformSimilarityInitializerUintType;
typedef itk::AffineTransform<
                            double,
                            Dimension >                     TransformAffineType;
typedef itk::CenteredTransformInitializer<
                            TransformAffineType,
                            ImageType,
                            ImageType >                     TransformAffineInitializerType;
typedef itk::CenteredTransformInitializer<
                            TransformAffineType,
                            UintImageType,
                            UintImageType >                 TransformAffineInitializerUintType;
typedef itk::CompositeTransform<
                            double,
                            Dimension >                     CompositeTransformType;

typedef itk::RegularStepGradientDescentOptimizerv4<
                            double>                         OptimizerType;
typedef itk::MeanSquaresImageToImageMetricv4<
                            ImageType,
                            ImageType >                     MetricType;
typedef itk::MeanSquaresImageToImageMetricv4<
                            UintImageType,
                            UintImageType >                 MetricUintType;

typedef itk::ImageRegistrationMethodv4<
                            ImageType,
                            ImageType,
                            TransformRigidType >            RegistrationRigidType;
typedef itk::ImageRegistrationMethodv4<
                            UintImageType,
                            UintImageType,
                            TransformRigidType >            RegistrationRigidUintType;
typedef itk::ImageRegistrationMethodv4<
                            ImageType,
                            ImageType,
                            TransformSimilarityType >       RegistrationSimilarityType;
typedef itk::ImageRegistrationMethodv4<
                            UintImageType,
                            UintImageType,
                            TransformSimilarityType >       RegistrationSimilarityUintType;
typedef itk::ImageRegistrationMethodv4<
                            ImageType,
                            ImageType,
                            TransformAffineType >           RegistrationAffineType;
typedef itk::ImageRegistrationMethodv4<
                            UintImageType,
                            UintImageType,
                            TransformAffineType >           RegistrationAffineUintType;
typedef itk::SubtractImageFilter<
                            ImageType,
                            ImageType,
                            ImageType >                     DifferenceFilterType;
typedef itk::SubtractImageFilter<
                            UintImageType,
                            UintImageType,
                            UintImageType >                 DifferenceFilterUintType;
typedef itk::ResampleImageFilter<
                            ImageType,
                            ImageType >                     ResampleFilterType;
typedef itk::ResampleImageFilter<
                            UintImageType,
                            UintImageType >                 ResampleFilterUintType;
typedef itk::ImageMaskSpatialObject<
                            Dimension >                     MaskType;

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



// Set up writer
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
DifferenceFilterType::Pointer diffFilter(
                            ImageType* const moving,
                            ResampleFilterType::Pointer resample );
/*
// Generic handlers, uint
RegistrationRigidType::Pointer registrationRigidContainer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            OptimizerType::Pointer optimizer );
RegistrationSimilarityUintType::Pointer registrationSimilarityUintContainer(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            OptimizerType::Pointer optimizer );
RegistrationAffineUintType::Pointer registrationAffineUintContainer(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            OptimizerType::Pointer optimizer );
RegistrationRigidType::Pointer registrationMaskContainer(
                            ImageType* const fixed,
                            ImageType* const moving,
                            MetricType::Pointer metric,
                            OptimizerType::Pointer optimizer );
TransformRigidInitializerUintType::Pointer initializerRigidUintContainer(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            TransformRigidType::Pointer transform );
TransformSimilarityInitializerUintType::Pointer initializerSimilarityUintContainer(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            TransformSimilarityType::Pointer transform );
TransformAffineInitializerUintType::Pointer initializerAffineUintContainer(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            TransformAffineType::Pointer transform );
ResampleFilterUintType::Pointer resampleRigidUintPointer(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            TransformRigidType::Pointer transform );
ResampleFilterUintType::Pointer resampleSimilarityUintPointer(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            TransformSimilarityType::Pointer transform );
ResampleFilterUintType::Pointer resampleAffineUintPointer(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            TransformAffineType::Pointer transform );
DifferenceFilterUintType::Pointer diffUintFilter(
                            UintImageType* const moving,
                            ResampleFilterType::Pointer resample );
*/
// Image I/O, float
ImageType::Pointer            gradientFilter(
                              ImageType* const fixed,
                              int sigma );
ImageType::Pointer            medianFilter(
                              ImageType* const fixed,
                              int radius );
CastFilterFloatType::Pointer  castFloatImage(
                              UintImageType* const img );

// Image I/O, uint
UintImageType::Pointer        gradientUintFilter(
                              UintImageType* const fixed,
                              int sigma );
UintImageType::Pointer        medianUintFilter(
                              UintImageType* const fixed,
                              int radius );
CastFilterUintType::Pointer   castUintImage(
                              ImageType* const img );

// Printing parameters
void finalRigidParameters( TransformRigidType::Pointer transform,
                      OptimizerType::Pointer optimizer);
void finalSimilarityParameters( TransformSimilarityType::Pointer transform,
                      OptimizerType::Pointer optimizer);
void finalAffineParameters( TransformAffineType::Pointer transform,
                      OptimizerType::Pointer optimizer);

#include "hyperspec.h"
// Image registrations, float
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

// Image registrations, uint
TransformRigidType::Pointer registrationUint1(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            reg_params params );
TransformSimilarityType::Pointer registrationUint2(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            reg_params params );
TransformAffineType::Pointer registrationUint3(
                            UintImageType* const fixed,
                            UintImageType* const moving,
                            reg_params params );
#endif // REGISTRATION_H_DEFINED
