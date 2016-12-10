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
                                        moving );
    registration->SetInitialTransform( ttransform );
  } else {
    registration->SetInitialTransform( transform );
  }
  registration->InPlaceOn();
  registration->SetFixedImage(    fixed     );
  registration->SetMovingImage(   moving    );

  // Scale estimator
  ScalesEstimatorType::Pointer scalesEstimator = ScalesEstimatorType::New();
  scalesEstimator->SetMetric( metric );
  scalesEstimator->SetTransformForward( true );
  scalesEstimator->SetSmallParameterVariation( 1.0 );

  // Set Optimizer
  optimizer->SetGradientConvergenceTolerance( params.slength );
  optimizer->SetLineSearchAccuracy( 1.2 );
  optimizer->SetDefaultStepLength( 1.5 );
  optimizer->TraceOn();
  optimizer->SetMaximumNumberOfFunctionEvaluations( params.niter );
  optimizer->SetScalesEstimator( scalesEstimator );

  RegistrationBSplineType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
  shrinkFactorsPerLevel.SetSize( 1 );
  shrinkFactorsPerLevel[0] = 1;

  RegistrationBSplineType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
  smoothingSigmasPerLevel.SetSize( 1 );
  smoothingSigmasPerLevel[0] = 0;

  registration->SetNumberOfLevels( params.numberOfLevels );
  registration->SetSmoothingSigmasPerLevel( smoothingSigmasPerLevel );
  registration->SetShrinkFactorsPerLevel( shrinkFactorsPerLevel );

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
