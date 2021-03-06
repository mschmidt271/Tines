/*----------------------------------------------------------------------------------
Tines - Time Integrator, Newton and Eigen Solver -  version 1.0
Copyright (2021) NTESS
https://github.com/sandialabs/Tines

Copyright 2021 National Technology & Engineering Solutions of Sandia, LLC (NTESS). 
Under the terms of Contract DE-NA0003525 with NTESS, the U.S. Government retains 
certain rights in this software.

This file is part of Tines. Tines is open-source software: you can redistribute it
and/or modify it under the terms of BSD 2-Clause License
(https://opensource.org/licenses/BSD-2-Clause). A copy of the license is also
provided under the main directory
Questions? Kyungjoo Kim <kyukim@sandia.gov>, or
	   Oscar Diaz-Ibarra at <odiazib@sandia.gov>, or
	   Cosmin Safta at <csafta@sandia.gov>, or
	   Habib Najm at <hnnajm@sandia.gov>

Sandia National Laboratories, New Mexico, USA
----------------------------------------------------------------------------------*/
#ifndef __TINES_APPLY_PIVOT_HPP__
#define __TINES_APPLY_PIVOT_HPP__

#include "Tines_ApplyPivot_Internal.hpp"
#include "Tines_Internal.hpp"

namespace Tines {

  template <typename ArgSide, typename ArgDirect> struct ApplyPivot;

  ///
  /// Forward pivot apply
  ///

  /// row swap
  template <> struct ApplyPivot<Side::Left, Direct::Forward> {
    template <typename MemberType, typename AViewType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int piv, const AViewType &A) {
      if (AViewType::rank == 1) {
        const int as0 = A.stride(0);
        ApplyPivotVectorForwardInternal::invoke(member, piv, A.data(), as0);
      } else if (AViewType::rank == 2) {
        const int n = A.extent(1), as0 = A.stride(0), as1 = A.stride(1);
        ApplyPivotMatrixForwardInternal::invoke(member, piv, n, A.data(), as0,
                                                as1);
      }
      return 0;
    }

    template <typename MemberType, typename PivViewType, typename AViewType>
    KOKKOS_INLINE_FUNCTION static int invoke(const MemberType &member,
                                             const PivViewType piv,
                                             const AViewType &A) {
      static_assert(PivViewType::rank == 1, "pivot view is not rank-1");
      if (AViewType::rank == 1) {
        const int plen = piv.extent(0), ps0 = piv.stride(0), as0 = A.stride(0);
        ApplyPivotVectorForwardInternal::invoke(member, plen, piv.data(), ps0,
                                                A.data(), as0);
      } else if (AViewType::rank == 2) {
        // row permutation
        const int plen = piv.extent(0), ps0 = piv.stride(0), n = A.extent(1),
                  as0 = A.stride(0), as1 = A.stride(1);
        ApplyPivotMatrixForwardInternal::invoke(member, plen, n, piv.data(),
                                                ps0, A.data(), as0, as1);
      }
      return 0;
    }
  };

  /// column swap
  template <> struct ApplyPivot<Side::Right, Direct::Forward> {
    template <typename MemberType, typename AViewType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int piv, const AViewType &A) {
      if (AViewType::rank == 1) {
        const int as0 = A.stride(0);
        ApplyPivotVectorForwardInternal::invoke(member, piv, A.data(), as0);
      } else if (AViewType::rank == 2) {
        const int m = A.extent(0), as0 = A.stride(0), as1 = A.stride(1);
        ApplyPivotMatrixForwardInternal::invoke(member, piv, m, A.data(), as1,
                                                as0);
      }
      return 0;
    }

    template <typename MemberType, typename PivViewType, typename AViewType>
    KOKKOS_INLINE_FUNCTION static int invoke(const MemberType &member,
                                             const PivViewType &piv,
                                             const AViewType &A) {
      static_assert(PivViewType::rank == 1, "pivot view is not rank-1");
      if (AViewType::rank == 1) {
        const int plen = piv.extent(0), as0 = A.stride(0);
        ApplyPivotVectorForwardInternal ::invoke(member, plen, piv.data(),
                                                 A.data(), as0);
      } else if (AViewType::rank == 2) {
        // column permutation
        const int plen = piv.extent(0), ps = piv.stride(0), m = A.extent(0),
                  as0 = A.stride(0), as1 = A.stride(1);
        ApplyPivotMatrixForwardInternal ::invoke(member, plen, m, piv.data(),
                                                 ps, A.data(), as1, as0);
      }
      return 0;
    }
  };

  ///
  /// Backward pivot apply
  ///

  /// row swap
  template <> struct ApplyPivot<Side::Left, Direct::Backward> {
    template <typename MemberType, typename AViewType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int piv, const AViewType &A) {
      if (AViewType::rank == 1) {
        const int as0 = A.stride(0);
        ApplyPivotVectorBackwardInternal::invoke(member, piv, A.data(), as0);
      } else if (AViewType::rank == 2) {
        const int n = A.extent(1), as0 = A.stride(0), as1 = A.stride(1);
        ApplyPivotMatrixBackwardInternal::invoke(member, piv, n, A.data(), as0,
                                                 as1);
      }
      return 0;
    }

    template <typename MemberType, typename PivViewType, typename AViewType>
    KOKKOS_INLINE_FUNCTION static int invoke(const MemberType &member,
                                             const PivViewType piv,
                                             const AViewType &A) {
      static_assert(PivViewType::rank == 1, "pivot view is not rank-1");
      if (AViewType::rank == 1) {
        const int plen = piv.extent(0), ps0 = piv.stride(0), as0 = A.stride(0);
        ApplyPivotVectorBackwardInternal::invoke(member, plen, piv.data(), ps0,
                                                 A.data(), as0);
      } else if (AViewType::rank == 2) {
        // row permutation
        const int plen = piv.extent(0), ps0 = piv.stride(0), n = A.extent(1),
                  as0 = A.stride(0), as1 = A.stride(1);
        ApplyPivotMatrixBackwardInternal::invoke(member, plen, n, piv.data(),
                                                 ps0, A.data(), as0, as1);
      }
      return 0;
    }
  };

  /// column swap
  template <> struct ApplyPivot<Side::Right, Direct::Backward> {
    template <typename MemberType, typename AViewType>
    KOKKOS_INLINE_FUNCTION static int
    invoke(const MemberType &member, const int piv, const AViewType &A) {
      if (AViewType::rank == 1) {
        const int as0 = A.stride(0);
        ApplyPivotVectorBackwardInternal::invoke(member, piv, A.data(), as0);
      } else if (AViewType::rank == 2) {
        const int m = A.extent(0), as0 = A.stride(0), as1 = A.stride(1);
        ApplyPivotMatrixBackwardInternal::invoke(member, piv, m, A.data(), as1,
                                                 as0);
      }
      return 0;
    }

    template <typename MemberType, typename PivViewType, typename AViewType>
    KOKKOS_INLINE_FUNCTION static int invoke(const MemberType &member,
                                             const PivViewType &piv,
                                             const AViewType &A) {
      static_assert(PivViewType::rank == 1, "pivot view is not rank-1");
      if (AViewType::rank == 1) {
        const int plen = piv.extent(0), as0 = A.stride(0);
        ApplyPivotVectorBackwardInternal ::invoke(member, plen, piv.data(),
                                                  A.data(), as0);
      } else if (AViewType::rank == 2) {
        // column permutation
        const int plen = piv.extent(0), ps = piv.stride(0), m = A.extent(0),
                  as0 = A.stride(0), as1 = A.stride(1);
        ApplyPivotMatrixBackwardInternal ::invoke(member, plen, m, piv.data(),
                                                  ps, A.data(), as1, as0);
      }
      return 0;
    }
  };

} // namespace Tines

#endif
