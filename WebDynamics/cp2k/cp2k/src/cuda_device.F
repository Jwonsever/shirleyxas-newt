MODULE cuda_device

  USE cp_error_handling,               ONLY: cp_error_type
  USE dbcsr_message_passing,           ONLY: mp_environ
  USE ma_affinity,                     ONLY: ma_set_gpu_affinity
  USE machine_architecture_types,      ONLY: has_ma

  IMPLICIT NONE

  PUBLIC :: cuda_device_init
  PUBLIC :: cuda_device_release

  PRIVATE

  LOGICAL, SAVE :: is_configured = .FALSE.
  CHARACTER(len=*), PARAMETER, PRIVATE :: moduleN = 'cuda_device'

#if defined (__PW_CUDA) && !defined (__DBCSR_CUDA)
  INTERFACE
     FUNCTION cuda_set_device_cu (device_id) RESULT (istat) &
          BIND(C, name="dc_set_device_cu")
       USE ISO_C_BINDING
    INTEGER(KIND=C_INT), INTENT(IN), VALUE   :: device_id
    INTEGER(KIND=C_INT)                      :: istat

     END FUNCTION cuda_set_device_cu
  END INTERFACE

  INTERFACE
     FUNCTION cuda_get_ndevices_cu (n_devices) RESULT (istat) &
          BIND(C, name="dc_get_ndevices_cu")
       USE ISO_C_BINDING
    INTEGER(KIND=C_INT), INTENT(OUT)         :: n_devices
    INTEGER(KIND=C_INT)                      :: istat

     END FUNCTION cuda_get_ndevices_cu
  END INTERFACE

  INTERFACE
     FUNCTION cuda_device_sync_cu() RESULT (istat) &
          BIND(C, name="dc_device_sync_cu")
       USE ISO_C_BINDING
    INTEGER(KIND=C_INT)                      :: istat

     END FUNCTION cuda_device_sync_cu
  END INTERFACE

  INTERFACE
     FUNCTION cuda_device_reset_cu() RESULT (istat) &
          BIND(C, name="dc_device_reset_cu")
       USE ISO_C_BINDING
    INTEGER(KIND=C_INT)                      :: istat

     END FUNCTION cuda_device_reset_cu
  END INTERFACE
#endif

CONTAINS
! *****************************************************************************
!> \brief Initialize the CUDA devices
!>
!> Associates the CUDA devices to the processes for use.
!> \param[in,out] error     error
! *****************************************************************************
  SUBROUTINE cuda_device_init (group, error)
    INTEGER, INTENT(IN)                 :: group
    TYPE(cp_error_type), INTENT(INOUT)  :: error

    CHARACTER(len=*), PARAMETER :: routineN = 'cuda_device_init', &
      routineP = moduleN//':'//routineN

    INTEGER :: mynode, ngpus, numnode, card_num, cerror

!   ---------------------------------------------------------------------------

#if defined (__PW_CUDA) && !defined (__DBCSR_CUDA)
    IF (is_configured) RETURN

    CALL mp_environ (numnode, mynode, group)
    cerror = cuda_get_ndevices_cu (ngpus)
    IF (cerror /= 0) STOP 'Error(1) in cuda_device_init'

    !$OMP PARALLEL default (none), &
    !$OMP          private (cerror), &
    !$OMP          shared (mynode, ngpus, card_num)
    card_num=MOD(mynode, ngpus)
    IF (has_ma) card_num=ma_set_gpu_affinity(mynode)

    cerror = cuda_set_device_cu (card_num)
    IF (cerror /= 0) STOP 'Error(2) in cuda_device_init'
    !$OMP END PARALLEL

    is_configured = .TRUE.
#endif
  END SUBROUTINE cuda_device_init


! *****************************************************************************
!> \brief Release the CUDA devices
!>
!> Release CUDA device assignment.
!> \param[in,out] error     error
! *****************************************************************************
  SUBROUTINE cuda_device_release (error)
    TYPE(cp_error_type), INTENT(INOUT)  :: error

    CHARACTER(len=*), PARAMETER  :: routineN = 'cuda_device_release', &
      routineP = moduleN//':'//routineN

    INTEGER :: cerror

#if defined (__PW_CUDA) && !defined (__DBCSR_CUDA)
    IF (.NOT. is_configured) RETURN

    cerror = cuda_device_sync_cu ()
    IF (cerror /= 0) STOP 'Error(1) in cuda_device_release'
    cerror = cuda_device_reset_cu ()
    IF (cerror /= 0) STOP 'Error(2) in cuda_device_release'

    is_configured = .FALSE.
#endif
  END SUBROUTINE cuda_device_release

END MODULE cuda_device
