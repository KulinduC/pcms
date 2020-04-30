#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include "classes.h"

namespace coupler {

void InitDatasProc3Dparameters(DatasProc3D& dp3d,Part1ParalPar3D& p1pp3d,Part3Mesh3D &p3m3d );

void AllocDatasProc3dDensityArraies(DatasProc3D& dp3d,Part1ParalPar3D& p1pp3d,Part3Mesh3D& p3m3d);

void AllocDatasProc3dPotentArraies(DatasProc3D& dp3d,Part1ParalPar3D& p1pp3d,Part3Mesh3D &p3m3d);


//boundary treatment
void InitBoundaryDescr3D(BoundaryDescr3D &bdesc,Part3Mesh3D& p3m3d, Part1ParalPar3D &p1pp3d,DatasProc3D& dp3d);

void zPotentBoundaryBufAssign(MPI_Datatype mpitype,BoundaryDescr3D &bdesc,DatasProc3D& dp3d,
        Part3Mesh3D& p3m3d, Part1ParalPar3D &p1pp3d);

  template<class T>
void  zDensityBoundaryBufAssign(MPI_Datatype mpitype,LO nzb,LO lx,LO ly,LO lz,T*** lowbuf,
       T*** upbuf,T*** box,Part1ParalPar3D &p1pp3d)
  {
   if(lowbuf==NULL||upbuf==NULL){
     std::cout<<"ERROR:the boundary buffer must be alloctted beforing involing this routine.";
     std::exit(EXIT_FAILURE);
  }
  if(p1pp3d.npz>1){
    if(lz>=nzb){
      mpisendrecv_aux2D(mpitype,lx,ly,lz,lowbuf,upbuf,box,p1pp3d);
    } else {
      std::cout<<"ERROR: nzb is larger than lzb. Large lzb is required.";
      std::exit(EXIT_FAILURE);
      }
  } else{
      if(p1pp3d.periods[2]==1){
        for(LO i=0;i<lx-1;i++){
          for(LO j=0;j<ly-1;j++){
            for(LO k=0;k<nzb-1;k++){
              lowbuf[i][j][k]=box[i][j][lz-nzb+k];
              upbuf[i][j][k]=box[i][j][k];
            }
          }
        }
      } else{
          std::cout<<"The topology is not right."<<'\n';
          std::exit(EXIT_FAILURE);
        }
    }
}


template<class T>
void mpisendrecv_aux2D(MPI_Datatype mpitype,MPI_Comm comm,LO nzb,LO lx,LO ly,LO lz,
     T*** lowbuf,T*** upbuf,T*** box)
{
      T* sendbuf=new T[lx*ly*nzb];
      T* recvbuf=new T[lx*ly*nzb];
      MPI_Status status;
      int rank_source,rank_dest;
      MPI_Cart_shift(comm,2,1,&rank_source,&rank_dest);
      for(LO i=0;i<lx-1;i++){
       for(LO j=0;j<ly-1;j++){
         for(LO k=0;k<nzb-1;k++)
           sendbuf[i*ly*nzb+j*nzb+k]=box[i][j][k];
       }
      }
      MPI_Sendrecv(sendbuf,lx*ly*nzb,mpitype,rank_dest,100,recvbuf,lx*ly*nzb,mpitype,rank_source,101,
                  comm,&status);
      for(LO i=0;i<lx-1;i++){
       for(LO j=0;j<ly-1;j++){
         for(LO k=0;k<nzb-1;k++)
              upbuf[i][j][k]=recvbuf[i*ly*nzb+j*nzb+k];
       }
      }
      MPI_Cart_shift(comm,2,-1, &rank_source, &rank_dest);
      for(LO i=0;i<lx-1;i++){
       for(LO j=0;j<ly-1;j++){
         for(LO k=0;k<nzb-1;k++)
            sendbuf[i*ly*nzb+j*nzb+k]=box[i][j][lz-nzb+k];
       }
     }
      MPI_Sendrecv(sendbuf,lx*ly*nzb,mpitype,&rank_dest,102,recvbuf,lx*ly*nzb,mpitype,&rank_source,103,
                  comm,&status);
      for(LO i=0;i<lx-1;i++){
       for(LO j=0;j<ly-1;j++){
         for(LO k=0;k<nzb;k++)
              lowbuf[i][j][k]=recvbuf[i*ly*nzb+j*nzb+k];
       }
      }
  delete[] sendbuf;
  delete[] recvbuf;
}

template<class T>
void mpisendrecv_aux1D(MPI_Datatype mpitype,MPI_Comm comm,LO nzb,LO xind,LO yind,LO zind,
     T* lowbuf,T* upbuf,T* box1d)
{
      MPI_Status status;
      int rank_source, rank_dest;
      MPI_Cart_shift(comm,3,1,&rank_source,&rank_dest);
      MPI_Sendrecv(box1d,nzb,mpitype,rank_dest,100,upbuf,nzb,mpitype,rank_source,101,
            comm,&status);
      MPI_Cart_shift(comm,3,-1,&rank_source,&rank_dest);
      MPI_Sendrecv(&box1d[zind-nzb],nzb,mpitype,rank_dest,102,lowbuf,nzb,mpitype,rank_source,103,
            comm,&status);
}


//routines for Fourier transform

void CmplxdataToRealdata3D(DatasProc3D& dp3d, Part1ParalPar3D& p1pp3d);

void RealdataToCmplxdata3D(DatasProc3D& dp3d, Part1ParalPar3D& p1pp3d,Part3Mesh3D &p3m3d);

void TransposeComplex(std::complex<double>** InMatrix,std::complex<double>** OutMatrix, DatasProc3D& dp3d,
     Part1ParalPar3D& p1pp3d);

void ExecuteCmplxToReal(DatasProc3D& dp3d, Part1ParalPar3D& p1pp3d);

void ExecuteRealToCmplx(DatasProc3D& dp3d, Part1ParalPar3D& p1pp3d);

void InitFourierPlan3D( DatasProc3D& dp3d);


// routines for interpolation
template<class T>
T Lag3dInterpo1D(const T yin[4],const double xin[4],const double x)
{
  double l0,l1,l2,l3;
  T yout;
  l0=(x-xin[1])*(x-xin[2])*(x-xin[3])/(xin[0]-xin[1])/(xin[0]-xin[2])/(xin[0]-xin[3]);
  l1=(x-xin[0])*(x-xin[2])*(x-xin[3])/(xin[1]-xin[0])/(xin[1]-xin[2])/(xin[1]-xin[3]);
  l2=(x-xin[0])*(x-xin[1])*(x-xin[3])/(xin[2]-xin[0])/(xin[2]-xin[1])/(xin[2]-xin[3]);
  l3=(x-xin[0])*(x-xin[1])*(x-xin[2])/(xin[3]-xin[0])/(xin[3]-xin[1])/(xin[3]-xin[2]);
  yout=yin[0]*l0+yin[1]*l1+yin[2]*l2+yin[3]*l3;
  return yout;
}

//central 3rd order Lagrange polynormal interpolation
template<class T>
void Lag3dArray(T* yin,double* xin,LO nin,T* yout,double* xout,LO nout){
       LO jstart=2;
       LO j1=jstart;
       LO j2,j0,jm;
       double x;
       T func[4];
       double coords[4];
       for(LO j=0;j<nout;j++){
         x=xout[j];
         while(x>=xin[j1] && j1<nin-2 && j1>1){
           j1=+1;
         }
         j2=j1+1;
         j0=j1-1;
         jm=j1-2;
         coords[0]=xin[jm];
         coords[1]=xin[j0];
         coords[2]=xin[j1];
         coords[3]=xin[j2];
         func[0]=yin[jm];
         func[1]=yin[j0];
         func[2]=yin[j1];
         func[3]=yin[j2];
         yout[j]=Lag3dInterpo1D(func,coords,x);
       }

     }

void InterpoDensity3D(BoundaryDescr3D &bdesc,Part3Mesh3D& p3m3d,  
     Part1ParalPar3D &p1pp3d,DatasProc3D& dp3d);

void InterpoPotential3D(BoundaryDescr3D &bdesc,Part3Mesh3D& p3m3d,
     Part1ParalPar3D &p1pp3d,DatasProc3D& dp3d);

}

#endif
