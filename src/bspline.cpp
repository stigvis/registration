//==========================================================================
// Copyright 2016 Stig Viste, Norwegian University of Science and Technology
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT
// =========================================================================

#include "registration.h"
using namespace std;

// =====================
// Registration method 4
// =====================

TransformBSplineType::Pointer registration4(  ImageType* const fixed,
                                              ImageType* const moving,
                                              reg_params params){

  MetricType::Pointer                 metric        = MetricType::New();
  OptimizerBSplineType::Pointer       optimizer     = OptimizerBSplineType::New();
  RegistrationBSplineType::Pointer    registration  = RegistrationBSplineType::New();

  registration->SetMetric(        metric    );
  registration->SetOptimizer(     optimizer );

  TransformBSplineType::Pointer  transform              = TransformBSplineType::New();

  // Initialize the fixed parameters of transform
  InitializerBSplineType::Pointer transformInitializer  = InitializerBSplineType::New();

  unsigned int numberOfGridNodesInOneDimension = 8;

  TransformBSplineType::MeshSizeType                  meshSize;
  meshSize.Fill( numberOfGridNodesInOneDimension - SplineOrder );

  transformInitializer->SetTransform(         transform       );
  transformInitializer->SetImage(             fixed           );
  transformInitializer->SetTransformDomainMeshSize( meshSize  );
  transformInitializer->InitializeTransform();

  transform->SetIdentity();

  if (params.translation == 1 ){
    CompositeTransformType::Pointer ttransform = translation(
                                        fixed,
                                        moving,
                                        params );
    registration->SetMovingInitialTransform( ttransform );
  } else {
    registration->SetInitialTransform( transform );
  }
  registration->InPlaceOn();
  registration->SetFixedImage(    fixed     );
  registration->SetMovingImage(   moving    );

  RegistrationBSplineType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
  shrinkFactorsPerLevel.SetSize( params.numberOfLevels );
  shrinkFactorsPerLevel[0] = 3;
  shrinkFactorsPerLevel[1] = 2;
  shrinkFactorsPerLevel[2] = 1;

  RegistrationBSplineType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
  smoothingSigmasPerLevel.SetSize( params.numberOfLevels );
  smoothingSigmasPerLevel[0] = 2;
  smoothingSigmasPerLevel[1] = 1;
  smoothingSigmasPerLevel[2] = 0;
/*
  RegistrationBSplineType::TransformParametersAdaptorsContainerType adaptors;
	// First, get fixed image physical dimensions
  TransformBSplineType::PhysicalDimensionsType             PhysDim;
  for( unsigned int i=0; i< Dimension; i++ ){
    PhysDim[i] = fixed->GetSpacing()[i] *
    static_cast<double>(
      fixed->GetLargestPossibleRegion().GetSize()[i] - 1 );
  }

  // Create the transform adaptors specific to B-splines
  for( unsigned int level = 0; level < params.numberOfLevels; level++ ){
    ShrinkFilterType::Pointer shrinkFilter = ShrinkFilterType::New();
    shrinkFilter->SetShrinkFactors( shrinkFactorsPerLevel[level] );
    shrinkFilter->SetInput( fixed );
    shrinkFilter->Update();
    // A good heuristic is to double the b-spline mesh resolution at each level
    //
    //TransformType::MeshSizeType requiredMeshSize;
    for( unsigned int d = 0; d < Dimension; d++ ){
      meshSize[d] = meshSize[d] << level;
    }
    BSplineAdaptorType::Pointer bsplineAdaptor = BSplineAdaptorType::New();
    bsplineAdaptor->SetTransform( transform );
    bsplineAdaptor->SetRequiredTransformDomainMeshSize( meshSize );
    bsplineAdaptor->SetRequiredTransformDomainOrigin(
      															shrinkFilter->GetOutput()->GetOrigin() );
    bsplineAdaptor->SetRequiredTransformDomainDirection(
      															shrinkFilter->GetOutput()->GetDirection() );
    bsplineAdaptor->SetRequiredTransformDomainPhysicalDimensions(
      															PhysDim );
    adaptors.push_back( bsplineAdaptor.GetPointer() );
    }
*/
  // Scale estimator
  ScalesEstimatorType::Pointer scalesEstimator = ScalesEstimatorType::New();
  scalesEstimator->SetMetric( metric );
  scalesEstimator->SetTransformForward( true );
  scalesEstimator->SetSmallParameterVariation( 1.0 );

  //registration->SetTransformParametersAdaptorsPerLevel( adaptors );
  registration->SetNumberOfLevels( params.numberOfLevels );
  registration->SetSmoothingSigmasPerLevel( smoothingSigmasPerLevel );
  registration->SetShrinkFactorsPerLevel( shrinkFactorsPerLevel );

  // Set Optimizer
  optimizer->SetGradientConvergenceTolerance( params.slength );
  optimizer->SetLineSearchAccuracy( 0.9 );
  optimizer->SetDefaultStepLength( params.lrate );
  optimizer->TraceOn();
  optimizer->SetMaximumNumberOfFunctionEvaluations( params.niter );
  optimizer->SetScalesEstimator( scalesEstimator );

  // Add time and memory probes
  itk::TimeProbesCollectorBase chronometer;
  itk::MemoryProbesCollectorBase memorymeter;

  cout << "Starting Registration"       << endl;

  try
    {
    memorymeter.Start( "Registration" );
    chronometer.Start( "Registration" );

    registration->Update();

    chronometer.Stop( "Registration" );
    memorymeter.Stop( "Registration" );

    cout << "Optimizer stop condition = "
              << registration->GetOptimizer()->GetStopConditionDescription()
              << endl;
    }
  catch( itk::ExceptionObject & err )
    {
    cerr << "ExceptionObject caught !"  << endl;
    cerr << err                         << endl;
    exit(1);
    }

  // Report the time and memory taken by the registration
  chronometer.Report( cout );
  memorymeter.Report( cout );

  OptimizerType::ParametersType finalParameters = transform->GetParameters();

  cout << "Last Transform Parameters"   << endl;
  cout << finalParameters               << endl;

  return transform;
}
