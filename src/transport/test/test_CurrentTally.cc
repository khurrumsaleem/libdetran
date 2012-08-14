//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   test_CurrentTally.cc
 * \author Jeremy Roberts
 * \date   Apr 1, 2012
 * \brief  Test of Equation_DD_2D
 * \note   Copyright (C) 2012 Jeremy Roberts. 
 */
//---------------------------------------------------------------------------//

// LIST OF TEST FUNCTIONS
#define TEST_LIST                     \
        FUNC(test_CurrentTally_1D)    \
        FUNC(test_CurrentTally_2D)    \
        FUNC(test_CurrentTally_3D)

// Detran headers
#include "TestDriver.hh"
#include "CurrentTally.hh"
#include "GaussLegendre.hh"
#include "LevelSymmetric.hh"
#include "Definitions.hh"

// Setup
#include "coarsemesh_fixture.hh"

using namespace detran;
using namespace detran_test;
using namespace std;

int main(int argc, char *argv[])
{
  RUN(argc, argv);
}

//---------------------------------------------------------------------------//
// TEST DEFINITIONS
//---------------------------------------------------------------------------//

int test_CurrentTally_1D(int argc, char *argv[])
{
  using detran::u_int;
  typedef CurrentTally<_1D> CurrentTally_T;
  /*
   *  The 1D sample problem has a coarse mesh like
   *    0         1      2   3   4   5   6   7
   *    |    3    |   2  | 1 | 1 | 1 | 1 | 1 |
   *
   *  1001010101010101
   *
   *  where the numbers are in cm.  This was based on a fine mesh with two
   *  5 cm regions, the first of which had 5 fine meshes (now 2 of unequal
   *  widths), and the second which had 10 fine meshes (for 5 equal width
   *  coarse meshes).  Hence, the "level" factor was 2.
   *
   *  In this 1D example, we will use an S4 quadrature, so two angles
   *  per direction.  We will assume that all angular fluxes are
   *  attenuated by a factor dx, where dx is the fine mesh dx.
   *
   * We define the incident flux to be 100*o + 10*a + 1, or [1, 11, 101, and 111]
   * Using the quadrature parameters, we find that the partial currents
   * at the coarse mesh boundaries are
   *         RIGHT                  LEFT
   * 0 (2.73843733182052e+00) 5.35792775340790e-02
   * 1  2.73843733182052e+00  5.35792775340790e-02
   * 2  2.73843733182052e+00  5.35792775340790e-02
   * 3  6.84609332955131e-01  2.14317110136316e-01
   * 4  1.71152333238783e-01  8.57268440545264e-01
   * 5  4.27880833096957e-02  3.42907376218105e+00
   * 6  1.06970208274239e-02  1.37162950487242e+01
   * 7  2.67425520685598e-03 (5.48651801948969e+01)
   *
   *
   */

  // Reference partial currents
  double Jright[] = {2.73843733182052E+00, 2.73843733182052E+00, 2.73843733182052E+00,
                     6.84609332955131E-01, 1.71152333238783E-01, 4.27880833096957E-02,
                     1.06970208274239E-02, 2.67425520685598E-03};
  double Jleft[]  = {5.35792775340790E-02, 5.35792775340790E-02, 5.35792775340790E-02,
                     2.14317110136316E-01, 8.57268440545264E-01, 3.42907376218105E+00,
                     1.37162950487242E+01, 5.48651801948969E+01};

  // Get the coarse mesh
  CurrentTally_T::SP_coarsemesh mesh = coarsemesh_1d();
  TEST(mesh);

  // Get the fine mesh.
  CoarseMesh::SP_mesh finemesh = mesh->get_fine_mesh();
  TEST(finemesh->dimension() == 1);
  TEST(finemesh->number_cells() == 15);

  // Create an S4 quadrature.
  CurrentTally_T::SP_quadrature quad(new GaussLegendre(4));

  // Create the tally.
  CurrentTally_T::SP_currenttally tally(new CurrentTally_T(mesh, quad, 1));

  // Create the face flux.  For 1D, it's just a double.
  CurrentTally_T::face_flux_type psi_in, psi_out;

  // Now, fake a sweep.

  // Loop through all octants.
  for (u_int o = 0; o < 2; o++)
  {

    // Loop through azimuths in an octant.
    for (u_int a = 0; a < 2; a++)
    {

      // Start with a fixed psi at the boundary.
      psi_out = 100.0 * (double) o + 10.0 * (double) a + 1.0;

      // TALLY THE INCIDENT BOUNDARY
      u_int io = 0;
      u_int zero = 0;
      if (o == 1) io = 14;
      tally->tally(io, zero, zero, zero, o, a, 0, psi_out);

      // Loop over cells.
      for (u_int ii = 0; ii < 15; ii++)
      {
        u_int i = ii;
        if (o == 1) i = 15 - i - 1;

        psi_in = psi_out;
        psi_out = psi_in * finemesh->dx(i);

        // TALLY THE OUTGOING CELL FLUX
        tally->tally(i, 0, 0, 0, o, a, psi_out);
      }
    }
  }

  // Test
  for (int i = 0; i < 8; i++)
  {
    TEST(soft_equiv(Jright[i], tally->partial_current(i, 0, 0, 0, 0, true)));
    TEST(soft_equiv(Jleft[i],  tally->partial_current(i, 0, 0, 0, 0, false)));
  }

  return 0;
}

int test_CurrentTally_2D(int argc, char *argv[])
{
  using detran::u_int;
  typedef CurrentTally<_2D> CurrentTally_T;

  /*
   *  The 2D sample problem uses the same coarse mesh on each axis:
   *    0         1      2   3   4   5   6   7
   *    |    3    |   2  | 1 | 1 | 1 | 1 | 1 |
   *
   *  which is the same as the 1D problem.
   *
   *  For the 2D problem, we will simplify compared to the 1D
   *  problem.  We'll use an S2 quadrature.  We'll assume
   *  no attenuation of the flux, and all fluxes are
   *  unit at the boundary. The result should be that the
   *  partial current at any location is the width of the
   *  face times the cosine times the weight times the two
   *  octants that contribute.
   *
   *  mu = eta = 0.577350269189625764509149
   *  wt = 1.0
   */

  const double cos_times_weight = 2.0 * 0.577350269189625764509149 * pi;

  // Get the coarse mesh
  CurrentTally_T::SP_coarsemesh mesh = coarsemesh_2d();
  TEST(mesh);

  int ceflag[] = {0, -1, 1, -1, 2};

  // Get the fine mesh.
  CoarseMesh::SP_mesh finemesh = mesh->get_fine_mesh();
  TEST(finemesh->dimension()    == 2);
  TEST(finemesh->number_cells() == 15*15);

  // Get the coarse mesh.
  CoarseMesh::SP_mesh coarsemesh = mesh->get_coarse_mesh();
  TEST(coarsemesh->dimension()    == 2);
  TEST(coarsemesh->number_cells() == 7*7);

  // Create an S2 quadrature.
  CurrentTally_T::SP_quadrature quad(new LevelSymmetric(2, 2));

  // Create the tally.
  CurrentTally_T::SP_currenttally tally(new CurrentTally_T(mesh, quad, 1));

  // Create the face flux.
  CurrentTally_T::face_flux_type psi_in, psi_out;

  // Now, fake a sweep.

  // Loop through all octants.
  for (u_int o = 0; o < 4; o++)
  {
    // Loop through azimuths in an octant.
    for (u_int a = 0; a < 1; a++)
    {
      // Start with a psi of unity at boundaries.
      psi_out[0] = 1.0;
      psi_out[1] = 1.0;

      // Tally x-directed face
      {
        // Pick left or right side
        u_int i = 0;
        if (o == 1 or o == 2) i = finemesh->number_cells_x() - 1;
        // Loop over vertical
        for (u_int jj = 0; jj < finemesh->number_cells_y(); jj++)
        {
          u_int j = jj;
          if (o > 1) j = finemesh->number_cells_y() - j - 1;
          tally->tally(i, j, 0, 0, o, a, 0, 1.0);
        }
      }

      // Tally y-directed face
      {
        u_int j = 0;
        if (o > 1) j = finemesh->number_cells_y() - 1;
        for (u_int ii = 0; ii < finemesh->number_cells_x(); ii++)
        {
          u_int i = ii;
          if (o == 1 or o == 2) i = finemesh->number_cells_x() - i - 1;
          tally->tally(i, j, 0, 0, o, a, 1, 1.0);
        }
      }

      // Loop over y.
      for (u_int jj = 0; jj < finemesh->number_cells_y(); jj++)
      {
        u_int j = jj;
        if (o > 1) j = finemesh->number_cells_y() - j - 1;

        // Loop over x.
        for (u_int ii = 0; ii < finemesh->number_cells_x(); ii++)
        {
          u_int i = ii;
          if (o == 1 or o == 2) i = finemesh->number_cells_x() - i - 1;

          // TALLY THE OUTGOING CELL FLUX
          tally->tally(i, j, 0, 0, o, a, psi_out);

        } // end x loop

      } // end y loop

    } // end angle loop

  } // end octant loop

  // Test x-directed.
  for (int j = 0; j < coarsemesh->number_cells_y(); j++)
  {
    for (int i = 0; i < coarsemesh->number_cells_x() + 1; i++)
    {
      double tmp = cos_times_weight * coarsemesh->dy(j);
      TEST(soft_equiv(
           tally->partial_current(i, j, 0, 0,
           CurrentTally_T::X_DIRECTED, CurrentTally_T::NEGATIVE), tmp));
      TEST(soft_equiv(
           tally->partial_current(i, j, 0, 0,
           CurrentTally_T::X_DIRECTED, CurrentTally_T::POSITIVE), tmp));
    }
  }
  // Test y-directed.
  for (int i = 0; i < coarsemesh->number_cells_x(); i++)
  {
    for (int j = 0; j < coarsemesh->number_cells_y() + 1; j++)
    {
      double tmp = cos_times_weight * coarsemesh->dx(i);
      TEST(soft_equiv(
           tally->partial_current(i, j, 0, 0,
           CurrentTally_T::Y_DIRECTED, CurrentTally_T::NEGATIVE), tmp));
      TEST(soft_equiv(
           tally->partial_current(i, j, 0, 0,
           CurrentTally_T::Y_DIRECTED, CurrentTally_T::POSITIVE), tmp));
    }
  }

  return 0;
}

int test_CurrentTally_3D(int argc, char *argv[])
{
  using detran::u_int;
  typedef CurrentTally<_3D> CurrentTally_T;

  /*
   *  The 2D sample problem uses the same coarse mesh on each axis:
   *    0         1      2   3   4   5   6   7
   *    |    3    |   2  | 1 | 1 | 1 | 1 | 1 |
   *
   *  which is the same as the 1D problem.
   *
   *  For the 3D problem, we do the same as the 2D problem, only
   *  now the current at any location is the area of the
   *  face times the cosine times the weight times the four
   *  octants that contribute.
   *
   *  mu = eta = 0.577350269189625764509149
   *  wt = 1.0
   */

  const double cos_times_weight = 2.0 * 0.577350269189625764509149 * pi;

  // Get the coarse mesh
  CurrentTally_T::SP_coarsemesh mesh = coarsemesh_3d();
  TEST(mesh);

  // Get the fine mesh.
  CoarseMesh::SP_mesh finemesh = mesh->get_fine_mesh();
  TEST(finemesh->dimension()    == 3);
  //TEST(finemesh->number_cells() == 4*4*4);
  TEST(finemesh->number_cells() == 15*15*15);

  // Get the coarse mesh.
  CoarseMesh::SP_mesh coarsemesh = mesh->get_coarse_mesh();
  TEST(coarsemesh->dimension()    == 3);
  //TEST(coarsemesh->number_cells() == 2*2*2);
  TEST(coarsemesh->number_cells() == 7*7*7);

  // Create an S2 quadrature.
  CurrentTally_T::SP_quadrature quad(new LevelSymmetric(2, 3));

  // Create the tally.
  CurrentTally_T::SP_currenttally tally(new CurrentTally_T(mesh, quad, 1));

  // Create the face flux.
  CurrentTally_T::face_flux_type psi_in, psi_out;

  // Now, fake a sweep.

  // Loop through all octants.
  for (u_int o = 0; o < 8; o++)
  {
    // Loop through azimuths in an octant.
    for (u_int a = 0; a < 1; a++)
    {
      // Start with a psi of unity at boundaries.
      psi_out[0] = 1.0;
      psi_out[1] = 1.0;
      psi_out[2] = 1.0;

      // Tally x-directed face
      {
        // Pick left or right side
        u_int i = 0;
        if (o == 1 or o == 2 or o == 5 or o == 6)
          i = finemesh->number_cells_x() - 1;
        // Loop over vertical
        for (u_int jj = 0; jj < finemesh->number_cells_y(); jj++)
        {
          u_int j = jj;
          if (o == 2 or o == 3 or o == 6 or o == 7)
            j = finemesh->number_cells_y() - j - 1;
          for (u_int kk = 0; kk < finemesh->number_cells_z(); kk++)
          {
            u_int k = kk;
            if (o > 3) k = finemesh->number_cells_z() - k - 1;
            tally->tally(i, j, k, 0, o, a, CurrentTally_T::X_DIRECTED, 1.0);
          }
        }
      }

      // Tally y-directed face
      {
        // Pick left or right side
        u_int j = 0;
        if (o == 2 or o == 3 or o == 6 or o == 7)
          j = finemesh->number_cells_y() - 1;
        // Loop over vertical
        for (u_int ii = 0; ii < finemesh->number_cells_x(); ii++)
        {
          u_int i = ii;
          if (o == 1 or o == 2 or o == 5 or o == 6)
            i = finemesh->number_cells_x() - i - 1;
          for (u_int kk = 0; kk < finemesh->number_cells_z(); kk++)
          {
            u_int k = kk;
            if (o > 3) k = finemesh->number_cells_z() - k - 1;
            tally->tally(i, j, k, 0, o, a, CurrentTally_T::Y_DIRECTED, 1.0);
          }
        }
      }

      // Tally z-directed face
      {
        // Pick left or right side
        u_int k = 0;
        if (o > 3)
          k = finemesh->number_cells_z() - 1;
        // Loop over vertical
        for (u_int ii = 0; ii < finemesh->number_cells_x(); ii++)
        {
          u_int i = ii;
          if (o == 1 or o == 2 or o == 5 or o == 6)
            i = finemesh->number_cells_x() - i - 1;
          for (u_int jj = 0; jj < finemesh->number_cells_y(); jj++)
          {
            u_int j = jj;
            if (o == 2 or o == 3 or o == 6 or o == 7)
              j = finemesh->number_cells_y() - j - 1;
            tally->tally(i, j, k, 0, o, a, CurrentTally_T::Z_DIRECTED, 1.0);
          }
        }
      }

      // Loop over z.
      for (u_int kk = 0; kk < finemesh->number_cells_z(); kk++)
      {
        u_int k = kk;
        if (o > 3) k = finemesh->number_cells_z() - k - 1;

        // Loop over y.
        for (u_int jj = 0; jj < finemesh->number_cells_y(); jj++)
        {
          u_int j = jj;
          if (o == 2 or o == 3 or o == 6 or o == 7)
            j = finemesh->number_cells_y() - j - 1;

          // Loop over x.
          for (u_int ii = 0; ii < finemesh->number_cells_x(); ii++)
          {
            u_int i = ii;
            if (o == 1 or o == 2 or o == 5 or o == 6)
              i = finemesh->number_cells_x() - i - 1;

            // TALLY THE OUTGOING CELL FLUX
            tally->tally(i, j, k, 0, o, a, psi_out);

          } // end x loop

        } // end y loop

      } // end z loop

    } // end angle loop

  } // end octant loop

//  tally->display();
//  return 0;

  // Test x-directed.
  for (int k = 0; k < coarsemesh->number_cells_z(); k++)
  {
    for (int j = 0; j < coarsemesh->number_cells_y(); j++)
    {
      for (int i = 0; i < coarsemesh->number_cells_x() + 1; i++)
      {
        double tmp = cos_times_weight * coarsemesh->dy(j) * coarsemesh->dz(k);
        TEST(soft_equiv(
             tally->partial_current(i, j, k, 0,
             CurrentTally_T::X_DIRECTED, CurrentTally_T::NEGATIVE), tmp));
        TEST(soft_equiv(
             tally->partial_current(i, j, k, 0,
             CurrentTally_T::X_DIRECTED, CurrentTally_T::POSITIVE), tmp));
      }
    }
  }
  // Test y-directed.
  for (int k = 0; k < coarsemesh->number_cells_z(); k++)
  {
    for (int j = 0; j < coarsemesh->number_cells_y() + 1; j++)
    {
      for (int i = 0; i < coarsemesh->number_cells_x(); i++)
      {
        double tmp = cos_times_weight * coarsemesh->dx(i) * coarsemesh->dz(k);
        TEST(soft_equiv(
             tally->partial_current(i, j, k, 0,
             CurrentTally_T::Y_DIRECTED, CurrentTally_T::NEGATIVE), tmp));
        TEST(soft_equiv(
             tally->partial_current(i, j, k, 0,
             CurrentTally_T::Y_DIRECTED, CurrentTally_T::POSITIVE), tmp));
      }
    }
  }
  // Test z-directed
  for (int k = 0; k < coarsemesh->number_cells_z() + 1; k++)
  {
    for (int j = 0; j < coarsemesh->number_cells_y(); j++)
    {
      for (int i = 0; i < coarsemesh->number_cells_x(); i++)
      {
        double tmp = cos_times_weight * coarsemesh->dx(i) * coarsemesh->dy(j);
        TEST(soft_equiv(
             tally->partial_current(i, j, k, 0,
             CurrentTally_T::Z_DIRECTED, CurrentTally_T::NEGATIVE), tmp));
        TEST(soft_equiv(
             tally->partial_current(i, j, k, 0,
             CurrentTally_T::Z_DIRECTED, CurrentTally_T::POSITIVE), tmp));
      }
    }
  }

  return 0;
}

//---------------------------------------------------------------------------//
//              end of test_CurrentTally.cc
//---------------------------------------------------------------------------//
