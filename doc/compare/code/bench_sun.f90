! Copyright (c) 2026 ENSTA, France
!
! Created 2026-09-20 by Jordan NININ
!
! The benchmark with the intervals of Solaris Studio's Fortran (f90 -xia): the
! operations of bench_ops.h, on the intervals of bench.py, written as CSV lines
!   library,operation,n,repeats,best time (s),time per operation (ns),
!   sum of the midpoints of the results,sum of their widths
! Usage: bench_sun DATA REPEATS [OPERATIONS]
module bench_tools
  use iso_c_binding
  implicit none

  type, bind(c) :: timespec
    integer(c_int64_t) :: sec, nsec
  end type timespec

  interface
    function clock_gettime(id, ts) bind(c, name='clock_gettime')
      import :: c_int, timespec
      integer(c_int), value :: id
      type(timespec) :: ts
      integer(c_int) :: clock_gettime
    end function clock_gettime
  end interface

  integer(8) :: n
  integer :: repeats
  character(len=512) :: only
  interval(8), allocatable :: a(:), b(:), p(:), e(:), s1(:), s2(:), s3(:), s4(:), r(:)

contains

  ! Seconds of CLOCK_MONOTONIC: system_clock counts milliseconds only
  real(8) function now()
    type(timespec) :: ts
    integer(c_int) :: rc
    rc = clock_gettime(1_c_int, ts)
    now = real(ts%sec, 8) + real(ts%nsec, 8)*1d-9
  end function now

  logical function selected(name)
    character(len=*), intent(in) :: name
    selected = len_trim(only) == 0 .or. index(',' // trim(only) // ',', ',' // name // ',') > 0
  end function selected

  subroutine load(path)
    character(len=*), intent(in) :: path
    real(8), allocatable :: lo(:), hi(:)
    integer :: k
    integer(8) :: i
    open(10, file=path, access='stream', form='unformatted', status='old', action='read')
    read(10) n
    allocate(lo(n), hi(n), a(n), b(n), p(n), e(n), s1(n), s2(n), s3(n), s4(n), r(n))
    do k = 1, 8
      read(10) lo
      read(10) hi
      do i = 1, n
        select case (k)
        case (1)
          a(i) = interval(lo(i), hi(i))
        case (2)
          b(i) = interval(lo(i), hi(i))
        case (3)
          p(i) = interval(lo(i), hi(i))
        case (4)
          e(i) = interval(lo(i), hi(i))
        case (5)
          s1(i) = interval(lo(i), hi(i))
        case (6)
          s2(i) = interval(lo(i), hi(i))
        case (7)
          s3(i) = interval(lo(i), hi(i))
        case (8)
          s4(i) = interval(lo(i), hi(i))
        end select
      end do
    end do
    close(10)
    r = interval(0d0)
  end subroutine load

  subroutine report(name, best)
    character(len=*), intent(in) :: name
    real(8), intent(in) :: best
    real(8) :: sum_mid, sum_wid
    integer(8) :: i
    sum_mid = 0d0
    sum_wid = 0d0
    do i = 1, n
      sum_mid = sum_mid + mid(r(i))
      sum_wid = sum_wid + wid(r(i))
    end do
    write(*, '(A,A,A,I0,A,I0,A,ES16.9,A,F0.3,A,ES25.17,A,ES25.17)') 'solaris_f90,', name, ',', n, ',', &
      repeats, ',', best, ',', best/real(n, 8)*1d9, ',', sum_mid, ',', sum_wid
  end subroutine report
end module bench_tools

program bench_sun
  use bench_tools
  implicit none
  character(len=512) :: path, arg
  interval(8) :: sha(5, 4), shc(5), zeroi, onei, f, t1, t2, t3, t4
  real(8) :: sa(5, 4), t0, best
  character(len=8) :: sc(5)
  integer :: j, k
  integer(8) :: i

  if (command_argument_count() < 2) then
    write(*, '(A)') 'usage: bench_sun DATA REPEATS [OPERATIONS]'
    stop 1
  end if
  call get_command_argument(1, path)
  call get_command_argument(2, arg)
  read(arg, *) repeats
  only = ''
  if (command_argument_count() > 2) call get_command_argument(3, only)
  call load(trim(path))

  ! Shekel 5, as in bench_ops.h; the c_i are the tightest intervals enclosing
  ! the decimal numbers, read as the other libraries read them
  ! (/ /) rather than [ ], which -xia takes for interval constants
  sa = reshape((/ 4d0, 1d0, 8d0, 6d0, 3d0, 4d0, 1d0, 8d0, 6d0, 7d0, &
                  4d0, 1d0, 8d0, 6d0, 3d0, 4d0, 1d0, 8d0, 6d0, 7d0 /), (/ 5, 4 /))
  sc = (/ '[0.1]', '[0.2]', '[0.2]', '[0.4]', '[0.4]' /)
  do j = 1, 5
    do k = 1, 4
      sha(j, k) = interval(sa(j, k), sa(j, k))
    end do
    read(sc(j), *) shc(j)
  end do
  zeroi = interval(0d0, 0d0)
  onei = interval(1d0, 1d0)

  if (selected('add')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = a(i) + b(i)
      end do
      best = min(best, now() - t0)
    end do
    call report('add', best)
  end if

  if (selected('sub')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = a(i) - b(i)
      end do
      best = min(best, now() - t0)
    end do
    call report('sub', best)
  end if

  if (selected('mul')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = a(i) * b(i)
      end do
      best = min(best, now() - t0)
    end do
    call report('mul', best)
  end if

  if (selected('div')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = a(i) / p(i)
      end do
      best = min(best, now() - t0)
    end do
    call report('div', best)
  end if

  if (selected('sqr')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = a(i)**2
      end do
      best = min(best, now() - t0)
    end do
    call report('sqr', best)
  end if

  if (selected('sqrt')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = sqrt(p(i))
      end do
      best = min(best, now() - t0)
    end do
    call report('sqrt', best)
  end if

  if (selected('exp')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = exp(a(i))
      end do
      best = min(best, now() - t0)
    end do
    call report('exp', best)
  end if

  if (selected('log')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = log(p(i))
      end do
      best = min(best, now() - t0)
    end do
    call report('log', best)
  end if

  if (selected('sin')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = sin(a(i))
      end do
      best = min(best, now() - t0)
    end do
    call report('sin', best)
  end if

  if (selected('cos')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = cos(a(i))
      end do
      best = min(best, now() - t0)
    end do
    call report('cos', best)
  end if

  if (selected('pow_int')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = a(i)**3
      end do
      best = min(best, now() - t0)
    end do
    call report('pow_int', best)
  end if

  if (selected('pow_real')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = p(i)**e(i)
      end do
      best = min(best, now() - t0)
    end do
    call report('pow_real', best)
  end if

  if (selected('line_arith')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = (a(i) + b(i)) * (a(i) - b(i)) / p(i)
      end do
      best = min(best, now() - t0)
    end do
    call report('line_arith', best)
  end if

  if (selected('line_trig')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = sin(a(i)) * cos(b(i)) + a(i)**2
      end do
      best = min(best, now() - t0)
    end do
    call report('line_trig', best)
  end if

  if (selected('line_pow')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        r(i) = sqrt(p(i)) * a(i)**3 - exp(b(i) / p(i))
      end do
      best = min(best, now() - t0)
    end do
    call report('line_pow', best)
  end if

  if (selected('shekel5')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        f = zeroi
        do j = 1, 5
          f = f - onei / ((s1(i) - sha(j, 1))**2 + (s2(i) - sha(j, 2))**2 &
                          + (s3(i) - sha(j, 3))**2 + (s4(i) - sha(j, 4))**2 + shc(j))
        end do
        r(i) = f
      end do
      best = min(best, now() - t0)
    end do
    call report('shekel5', best)
  end if

  if (selected('block5')) then
    best = huge(1d0)
    do k = 1, repeats
      t0 = now()
      do i = 1, n
        t1 = a(i) * b(i) + p(i)
        t2 = sin(t1) * cos(b(i))
        t3 = a(i)**2 + t2 / p(i)
        t4 = exp(t2) - b(i)**3
        r(i) = t3 * t4 + sqrt(p(i))
      end do
      best = min(best, now() - t0)
    end do
    call report('block5', best)
  end if
end program bench_sun
