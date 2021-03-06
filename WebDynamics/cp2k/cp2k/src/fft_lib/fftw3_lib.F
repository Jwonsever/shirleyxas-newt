!-----------------------------------------------------------------------------!
!   CP2K: A general program to perform molecular dynamics simulations         !
!   Copyright (C) 2000 - 2013  CP2K developers group                          !
!-----------------------------------------------------------------------------!

#if defined(__HAS_NO_OMP_3)
#define __COLLAPSE3
#else
#define __COLLAPSE3 collapse(3)
#endif

#define XFFTW_PLAN_WITH_NTHREADS dfftw_plan_with_nthreads
#define XFFTW_PLAN_DFT_3D dfftw_plan_dft_3d
#define XFFTW_PLAN_GURU_DFT dfftw_plan_guru_dft
#define XFFTW_EXECUTE_DFT dfftw_execute_dft
#define XFFTW_DESTROY_PLAN dfftw_destroy_plan


! *****************************************************************************
!> \par History
!>      JGH 23-Jan-2006 : initial version
!>      Adapted for new interface
!>      IAB 09-Jan-2009 : Modified to cache plans in fft_plan_type
!>                        (c) The Numerical Algorithms Group (NAG) Ltd, 2009 on behalf of the HECToR project
!>      IAB 09-Oct-2009 : Added OpenMP directives to 1D FFT, and planning routines
!>                        (c) The Numerical Algorithms Group (NAG) Ltd, 2009 on behalf of the HECToR project
!>      IAB 11-Sep-2012 : OpenMP parallel 3D FFT (Ruyman Reyes, PRACE)
!> \author JGH
! *****************************************************************************
SUBROUTINE fftw3_get_lengths ( DATA, max_length )

  IMPLICIT NONE

  INTEGER, DIMENSION(*)                    :: DATA
  INTEGER, INTENT(INOUT)                   :: max_length

  INTEGER, DIMENSION(:), ALLOCATABLE       :: dlocal, idx
  INTEGER                                  :: h, i, j, k, m, maxn, &
                                              maxn_elevens, maxn_fives, &
                                              maxn_sevens, maxn_thirteens, &
                                              maxn_threes, maxn_twos, &
                                              ndata, nmax, number

!------------------------------------------------------------------------------
! compute ndata
!! FFTW can do arbitrary(?) lengths, maybe you want to limit them to some
!!    powers of small prime numbers though...

  maxn_twos = 15
  maxn_threes = 3
  maxn_fives = 2
  maxn_sevens = 1
  maxn_elevens = 1
  maxn_thirteens = 0
  maxn = 37748736

  ndata = 0
  DO h = 0, maxn_twos
     nmax = HUGE(0) / 2**h
     DO i = 0, maxn_threes
        DO j = 0, maxn_fives
           DO k = 0, maxn_sevens
              DO m = 0, maxn_elevens
                 number = (3**i) * (5**j) * (7**k) * (11**m)

                 IF ( number > nmax ) CYCLE

                 number = number * 2 ** h
                 IF ( number >= maxn ) CYCLE

                 ndata = ndata + 1
              END DO
           END DO
        END DO
     END DO
  END DO

  ALLOCATE ( dlocal ( ndata ), idx ( ndata ) )

  ndata = 0
  dlocal ( : ) = 0
  DO h = 0, maxn_twos
     nmax = HUGE(0) / 2**h
     DO i = 0, maxn_threes
        DO j = 0, maxn_fives
           DO k = 0, maxn_sevens
              DO m = 0, maxn_elevens
                 number = (3**i) * (5**j) * (7**k) * (11**m)

                 IF ( number > nmax ) CYCLE

                 number = number * 2 ** h
                 IF ( number >= maxn ) CYCLE

                 ndata = ndata + 1
                 dlocal ( ndata ) = number
              END DO
           END DO
        END DO
     END DO
  END DO

  CALL sortint ( dlocal, ndata, idx )
  ndata = MIN ( ndata, max_length )
  DATA(1:ndata) = dlocal(1:ndata)
  max_length = ndata

  DEALLOCATE ( dlocal, idx )

END SUBROUTINE fftw3_get_lengths

! *****************************************************************************

SUBROUTINE fftw3_create_guru_plan(plan, fft_rank, dim_n, &
                                  dim_istride, dim_ostride, hm_rank, &
                                  hm_n, hm_istride, hm_ostride, &
                                  zin, zout, fft_direction, fftw_plan_type, &
                                  valid)

  USE fft_kinds,                          ONLY : dp, integer8_kind

  IMPLICIT NONE

  INTEGER(KIND=integer8_kind), INTENT ( INOUT )      :: plan
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(IN)         :: zin
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(IN)         :: zout
  INTEGER, INTENT(IN) :: dim_n(2), dim_istride(2), dim_ostride(2), &
             hm_n(2), hm_istride(2), hm_ostride(2), fft_rank, &
             fft_direction, fftw_plan_type, hm_rank
  LOGICAL, INTENT(OUT)                               :: valid

#if defined (__FFTW3)

  CALL XFFTW_PLAN_GURU_DFT(plan,fft_rank, &
                                 dim_n,dim_istride,dim_ostride, &
                                 hm_rank,hm_n,hm_istride,hm_ostride, &
                                 zin, zout, &
                                 fft_direction, fftw_plan_type)

  IF (plan .EQ. 0) THEN
     valid = .FALSE.
  ELSE
     valid = .TRUE.
  ENDIF

#else
  
  valid = .FALSE. 

#endif

END SUBROUTINE

! *****************************************************************************

FUNCTION fftw3_is_mkl_wrapper() RESULT(is_mkl)

  USE fft_kinds,                          ONLY : dp, integer8_kind

  IMPLICIT NONE

  LOGICAL :: is_mkl
  LOGICAL :: guru_supported
  INTEGER :: dim_n(2), dim_istride(2), dim_ostride(2), &
             howmany_n(2), howmany_istride(2), howmany_ostride(2)
  INTEGER (KIND=integer8_kind) :: test_plan
  COMPLEX(KIND=dp), DIMENSION(1,1,1) :: zin

#if defined ( __FFTW3 )
#include "fftw3.f"

  ! Attempt to create a plan with the guru interface for a 2d sub-space
  ! If this fails (e.g. for MKL's FFTW3 interface), fall back to the
  ! FFTW3 threaded 3D transform instead of the hand-optimised version
  dim_n(1) = 1
  dim_istride(1) = 1
  dim_ostride(1) = 1
  howmany_n(1) = 1
  howmany_n(2) = 1
  howmany_istride(1) = 1
  howmany_istride(2) = 1
  howmany_ostride(1) = 1
  howmany_ostride(2) = 1
  CALL fftw3_create_guru_plan(test_plan,1, &
                              dim_n,dim_istride,dim_ostride, &
                              2,howmany_n,howmany_istride,howmany_ostride, &
                              zin, zin, &
                              FFTW_FORWARD, FFTW_ESTIMATE, guru_supported)
  IF (guru_supported) THEN
    CALL XFFTW_DESTROY_PLAN(test_plan)
    is_mkl = .FALSE.
  ELSE
    is_mkl = .TRUE.
  ENDIF

#else
  is_mkl = .FALSE.
#endif 

END FUNCTION

! *****************************************************************************

SUBROUTINE fftw3_compute_rows_per_th(nrows,nt,rows_per_thread,rows_per_thread_r,&
                                     remaining,th_planA, th_planB)

         IMPLICIT NONE
         INTEGER, INTENT(IN) :: nrows,nt 
         INTEGER, INTENT(OUT)    :: rows_per_thread,rows_per_thread_r 
         INTEGER, INTENT(OUT)    :: remaining, th_planA, th_planB

         IF (MOD(nrows,nt) .EQ. 0) THEN
            rows_per_thread = nrows/nt
            rows_per_thread_r = 0
            remaining = 0  
            th_planA = nt 
            th_planB = 0
         ELSE
            rows_per_thread   = nrows/nt + 1
            rows_per_thread_r = nrows/nt
            th_planA = MOD(nrows,nt)
            remaining = MOD(nrows,nt) 
            th_planB = nt - th_planA
         ENDIF

END SUBROUTINE


! *****************************************************************************

SUBROUTINE fftw3_create_3d_plans(plan, plan_r, dim_n, dim_istride, dim_ostride, &
                                 hm_n, hm_istride, hm_ostride, &
                                 input, output, &
                                 fft_direction, fftw_plan_type, rows_per_th, & 
                                 rows_per_th_r) 
  USE fft_kinds,                          ONLY : dp, integer8_kind

  IMPLICIT NONE

  INTEGER(KIND=integer8_kind), INTENT ( INOUT )         :: plan, plan_r
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(INOUT)         :: input
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(INOUT)         :: output
  INTEGER, INTENT(INOUT) :: dim_n(2), dim_istride(2), dim_ostride(2), &
             hm_n(2), hm_istride(2), hm_ostride(2), &
             fft_direction, fftw_plan_type
  INTEGER, INTENT(IN)                    :: rows_per_th
  INTEGER, INTENT(IN)                    :: rows_per_th_r
  LOGICAL                                :: valid

    ! First remaining_rows plans will have an additional row
    hm_n(2) = rows_per_th
    CALL fftw3_create_guru_plan(plan,1, &
                                 dim_n,dim_istride,dim_ostride, &
                                 2,hm_n,hm_istride,hm_ostride, &
                                 input, output, &
                                 fft_direction, fftw_plan_type, valid)

    IF (.NOT. valid) THEN
         STOP "fftw3_create_plan"
    ENDIF

    !!!! Remainder
    hm_n(2) = rows_per_th_r
    CALL fftw3_create_guru_plan(plan_r,1, &
                                 dim_n,dim_istride,dim_ostride, &
                                 2,hm_n,hm_istride,hm_ostride, &
                                 input, output, &
                                 fft_direction, fftw_plan_type, valid)
    IF (.NOT. valid) THEN
         STOP "fftw3_create_plan (remaining)"
    ENDIF


END SUBROUTINE

! *****************************************************************************

SUBROUTINE fftw3_create_plan_3d(plan, zin, zout, plan_style)

  USE fft_plan,                           ONLY : fft_plan_type
  USE fft_kinds,                          ONLY : dp
!$  USE OMP_LIB

  IMPLICIT NONE

  TYPE(fft_plan_type), INTENT ( INOUT )              :: plan
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(INOUT)      :: zin
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(INOUT)      :: zout
  INTEGER                                            :: plan_style
  INTEGER                                            :: n1,n2,n3
  INTEGER                                            :: nt
  INTEGER                                            :: rows_per_th, remaining 
  INTEGER                                            :: rows_per_th_r
  INTEGER                                            :: fft_direction
  INTEGER                                            :: th_planA, th_planB
  COMPLEX(KIND=dp), ALLOCATABLE                      :: tmp(:)

  ! GURU Interface
  INTEGER :: dim_n(2), dim_istride(2), dim_ostride(2), &
             howmany_n(2), howmany_istride(2), howmany_ostride(2)

  LOGICAL :: fftw3_is_mkl_wrapper

#if defined ( __FFTW3 )
#include "fftw3.f"
  INTEGER :: fftw_plan_type
  SELECT CASE(plan_style)
  CASE(1)
         fftw_plan_type = FFTW_ESTIMATE
  CASE(2)
         fftw_plan_type = FFTW_MEASURE
  CASE(3)
         fftw_plan_type = FFTW_PATIENT
  CASE(4)
         fftw_plan_type = FFTW_EXHAUSTIVE
  CASE DEFAULT
         STOP "fftw3_create_plan_3d"
  END SELECT
 
#if defined (__FFTW3_UNALIGNED)
  fftw_plan_type = fftw_plan_type + FFTW_UNALIGNED
#endif

  IF ( plan%fsign == +1 ) THEN
    fft_direction = FFTW_FORWARD
  ELSE
    fft_direction = FFTW_BACKWARD
  END IF

  n1 = plan%n_3d(1)
  n2 = plan%n_3d(2)
  n3 = plan%n_3d(3)
 

  nt = 1
!$OMP PARALLEL shared(nt)
!$OMP MASTER
!$ nt = omp_get_num_threads()
!$OMP END MASTER
!$OMP END PARALLEL

  IF ( (fftw3_is_mkl_wrapper()) .OR. &
       (.NOT. plan_style == 1 ) .OR. &
       (n1 < 256 .AND. n2 < 256 .AND. n3 < 256 .AND. nt== 1)) THEN
    ! If the plan type is MEASURE, PATIENT and EXHAUSTIVE or
    ! the grid size is small (and we are single-threaded) then 
    ! FFTW3 does a better job than handmade optimization
    ! so plan a single 3D FFT which will execute using all the threads

    plan%separated_plans = .FALSE.
!$  CALL XFFTW_PLAN_WITH_NTHREADS(nt)

    IF ( plan%fft_in_place) THEN
      CALL XFFTW_PLAN_DFT_3D(plan%fftw_plan,n1,n2,n3,zin,zin,fft_direction,fftw_plan_type)
    ELSE
      CALL XFFTW_PLAN_DFT_3D(plan%fftw_plan,n1,n2,n3,zin,zout,fft_direction,fftw_plan_type)
    ENDIF
  ELSE
    ALLOCATE(tmp(n1*n2*n3))
    ! ************************* PLANS WITH TRANSPOSITIONS ****************************
    !  In the cases described above, we manually thread each stage of the 3D FFT.
    ! 
    !  The following plans replace the 3D FFT call by running 1D FFTW across all 
    !  3 directions of the array.
    !
    !  Output of FFTW is transposed to ensure that the next round of FFTW access 
    !  contiguous information.
    !    
    !  Assuming the input matrix is M(n3,n2,n1), FFTW/Transp are :
    !  M(n3,n2,n1) -> fftw(x) -> M(n3,n1,n2) -> fftw(y) -> M(n1,n2,n3) -> fftw(z) -> M(n1,n2,n3)
    !  Notice that last matrix is transposed in the Z axis. A DO-loop in the execute routine
    !  will perform the final transposition. Performance evaluation showed that using an external
    !  DO loop to do the final transposition performed better than directly transposing the output.
    !  However, this might vary depending on the compiler/platform, so a potential tuning spot 
    !  is to perform the final transposition within the fftw library rather than using the external loop
    !  See comments below in Z-FFT for how to tranpose the output to avoid the final DO loop.
    !
    !  Doc. for the Guru interface is in http://www.fftw.org/doc/Guru-Interface.html
    !
    !  OpenMP : Work is distributed on the Z plane.
    !           All transpositions are out-of-place to facilitate multi-threading
    !
    !!!! Plan for X : M(n3,n2,n1) -> fftw(x) -> M(n3,n1,n2)
    CALL fftw3_compute_rows_per_th(n3, nt, rows_per_th, rows_per_th_r, & 
                                   remaining, th_planA, th_planB)

    dim_n(1) = n1
    dim_istride(1) = 1
    dim_ostride(1) = n2
    howmany_n(1) = n2 
    howmany_n(2) = rows_per_th
    howmany_istride(1) = n1
    howmany_istride(2) = n1*n2
    howmany_ostride(1) = 1
    howmany_ostride(2) = n1*n2
    CALL fftw3_create_3d_plans(plan%fftw_plan_nx, plan%fftw_plan_nx_r, &
                               dim_n, dim_istride, dim_ostride,howmany_n, &
                               howmany_istride, howmany_ostride, &
                               zin, tmp, &
                               fft_direction, fftw_plan_type, rows_per_th, &
                               rows_per_th_r)

    !!!! Plan for Y : M(n3,n1,n2) -> fftw(y) -> M(n1,n2,n3)
    CALL fftw3_compute_rows_per_th(n3, nt, rows_per_th, rows_per_th_r, &
                                   remaining, th_planA, th_planB)
    dim_n(1) = n2
    dim_istride(1) = 1
    dim_ostride(1) = n3
    howmany_n(1) = n1 
    howmany_n(2) = rows_per_th
    howmany_istride(1) = n2
    howmany_istride(2) = n1*n2
    !!! transposed Z axis on output
    howmany_ostride(1) = n2*n3 
    howmany_ostride(2) = 1 

    CALL fftw3_create_3d_plans(plan%fftw_plan_ny, plan%fftw_plan_ny_r, &
                               dim_n, dim_istride, dim_ostride,        &
                               howmany_n, howmany_istride, howmany_ostride, &
                               tmp, zin, &
                               fft_direction, fftw_plan_type, rows_per_th, &
                               rows_per_th_r)

    !!!! Plan for Z : M(n1,n2,n3) -> fftw(z) -> M(n1,n2,n3)
    CALL fftw3_compute_rows_per_th(n1, nt, rows_per_th, rows_per_th_r, remaining, &
                                   th_planA, th_planB)
    dim_n(1) = n3
    dim_istride(1) = 1
    dim_ostride(1) = 1          ! To transpose: n2*n1
    howmany_n(1) = n2 
    howmany_n(2) = rows_per_th
    howmany_istride(1) = n3 
    howmany_istride(2) = n2*n3 
    howmany_ostride(1) = n3     ! To transpose: n1 
    howmany_ostride(2) = n2*n3  ! To transpose: 1

    CALL fftw3_create_3d_plans(plan%fftw_plan_nz, plan%fftw_plan_nz_r, &
                               dim_n, dim_istride, dim_ostride,        &
                               howmany_n, howmany_istride, howmany_ostride, &
                               zin, tmp, &
                               fft_direction, fftw_plan_type, rows_per_th, &
                               rows_per_th_r)

    plan%separated_plans = .TRUE.

    DEALLOCATE(tmp)
  ENDIF


#endif


END SUBROUTINE fftw3_create_plan_3d


! *****************************************************************************

SUBROUTINE fftw3_workshare_execute_dft(plan, plan_r, split_dim, nt, tid, &
                                       input, istride, output, ostride)

   USE fft_kinds,                          ONLY : dp, integer8_kind
!$ USE OMP_LIB 

  IMPLICIT NONE 

  INTEGER, INTENT(IN)                           :: split_dim,nt, tid
  INTEGER, INTENT(IN)                           :: istride, ostride
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(INOUT) :: input, output
    INTEGER (KIND=integer8_kind)                :: plan, plan_r
    INTEGER                                     :: i_off, o_off
    INTEGER                                     :: th_planA, th_planB
   INTEGER :: rows_per_thread, rows_per_thread_r, remaining

#if defined (__FFTW3)
      CALL fftw3_compute_rows_per_th(split_dim, nt, rows_per_thread, &
                                     rows_per_thread_r, remaining, &
                                     th_planA, th_planB)
 
      IF (th_planB .GT. 0) THEN   
        IF (tid .LT. th_planA) THEN
         i_off = (tid) * (istride*(rows_per_thread)) + 1
         o_off = (tid) * (ostride*(rows_per_thread)) + 1
         IF (rows_per_thread .GT. 0) THEN
            CALL XFFTW_EXECUTE_DFT(plan, input(i_off), &
                                    output(o_off))
         ENDIF
        ELSE IF ((tid - th_planA) < th_planB) THEN

         i_off = (remaining)*istride*(rows_per_thread) + &
                 (tid-remaining)*istride*(rows_per_thread_r) + 1
         o_off = (remaining)*ostride*(rows_per_thread) + &
                 (tid-remaining)*ostride*(rows_per_thread_r) + 1

         CALL XFFTW_EXECUTE_DFT(plan_r, input(i_off), &
                                       output(o_off))
        ENDIF

      ELSE
         i_off = (tid) * (istride*(rows_per_thread)) + 1
         o_off = (tid) * (ostride*(rows_per_thread)) + 1

         CALL XFFTW_EXECUTE_DFT(plan, input(i_off), &
                                    output(o_off))

      ENDIF 
#else

#endif

END SUBROUTINE


! *****************************************************************************

SUBROUTINE fftw33d ( plan, scale, zin, zout, stat )

  USE fft_plan,                           ONLY : fft_plan_type
  USE fft_kinds,                          ONLY : dp
!$ USE OMP_LIB

  IMPLICIT NONE

  TYPE(fft_plan_type), INTENT(IN)                      :: plan
  REAL(KIND=dp), INTENT(IN)                            :: scale
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(INOUT), TARGET:: zin
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(INOUT), TARGET:: zout
  COMPLEX(KIND=dp), POINTER                            :: xout(:)
  COMPLEX(KIND=dp), ALLOCATABLE                        :: tmp1(:)
  INTEGER, INTENT(OUT)                                 :: stat
  INTEGER                                              :: n1, n2, n3
  INTEGER                                              :: tid, nt
  INTEGER                                              :: i,j,k

!------------------------------------------------------------------------------

#if defined ( __FFTW3 )
#include "fftw3.f"

  n1 = plan%n_3d(1)
  n2 = plan%n_3d(2)
  n3 = plan%n_3d(3)

  stat = 1

  ! We use a POINTER to the output array to avoid duplicating code
  IF (plan%fft_in_place) THEN
     xout => zin(:n1*n2*n3)
  ELSE 
     xout => zout(:n1*n2*n3)
  ENDIF

  ! Either compute the full 3D FFT using a multithreaded plan
  IF (.NOT. plan%separated_plans) THEN
      CALL XFFTW_EXECUTE_DFT(plan%fftw_plan,zin,xout)
  ELSE
  ! Or use the 3 stage FFT scheme described in fftw3_create_plan_3d
       ALLOCATE(tmp1(n1*n2*n3))   ! Temporary vector used for transpositions
      !$OMP PARALLEL PRIVATE(tid,nt) SHARED(zin,tmp1,n1,n2,n3,plan)
      tid = 0
      nt = 1

!$    tid = omp_get_thread_num()
!$    nt = omp_get_num_threads()
      CALL fftw3_workshare_execute_dft(plan%fftw_plan_nx, plan%fftw_plan_nx_r, &
                                        n3,nt, tid,&
                                        zin, n1*n2, tmp1, n1*n2)        

      !$OMP BARRIER
      CALL fftw3_workshare_execute_dft(plan%fftw_plan_ny, plan%fftw_plan_ny_r, &
                                        n3,nt, tid,&
                                        tmp1, n1*n2, xout, 1)        
      !$OMP BARRIER
      CALL fftw3_workshare_execute_dft(plan%fftw_plan_nz, plan%fftw_plan_nz_r, &
                                        n1,nt, tid,&
                                        xout, n2*n3, tmp1, n2*n3)        
      !$OMP BARRIER

      !$OMP DO __COLLAPSE3 
      DO i = 1,n1
        DO j = 1,n2
          DO k = 1,n3
            xout((i-1) + (j-1) * n1 + (k-1) * n1 * n2 + 1) = & 
                    tmp1((k-1) + (j-1) * n3 + (i-1) * n3 * n2 + 1)
          ENDDO
        ENDDO
      ENDDO
      !$OMP END DO

      !$OMP END PARALLEL
  END IF  


  IF ( scale /= 1.0_dp ) THEN
      CALL zdscal(n1*n2*n3,scale,xout,1)
  END IF

#else

  stat = 0

#endif

END SUBROUTINE fftw33d

! *****************************************************************************

SUBROUTINE fftw3_create_plan_1dm(plan, zin, zout, plan_style)

  USE fft_plan,                           ONLY : fft_plan_type
  USE fft_kinds,                          ONLY : dp
  !$ USE OMP_LIB

  IMPLICIT NONE

  TYPE(fft_plan_type), INTENT ( INOUT )              :: plan
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(IN)         :: zin
  COMPLEX(KIND=dp), DIMENSION(*), INTENT(IN)         :: zout
  INTEGER, INTENT(IN)                                :: plan_style
  INTEGER                                            :: ii,di,io,DO, num_threads, num_rows

#if defined ( __FFTW3 )
#include "fftw3.f"
  INTEGER :: fftw_plan_type
  SELECT CASE(plan_style)
  CASE(1)
         fftw_plan_type = FFTW_ESTIMATE
  CASE(2)
         fftw_plan_type = FFTW_MEASURE
  CASE(3)
         fftw_plan_type = FFTW_PATIENT
  CASE(4)
         fftw_plan_type = FFTW_EXHAUSTIVE
  CASE DEFAULT
   STOP "fftw3_create_plan_1dm"
  END SELECT

#if defined (__FFTW3_UNALIGNED)
  fftw_plan_type = fftw_plan_type + FFTW_UNALIGNED
#endif
num_threads = 1
plan%separated_plans = .FALSE.
!$omp parallel default(none), &
!$omp          shared(num_threads)
!$OMP MASTER
!$ num_threads = omp_get_num_threads()
!$OMP END MASTER
!$omp end parallel

num_rows = plan%m / num_threads
!$   plan%num_threads_needed = num_threads

! Check for number of rows less than num_threads
!$ IF (plan%m < num_threads) THEN
!$   num_rows = 1
!$   plan%num_threads_needed = plan%m
!$ ENDIF

! Check for total number of rows not divisible by num_threads
!$ IF (num_rows*plan%num_threads_needed .NE. plan%m) THEN
!$   plan%need_alt_plan = .TRUE.
!$ ENDIF

!$  plan%num_rows = num_rows
  ii = 1
  di = plan%n
  io = 1
  DO = plan%n
  IF ( plan%fsign == +1 .AND. plan%trans ) THEN
    ii = plan%m
    di = 1
  ELSEIF ( plan%fsign == -1 .AND. plan%trans ) THEN
    io = plan%m
    DO = 1
  END IF

  IF ( plan%fsign == +1 ) THEN
    CALL dfftw_plan_many_dft(plan%fftw_plan,1,plan%n,num_rows,zin,0,ii,di,&
              zout,0,io,DO,FFTW_FORWARD,fftw_plan_type)
  ELSE
    CALL dfftw_plan_many_dft(plan%fftw_plan,1,plan%n,num_rows,zin,0,ii,di,&
              zout,0,io,DO,FFTW_BACKWARD,fftw_plan_type)
  END IF

!$ IF (plan%need_alt_plan) THEN
!$  plan%alt_num_rows = plan%m - (plan%num_threads_needed - 1)*num_rows
!$  IF ( plan%fsign == +1 ) THEN
!$    CALL dfftw_plan_many_dft(plan%alt_fftw_plan,1,plan%n,plan%alt_num_rows,zin,0,ii,di,&
!$              zout,0,io,DO,FFTW_FORWARD,fftw_plan_type)
!$  ELSE
!$    CALL dfftw_plan_many_dft(plan%alt_fftw_plan,1,plan%n,plan%alt_num_rows,zin,0,ii,di,&
!$              zout,0,io,DO,FFTW_BACKWARD,fftw_plan_type)
!$  END IF
!$ END IF

#endif

END SUBROUTINE fftw3_create_plan_1dm

SUBROUTINE fftw3_destroy_plan ( plan )

  USE fft_plan,                           ONLY: fft_plan_type

  IMPLICIT NONE

  TYPE(fft_plan_type), INTENT (INOUT)   :: plan

#if defined ( __FFTW3 )
!$  IF (plan%need_alt_plan) THEN
!$    CALL XFFTW_DESTROY_PLAN(plan%alt_fftw_plan)
!$  END IF

  IF (.NOT. plan%separated_plans) THEN
    CALL XFFTW_DESTROY_PLAN(plan%fftw_plan)
  ELSE
    ! If it is a separated plan then we have to destroy
    ! each dim plan individually
    CALL XFFTW_DESTROY_PLAN(plan%fftw_plan_nx)
    CALL XFFTW_DESTROY_PLAN(plan%fftw_plan_ny)
    CALL XFFTW_DESTROY_PLAN(plan%fftw_plan_nz)
    CALL XFFTW_DESTROY_PLAN(plan%fftw_plan_nx_r)
    CALL XFFTW_DESTROY_PLAN(plan%fftw_plan_ny_r)
    CALL XFFTW_DESTROY_PLAN(plan%fftw_plan_nz_r)
  ENDIF

#endif

END SUBROUTINE fftw3_destroy_plan

SUBROUTINE fftw31dm ( plan, zin, zout, scale, stat )

  USE fft_plan,                 ONLY : fft_plan_type
  USE fft_kinds,                          ONLY : dp
  !$ USE OMP_LIB

  IMPLICIT NONE
#if defined (__NAG)
  INTEGER*8          :: dummy_64_bit
  INTEGER, PARAMETER :: integer8_kind = KIND(dummy_64_bit)
#else
  INTEGER, PARAMETER :: integer8_kind = 8
#endif

  TYPE (fft_plan_type), INTENT (IN)                  :: plan
  COMPLEX(KIND=dp), DIMENSION(*), TARGET, INTENT(IN)         :: zin
  COMPLEX(KIND=dp), DIMENSION(*), TARGET, INTENT(INOUT)      :: zout
  REAL (KIND=dp), INTENT ( IN )                      :: scale
  INTEGER, INTENT(OUT)                               :: stat
  INTEGER :: num_rows, my_id, in_offset, out_offset, scal_offset
  COMPLEX(KIND=dp), POINTER :: zin_ptr, zout_ptr, zscal_ptr
  INTEGER (KIND=integer8_kind)        :: fftw_plan

!------------------------------------------------------------------------------

my_id = 0
num_rows = plan%m

!$omp parallel default(none), &
!$omp          private(my_id,num_rows,zin_ptr,zout_ptr,zscal_ptr,in_offset,out_offset,scal_offset,fftw_plan), &
!$omp          shared(zin,zout), &
!$omp          shared(plan,scale,stat)
!$ my_id = omp_get_thread_num()

!$ if (my_id < plan%num_threads_needed) then

fftw_plan = plan%fftw_plan

in_offset = 1
out_offset = 1
scal_offset = 1

!$ in_offset = 1 + plan%num_rows * my_id * plan%n
!$ out_offset = 1 + plan%num_rows * my_id * plan%n
!$ IF ( plan%fsign == +1 .AND. plan%trans ) THEN
!$  in_offset = 1 + plan%num_rows*my_id
!$ ELSEIF ( plan%fsign == -1 .AND. plan%trans ) THEN
!$  out_offset = 1 + plan%num_rows*my_id
!$ ENDIF
!$ scal_offset = 1 + plan%n*plan%num_rows*my_id
!$ IF ( plan%need_alt_plan .AND. my_id .EQ. plan%num_threads_needed - 1 ) THEN
!$   num_rows = plan%alt_num_rows
!$   fftw_plan = plan%alt_fftw_plan
!$ ELSE
!$   num_rows = plan%num_rows
!$ ENDIF

zin_ptr => zin(in_offset)
zout_ptr => zout(out_offset)
zscal_ptr => zout(scal_offset)

#if defined ( __FFTW3 )
!$OMP MASTER
  stat=1
!$OMP END MASTER
  CALL dfftw_execute_dft(fftw_plan, zin_ptr, zout_ptr)
!$ endif
! all theads need to meet at this barrier
!$omp barrier
!$ if (my_id < plan%num_threads_needed) then
  IF ( scale /= 1.0_dp ) CALL zdscal(plan%n*num_rows,scale,zscal_ptr,1)
!$ endif

#else

  stat=0

!$ else
!$ end if

#endif

!$omp end parallel

END SUBROUTINE fftw31dm

!     Copyright (c) 2003, 2006 Matteo Frigo
!     Copyright (c) 2003, 2006 Massachusetts Institute of Technology
!
!     This program is free software; you can redistribute it and/or modify
!     it under the terms of the GNU General Public License as published by
!     the Free Software Foundation; either version 2 of the License, or
!     (at your option) any later version.
!
!     This program is distributed in the hope that it will be useful,
!     but WITHOUT ANY WARRANTY; without even the implied warranty of
!     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!     GNU General Public License for more details.
!
!     You should have received a copy of the GNU General Public License
!     along with this program; if not, write to the Free Software
!     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
!
!ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
!
!     This is an example implementation of Fortran wisdom export/import
!     to/from a Fortran unit (file), exploiting the generic
!     dfftw_export_wisdom/dfftw_import_wisdom functions.
!
!     We cannot compile this file into the FFTW library itself, lest all
!     FFTW-calling programs be required to link to the Fortran I/O
!     libraries.
!
!     adapted to become more standard Fortran 90 [2007-10] Joost VandeVondele
!     and added some namespacing
!
!ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

! *****************************************************************************
      SUBROUTINE fftw_write_char(c, iunit)
         IMPLICIT NONE
         CHARACTER c
         INTEGER iunit
         WRITE(iunit,'(a)',ADVANCE="NO") c
      END

! *****************************************************************************
      SUBROUTINE fftw_export_wisdom_to_file(iunit)
         IMPLICIT NONE
         INTEGER iunit
         EXTERNAL fftw_write_char
#if defined ( __FFTW3 )
         CALL dfftw_export_wisdom(fftw_write_char, iunit)
#endif
      END

!     Fortran 77 does not have any portable way to read an arbitrary
!     file one character at a time [needs to wait for stream IO of F2003].
!     The best alternative seems to be to
!     read a whole line into a buffer, since for fftw-exported wisdom we
!     can bound the line length.  (If the file contains longer lines,
!     then the lines will be truncated and the wisdom import should
!     simply fail.)  Ugh (and not thread safe).

! *****************************************************************************
      SUBROUTINE fftw_read_char(ic, iunit)
         IMPLICIT NONE
         INTEGER ic
         INTEGER iunit
         CHARACTER*256 buf
         SAVE buf
         INTEGER ibuf
         DATA ibuf/257/
         SAVE ibuf
         IF (ibuf .LT. 257) THEN
            ic = ICHAR(buf(ibuf:ibuf))
            ibuf = ibuf + 1
            RETURN
         ENDIF
         READ(iunit,123,END=666) buf
         ic = ICHAR(buf(1:1))
         ibuf = 2
         RETURN
 666     ic = -1
         ibuf = 257
 123     FORMAT(a256)
      END

! *****************************************************************************
      SUBROUTINE fftw_import_wisdom_from_file(isuccess, iunit)
         IMPLICIT NONE
         INTEGER isuccess
         INTEGER iunit
         EXTERNAL fftw_read_char
         isuccess=0
#if defined ( __FFTW3 )
         CALL dfftw_import_wisdom(isuccess, fftw_read_char, iunit)
#endif
      END

