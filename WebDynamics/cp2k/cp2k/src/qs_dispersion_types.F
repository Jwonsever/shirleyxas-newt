!-----------------------------------------------------------------------------!
!   CP2K: A general program to perform molecular dynamics simulations         !
!   Copyright (C) 2000 - 2013  CP2K developers group                          !
!-----------------------------------------------------------------------------!

! *****************************************************************************
!> \brief Definition of disperson types for DFT calculations
!> \author JGH (20.10.2008)
! *****************************************************************************
MODULE qs_dispersion_types

  USE kinds,                           ONLY: default_string_length,&
                                             dp
#include "cp_common_uses.h"

  IMPLICIT NONE

  PRIVATE

  CHARACTER(len=*), PARAMETER, PRIVATE :: moduleN = 'qs_dispersion_types'

! *****************************************************************************
  INTEGER, PARAMETER                       :: dftd2_pp = 1
  INTEGER, PARAMETER                       :: dftd3_pp = 2

  TYPE qs_dispersion_type
    INTEGER                                :: TYPE
    INTEGER                                :: pp_type
    INTEGER                                :: nl_type
    CHARACTER (LEN=default_string_length)  :: ref_functional
    REAL(KIND=dp)                          :: scaling
    REAL(KIND=dp)                          :: rc_disp
    REAL(KIND=dp)                          :: exp_pre
    LOGICAL                                :: verbose  !extended output
    CHARACTER (LEN=default_string_length)  :: parameter_file_name
    CHARACTER (LEN=default_string_length)  :: kernel_file_name
    !DFT-D3 global parameters
    INTEGER                                :: max_elem !elements parametrized
    INTEGER                                :: maxc     !max coordination number references per element
    REAL(KIND=dp)                          :: k1,k2,k3 !ad hoc parameters
    REAL(KIND=dp)                          :: alp      !ad hoc parameters
    REAL(KIND=dp)                          :: s6,s8,sr6!scaling parameters
    REAL(KIND=dp)                          :: eps_cn
    LOGICAL                                :: doabc    !neglect C9 terms
    LOGICAL                                :: c9cnst   !use constant c9 terms
    LOGICAL                                :: lrc      !calculate a long range correction
    REAL(KIND=dp),DIMENSION(:,:,:,:,:), POINTER &
                                           :: c6ab
    INTEGER,DIMENSION(:),POINTER           :: maxci
    REAL(KIND=dp),DIMENSION(:,:), POINTER  :: r0ab
    REAL(KIND=dp),DIMENSION(:), POINTER    :: rcov     !covalent radii
    REAL(KIND=dp),DIMENSION(:), POINTER    :: r2r4     !atomic <r^2>/<r^4> values
    REAL(KIND=dp),DIMENSION(:), POINTER    :: cn       !coordination numbers (defaults)
    TYPE(cn_kind_list),DIMENSION(:), POINTER &
                                           :: cnkind
    TYPE(cn_atom_list),DIMENSION(:), POINTER &
                                           :: cnlist
    !vdW-DF variables
    REAL(KIND=dp)                          :: pw_cutoff
    REAL(KIND=dp)                          :: b_value, c_value !parameters for the rVV10 functional
    INTEGER                                :: nqs, nr_points
                                                       !! The number of q points and radial points
                                                       !! used in generating the kernel phi(q1*r, q2*r)
                                                       !! (see DION 14-16 and SOLER 3)
    REAL(KIND=dp)                          :: r_max, q_cut, q_min, dk
                                                       !! The maximum value of r, the maximum and minimum
                                                       !! values of q and the k-space spacing of grid points.
                                                       !! Note that, during a vdW run, values of q0 found
                                                       !! larger than q_cut will be saturated (SOLER 6-7) to
                                                       !! q_cut
    REAL(KIND=dp),DIMENSION(:), POINTER    :: q_mesh   !! The values of all the q points used
    REAL(KIND=dp),DIMENSION(:,:,:), POINTER &
                                           :: kernel   !! A matrix holding the Fourier transformed kernel function
                                                       !! for each pair of q values.  The ordering is
                                                       !! kernel(k_point, q1_value, q2_value)
    REAL(KIND=dp),DIMENSION(:,:,:), POINTER &
                                           :: d2phi_dk2!! A matrix holding the second derivatives of the above
                                                       !! kernel matrix at each of the q points.  Stored as  
                                                       !! d2phi_dk2(k_point, q1_value, q2_value)
    REAL(KIND=dp),DIMENSION(:,:), POINTER  :: d2y_dx2  !! 2nd derivatives of q_mesh for interpolation
  END TYPE qs_dispersion_type

  TYPE qs_atom_dispersion_type
    INTEGER                                :: TYPE
    LOGICAL                                :: defined
    REAL(KIND=dp)                          :: vdw_radii  !van der Waals radii
    REAL(KIND=dp)                          :: c6         !c6 coefficients
  END TYPE qs_atom_dispersion_type

  TYPE cn_kind_list
    REAL(KIND=dp)                          :: cnum
    INTEGER                                :: kind
  END TYPE cn_kind_list
  TYPE cn_atom_list
    REAL(KIND=dp)                          :: cnum
    INTEGER                                :: natom
    INTEGER,DIMENSION(:), POINTER          :: atom
  END TYPE cn_atom_list

! *****************************************************************************

  PUBLIC :: qs_atom_dispersion_type, qs_dispersion_type, cn_atom_list, cn_kind_list
  PUBLIC :: dftd2_pp, dftd3_pp

! *****************************************************************************

END MODULE qs_dispersion_types

