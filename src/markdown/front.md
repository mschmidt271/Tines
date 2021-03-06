# TINES - Time Integration, Newton and Eigen Solver

1\.  [Introduction](#introduction)  
1.1\.  [Citing](#citing)  
2\.  [Building TINES](#buildingtines)  
2.1\.  [Download Libraries](#downloadlibraries)  
2.2\.  [Building Libraries and Configuring TINES](#buildinglibrariesandconfiguringtines)  
2.2.1\.  [Kokkos](#kokkos)  
2.2.2\.  [TINES](#tines)  
2.2.3\.  [GTEST](#gtest)  
3\.  [Interface to Source Term Function](#interfacetosourcetermfunction)  
4\.  [Numerical Jacobian](#numericaljacobian)  
4.1\.  [Interface to Numerical Jacobian Evaluations](#interfacetonumericaljacobianevaluations)  
5\.  [Compute Analytic Jacobians using Sacado](#computeanalyticjacobiansusingsacado)  
6\.  [Newton Solver](#newtonsolver)  
7\.  [Time Integration](#timeintegration)  
7.1\.  [TrBDF2](#trbdf2)  
7.2\.  [TrBDF2 for DAEs](#trbdf2fordaes)  
7.3\.  [Timestep Adjustment](#timestepadjustment)  
7.4\.  [Interface to Time Integrator](#interfacetotimeintegrator)  
8\.  [Eigen Solver](#eigensolver)  
8.1\.  [Interface to Eigen Solver](#interfacetoeigensolver)  
9\.  [Acknowledgement](#acknowledgement)  

<a name="introduction"></a>

## 1\. Introduction

TINES is an open source software library that provides a set of algorithms for solving many stiff time ordinary differential equations (ODEs) and/or differential algebraic equations (DAEs) using a batch hierarchical parallelism. The code is written using a parallel programming model i.e., Kokkos to future-proof the next generation parallel computing platforms such as GPU accelerators. The code is developed to support Exascale Catalytic Chemistry (ECC) Project. In particular it focuses on problems arising from catalytic chemistry applications, e.g. TChem and CSPlib. However, the library provides fundamental math tools that can aid other research projects or production frameworks.

The software provides the following capabilities:
1. **Jacobians**
   The code provides adaptive schemes [1] for computing numerical Jacobians matrices for a given source term function provided by the user. We also provide an interface for computing analytic Jacobians using SACADO (auto derivative data type) library [2]. The same source term function, as for the numerical Jacobian estimates, can be reused for analytical Jacobian computations by switching to a SACADO type as its template parameter for the value type.   

2. **Team Level Dense Linear Algebra using Kokkos Hierarchical Parallelism**
   The software primarily provides a batch parallelism framework that can be used to solve for many samples in parallel, e.g. for large scale sensitivity study that require a large number of model evaluations. In general, exploiting the batch parallelism only is not enough to efficiently use massively parallel computing architectures like GPUs. To expand the number of operations that can be performed in parallel, we also use Kokkos nested team-level parallelism to solve a single instance of the problem. On host CPU platforms, LAPACKE and CBLAS are used for single model evaluations while OpenMP is used for batch parallelism.

3. **Time Integration**
   TINES provides a stable implicit time integration scheme i.e., the second order Trapezoidal Backward Difference Formula (TrBDF2) [3]. As the name specifies, the scheme consists of trapezoidal rule to start the time step and the second order BDF for the remainder of the time step. The scheme is L-stable and suitable for robustly solving stiff systems of ODEs. The time step is adjustable using a local error estimator. In a batch parallel study, each sample can adjust its own time step size.

4. **Eigen Solver**
   A hybrid GPU version of a batched eigensolver is implemented for the Computational Singular Perturbation (CSP) analysis of Jacobian matrices. The implementation follows the Francis double shifting QR algorithm [4,5] for un-symmetric eigenproblems.   

[1]: D.E. Salane, Adaptive Routines for Forming Jacobians Numerically, SAND-86-1319

[2]: E.T. Phipps et. al., Large-Scale Transient Sensitivity Analysis of a Radiation-Damaged Bipolar Junction Transistor via Automatic Differentiation

[3]: R. E. Bank et. al.,, "Transient Simulation of Silicon Devices and Circuits," in IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems doi: 10.1109/TCAD.1985.1270142 (1985)

[4]: J.G.F. Francis, "The QR Transformation, I", The Computer Journal, 1961

[5]: J.G.F. Francis, "The QR Transformation, II". The Computer Journal, 1962

<a name="citing"></a>

### 1.1\. Citing

* Kyungjoo Kim, Oscar Diaz-Ibarra, Cosmin Safta, and Habib Najm, TINES - Time Integration, Newton and Eigen Solver, Sandia National Laboratories, SAND 2021-XXXX, 2021.*
<a name="buildingtines"></a>

## 2\. Building TINES

TINES requires Kokkos and the code uses CBLAS and LAPACKE interfaces from OpenBLAS or Intel MKL. For testing, we use GTEST library. For convenience, we explain how to build the TINES using the following environment variables.

```
/// repositories
export KOKKOS_REPOSITORY_PATH=/where/you/clone/kokkos/git/repo
export TINES_REPOSITORY_PATH=/where/you/clone/tines/git/repo
export GTEST_REPOSITORY_PATH=/where/you/clone/gtest/git/repo

/// build directories
export KOKKOS_BUILD_PATH=/where/you/build/kokkos
export TINES_BUILD_PATH=/where/you/build/tines
export GTEST_BUILD_PATH=/where/you/build/gtest

/// install directories
export KOKKOS_INSTALL_PATH=/where/you/install/kokkos
export TINES_INSTALL_PATH=/where/you/install/tines
export GTEST_INSTALL_PATH=/where/you/install/gtest
export OPENBLAS_INSTALL_PATH=/where/you/install/openblas
export LAPACKE_INSTALL_PATH=/where/you/install/lapacke
```

<a name="downloadlibraries"></a>

### 2.1\. Download Libraries

Clone Kokkos and TINES repositories.

```
git clone https://github.com/kokkos/kokkos.git ${KOKKOS_REPOSITORY_PATH};
git clone https://github.com/google/googletest.git ${GTEST_REPOSITORY_PATH}
git clone getz.ca.sandia.gov:/home/gitroot/math_utils ${TINES_REPOSITORY_PATH};
```

Here, we assume that TPLs (OpenBLAS and LAPACKE) are compiled and installed separately as these TPLs can be easily built using a distribution tool e.g., apt, yum, macports. We also recommend to turn off the threading capability of these TPLs as OpenMP is used for processing batch parallelism.

<a name="buildinglibrariesandconfiguringtines"></a>

### 2.2\. Building Libraries and Configuring TINES

<a name="kokkos"></a>

#### 2.2.1\. Kokkos

This example builds Kokkos on Intel SandyBridge architectures and install it to ``${KOKKOS_INSTALL_PATH}``. For other available options, see [Kokkos github pages](https://github.com/kokkos/kokkos).

```
cd ${KOKKOS_BUILD_PATH}
cmake \
    -D CMAKE_INSTALL_PREFIX="${KOKKOS_INSTALL_PATH}" \
    -D CMAKE_CXX_COMPILER="${CXX}"  \
    -D Kokkos_ENABLE_SERIAL=ON \
    -D Kokkos_ENABLE_OPENMP=ON \
    -D Kokkos_ENABLE_DEPRECATED_CODE=OFF \
    -D Kokkos_ARCH_SNB=ON \
    ${KOKKOS_REPOSITORY_PATH}
make -j install
```

To compile for NVIDIA GPUs, one can customize the cmake script. Note that we use ``nvcc_wrapper`` provided by Kokkos as its compiler. The architecture flag in the following example indicates that the host architecture is Intel SandyBridge and the GPU is a NVIDIA Volta 70 generation. With Kokkos 3.1, the CUDA architecture flag is optional (the script automatically detects a correct CUDA arch flag).
```
cd ${KOKKOS_BUILD_PATH}
cmake \
    -D CMAKE_INSTALL_PREFIX="${KOKKOS_INSTALL_PATH}" \
    -D CMAKE_CXX_COMPILER="${KOKKOS_REPOSITORY_PATH}/bin/nvcc_wrapper"  \
    -D Kokkos_ENABLE_SERIAL=ON \
    -D Kokkos_ENABLE_OPENMP=ON \
    -D Kokkos_ENABLE_CUDA:BOOL=ON \
    -D Kokkos_ENABLE_CUDA_UVM:BOOL=OFF \
    -D Kokkos_ENABLE_CUDA_LAMBDA:BOOL=ON \
    -D Kokkos_ENABLE_DEPRECATED_CODE=OFF \
    -D Kokkos_ARCH_VOLTA70=ON \
    -D Kokkos_ARCH_SNB=ON \
    ${KOKKOS_REPOSITORY_PATH}
make -j install
```

It its worth for noting that (1) the serial execution space is enabled for Kokkos when the serial execution space is used for example and unit tests and (2) when CUDA is enabled, we do not explicitly use universal virtual memory (UVM). A user can enable UVM if it is used for the application. However, TINES does not assume that the host code can access device memory.

<a name="tines"></a>

#### 2.2.2\. TINES

Compiling TINES follows Kokkos configuration settings, also available at ``${KOKKOS_INSTALL_PATH}``. The OpenBLAS and LAPACKE libraries are required on the host device. These provide optimized dense linear algebra tools. When an Intel compiler is available, one can replace these libraries with Intel MKL by adding an option ``TINES_ENABLE_MKL=ON``. On Mac OSX, we use the OpenBLAS library managed by **macports**. This version of the OpenBLAS has different header names and we need to distinguish this version of the code from others which are typically used in Linux distributions. To discern the two version of the code, cmake looks for ``cblas_openblas.h`` to tell that the installed version is from MacPorts. This mechanism can be broken if MacPorts' OpenBLAS is changed later. The MacPorts OpenBLAS version also include LAPACKE interface and one can remove ``LAPACKE_INSTALL_PATH`` from the configure script. SACADO library is a header only library and it is included in the TINES distributions.

```
cd ${KOKKOS_BUILD_PATH}
cmake \
    -D CMAKE_INSTALL_PREFIX=${TINES_INSTALL_PATH} \
    -D CMAKE_CXX_COMPILER="${CXX}" \
    -D CMAKE_CXX_FLAGS="-g" \
    -D TINES_ENABLE_DEBUG=OFF \
    -D TINES_ENABLE_VERBOSE=OFF \
    -D TINES_ENABLE_TEST=ON \
    -D TINES_ENABLE_EXAMPLE=ON \
    -D KOKKOS_INSTALL_PATH="${HOME}/Work/lib/kokkos/install/butter/release" \
    -D GTEST_INSTALL_PATH="${HOME}/Work/lib/gtest/install/butter/release" \
    -D OPENBLAS_INSTALL_PATH="${OPENBLAS_INSTALL_PATH}" \
    -D LAPACKE_INSTALL_PATH="${LAPACKE_INSTALL_PATH}" \
    ${TINES_REPOSITORY_PATH}/src
make -j install
```

For GPUs, the compiler is changed with ``nvcc_wrapper`` by adding ``-D CMAKE_CXX_COMPILER="${KOKKOS_INSTALL_PATH}/bin/nvcc_wrapper"``.

<a name="gtest"></a>

#### 2.2.3\. GTEST

We use GTEST as our testing infrastructure. GTEST can be compiled and installed using the following cmake script

```
cd ${GTEST_BUILD_PATH}
cmake \
    -D CMAKE_INSTALL_PREFIX="${GTEST_INSTALL_PATH}" \
    -D CMAKE_CXX_COMPILER="${CXX}"  \
    ${GTEST_REPOSITORY_PATH}
make -j install
```
<a name="interfacetosourcetermfunction"></a>

## 3\. Interface to Source Term Function

TINES uses a so-called ``struct Problem`` to interface with user-defined source term functions. The basic design of the problem interface is intended to be used in a Newton solver and has two template arguments. ``ValueType`` represents either a built-in scalar type such as ``double`` or the SACADO type ``SLFAD<double,FadLength>`` to use automatic derivatives (AD) types with template polymorphism.  ``DeviceType`` is a pair of execution and memory spaces, abstracted by ``Kokkos::Device<ExecSpace,MemorySpace>``. These template parameters give us an opportunity to perform partial specialization for different data type and different execution spaces e.g., GPUs.
```
template<typename ValueType,
         typename DeviceType>
struct ProblemExample {
  using value_type = ValueType;
  using device_type = DeviceType;

  KOKKOS_DEFAULTED_FUNCTION
  ProblemExample() = default;
};
```

All member functions are decorated with ``KOKKOS_INLINE_FUNCTION`` where the Kokkos macro states that the function is callable as a device function. For both ODE and DAE configurations, the struct requires the following member functions specifying the number of ODEs and the number of constraint equations,
```
KOKKOS_INLINE_FUNCTION int ProblemExample::getNumberOfTimeODEs() const;
KOKKOS_INLINE_FUNCTION int ProblemExample::getNumberOfConstraints() const;
KOKKOS_INLINE_FUNCTION int ProblemExample::getNumberOfEquations() const {
  return getNumberOfTimeODEs() + getNumberOfConstraints();
}
```

The following basic interface are required for the Newton solver: 1) setting initial values, 2) computing right-hand side function, and 3) computing Jacobians.

The input vector, $x$, is initialized by ``computeInitValues`` interface. The function has a template argument ``MemberType`` that represents the ``Kokkos::Team`` object. The Kokkos team object can be understood as a thread communicator and a team of threads are cooperatively used in parallel to solve a problem. The Kokkos hierarchical team parallelism is critical in processing on many-thread architectures like GPUs. Almost all device functions decorated with ``KOKKOS_INLINE_FUNCTION`` have this member object as their input argument to control thread mapping to workloads and their synchronizations.   
```
/// [in]  member - Kokkos team object specifying a group of team threads.
/// [out] x - an input vector to be initialized
using real_type_1d_view_type = Tines::value_type_1d_view_type<value_type,device_type>;
template <typename MemberType>
KOKKOS_INLINE_FUNCTION void
ProblemExample::computeInitValues(const MemberType &member,
                                  const real_type_1d_view_type &x) const;
```

The right-hand side function evaluation is required to proceed the Newton iterations and the interface is illustrated below. This interface is also used for computing numerical Jacobians using finite difference schemes. It is also worth noting that the same source term works with SACADO AD types for computing analytic Jacobians.
```
/// [in]  member - Kokkos team object specifying a group of team threads.
/// [in]  x - input variables
/// [out] f - function output
template <typename MemberType>
KOKKOS_INLINE_FUNCTION void
ProblemExample::computeFunction(const MemberType &member,
                                const real_type_1d_view_type &x,
                                const real_type_1d_view_type &f) const;
```

The Jacobian interface is provided next. Although the user can provide Jacobian function written separately, one can just rely on TINES numerical Jacobian scheme or the analytic Jacobian computations via SACADO. More details will be provided later.
```
template <typename MemberType>
KOKKOS_INLINE_FUNCTION void
ProblemExample::computeAnalyticJacobian(const MemberType &member,
                                        const real_type_1d_view_type &x,
                                        const real_type_2d_view_type &J) const {
  NumericalJacobianForwardDifference<value_type,device_type>
    ::invoke(member, *this, x, J);
}
```
These are the major components of the problem interface for the Newton solver. A complete code example of the problem struct is listed below.
```
template <typename ValueType, typename DeviceType>
struct ProblemExample {
  using value_type = ValueType;
  using device_type = DeviceType;
  using scalar_type = typename ats<value_type>::scalar_type;

  using real_type_1d_view_type = value_type_1d_view<real_type, device_type>;
  using real_type_2d_view_type = value_type_2d_view<real_type, device_type>;

  /// numerical jacobian workspace (see Jacobian section for details)
  real_type _fac_min, _fac_max;
  real_type_1d_view_type _fac, _f_0, _f_h;

  KOKKOS_DEFAULTED_FUNCTION
  ProblemExample() = default;

  /// users function
  KOKKOS_INLINE_FUNCTION
  int getNumberOfTimeODEs() const;

  /// users function
  KOKKOS_INLINE_FUNCTION
  int getNumberOfConstraints() const;

  KOKKOS_INLINE_FUNCTION
  int getNumberOfEquations() const {
    return getNumberOfTimeODEs() + getNumberOfConstraints();
  }

  /// users may need more workspace for computing their source terms
  KOKKOS_INLINE_FUNCTION
  void workspace(int &wlen) const;

  /// for newton solver, this set the initial values on input vector x
  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION void
  computeInitValues(const MemberType &member,
                    const real_type_1d_view_type &x) const;

  /// users function
  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION void
  computeFunction(const MemberType &member,
                  const real_type_1d_view_type &x,
                  const real_type_1d_view_type &f) const;

  /// users function
  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION void
  computeNumericalJacobian(const MemberType &member,
                           const real_type_1d_view_type &x,
                           const real_type_2d_view_type &J) const;
};
```

An extension of the above problem interface is used for time integration and the Newton solver algorithms and will be discussed later.
<a name="numericaljacobian"></a>

## 4\. Numerical Jacobian

Tines has three routines to estimate Jacobian matrices numerically using the adaptive scheme described in SAND-86-1319. Three finite difference schemes are described below. All implementations require a workspace of size $2\times m$, where $m$ is the number of equations.

**Forward Differencing Routine**
Jacobians are computed using a forward finite differences. This approach is 1st order accurate and is the least expensive approach among the methods considered here because it only requires $m+1$ function evaluations.
$$
J_{ij} = \frac{f(x_i + \Delta h_j) - f(x_i )}{\Delta h_j}
$$

**Central Differencing Routine**
The central differencing scheme is 2nd order accurate and it requires $2\times m$ function evaluations.
$$
J_{ij} = \frac{f(x_i + \Delta h_j) - f(x_i - \Delta h_j )}{2  \Delta h_j}
$$

**Richardson's Extrapolation**
Jacobians are computed using a Richardson's extrapolation scheme. This scheme is 4th order accurate and the most expensive with $4\times m$ function evaluations.
$$
J_{ij} = \frac{-f(x_i + 2 \Delta h_j) + 8 f(x_i +  \Delta h_j) -8 f(x_i -  \Delta h_j)  + f(x_i - \Delta h_j )}{12  \Delta h_j}
$$

**Adjustment of Differencing Size**
The quality of the numerical derivative largely depends on the choice of the increment $\Delta h_j$. If the increment is too small, round-off error degrades the numerical derivatives. On the other hand, using a large increment causes truncation error. To control the differencing size we adopt the strategy proposed by Salane [ref] which used an adaptive approach to determine the differencing size in a sequence of Jacobian evaluations for solving a non-linear problem.

The increment $\Delta h_j$ is defined as the absolute value of a factor $\mathrm{fac}_j$ of the $x_j$ plus the machine error precision ($\epsilon$); to avoid devision by zero in the case where $x_j=0$.
$$
\Delta h_j = |\mathrm{fac}_j\times x_j | + \epsilon
$$

The value of $\mathrm{fac_j}$ is updated after the Jacobian is computed.
$$
\mathrm{fac}_j =
\begin{cases}
\mathrm{fac}_{\mathrm{min}} \text{ : if } \mathrm{fac}_j^{\mathrm{prev}} < \mathrm{fac}_{\mathrm{min}} \\
\mathrm{fac}_{\mathrm{max}} \text{ : if } \mathrm{fac}_j^{\mathrm{prev}} > \mathrm{fac}_{\mathrm{max}} \\
\mathrm{fac}_j^{\mathrm{prev}} \text{ : else }\\
\end{cases}
$$

In the expression above $\mathrm{fac}_{\mathrm{min}}$ and $\mathrm{fac}_{\mathrm{max}}$ are the lower and upper bounds of $\mathrm{fac}_j$. These bounds are set to $\mathrm{fac}_{\mathrm{min}}=\epsilon ^ {3/4}$ and $\mathrm{fac}_{\mathrm{max}}=(2 \epsilon)^{1/2}$ unless the user provides its own min/max values. The value of $\mathrm{fac}_j^{\mathrm{prev}}$ is used when evaluating Jacobians and the increment factors ($\mathrm{fac}_j$) are refined by examining the function values.
Cosmin: perhaps re-work the statement above for clarity.
$$
\mathrm{fac}_j =
\begin{cases}
\mathrm{fac}_j\times\epsilon^{1/2} \text{ : if } \mathrm{diff} > \epsilon ^{1/4}\times\mathrm{scale}\text{ (1)) }  \\
\mathrm{fac}_j/\epsilon^{1/2} \text{ : if } \mathrm{diff} > \epsilon ^{7/8}\times\mathrm{scale} \ \mathrm{and} \ \mathrm{diff} < \epsilon ^{3/4}\times\mathrm{scale}  \text{ (2)}\\
(\mathrm{fac}_j)^{1/2} \text{ : if } \ \mathrm{diff} < \epsilon ^{7/8}\times\mathrm{scale} \text{ (3)}\\
\mathrm{fac}_j  \text{ : otherwise}\\
\end{cases}
$$
where $\mathrm{diff} = |f_{h}(k) - f_{0}(k)|$ and $\mathrm{scale} = \mathrm{max}(|f_{h}(k)|, |f_{l}(k)|)$ and $k = \mathrm{argmax_i |f_h(i) - f_0(i)|}$. For case (1) above the trunctation error is dominant, while the round-off error is dominant for both (2) and (3). This schedule is designed for forward difference schemes. The diff function can be updated for other differencing schemes. This workflow is designed for solving a non-linear problems for which Jacobians matrices are iteratively evaluated with evolving input variables ($x_j$).

Cosmin: pls consider if you can use for example $f_j$ instead of $fac_j$... or other one letter notation. Then you can mention below that fac is the coded version of f. Just a thought.

<a name="interfacetonumericaljacobianevaluations"></a>

### 4.1\. Interface to Numerical Jacobian Evaluations
Including the adaptive workflow, numerical Jacobians are evaluated with a factor array. The corresponding workspace (``fac`` array)  is provided by users when defining the problem struct. The ``fac`` array is refined when the Jacobian is evaluated in a sequence of Newton iterations. The code below describes the interface for the numerical Jacobian evaluated in TINES. Here, we show an example for the forward difference scheme. Similar interfaces can be used for the other schemes.

```
/// Problem interface example.
template <typename ValueType, typename DeviceType>
struct ProblemExample {
  /// users parameters for increment factors  
  real_type _fac_min, _fac_max;
  real_type_1d_view_type _fac;

  /// workspace for computing numerical jacobians (2m)
  real_type_1d_view_type _f_0, _f_h;

  /// source term function
  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION void
  computeFunction(const MemberType &member,
                  const real_type_1d_view_type &x,
                  const real_type_1d_view_type &f) const;

  /// numerical jacobian is used
  template <typename MemberType>
  KOKKOS_INLINE_FUNCTION void
  computeJacobian(const MemberType &member,
                  const real_type_1d_view_type &x,
                  const real_type_2d_view_type &J) const {
    NumericalJacobianForwardDifference<real_type, device_type>
      ::invoke(member, *this, _fac_min, _fac_max, _fac, x, _f_0, _f_h, J);
    member.team_barrier();
  }
}

/// Function to evaluate numerical Jaocobian using problem interface.
template<typename ValueType, typename DeviceType>  
struct NumericalJacobianForwardDifference
{
  /// [in] member - Kokkos team
  /// [in[ problem - abstraction for computing a source term
  /// [in] fac_min - minimum value for fac
  /// [in] fac_max - maximum value for fac
  /// [in] fac - fac for last iteration
  /// [in] x - input values
  /// [out] J - numerical jacobian
  /// [scratch] work - work array sized by wlen given from workspace function
  static void invoke(const MemberType& member,
         const ProblemType<real_type,device_type>& problem,
         const real_type& fac_min,
         const real_type& fac_max,
         const real_type_1d_view_type& fac,
         const real_type_1d_view_type& x,
         const real_type_2d_view_type& J,
         const real_type_1d_view_type& work);
};
```  
<a name="computeanalyticjacobiansusingsacado"></a>

## 5\. Compute Analytic Jacobians using Sacado

TINES provides analytic Jacobian evaluations using automatic differentiation (AD) via the [SACADO](https://docs.trilinos.org/dev/packages/sacado/doc/html/index.html) library. The AD computes derivatives using the chain rule for basic arithmetic operations and known analytic derivatives. SACADO is implemented using operator overloading so that users can convert their scalar based function to compute derivatives by replacing the input types with template parameters. This allows to compute the analytic Jacobians elements using the same source term functions. We illustrate the user interface with the following example.
```
template<typename ValueType, typename DeviceType>
struct ProblemSacadoExample
{
  /// in this particular example, we consider ValueType is SACADO FadType
  /// e.g., SLFad<double,FadLength>
  using fad_type = ValueType;
  using device_type = DeviceType;

  /// scalar_type is defined as "double"
  using scalar_type = typename ats<fad_type>::scalar_type;

  /// view interface
  using real_type = scalar_type;
  using real_type_1d_view_type = value_type_1d_view<real_type,device_type>;
  using real_type_2d_view_type = value_type_2d_view<real_type,device_type>;

  /// sacado is value type
  using fad_type_1d_view_type = value_type_1d_view<fad_type,device_type>;
  using fad_type_2d_view_type = value_type_2d_view<fad_type,device_type>;

  /// workspace for interfacing fad type view for x and f
  real_type_1d_view_type _work;

  /// source term function interface with fad type view
  template<typename MemberType>
  KOKKOS_INLINE_FUNCTION
  void computeFunction(const MemberType& member,
                       const fad_type_1d_view_type& x,
                       const fad_type_1d_view_type& f) const
  {
    /// here, we call users source term function
    UserSourceTermFunction(member, x, f);
  }

  /// source term function interface with real type view
  template<typename MemberType>
  KOKKOS_INLINE_FUNCTION
  void computeFunction(const MemberType& member,
                       const real_type_1d_view_type& x,
                       const real_type_1d_view_type& f) const
  {
    /// here, we call the same users source term function
    UserSourceTermFunction(member, x, f);
  }

  /// analytic Jacobian is computed via computeFunction
  template<typename MemberType>
  KOKKOS_INLINE_FUNCTION
  void computeJacobian(const MemberType& member,
                       const real_type_1d_view_type& s,
                       const real_type_2d_view_type& J) const {
    /// as the input is real type view, we need to convert
    /// them to fad type view                
    const int m = getNumberOfEquations();
    const int fad_length = fad_type().length();

    real_type *wptr = _work.data();
    /// wrapping the work space for fad type
    /// fad type view is 1D but its construction is considered as 2D
    /// including the hidden dimension for storing derivatives
    fad_type_1d_view_type x(wptr, m, m+1); wptr += m*fad_length;
    fad_type_1d_view_type f(wptr, m, m+1); wptr += m*fad_length;

    /// assign scalar values with derivative indices
    Kokkos::parallel_for
      (Kokkos::TeamVectorRange(member, m),
       [=](const int &i) { x(i) = value_type(m, i, s(i)); });
    member.team_barrier();

    /// invoke computeFunction with fad type views
    computeFunction(member, x, f);
    member.team_barrier();

    /// extract Jacobian
    Kokkos::parallel_for
      (Kokkos::TeamThreadRange(member, m), [=](const int &i) {
        Kokkos::parallel_for
         (Kokkos::ThreadVectorRange(member, m),
          [=](const int &j) {
             J(i, j) = f(i).fastAccessDx(j);
         });
    });
    member.team_barrier();
  }
}  
```
For a complete example, we refer the example described in ``${TINES_REPOSITORY_PATH}/src/example/Tines_AnalyticJacobian.cpp``.
<a name="newtonsolver"></a>

## 6\. Newton Solver

A team-parallel Newton solver is implemented for the solution of non-linear equations. The solver iteratively solves for the solution of problems cast as $F(x) = 0$. The Newton iteration starts with an initial guess $x^{(0)}$ and proceeds to refine the solution in a sequence $x^{(1)}, x^{(2)}, ...$ until it meets the convergence criteria. The sequence of the solution is updated as
$$
x^{(n+1)} = x^{(n)} - J(x^{(n)})^{-1} (x^{(n)}) \quad \textrm{where}  \quad J_{ij}(x) = \frac{\partial F_i(x)}{\partial x_j}
$$

The solver uses a dense linear solver to compute $J(x_{n})^{-1} (x_{n})$. When the Jacobian matrix is rank-defficient, a pseudo inverse is used instead.

For a stopping criterion, we use the weighted root-mean-square (WRMS) norm. A weighting factor is computed as
$$
w_i = 1/\left( \text{rtol}_i | x_i | + \text{atol}_i \right)
$$
and the normalized error norm is computed as follows.
$$
\text{norm} = \left( \sum_i^m \left( \text{err}_i*w_i \right)^2 \right)/m
$$
where $err_i=x_i^{(n+1)}-x_i^{(n)}$ is the solution change for component $i$ between two successive Newton solves. The solution is considered converged when the norm above is close to 1.

The Newton solver uses the following interface. For the problem interface, see the [Problem]() section and the code example described in ``${TINES_REPOSITORY_PATH}/src/example/time-integration/Tines_NewtonSolver.cpp``
```
/// Newton solver interface
template <typename ValueType, typename DeviceType>
struct NewtonSolver {
  /// [in] member - Kokkos team
  /// [in] problem - problem object given from users
  /// [in] atol - absolute tolerence checking for convergence
  /// [in] rtol - relative tolerence checking for convergence
  /// [in] max_iter - the max number of Newton iterations
  /// [in/out] x - solution vector which is iteratively updated
  /// [out] dx - increment vector that is used for updating "x"
  /// [work] f - workspace for evaluating the function
  /// [work] J - workspace for evaluating the Jacobian
  /// [work] w - workspace used in linear solver
  /// [out] iter_count - Newton iteration count at convergence
  /// [out] convergence - a flag to indicate convergence of the solution
  template <typename MemberType, typename ProblemType>
  KOKKOS_INLINE_FUNCTION static void
  invoke(const MemberType &member,
         const ProblemType &problem, const real_type &atol,
         const real_type &rtol, const int &max_iter,
         const real_type_1d_view_type &x,
         const real_type_1d_view_type &dx, const real_type_1d_view_type &f,
         const real_type_2d_view_type &J,
         const real_type_1d_view_type &work,
         int &iter_count,
         int &converge);
}
```
<a name="timeintegration"></a>

## 7\. Time Integration

When solving a *stiff* time ODEs, the time step size is limited by a stability condition rather than a truncation error. For these class of applications, TINES provides a 2nd order Trapezoidal Backward Difference Formula (TrBDF2) scheme. The TrBDF2 scheme is a composite single step method. The method is 2nd order accurate and $L$-stable.

<a name="trbdf2"></a>

### 7.1\. TrBDF2

Consider for example the following system of Ordinary Differential Equations (ODEs).
$$
\frac{du_{i}}{dt} = f_{i}(u,t),\,\,\, i=1,\ldots,m
$$
The TrBDF2 scheme first advances the solution from $t_{n}$ to an intermediate time $t_{n+\gamma} = t_{n} + \gamma \Delta t$ by applying the trapezoidal rule.
$$
u_{n+\gamma} - \gamma \frac{\Delta t}{2} f_{n+\gamma} = u_{n} + \gamma \frac{\Delta t}{2} f_{n}
$$
Next, it uses the BDF2 algorithm to march the solution from $t_{n+\gamma}$ to $t_{n+1} = t_{n} + \Delta t$ as follows.
$$
u_{n+1} - \frac{1-\gamma}{2-\gamma} \Delta t f_{n+1} = \frac{1}{\gamma(2-\gamma)}u_{n+\gamma} - \frac{(1-\gamma)^2}{\gamma(2-\gamma)} u_{n}
$$
We solve the above non-linear equations iteratively using the Newton method. The Newton equation of the first step is described:
$$
\left[ I - \gamma \frac{\Delta}{2} \left(\frac{\partial f}{\partial u}\right)^{(k)}\right]\delta u^{(k)} = -(u_{n+\gamma}^{(k)} - u_{n}) + \gamma \frac{\Delta t}{2}(f_{n+\gamma}^{(k)}+f_{n})
$$  
Cosmin: please check the superscripts above. Is $\delta u^{(k)}=u_{n+\gamma}^{(k+1)}-u_{n+\gamma}^{(k)}$?
Then, the Newton equation for the second step is given by
$$
\left[I-\frac{1-\gamma}{2-\gamma} \Delta t \left(\frac{\partial f}{\partial u}\right)^{(k)}\right]\delta u^{(k)} =
-\left(u_{n+1}^{(k)} - \frac{1}{\gamma(2-\gamma)} u_{n+\gamma}+\frac{(1-\gamma)^2}{\gamma(2-\gamma)}u_{n}\right) + \frac{1-\gamma}{2-\gamma}\Delta t f_{n+1}^{(k)}
$$
Here, we denote a Jacobian as $J_{prob} = \frac{\partial f}{\partial u}$. The modified Jacobian's used for solving the Newton equations for the first (trapezoidal rule) and second (BDF2) are given by
$$
A_{tr} = I - \gamma \frac{\Delta t}{2} J_{prob} \qquad
A_{bdf} = I - \frac{1-\gamma}{2-\gamma}\Delta t J_{prob}
$$
while their right-hand sides are defined as
$$
b_{tr} = -(u_{n+\gamma}^{(k)} - u_{n}) + \gamma \frac{\Delta t}{2}(f_{n+\gamma}^{(k)}+f_{n})
$$

$$
b_{bdf} = -\left(u_{n+1}^{(k)} - \frac{1}{\gamma(2-\gamma)} u_{n+\gamma}+\frac{(1-\gamma)^2}{\gamma(2-\gamma)}u_{n}\right) + \frac{1-\gamma}{2-\gamma}\Delta t f_{n+1}^{(k)}
$$
In this way, a Newton solver can iteratively solves a problem $A(u) \delta u = b(u)$ with updating $u\leftarrow u +\delta u$.

The timestep size $\Delta t$ can be adapted within a range $(\Delta t_{min}, \Delta t_{max})$ using a local error estimator.
$$
\text{error} \approx 2 k_{\gamma} \Delta t \left( \frac{1}{\gamma} f_{n} = \frac{1}{\gamma(1-\gamma)}f_{n+\gamma} + \frac{1}{1-\gamma} f_{n+1}\right) \quad \text{where} \quad  
k_{\gamma} = \frac{-3 \gamma^2 + 4 \gamma - 2}{12(2-\gamma)}
$$
Cosmin: the notation above is confusing to me. Do you mean to say that you choose delta t to match
$\frac{1}{\gamma} f_{n} = \frac{1}{\gamma(1-\gamma)}f_{n+\gamma} + \frac{1}{1-\gamma} f_{n+1}$?
This error is minimized when using a $\gamma = 2- \sqrt{2}$.

<a name="trbdf2fordaes"></a>

### 7.2\. TrBDF2 for DAEs

We consider the following system of differential-algebraic equations (DAEs).

$$
\frac{du_i}{dt} = f_i(u,v) \\
g_i(u,v)= 0
$$

Step 1.  trapezoidal rule  to advance from $t_n$ to $t_{n+\gamma}$

$$
u_{n+\gamma} - \gamma \frac{\Delta t}{2} f_{n+\gamma} = u_{n} + \gamma \frac{\Delta t}{2} f_{n} \\
\frac{g_{n+\gamma} + g_n}{2} =0
$$

Step 2. BDF

$$
u_{n+1} - \frac{1-\gamma}{2-\gamma} \Delta t f_{n+1} = \frac{1}{\gamma(2-\gamma)}u_{n+\gamma} - \frac{(1-\gamma)^2}{\gamma(2-\gamma)} u_{n} \\
g_{n+1} =0
$$

We also solve the above non-linear equations iteratively using the Newton method. The modified Jacobian's used for solving the Newton equations of the above Trapezoidal rule and the BDF2 are given as follows

$$
A_{tr}=
\left(
\begin{matrix}
 I -  \frac{\gamma \Delta t}{2} \frac{\partial f}{\partial u}\Bigr|_{\substack{v}}  & \Big |&  -  \frac{\gamma \Delta t}{2} \frac{\partial f}{\partial v}\Bigr|_{\substack{u}}     \\
\hline
 \frac{\partial g}{\partial u}\Bigr|_{\substack{v}}  &  \Big |&   \frac{\partial g}{\partial v}\Bigr|_{\substack{u}}
\end{matrix}
\right)
$$

$$
A_{bdf}=
\left(
\begin{matrix}
 I  - \frac{1-\gamma}{2 - \gamma}\Delta t \frac{\partial f_{n+1}}{\partial u}\Bigr|_{\substack{v}}  & \Big |& - \frac{1-\gamma}{2 - \gamma}\Delta t \frac{\partial f_{n+1}}{\partial v}\Bigr|_{\substack{u}}   \\
 \hline
 \frac{\partial g}{\partial u}\Bigr|_{\substack{v}}  &  \Big |&   \frac{\partial g}{\partial v}\Bigr|_{\substack{u}}
\end{matrix}
\right)
$$

<a name="timestepadjustment"></a>

### 7.3\. Timestep Adjustment

TINES uses weighted root-mean-square (WRMS) norms as discussed in [Newton solver]() when evaluating the estimated error. This approach is used in [Sundial package](https://computing.llnl.gov/sites/default/files/public/ida_guide.pdf). This error norm close to 1 is considered as *small* and we increase the time step size and if the error norm is bigger than 10, the time step size decreases by half.

<a name="interfacetotimeintegrator"></a>

### 7.4\. Interface to Time Integrator

The code in the below describes the interface of TINES time integrator.
```
template<typename ValueType,typename DeviceType>
struct TimeIntegratorTrBDF2 {
  /// [in] m - the number of variables
  /// [out] wlen - real type array length
  static void workspace(const int m, int& wlen);

  /// [in] member - Kokkos thread communicator
  /// [in[ problem - abstraction for computing a source term and its Jacobian
  /// [in] max_num_newton_iterations - max number of newton iterations for each nonlinear solve
  /// [in] max_num_time_iterations - max number of time iterations
  /// [in] tol_newton - a pair of abs/rel tolerence for the newton solver
  /// [in] tol_time - pairs of abs/rel tolerence corresponding to different variables
  /// [in] dt_in - current time step size (possibly from a restarting point)
  /// [in] dt_min - minimum time step size
  /// [in] dt_max - maximum time step size  
  /// [in] t_beg - time to begin
  /// [in] t_end - time to end
  /// [in[ vals - input state variables at t_beg
  /// [out] t_out - time when reaching t_end or being terminated by max number time iterations
  /// [out] dt_out - delta time when reaching t_end or being terminated by max number time iterations
  /// [out] vals_out - state variables when reaching t_end or being terminated by max number time iterations
  /// [scratch] work - work array sized by wlen given from workspace function
  ///
  /// Note that t_out, dt_out, vals_out can be used as an input to restart time integration
  static int invoke(const MemberType& member,
                      const ProblemType<real_type,device_type>& problem,
                      const int& max_num_newton_iterations,
                      const int& max_num_time_iterations,
                      const real_type_1d_view_type& tol_newton,
                      const real_type_2d_view_type& tol_time,
                      const real_type& dt_in,
                      const real_type& dt_min,
                      const real_type& dt_max,
                      const real_type& t_beg,
                      const real_type& t_end,
                      const real_type_1d_view_type& vals,
                      const real_type_0d_view_type& t_out,
                      const real_type_0d_view_type& dt_out,
                      const real_type_1d_view_type& vals_out,
                      /// workspace
                      const real_type_1d_view_type& work);
```  
This ``TimeIntegrator`` code requires for a user to provide a problem object. A problem class includes the following interface.
```
template<typename ValueType,typename DeviceType>
struct MyProblem {
  ordinal_type getNumberOfTimeODEs();
  ordinal_type getNumberOfConstraints();
  ordinal_type getNumberOfEquations();

  /// temporal workspace necessary for this problem class
  void workspace(int &wlen);

  /// x is initialized in the first Newton iteration
  void computeInitValues(const MemberType& member,
                         const real_type_1d_view_type& x) const;

  /// compute f(x)
  void computeFunction(const MemberType& member,
                       const real_type_1d_view_type& x,
                       const real_type_1d_view_type& f) const;

  /// compute J_{prob} at x                       
  void computeJacobian(const MemberType& member,
                       const real_type_1d_view_type& x,
                       const real_type_2d_view_type& J) const;
};
```
<a name="eigensolver"></a>

## 8\. Eigen Solver

A batched eigen solver is developed in TINES. We implemented the standard Francis double shifting algorithm using for unsymmetric real matrices that ranges from 10 to 1k. The code uses Kokkos team parallelism (a group of threads) for solving a single instance of the eigen problem where the batch parallelism is implemented with Kokkos parallel-for.

The standard eigenvalue problem is described
$$
A v = \lambda v
$$
where $A$ is a matrix and the $\lambda$ and $v$ are corresponding eigen values and vectors. The QR algorithm is simple that it repeats 1) decompose $A = QR$ and 2) update $A = RQ$. To reduce the computational cost of the QR factorization, the QR algorithm can be improved using the Hessenberg reduction where the Householder transformation is applied to nonzero part of the Hessenberg form. To accelerate the convergence of eigen values, shifted matrix $A-\sigma I$ is used. The famous Francis QR algorithm consists of three phases: 1) reduction to Hessenberg form, 2) Schur decomposition using the double shifted QR iterations, and 3) solve for eigen vectors. As LAPACK is available for CPU platforms where the batch parallelism is implemented with OpenMP parallel-for, we focus on the GPU team-parallel implementation of the batch-parallel eigen solver.

**Reduction to Upper Hessenberg Form**

We perform a reduction to upper Hessenberg form by applying successive Householder transformation to a mtraix $A$ from both sides such that
$$
A = Q H Q^T
$$
where $A$ is a $n\times n$ real matrix and $Q$ is an orthogonal matrix, and $H$ is upper Hessenberg form. The orthogonal matrix is represented as a product of Householder transformations
$$
Q = H(0) H(1)... H(n-3)
$$
where $H(i) = I - \tau v v^T$ representing a Householder transformation that annihilates column entries of $(i+2:n,i)$. A basic algorithm described in the following pseudo code.
```
/// [in/out] A - input matrix; upper Hessenberg form on exit
/// [out] Q - orthogonal matrix Q = H(0) H(1) ... H(n-3)
for (int i=0;i<(n-2);++i) {
  /// compute Householder transformation, H(i) = I - 2*u*u^T
  /// reflectors are stored as "u" vector
  ComputeHouseholder(A(i+2:n,i));

  /// take the Householder vector to apply the transformation to the trailing part of the matrix
  u = A(i+2:n,i);

  /// apply from left, A := H(i) A
  ApplyLeftHouseholder(u, A(i+2:n,i:n)

  /// apply from right, A := A H(i)
  ApplyRightHouseholder(u, A(0:n,i+2:n)
}

/// Q = I
SetIdentity(Q);
for (int i=(n-3);i>=0;--i) {
  u = A(i+2:n,i);
  ApplyLeftHouseholder(u, Q(i+2:n,i+2:n));
}
```
The source of the parallelism in this code comes from The ``Apply{Left/Right}Householder`` where each entry of the part of $A$ can be concurrently updated by rank-one update. We also note that there is a blocked version for accumulating and applying the Householder vectors. However, we do not use the blocked version as it is difficult to gain efficiency from the blocked algorithm for small problem sizes.

**Schur Decomposition**

After the Hessenberg reduction is performed, we compute its Schur decomposition such that
$$
H = Z T Z^H
$$
where $T$ is quasi upper triangular matrix and $Z$ is an orthogoanl matrix of Schur vectors. Eigen values appear in any order along the diagonal entries of the Schur form. $1\times 1$ blocks represent real eigen values and $2\times 2$ blocks correspond to conjugate complex eigen values.

The Schur decomposition is computed using the Francis double shift QR algorithm. Here, we just sketch the algorithm to discuss its computational aspects. For details of the Francis algorithm, we recommend following books: G.H. Golub and C.F. van Loan, Matrix Computations and D.S. Watkins, Fundamentals of Matrix Computations.
1. Set an active submatrix of the input Hessenberg matrix, H := H(1:p,1:p) and let $\sigma$ and $\bar{\sigma}$ are the complex pair of eigen values of the last diagonal $2\times 2$ block.
2. Perform two step QR iterations with a conjugate pair of shifts and form the real matrix $M = H^2 - sH + tI$ where $s = 2Re(\sigma)$ and $t = |\sigma|^2$.
3. Update $H := Z^T H Z$ where $Z$ is the QR factorization of the matrix $M$.
4. Repeat the step 2 and 3 until it converges to the real or complex eigen values.
5. Adjust $p$ and reduce the submatrix size and repeat from 1.
Using the implicit-Q theorem, the QR factorization of the step 3 can be computed by applying a sequence of inexpensive Householder transformations. This is called chasing bulge and the algorithm is essentially sequential, which makes it difficult to efficiently parallelize the QR iterations on GPUs. Thus, we choose to implement an hybrid algorithm computhing the Francis QR algorithm on CPU platforms.  

**Solve for Right Eigen Vectors**

After the Schur form is computed, corresponding eigen vectors are computed by solving a singular system. For instance, consider following partitioned matrix with $i$th eigen value and eigen vector
$$
T - t_{ii} I =
\left(
\begin{matrix}
  T_{TL} & \Big |& T_{TR} \\ \hline
       0 & \Big |& T_{BR} \\
\end{matrix}
\right)
\quad\quad
v = \left(
\begin{matrix}
v_{T} \\ \hline
v_{B}
\end{matrix}
\right)
$$
Then, the equation $(T-t_{ii}I)v$ translates to
$$
\begin{matrix}
T_{TL} v_T + & T_{TR} v_B = 0 \\
\quad & T_{BR} v_B = 0 \\
\end{matrix}
$$
where $T_{TL}$ and $T_{BR}$ are upper triangular matrices. Since $T_{BR}$ is non-singular, $v_B$ is zero. Next, we partition $T_{TL}$ again so that
$$
T_{TL} =
\left(
\begin{matrix}
  S & \Big |& r \\ \hline
       0 & \Big |& 0 \\
\end{matrix}
\right)
\quad\quad
v_T = \left(
\begin{matrix}
u \\ \hline
w
\end{matrix}
\right)
$$
This reads the equation $T_{TL} v_T = 0$ as
$$
Su + rw = 0
$$.
By setting $w=1$, we can compute $u = -S^{-1} r$. As each eigen vector can be computed independently, a team of threads can be distributed for computing different eigen vectors. The eigen vectors of the given matrix $A$ are computed by multiplying the $Q$ and $Z$. 


<a name="interfacetoeigensolver"></a>

### 8.1\. Interface to Eigen Solver

A user may want to use our "device" level interface for solving many eigen problems in parallel. A device-level interface takes an input argument of ``exec_instance`` representing an execution space instance i.e., ``Kokkos::OpenMP()`` and ``Kokkos::Cuda()``.
```
/// [in] exec_instance - Kokkos execution space instance (it can wrap CUDA stream)
/// [in] A - array of input matrices
/// [out] er - array of real part of eigen vectors
/// [out] ei - array of imaginery part of eigen vectors
/// [out] V - array of right eigen vectors
/// [in] W - workspace array
/// [in] user_tpl_if_avail - a flag to indicate to use tpl when it is available
template <typename SpT>
struct SolveEigenvaluesNonSymmetricProblemDevice {
  static int invoke(const SpT &exec_instance,
                    const value_type_3d_view<double, typename UseThisDevice<SpT>::type> &A,
                    const value_type_2d_view<double, typename UseThisDevice<SpT>::type> &er,
                    const value_type_2d_view<double, typename UseThisDevice<SpT>::type> &ei,
                    const value_type_3d_view<double, typename UseThisDevice<SpT>::type> &V,
                    const value_type_2d_view<double, typename UseThisDevice<SpT>::type> &W,
                    const bool use_tpl_if_avail = true);
```

<a name="acknowledgement"></a>

## 9\. Acknowledgement

This work is supported as part of the Computational Chemical Sciences Program funded by the U.S. Department of Energy, Office of Science, Basic Energy Sciences, Chemical Sciences, Geosciences and Biosciences Division.

Award number: 0000232253
