!-----------------------------------------------------------------------------!
!   CP2K: A general program to perform molecular dynamics simulations         !
!   Copyright (C) 2000 - 2013  CP2K developers group                          !
!-----------------------------------------------------------------------------!

! *****************************************************************************
!> \brief Utility subroutine for qs energy calculation
!> \par History
!>      none
!> \author MK (29.10.2002)
! *****************************************************************************
MODULE qs_energy_utils
  USE atprop_types,                    ONLY: atprop_array_add,&
                                             atprop_type
  USE cp_control_types,                ONLY: dft_control_type
  USE cp_control_utils,                ONLY: read_becke_section,&
                                             read_ddapc_section
  USE cp_dbcsr_interface,              ONLY: cp_dbcsr_get_block_p,&
                                             cp_dbcsr_get_info,&
                                             cp_dbcsr_iterator_blocks_left,&
                                             cp_dbcsr_iterator_next_block,&
                                             cp_dbcsr_iterator_start,&
                                             cp_dbcsr_iterator_stop,&
                                             cp_dbcsr_set
  USE cp_dbcsr_types,                  ONLY: cp_dbcsr_iterator,&
                                             cp_dbcsr_p_type,&
                                             cp_dbcsr_type
  USE cp_para_types,                   ONLY: cp_para_env_type
  USE efield_utils,                    ONLY: calculate_ecore_efield
  USE et_coupling,                     ONLY: calc_et_coupling
  USE f77_blas
  USE input_section_types,             ONLY: section_vals_get_subs_vals,&
                                             section_vals_type
  USE kg_environment,                  ONLY: kg_build_neighborlist,&
                                             kg_build_subsets
  USE kinds,                           ONLY: dp
  USE molecule_types_new,              ONLY: molecule_of_atom,&
                                             molecule_type
  USE mp2
  USE pw_env_types,                    ONLY: pw_env_get
  USE pw_pool_types,                   ONLY: pw_pool_give_back_pw,&
                                             pw_pool_type
  USE qs_core_energies,                ONLY: calculate_ecore_overlap,&
                                             calculate_ecore_self
  USE qs_core_hamiltonian,             ONLY: build_core_hamiltonian_matrix
  USE qs_dftb_dispersion,              ONLY: calculate_dftb_dispersion
  USE qs_dftb_matrices,                ONLY: build_dftb_matrices
  USE qs_dispersion_pairpot,           ONLY: calculate_dispersion_pairpot
  USE qs_environment_methods,          ONLY: qs_env_update_s_mstruct
  USE qs_environment_types,            ONLY: get_qs_env,&
                                             qs_environment_type
  USE qs_external_potential,           ONLY: external_c_potential,&
                                             external_e_potential
  USE qs_integrate_potential,          ONLY: integrate_v_core_rspace
  USE qs_ks_methods,                   ONLY: calculate_w_matrix,&
                                             calculate_w_matrix_ot,&
                                             qs_ks_allocate_basics,&
                                             qs_ks_update_qs_env
  USE qs_ks_types,                     ONLY: qs_ks_env_type
  USE qs_mo_types,                     ONLY: mo_set_p_type,&
                                             mo_set_type
  USE qs_neighbor_lists,               ONLY: build_qs_neighbor_lists
  USE qs_rho_types,                    ONLY: qs_rho_type
  USE qs_scf,                          ONLY: scf
  USE scf_control_types,               ONLY: scf_control_type
  USE scp_dispersion,                  ONLY: scp_nddo_dispersion,&
                                             scp_qs_dispersion
  USE scptb_core_interactions,         ONLY: scptb_core_interaction
  USE scptb_core_matrix,               ONLY: build_scptb_core_matrix
  USE se_core_core,                    ONLY: se_core_core_interaction
  USE se_core_matrix,                  ONLY: build_se_core_matrix
  USE se_ga_tools,                     ONLY: se_ga_initialize
  USE timings,                         ONLY: timeset,&
                                             timestop
  USE xas_methods,                     ONLY: xas
#include "cp_common_uses.h"

  IMPLICIT NONE

  PRIVATE

! *** Global parameters ***

  CHARACTER(len=*), PARAMETER, PRIVATE :: moduleN = 'qs_energy_utils'

  PUBLIC :: qs_energies_init,&
            qs_energies_compute_matrix_w,&
            qs_energies_properties

CONTAINS

! *****************************************************************************
!> \brief Refactoring of qs_energies_scf. Driver routine for the initial 
!>        setup and calculations for a qs energy calculation
!> \par History
!>      05.2013 created [Florian Schiffmann]
! *****************************************************************************

  SUBROUTINE qs_energies_init(qs_env, calc_forces, error)
    TYPE(qs_environment_type), POINTER       :: qs_env
    LOGICAL, INTENT(IN)                      :: calc_forces
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'qs_energies_init', &
      routineP = moduleN//':'//routineN

    LOGICAL                                  :: molecule_only

    CALL qs_energies_init_kg(qs_env,molecule_only,error)

    CALL qs_energies_init_hamiltonians(qs_env,calc_forces,molecule_only,error)

    CALL qs_ks_allocate_basics(qs_env,error)

  END SUBROUTINE qs_energies_init

! *****************************************************************************
!> \brief Refactoring of qs_energies_scf. Puts initialization of the Kim-Gordon
!>        settings into separate subroutine
!> \par History
!>      05.2013 created [Florian Schiffmann]
! *****************************************************************************

  SUBROUTINE qs_energies_init_kg(qs_env,molecule_only,error)
    TYPE(qs_environment_type), POINTER       :: qs_env
    LOGICAL                                  :: molecule_only
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'qs_energies_init_kg', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: handle, isubset, natom
    TYPE(molecule_type), DIMENSION(:), &
      POINTER                                :: molecule_set

    CALL timeset(routineN,handle)

    molecule_only = .FALSE.

    IF (qs_env%dft_control%qs_control%do_kg) THEN

      ! create neighbor lists with molecular blocks
      molecule_only = .TRUE.

      ! get the set of molecules
      CALL get_qs_env(qs_env=qs_env, molecule_set=molecule_set, natom=natom, error=error)

      qs_env%kg_env%natom = natom

      ! store set of molecules in kg_env
      qs_env%kg_env%molecule_set => molecule_set

      ! build the (new) full neighborlist
      CALL kg_build_neighborlist(qs_env%kg_env, qs_env, qs_env%kg_env%sab_orb_full, error=error)

      ! allocate the subset list
      IF (.NOT.ASSOCIATED(qs_env%kg_env%subset_of_mol)) ALLOCATE(qs_env%kg_env%subset_of_mol(SIZE(molecule_set)))

      IF (.NOT.ALLOCATED(qs_env%kg_env%atom_to_molecule)) THEN
        ALLOCATE(qs_env%kg_env%atom_to_molecule(natom))

        ! get the mapping from atoms to molecules
        CALL molecule_of_atom(molecule_set, atom_to_mol=qs_env%kg_env%atom_to_molecule, error=error)
      END IF

      CALL kg_build_subsets(qs_env%kg_env, qs_env%para_env, error)

      DO isubset=1,qs_env%kg_env%nsubsets

        ! build the (new) molecular neighborlist of the current subset
        CALL kg_build_neighborlist(qs_env%kg_env, qs_env, &
                                   qs_env%kg_env%subset(isubset)%sab_orb, molecular=.TRUE., &
                                   subset_of_mol=qs_env%kg_env%subset_of_mol, current_subset=isubset, error=error)
      END DO

    END IF
    CALL timestop(handle)

  END SUBROUTINE qs_energies_init_kg

! *****************************************************************************
!> \brief Refactoring of qs_energies_scf. Moves computation of the different
!>        core hamiltonians into separate subroutine
!> \par History
!>      05.2013 created [Florian Schiffmann]
! *****************************************************************************

  SUBROUTINE qs_energies_init_hamiltonians(qs_env,calc_forces,molecule_only,error)
    TYPE(qs_environment_type), POINTER       :: qs_env
    LOGICAL, INTENT(IN)                      :: calc_forces
    LOGICAL                                  :: molecule_only
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: &
      routineN = 'qs_energies_init_hamiltonians', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: handle
    TYPE(cp_para_env_type), POINTER          :: para_env
    TYPE(dft_control_type), POINTER          :: dft_control
    TYPE(section_vals_type), POINTER         :: input

    CALL timeset(routineN,handle)

    para_env=>qs_env%para_env
    CALL get_qs_env(qs_env=qs_env,input=input, dft_control=dft_control, error=error)

    ! create neighbor lists for standard use in QS
    CALL build_qs_neighbor_lists(qs_env,para_env,molecular=molecule_only,force_env_section=input,error=error)
    qs_env%dft_control%qs_control%becke_control%need_pot=.TRUE.

    ! *** Calculate the overlap and the core Hamiltonian integral matrix ***
    IF ( dft_control%qs_control%semi_empirical ) THEN
       CALL build_se_core_matrix(qs_env=qs_env, para_env=para_env,&
                                 calculate_forces=.FALSE.,error=error)
       CALL qs_env_update_s_mstruct(qs_env,error=error)
       CALL se_core_core_interaction(qs_env, para_env, calculate_forces=.FALSE., error=error)
       IF ( dft_control%qs_control%se_control%scp ) THEN
          CALL scp_nddo_dispersion (qs_env,calc_forces,error)
       END IF
    ! GA option
       CALL se_ga_initialize ( qs_env, error )
    ELSEIF ( dft_control%qs_control%dftb ) THEN
       CALL build_dftb_matrices(qs_env=qs_env, para_env=para_env,&
                              calculate_forces=.FALSE.,error=error)
       CALL calculate_dftb_dispersion(qs_env=qs_env, para_env=para_env,&
                              calculate_forces=.FALSE.,error=error)
       CALL qs_env_update_s_mstruct(qs_env,error=error)
    ELSEIF ( dft_control%qs_control%scptb ) THEN
       CALL build_scptb_core_matrix(qs_env=qs_env,calculate_forces=.FALSE.,error=error)
       CALL qs_env_update_s_mstruct(qs_env,error=error)
       CALL scptb_core_interaction(qs_env,calculate_forces=.FALSE.,error=error)
       CALL calculate_dispersion_pairpot(qs_env,calc_forces,error)
    ELSE
       CALL build_core_hamiltonian_matrix(qs_env=qs_env,calculate_forces=.FALSE.,error=error)

       CALL qs_env_update_s_mstruct(qs_env,error=error)
       CALL calculate_ecore_self(qs_env,error=error)
       CALL calculate_ecore_efield(qs_env,calculate_forces=.FALSE.,error=error)
       CALL calculate_ecore_overlap(qs_env, para_env, calculate_forces=.FALSE.,error=error)
       CALL external_c_potential(qs_env,calculate_forces=.FALSE.,error=error)
       CALL external_e_potential(qs_env,error=error)

       IF ( dft_control%scp ) THEN
          IF(dft_control%scp_control%dispersion ) THEN
             CALL scp_qs_dispersion ( qs_env, calc_forces, error )
          END IF
       ELSE
          ! Add possible pair potential dispersion energy - Evaluate first so we can print
          ! energy info at the end of the SCF
          CALL calculate_dispersion_pairpot(qs_env,calc_forces,error)
       END IF

    END IF

    CALL timestop(handle)

  END SUBROUTINE qs_energies_init_hamiltonians

! *****************************************************************************
!> \brief Refactoring of qs_energies_scf. Moves computation of matrix_w
!>        into separate subroutine
!> \par History
!>      05.2013 created [Florian Schiffmann]
! *****************************************************************************

  SUBROUTINE qs_energies_compute_matrix_w(qs_env,calc_forces,error)
    TYPE(qs_environment_type), POINTER       :: qs_env
    LOGICAL, INTENT(IN)                      :: calc_forces
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'qs_energies_compute_matrix_w', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: handle, ispin, nspin
    LOGICAL                                  :: has_unit_metric
    TYPE(cp_dbcsr_p_type), DIMENSION(:), &
      POINTER                                :: matrix_ks, matrix_s, &
                                                matrix_w, mo_derivs
    TYPE(mo_set_p_type), DIMENSION(:), &
      POINTER                                :: mos
    TYPE(mo_set_type), POINTER               :: mo_set
    TYPE(qs_ks_env_type), POINTER            :: ks_env
    TYPE(qs_rho_type), POINTER               :: rho
    TYPE(scf_control_type), POINTER          :: scf_control

    CALL timeset(routineN,handle)

    ! if calculate forces, time to compute the w matrix
    CALL get_qs_env(qs_env=qs_env,has_unit_metric=has_unit_metric,matrix_w=matrix_w,error=error)
    IF (calc_forces.AND..NOT.has_unit_metric) THEN
       CALL get_qs_env(qs_env=qs_env,&
            ks_env=ks_env,&
            matrix_ks=matrix_ks,&
            matrix_s=matrix_s,&
            mo_derivs=mo_derivs,&
            scf_control=scf_control,&
            mos=mos,&
            rho=rho,error=error)
       nspin = SIZE(mos)
       DO ispin=1,nspin
          mo_set => mos(ispin)%mo_set
          IF (qs_env%dft_control%roks) THEN
             IF (scf_control%use_ot) THEN
                IF (ispin > 1) THEN
                   ! not very elegant, indeed ...
                   CALL cp_dbcsr_set(matrix_w(ispin)%matrix,0.0_dp,error=error)
                ELSE
                   CALL calculate_w_matrix_ot(mo_set,mo_derivs(ispin)%matrix,&
                        matrix_w(ispin)%matrix,matrix_s(1)%matrix,error=error)
                END IF
             ELSE
                CALL calculate_w_matrix(mo_set=mo_set,&
                     matrix_ks=matrix_ks(ispin)%matrix,&
                     matrix_p=rho%rho_ao(ispin)%matrix,&
                     matrix_w=matrix_w(ispin)%matrix,&
                     error=error)
             END IF
          ELSE
             IF (scf_control%use_ot) THEN
                CALL calculate_w_matrix_ot(mo_set,mo_derivs(ispin)%matrix,&
                     matrix_w(ispin)%matrix,matrix_s(1)%matrix,error=error)
             ELSE
                CALL calculate_w_matrix(mo_set,matrix_w(ispin)%matrix,error=error)
             END IF
          END IF
       END DO
    END IF

    CALL timestop(handle)

  END SUBROUTINE qs_energies_compute_matrix_w

! *****************************************************************************
!> \brief Refactoring of qs_energies_scf. Moves computation of properties
!>        into separate subroutine
!> \par History
!>      05.2013 created [Florian Schiffmann]
! *****************************************************************************

  SUBROUTINE qs_energies_properties(qs_env,error)
    TYPE(qs_environment_type), POINTER       :: qs_env
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'qs_energies_properties', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: handle
    LOGICAL                                  :: do_et
    TYPE(atprop_type), POINTER               :: atprop
    TYPE(dft_control_type), POINTER          :: dft_control
    TYPE(pw_pool_type), POINTER              :: auxbas_pw_pool
    TYPE(qs_ks_env_type), POINTER            :: ks_env
    TYPE(section_vals_type), POINTER         :: input, rest_b_section

    CALL timeset(routineN,handle)

    ! atomic energies using Mulliken partition
    CALL get_qs_env(qs_env=qs_env,dft_control=dft_control,input=input,error=error)
    IF (qs_env%atprop%energy) THEN
       CALL get_qs_env(qs_env=qs_env,atprop=atprop,error=error)
       CALL qs_energies_mulliken(qs_env,error)
       IF ( .NOT.dft_control%qs_control%semi_empirical .AND. &
            .NOT.dft_control%qs_control%dftb) THEN
         ! Nuclear charge correction
         CALL integrate_v_core_rspace(qs_env%ks_env%v_hartree_rspace,qs_env,error=error)
         ! Kohn-Sham Functional corrections
       END IF
       CALL atprop_array_add(atprop%atener,atprop%ateb,error)
       CALL atprop_array_add(atprop%atener,atprop%ateself,error)
       CALL atprop_array_add(atprop%atener,atprop%atexc,error)
       CALL atprop_array_add(atprop%atener,atprop%atecoul,error)
       CALL atprop_array_add(atprop%atener,atprop%atevdw,error)
       CALL atprop_array_add(atprop%atener,atprop%atecc,error)
       CALL atprop_array_add(atprop%atener,atprop%ate1c,error)
    END IF

    ! **********  Calculate the electron transfer coupling elements********
    do_et=.FALSE.
    do_et=dft_control%qs_control%et_coupling_calc
    IF(do_et)THEN
       qs_env%et_coupling%energy=qs_env%energy%total
       qs_env%et_coupling%keep_matrix=.TRUE.
       qs_env%et_coupling%first_run=.TRUE.
       CALL qs_ks_update_qs_env(ks_env=qs_env%ks_env, &
                                qs_env=qs_env, &
                                calculate_forces=.FALSE., &
                                just_energy=.TRUE.,error=error)
       qs_env%et_coupling%first_run=.FALSE.
       IF(qs_env%dft_control%qs_control%ddapc_restraint)THEN
          rest_b_section =>  section_vals_get_subs_vals(input,"PROPERTIES%ET_COUPLING%DDAPC_RESTRAINT_B",&
                                                        error=error)
          CALL read_ddapc_section(qs_control=dft_control%qs_control,&
                                  ddapc_restraint_section=rest_b_section,error=error)
       END IF
       IF(qs_env%dft_control%qs_control%becke_restraint)THEN
          rest_b_section => section_vals_get_subs_vals(input,"PROPERTIES%ET_COUPLING%BECKE_RESTRAINT_B",&
                                                       error=error)
          CALL read_becke_section(qs_control=dft_control%qs_control,&
                                     becke_section=rest_b_section,error=error)
       END IF
       CALL scf(qs_env=qs_env, error=error)
       qs_env%et_coupling%keep_matrix=.TRUE.

       CALL qs_ks_update_qs_env(ks_env=qs_env%ks_env, &
                                qs_env=qs_env, &
                                calculate_forces=.FALSE., &
                                just_energy=.TRUE.,error=error)
       CALL calc_et_coupling(qs_env,error)
       IF(qs_env%dft_control%qs_control%becke_restraint)THEN
          CALL pw_env_get(qs_env%pw_env,auxbas_pw_pool=auxbas_pw_pool,error=error)
          CALL pw_pool_give_back_pw(auxbas_pw_pool,&
               qs_env%dft_control%qs_control%becke_control%becke_pot%pw,error=error)
          qs_env%dft_control%qs_control%becke_control%need_pot=.TRUE.
       END IF
    END IF

    !Properties
    IF(dft_control%do_xas_calculation) THEN
      CALL xas(qs_env, dft_control, error=error)
    END IF

    CALL timestop(handle)

  END SUBROUTINE qs_energies_properties 


! *****************************************************************************
!> \brief   Use a simple Mulliken-like energy decomposition
!> \author  JHU
!> \date    07.2011
!> \version 1.0
! *****************************************************************************
  SUBROUTINE qs_energies_mulliken(qs_env,error)

    TYPE(qs_environment_type), POINTER       :: qs_env
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'qs_energies_mulliken', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: ispin
    TYPE(atprop_type), POINTER               :: atprop
    TYPE(cp_dbcsr_p_type), DIMENSION(:), &
      POINTER                                :: matrix_h, matrix_ks
    TYPE(qs_rho_type), POINTER               :: rho

    IF(qs_env%atprop%energy) THEN
       CALL get_qs_env(qs_env=qs_env,matrix_ks=matrix_ks,matrix_h=matrix_h,&
                       rho=rho,atprop=atprop,error=error)

       ! E = 0.5*Tr(H*P+F*P)
       atprop%atener = 0._dp
       DO ispin = 1,SIZE(rho%rho_ao)
          CALL atom_trace(matrix_h(1)%matrix,rho%rho_ao(ispin)%matrix,&
               0.5_dp,atprop%atener,error)
          CALL atom_trace(matrix_ks(ispin)%matrix,rho%rho_ao(ispin)%matrix,&
               0.5_dp,atprop%atener,error)
       END DO

    END IF

  END SUBROUTINE qs_energies_mulliken

! *****************************************************************************
!> \brief Compute partial trace of product of two matrices
!> \param amat, bmat
!> \param charges previously allocated with the right size (natom,nspin)
!> \note
!>      charges are computed per spin in the LSD case
!> \par History
!>      06.2004 created [Joost VandeVondele]
! *****************************************************************************
  SUBROUTINE atom_trace(amat,bmat,factor,atrace,error)
    TYPE(cp_dbcsr_type), POINTER             :: amat, bmat
    REAL(kind=dp), INTENT(IN)                :: factor
    REAL(KIND=dp), DIMENSION(:), POINTER     :: atrace
    TYPE(cp_error_type), INTENT(inout)       :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'atom_trace', &
      routineP = moduleN//':'//routineN

    INTEGER                                  :: blk, iblock_col, iblock_row, &
                                                nblock
    LOGICAL                                  :: failure, found
    REAL(kind=dp)                            :: btr, mult
    REAL(KIND=dp), DIMENSION(:, :), POINTER  :: a_block, b_block
    TYPE(cp_dbcsr_iterator)                  :: iter

    CALL cp_dbcsr_get_info(bmat,nblkrows_total=nblock)
    CPPostcondition(nblock==SIZE(atrace),cp_warning_level,routineP,error,failure)

    CALL cp_dbcsr_iterator_start(iter, bmat)
    DO WHILE (cp_dbcsr_iterator_blocks_left(iter))
       CALL cp_dbcsr_iterator_next_block(iter, iblock_row, iblock_col, b_block, blk)
       CALL cp_dbcsr_get_block_p(matrix=amat,&
               row=iblock_row,col=iblock_col,BLOCK=a_block,found=found)

       ! we can cycle if a block is not present
       IF ( .NOT. (ASSOCIATED(b_block) .AND. ASSOCIATED(a_block) ) ) CYCLE

       IF (iblock_row.EQ.iblock_col) THEN
          mult=0.5_dp ! avoid double counting of diagonal blocks
       ELSE
          mult=1.0_dp
       ENDIF
       btr = factor*mult*SUM(a_block*b_block)
       atrace(iblock_row)=atrace(iblock_row) + btr
       atrace(iblock_col)=atrace(iblock_col) + btr

    ENDDO
    CALL cp_dbcsr_iterator_stop(iter)

  END SUBROUTINE atom_trace

END MODULE qs_energy_utils
