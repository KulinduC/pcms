#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include "couplingTypes.h"
#include <fftw3.h>

// FIXME indentation is wrong
namespace coupler {

// forward declare
class Part3Mesh3D;
class Part1ParalPar3D;
class BoundaryDescr3D;

class DatasProc3D {
public:
  LO part1li0; // part1li0 is the element number of subdomain of x on each
               // process belonging to comm_y. This x subdomain belongs to 2d
               // box on the x-y plane storing the input charged ion density for
               // fourier transform.
  LO part3li0; // part3li0 is the element number of subdomain of x on each
               // process belonging to comm_y. This x subdomain belongs to 2d 
               // box on the x-y plane storing the input electrostatic 
               // potential.
  LO part1lj0; // the count of elements on y domain on each process after
               // backward Fourier transform
  LO part3lj0; ////the count of elements on y domain on each process after
               ///forward Fourier transform

  LO sum;
  // here, pointers must be assigned a NULL;
  CV*** densin = NULL; // input 3d density in complex number
  CV* densintmp = NULL; // temporary 2d density array prepared for backward
                        // fourier transform
  double* densouttmp = NULL; // store the x-y 2d real density after backward 
                             // fourier transform
  double*** densout = NULL; // store xyz 3d real density
  double*** denspart3 = NULL; // storing the density being sent to the part3
  double*** potentin = NULL; // the input real electrostatic potential in 3d xyz
  double*** potentinterpo = NULL; // temporary xy 2d potential array for forward 
                              // fourier transform
  double* potentintmp = NULL;
  CV* potentouttmp = NULL;
  CV*** potentpart1 = NULL; // storing the electrostatic potential being sent
                            // to the part1.
  fftw_plan plan_forward = NULL, plan_backward = NULL;
  // The following parameters for yparal=true;
  LO myli0;
  /* constructor
   * optional argument supports setting
   * the prepoc and yparal modes
   */
  DatasProc3D(const Part1ParalPar3D& p1pp3d, const Part3Mesh3D& p3m3d,
              bool pproc = true, bool ypar = false)
    : preproc(pproc), yparal(ypar) {
    init(p1pp3d, p3m3d);
    AllocDensityArrays(p1pp3d, p3m3d);
    AllocPotentArrays(p1pp3d, p3m3d);
  }
  ~DatasProc3D();
  // routines for Fourier transform
  void CmplxdataToRealdata3D(const Part1ParalPar3D& p1pp3d);
  void RealdataToCmplxdata3D(const Part1ParalPar3D& p1pp3d,
                             const Part3Mesh3D& p3m3d);
  void InitFourierPlan3D(const Part1ParalPar3D& p1pp3d,
                         const Part3Mesh3D& p3m3d);

private:
  const bool preproc;
  const bool yparal;
  /* helper function for destructor */
  void
  FreeFourierPlan3D(); // called from the destructor - does that make sense?
  /* helper functions for constructor */
  void init(const Part1ParalPar3D& p1pp3d, const Part3Mesh3D& p3m3d);
  void AllocDensityArrays(const Part1ParalPar3D& p1pp3d,
                          const Part3Mesh3D& p3m3d);
  void AllocPotentArrays(const Part1ParalPar3D& p1pp3d,
                         const Part3Mesh3D& p3m3d);
  /* helper functions for CmplxdataToRealdata3D and RealdataToCmplxdata3D */
  void ExecuteRealToCmplx(const Part1ParalPar3D& p1pp3d);
  void ExecuteCmplxToReal(const Part1ParalPar3D& p1pp3d);
};

void TransposeComplex(CV** InMatrix, CV** OutMatrix, DatasProc3D& dp3d,
                      Part1ParalPar3D& p1pp3d);

void InterpoDensity3D(const BoundaryDescr3D& bdesc, const Part3Mesh3D& p3m3d,
                      const Part1ParalPar3D& p1pp3d, const DatasProc3D& dp3d,
                      const bool preproc);

void InterpoPotential3D(const BoundaryDescr3D& bdesc, const Part3Mesh3D& p3m3d,
                        const Part1ParalPar3D& p1pp3d, DatasProc3D& dp3d,
                        const bool preproc);

void TestInitPotentAlongz(DatasProc3D& dp3d,Part3Mesh3D& p3m3d,LO npy,LO n);

} // namespace coupler

#endif
