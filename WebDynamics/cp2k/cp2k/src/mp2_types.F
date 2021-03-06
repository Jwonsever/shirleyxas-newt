!-----------------------------------------------------------------------------!
!   CP2K: A general program to perform molecular dynamics simulations         !
!   Copyright (C) 2000 - 2013  CP2K developers group                          !
!-----------------------------------------------------------------------------!

! *****************************************************************************
!> \brief Types needed for MP2 calculations
!> \par History
!>       2011.05 created [Mauro Del Ben]
!> \author MDB
! *****************************************************************************
MODULE mp2_types
  USE hfx_types,                       ONLY: hfx_release,&
                                             hfx_type,&
                                             pair_list_element_type
  USE input_constants,                 ONLY: mp2_method_direct,&
                                             mp2_method_gpw,&
                                             mp2_method_laplace,&
                                             mp2_method_none,&
                                             ri_mp2_laplace,&
                                             ri_mp2_method_gpw,&
                                             ri_rpa_method_gpw
  USE kinds,                           ONLY: default_path_length,&
                                             dp
  USE timings,                         ONLY: timeset,&
                                             timestop
#include "cp_common_uses.h"

  IMPLICIT NONE

  PRIVATE

  CHARACTER(len=*), PARAMETER, PRIVATE :: moduleN = 'mp2_types'

  PUBLIC :: mp2_type, mp2_direct_type, &
            integ_mat_buffer_type,&
            mp2_method_none,&
            mp2_method_laplace,&
            mp2_method_direct,&
            mp2_method_gpw,&
            ri_mp2_method_gpw,&
            ri_rpa_method_gpw,&
            ri_mp2_laplace,&
            init_TShPSC_lmax

  PUBLIC :: mp2_env_create, mp2_env_release,mp2_biel_type,&
            pair_list_type_mp2,&
            mp2_potential_type, mp2_gpw_type, ri_rpa_type, ri_mp2_type,&
            mp2_laplace_type

  INTEGER, SAVE                                         :: init_TShPSC_lmax = -1

! TYPE definitions

  TYPE mp2_biel_type
    INTEGER, DIMENSION(:,:), ALLOCATABLE :: index_table
  END TYPE mp2_biel_type

  TYPE mp2_laplace_type
       INTEGER       :: n_quadrature
       INTEGER       :: integ_group_size
       LOGICAL       :: mo_sos
       REAL(dp)      :: threshold
  END TYPE

  TYPE mp2_direct_type
    LOGICAL  :: big_send
  END TYPE

  TYPE mp2_potential_type
    INTEGER                                  :: potential_type     !! 1/r, TShPSC
    REAL(dp)                                 :: truncation_radius  !! truncation radius if cutoff potential in use
    CHARACTER(default_path_length)           :: filename
  END TYPE

  TYPE mp2_gpw_type
       REAL(KIND=dp)            :: eps_grid, eps_filter
       INTEGER                  :: print_level
       REAL(KIND=dp)            :: cutoff
       REAL(KIND=dp)            :: relative_cutoff
  END TYPE mp2_gpw_type

  TYPE ri_mp2_type
       INTEGER                  :: block_size
  END TYPE

  TYPE ri_rpa_type
       INTEGER                  :: rpa_num_quad_points
       INTEGER                  :: rpa_integ_group_size
       TYPE(hfx_type),DIMENSION(:,:), POINTER     &
                                :: x_data
  END TYPE

  TYPE mp2_type
       REAL(KIND=dp)            :: energy
       LOGICAL                  :: compute_reference
       INTEGER                  :: method
       TYPE(mp2_laplace_type)   :: ri_laplace
       TYPE(mp2_direct_type)    :: direct_canonical
       TYPE(mp2_potential_type) :: potential_parameter
       TYPE(mp2_gpw_type)       :: mp2_gpw
       TYPE(ri_mp2_type)        :: ri_mp2
       TYPE(ri_rpa_type)        :: ri_rpa
       REAL(dp) :: mp2_memory
       REAL(dp) :: scale_S
       REAL(dp) :: scale_T
       INTEGER  :: mp2_num_proc
       INTEGER  :: block_size_row
       INTEGER  :: block_size_col
       LOGICAL  :: calc_PQ_cond_num
  END TYPE

  TYPE integ_mat_buffer_type
    REAL(KIND=dp), DIMENSION(:), POINTER  :: msg
    INTEGER :: proc
    INTEGER :: msg_req
  END TYPE

  TYPE pair_list_type_mp2
    TYPE(pair_list_element_type), DIMENSION(:), ALLOCATABLE :: elements
    INTEGER :: n_element
  END TYPE pair_list_type_mp2

CONTAINS

  SUBROUTINE mp2_env_release(mp2_env,error)
    TYPE(mp2_type), POINTER                  :: mp2_env
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(LEN=*), PARAMETER :: routineN = 'mp2_env_release', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: handle, stat
    LOGICAL                                  :: failure

    failure=.FALSE.

    CALL timeset(routineN,handle)

    CPPostcondition(ASSOCIATED(mp2_env),cp_failure_level,routineP,error,failure)

    ! release the HFX section for the EXX calculation
    IF(ASSOCIATED(mp2_env%ri_rpa%x_data)) THEN
      CALL hfx_release(mp2_env%ri_rpa%x_data, error=error)
    END IF

    DEALLOCATE(mp2_env,STAT=stat)
    CPPostcondition(stat==0,cp_failure_level,routineP,error,failure)

    CALL timestop(handle)

  END SUBROUTINE mp2_env_release

  SUBROUTINE mp2_env_create(mp2_env,error)
    TYPE(mp2_type), POINTER                  :: mp2_env
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(LEN=*), PARAMETER :: routineN = 'mp2_env_create', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: handle, stat
    LOGICAL                                  :: failure

    failure=.FALSE.

    CALL timeset(routineN,handle)

    CPPostcondition(.NOT.ASSOCIATED(mp2_env),cp_failure_level,routineP,error,failure)

    ALLOCATE(mp2_env,STAT=stat)
    CPPostcondition(stat==0,cp_failure_level,routineP,error,failure)

    NULLIFY(mp2_env%ri_rpa%x_data)

    CALL timestop(handle)

  END SUBROUTINE mp2_env_create


END MODULE mp2_types


