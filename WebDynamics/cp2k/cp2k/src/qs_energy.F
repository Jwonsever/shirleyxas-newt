!-----------------------------------------------------------------------------!
!   CP2K: A general program to perform molecular dynamics simulations         !
!   Copyright (C) 2000 - 2013  CP2K developers group                          !
!-----------------------------------------------------------------------------!

! *****************************************************************************
!> \brief Perform a QUICKSTEP wavefunction optimization (single point)
!> \par History
!>      none
!> \author MK (29.10.2002)
! *****************************************************************************
MODULE qs_energy
  USE cp_control_types,                ONLY: dft_control_type
  USE cp_dbcsr_interface,              ONLY: cp_dbcsr_copy,&
                                             cp_dbcsr_init,&
                                             cp_dbcsr_set
  USE cp_dbcsr_operations,             ONLY: cp_dbcsr_allocate_matrix_set
  USE cp_dbcsr_types,                  ONLY: cp_dbcsr_p_type
  USE cp_fm_types,                     ONLY: cp_fm_p_type,&
                                             cp_fm_to_fm
  USE cp_para_types,                   ONLY: cp_para_env_type
  USE dm_ls_scf,                       ONLY: ls_scf
  USE f77_blas
  USE input_constants,                 ONLY: do_diag,&
                                             ehrenfest
  USE kinds,                           ONLY: dp
  USE mp2
  USE qs_energy_utils,                 ONLY: qs_energies_compute_matrix_w,&
                                             qs_energies_init,&
                                             qs_energies_properties
  USE qs_environment_types,            ONLY: get_qs_env,&
                                             qs_environment_type,&
                                             set_qs_env
  USE qs_ks_methods,                   ONLY: qs_ks_update_qs_env
  USE qs_ks_scp_methods,               ONLY: qs_ks_scp_update
  USE qs_ks_types,                     ONLY: qs_ks_env_type
  USE qs_scf,                          ONLY: scf
  USE rt_propagation_methods,          ONLY: propagation_step,&
                                             put_data_to_history,&
                                             s_matrices_create
  USE rt_propagation_output,           ONLY: rt_prop_output
  USE rt_propagation_types,            ONLY: get_rtp,&
                                             rt_prop_type
  USE rt_propagation_utils,            ONLY: calc_S_derivs
  USE rt_propagator_init,              ONLY: init_emd_propagators
  USE timings,                         ONLY: timeset,&
                                             timestop
#include "cp_common_uses.h"

  IMPLICIT NONE

  PRIVATE

! *** Global parameters ***

  CHARACTER(len=*), PARAMETER, PRIVATE :: moduleN = 'qs_energy'

  PUBLIC :: qs_energies

CONTAINS

! *****************************************************************************
!> \brief   Driver routine for QUICKSTEP single point wavefunction optimization.
!> \author  MK
!> \date    29.10.2002
!> \par History
!>          - consistent_energies option added (25.08.2005, TdK)
!>          - introduced driver for energy in order to properly decide between
!>            SCF or RTP (fschiff 02.09)
!> \version 1.0
! *****************************************************************************
  SUBROUTINE qs_energies (qs_env, consistent_energies, calc_forces, &
                          error)
    TYPE(qs_environment_type), POINTER       :: qs_env
    LOGICAL, INTENT(IN), OPTIONAL            :: consistent_energies, &
                                                calc_forces
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'qs_energies', &
      routineP = moduleN//':'//routineN

    LOGICAL                                  :: run_rtp

    CALL get_qs_env(qs_env=qs_env,run_rtp=run_rtp,error=error)
    IF(run_rtp)THEN
       CALL qs_energies_rtp(qs_env,error=error)
    ELSE
       CALL qs_energies_scf(qs_env, consistent_energies, calc_forces, &
                          error)
    END IF

  END SUBROUTINE qs_energies


! *****************************************************************************
!> \brief   QUICKSTEP single point wavefunction optimization.
!> \author  MK
!> \date    29.10.2002
!> \par History
!>          - consistent_energies option added (25.08.2005, TdK)
!> \version 1.0
! *****************************************************************************
  SUBROUTINE qs_energies_scf (qs_env, consistent_energies, calc_forces, &
                          error)
    TYPE(qs_environment_type), POINTER       :: qs_env
    LOGICAL, INTENT(IN), OPTIONAL            :: consistent_energies
    LOGICAL, INTENT(IN)                      :: calc_forces
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'qs_energies_scf', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: handle, ispin
    LOGICAL                                  :: has_unit_metric
    TYPE(cp_dbcsr_p_type), DIMENSION(:), &
      POINTER                                :: matrix_s, matrix_w
    TYPE(cp_para_env_type), POINTER          :: para_env
    TYPE(dft_control_type), POINTER          :: dft_control
    TYPE(qs_ks_env_type), POINTER            :: ks_env

    CALL timeset(routineN,handle)

    para_env=>qs_env%para_env

    CALL qs_energies_init(qs_env, calc_forces, error)

    CALL get_qs_env(qs_env=qs_env, matrix_s=matrix_s,&
         dft_control=dft_control, has_unit_metric=has_unit_metric, error=error)

    IF (calc_forces.AND..NOT.has_unit_metric) THEN
       IF (.NOT.has_unit_metric) THEN
          CALL cp_dbcsr_allocate_matrix_set(matrix_w,dft_control%nspins,error=error)
          DO ispin=1,dft_control%nspins
             ALLOCATE(matrix_w(ispin)%matrix)
             CALL cp_dbcsr_init(matrix_w(ispin)%matrix,error=error)
             CALL cp_dbcsr_copy(matrix_w(ispin)%matrix,matrix_s(1)%matrix,&
                  name="W MATRIX",error=error)
             CALL cp_dbcsr_set(matrix_w(ispin)%matrix,0.0_dp,error=error)
          END DO
       END IF
       CALL set_qs_env(qs_env=qs_env,matrix_w=matrix_w,error=error)
    ENDIF

    ! *** Perform a SCF run ***
    IF (dft_control%qs_control%do_ls_scf) THEN
      CALL ls_scf(qs_env=qs_env, error=error)
    ELSE
      CALL scf(qs_env=qs_env, error=error)

      ! no forces so far with MP2 (except one electron systems ;-)
      IF (qs_env%energy%mp2.NE.0.0_dp .AND. calc_forces) THEN
         CALL cp_assert(.FALSE.,cp_failure_level,cp_assertion_failed,routineP,&
              " No forces/gradients with MP2.... yet(?!) "//&
               CPSourceFileRef, only_ionode=.TRUE.)
      ENDIF
       
      ! if calculate forces, time to compute the w matrix
      CALL qs_energies_compute_matrix_w(qs_env,calc_forces,error)

    END IF

    IF (PRESENT(consistent_energies)) THEN
      IF (consistent_energies) THEN
        IF ( dft_control%scp ) THEN
           CALL qs_ks_scp_update ( qs_env, just_energy=.TRUE., error=error )
        END IF
        CALL qs_ks_update_qs_env(ks_env=qs_env%ks_env, &
                                 qs_env=qs_env, &
                                 calculate_forces=.FALSE., &
                                 just_energy=.TRUE.,error=error)
      END IF
    END IF

    CALL qs_energies_properties(qs_env,error)

    CALL timestop(handle)

  END SUBROUTINE qs_energies_scf

! *****************************************************************************
!> \brief   energy calculation for real time propagation (iterated through integrator)
!> \author  Florian Schiffmann
!> \date    02.2009
!> \version 1.0
! *****************************************************************************
  SUBROUTINE qs_energies_rtp(qs_env,error)

    TYPE(qs_environment_type), POINTER       :: qs_env
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'qs_energies_rtp', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: aspc_order, handle, i, ihist
    LOGICAL                                  :: diagonalize
    REAL(Kind=dp)                            :: delta_iter, eps_ener
    TYPE(cp_dbcsr_p_type), DIMENSION(:), &
      POINTER                                :: s_mat
    TYPE(cp_fm_p_type), DIMENSION(:), &
      POINTER                                :: exp_H_new, exp_H_old, &
                                                mos_new, mos_old
    TYPE(rt_prop_type), POINTER              :: rtp

    CALL timeset(routineN,handle)

    eps_ener=qs_env%dft_control%rtp_control%eps_ener
    aspc_order=qs_env%dft_control%rtp_control%aspc_order
    IF(qs_env%dft_control%rtp_control%initial_step)THEN
       CALL init_emd_propagators(qs_env,error)
    ELSE
       IF(qs_env%rtp%iter==1)THEN
          CALL qs_energies_init(qs_env, .FALSE. , error)
       END IF
       CALL get_qs_env(qs_env=qs_env,rtp=rtp,matrix_s=s_mat,error=error)

       diagonalize=(qs_env%dft_control%rtp_control%mat_exp==do_diag)
       CALL s_matrices_create (s_mat,rtp,diagonalize,error)
       CALL calc_S_derivs(qs_env,error)

       CALL get_rtp(rtp=rtp,&
                 exp_H_old=exp_H_old,&
                 exp_H_new=exp_H_new,&
                 mos_old=mos_old,&
                 mos_new=mos_new,&
                 error=error)

       eps_ener=qs_env%dft_control%rtp_control%eps_ener

       CALL propagation_step(qs_env,delta_iter, error)
       rtp%energy_new=qs_env%energy%total
       rtp%converged=(delta_iter.LT.eps_ener)

       IF(rtp%converged)THEN
          ihist=MOD(rtp%istep,aspc_order)+1
          DO i=1,SIZE(mos_new)
             CALL cp_fm_to_fm(mos_new(i)%matrix,mos_old(i)%matrix,error)
             CALL cp_fm_to_fm(exp_H_new(i)%matrix,exp_H_old(i)%matrix,error)
          END DO
          IF(rtp%history%mos_or_H==2)&
               CALL  put_data_to_history(qs_env,exp_H=exp_H_new,ihist=ihist,error=error)
          IF(rtp%history%mos_or_H==1)&
               CALL  put_data_to_history(qs_env,mos=mos_new,s_mat=s_mat,ihist=ihist,error=error)
       END IF
       CALL rt_prop_output(qs_env,ehrenfest,delta_iter=delta_iter,error=error)
       rtp%energy_old=rtp%energy_new
    END IF
    CALL timestop(handle)
  END SUBROUTINE qs_energies_rtp

END MODULE qs_energy
