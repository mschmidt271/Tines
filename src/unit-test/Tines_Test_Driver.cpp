#include <gtest/gtest.h>

#include <Kokkos_Core.hpp>
#include <impl/Kokkos_Timer.hpp>

#include "Tines.hpp"

int TestExamplesInternal(std::string path, std::string exec) {
  {
    std::string logfile(exec + ".test-log");
    std::string rm=("rm -f " + logfile);
    const auto rm_c_str = rm.c_str();
    std::system(rm_c_str);
    
    std::string invoke=("../example/" + path + exec + " > " + logfile);
    const auto invoke_c_str = invoke.c_str();
    printf("Tines testing : %s\n", invoke_c_str);
    std::system(invoke_c_str);
    std::ifstream file(logfile);
    for (std::string line; getline(file, line); ) {
      printf("%s\n", line.c_str());
      EXPECT_TRUE(line.find("FAIL") == std::string::npos);
    }
  }
  return 0;
}

int TestViewAndPtrExamples(std::string path, std::string exec) {
  TestExamplesInternal(path, exec+".ptr.x");
  TestExamplesInternal(path, exec+".view.x");
  return 0;
}

///
/// Linear Algebra 
///
TEST(LinearAlgebra,Gemv) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_Gemv");
}
TEST(LinearAlgebra,Gemm) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_Gemm");
}
TEST(LinearAlgebra,ComputeConditionNumber) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_ComputeConditionNumber");
}
TEST(LinearAlgebra,InvertMatrix) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_InvertMatrix");
}
TEST(LinearAlgebra,QR) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_QR");
}
TEST(LinearAlgebra,QR_WithColumnPivoting) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_QR_WithColumnPivoting");
}
TEST(LinearAlgebra,UTV) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_UTV");
}
TEST(LinearAlgebra,SolveUTV) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_SolveUTV");
}
TEST(LinearAlgebra,SolveLinearSystemUTV) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_SolveLinearSystem");
}
TEST(LinearAlgebra,Eigendecomposition) {
  TestViewAndPtrExamples("linear-algebra/", "Tines_Eigendecomposition");
}

///
/// Sacado basic
///
TEST(Sacado,ToySacado) {
  TestExamplesInternal("sacado/", "Tines_ToySacado.x");
}

///
/// Time integration
///
TEST(TimeIntegration,NumericalJacobians) {
  TestExamplesInternal("time-integration/", "Tines_NumericalJacobian.x");
}
TEST(TimeIntegration,NewtonSolver) {
  TestExamplesInternal("time-integration/", "Tines_NewtonSolver.x");
}
TEST(TimeIntegration,TrBDF2) {
  TestExamplesInternal("time-integration/", "Tines_TrBDF2.x");
}
TEST(TimeIntegration,AnalyticJacobians) {
  TestExamplesInternal("time-integration/", "Tines_AnalyticJacobian.x");
}

int
main(int argc, char* argv[])
{
  int r_val(0);
  Kokkos::initialize(argc, argv);
  {
    const bool detail = false;
    auto device_exec_space = Kokkos::DefaultExecutionSpace();
    //auto host_exec_space = Kokkos::DefaultHostExecutionSpace();
    device_exec_space.print_configuration(std::cout, detail);
    //host_exec_space.print_configuration(std::cout, detail);
    
    ::testing::InitGoogleTest(&argc, argv);
    r_val = RUN_ALL_TESTS();
  }
  Kokkos::finalize();

  return r_val;
}