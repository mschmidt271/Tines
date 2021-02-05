#ifndef __TINES_APPLY_PIVOT_INTERNAL_HPP__
#define __TINES_APPLY_PIVOT_INTERNAL_HPP__

#include "Tines_Internal.hpp"

namespace Tines {

  ///
  /// Forward
  ///
  struct ApplyPivotVectorForwardInternal {
    template <typename MemberType, typename ValueType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int piv,
           /* */ ValueType *__restrict__ A, const int as0) {
      if (piv != 0) {
        Kokkos::single(Kokkos::PerTeam(member), [&]() {
          const int idx_p = piv * as0;
          const ValueType tmp = A[0];
          A[0] = A[idx_p];
          A[idx_p] = tmp;
        });
      }
      return 0;
    }

    template <typename MemberType, typename IntType, typename ValueType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int plen,
           const IntType *__restrict__ p, const int ps0,
           /* */ ValueType *__restrict__ A, const int as0) {
      Kokkos::single(Kokkos::PerTeam(member), [&]() {
        for (int i = 0; i < plen; ++i) {
          const int piv = p[i * ps0];
          if (piv != 0) {
            const int idx_i = i * as0, idx_p = (i + piv) * as0;
            const ValueType tmp = A[idx_i];
            A[idx_i] = A[idx_p];
            A[idx_p] = tmp;
          }
        }
      });
      return 0;
    }
  };

  /// Pivot a row
  struct ApplyPivotMatrixForwardInternal {
    template <typename MemberType, typename ValueType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int piv, const int n,
           /* */ ValueType *__restrict__ A, const int as0, const int as1) {
      if (piv != 0) {
        Kokkos::parallel_for(Kokkos::TeamVectorRange(member, n),
                             [&](const int &j) {
                               ValueType *__restrict__ A_at_j = A + j * as1;
                               const int idx_p = piv * as0;
                               const ValueType tmp = A_at_j[0];
                               A_at_j[0] = A_at_j[idx_p];
                               A_at_j[idx_p] = tmp;
                             });
      }
      return 0;
    }

    template <typename MemberType, typename IntType, typename ValueType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int plen, const int n,
           const IntType *__restrict__ p, const int ps0,
           /* */ ValueType *__restrict__ A, const int as0, const int as1) {
      Kokkos::parallel_for(
        Kokkos::TeamVectorRange(member, n), [&](const int &j) {
          ValueType *__restrict__ A_at_j = A + j * as1;
          for (int i = 0; i < plen; ++i) {
            const int piv = p[i * ps0];
            if (piv != 0) {
              const int idx_i = i * as0, idx_p = (i + piv) * as0;
              const ValueType tmp = A_at_j[idx_i];
              A_at_j[idx_i] = A_at_j[idx_p];
              A_at_j[idx_p] = tmp;
            }
          }
        });
      return 0;
    }
  };

  ///
  /// Backward
  ///
  struct ApplyPivotVectorBackwardInternal {
    template <typename MemberType, typename ValueType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int piv,
           /* */ ValueType *__restrict__ A, const int as0) {
      if (piv != 0) {
        Kokkos::single(Kokkos::PerTeam(member), [&]() {
          const int idx_p = piv * as0;
          const ValueType tmp = A[0];
          A[0] = A[idx_p];
          A[idx_p] = tmp;
        });
      }
      return 0;
    }

    template <typename MemberType, typename IntType, typename ValueType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int plen,
           const IntType *__restrict__ p, const int ps0,
           /* */ ValueType *__restrict__ A, const int as0) {
      Kokkos::single(Kokkos::PerTeam(member), [&]() {
        for (int i = (plen - 1); i >= 0; --i) {
          const int piv = p[i * ps0];
          if (piv != 0) {
            const int idx_i = i * as0, idx_p = (i + piv) * as0;
            const ValueType tmp = A[idx_i];
            A[idx_i] = A[idx_p];
            A[idx_p] = tmp;
          }
        }
      });
      return 0;
    }
  };

  /// Pivot a row
  struct ApplyPivotMatrixBackwardInternal {
    template <typename MemberType, typename ValueType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int piv, const int n,
           /* */ ValueType *__restrict__ A, const int as0, const int as1) {
      if (piv != 0) {
        Kokkos::parallel_for(Kokkos::TeamVectorRange(member, n),
                             [&](const int &j) {
                               ValueType *__restrict__ A_at_j = A + j * as1;
                               const int idx_p = piv * as0;
                               const ValueType tmp = A_at_j[0];
                               A_at_j[0] = A_at_j[idx_p];
                               A_at_j[idx_p] = tmp;
                             });
      }
      return 0;
    }

    template <typename MemberType, typename IntType, typename ValueType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int plen, const int n,
           const IntType *__restrict__ p, const int ps0,
           /* */ ValueType *__restrict__ A, const int as0, const int as1) {
      Kokkos::parallel_for(
        Kokkos::TeamVectorRange(member, n), [&](const int &j) {
          ValueType *__restrict__ A_at_j = A + j * as1;
          for (int i = (plen - 1); i >= 0; --i) {
            const int piv = p[i * ps0];
            if (piv != 0) {
              const int idx_i = i * as0, idx_p = (i + piv) * as0;
              const ValueType tmp = A_at_j[idx_i];
              A_at_j[idx_i] = A_at_j[idx_p];
              A_at_j[idx_p] = tmp;
            }
          }
        });
      return 0;
    }
  };

} // namespace Tines

#endif