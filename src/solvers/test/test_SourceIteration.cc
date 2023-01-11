//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   test_SourceIteration.cc
 * \author robertsj
 * \date   Apr 4, 2012
 * \brief  test_SourceIteration class definition.
 * \note   Copyright (C) 2012 Jeremy Roberts. 
 */
//---------------------------------------------------------------------------//

// LIST OF TEST FUNCTIONS
#define TEST_LIST                      \
        FUNC(test_SourceIteration_2D   )

// Detran headers
#include "TestDriver.hh"
#include "SourceIteration.hh"

// Setup
#include "angle/test/quadrature_fixture.hh"
#include "geometry/test/mesh_fixture.hh"
#include "material/test/material_fixture.hh"
#include "external_source/test/external_source_fixture.hh"

using namespace detran_test;
using namespace detran;
using namespace detran_angle;
using namespace detran_external_source;
using namespace detran_geometry;
using namespace detran_utilities;
using namespace std;

int main(int argc, char *argv[])
{
  RUN(argc, argv);
}

//----------------------------------------------//
// TEST DEFINITIONS
//----------------------------------------------//

int test_SourceIteration_2D(int argc, char *argv[])
{

  // Test fixtures
  SP_material mat       = material_fixture_1g();
  SP_mesh mesh          = mesh_2d_fixture(1);   // pure absorbing box
  SP_quadrature quad    = quadruplerange_fixture();

  // Source; uniform, unit, isotropic. (mesh, quad, ng)
  ConstantSource::SP_externalsource
    q_e(new ConstantSource(1, mesh, 1.0, quad));

  // Empty fission source
  SourceIteration<_2D>::SP_fissionsource q_f;

  // Input
  SourceIteration<_2D>::SP_input input;
  input  = std::make_shared<InputDB>();
  input->put<string>(  "equation",          "dd");
  input->put<int>(     "number_groups",     1);
  input->put<int>(     "inner_max_iters",   5);
  input->put<double>(  "inner_tolerance",   1e-14);

  // State
  SourceIteration<_2D>::SP_state state;
  state  = std::make_shared<State>(input, mesh, quad);

  // Boundary
  SourceIteration<_2D>::SP_boundary bound;
  bound  = std::make_shared<BoundarySN<_2D> >(input, mesh, quad);
  // SI
  SourceIteration<_2D> solver(input, state, mesh, mat,
                              quad, bound, q_e, q_f);

  // Solve first group.
  solver.solve(0);

  State::moments_type phi = state->phi(0);
  TEST(soft_equiv(phi[0], 3.402625949151646e-01));
  TEST(soft_equiv(phi[1], 4.334557647118027e-01));
  TEST(soft_equiv(phi[4], 5.486109632367223e-01));

  return 0;
}
//---------------------------------------------------------------------------//
//              end of test_SourceIteration.cc
//---------------------------------------------------------------------------//
