FILE(GLOB TINES_HDR 
	  "*.hpp"
	  "sacado/*.hpp"
  	  "sacado/*.h"
    	  "sacado/mpl/*.hpp"
    	  "sacado/experimental/*.hpp"
	  "linear-algebra/*.hpp"
	  "linear-algebra/internal/*.hpp"
	  "time-integration/*.hpp"	  	  
)

FILE(GLOB TINES_SRC
	  "Tines.cpp"
	  "linear-algebra/Tines_Gemv_HostTPL.cpp"
	  "linear-algebra/Tines_Gemm_HostTPL.cpp"	  
	  "linear-algebra/Tines_Gemm_Device.cpp"	  
	  "linear-algebra/Tines_ComputeConditionNumber_HostTPL.cpp"
	  "linear-algebra/Tines_InvertMatrix_HostTPL.cpp"
	  "linear-algebra/Tines_Hessenberg_HostTPL.cpp"
  	  "linear-algebra/Tines_HessenbergFormQ_HostTPL.cpp"	  
	  "linear-algebra/Tines_Hessenberg_Device.cpp"	  
	  "linear-algebra/Tines_Schur_HostTPL.cpp"	  
	  "linear-algebra/Tines_Schur_Device.cpp"	  
	  "linear-algebra/Tines_RightEigenvectorSchur_Device.cpp"	  
	  "linear-algebra/Tines_QR_HostTPL.cpp"	  
	  "linear-algebra/Tines_QR_WithColumnPivoting_HostTPL.cpp"	  
	  "linear-algebra/Tines_ApplyQ_HostTPL.cpp"	  
	  "linear-algebra/Tines_QR_FormQ_HostTPL.cpp"	  
	  "linear-algebra/Tines_UTV_HostTPL.cpp"	  
	  "linear-algebra/Tines_SolveUTV_HostTPL.cpp"
 	  "linear-algebra/Tines_SolveLinearSystem_HostTPL.cpp"
	  "linear-algebra/Tines_SolveEigenvaluesNonSymmetricProblem_HostTPL.cpp"
	  "linear-algebra/Tines_SolveEigenvaluesNonSymmetricProblem_Device.cpp"	  
)

ADD_LIBRARY(tines STATIC ${TINES_SRC})

# TARGET_LINK_LIBRARIES(tines
#   PRIVATE 
#   $<BUILD_INTERFACE:${TINES_INTERNAL_KOKKOS_TARGET}>
#   $<BUILD_INTERFACE:${TINES_INTERNAL_OPENBLAS_TARGET}>
#   $<BUILD_INTERFACE:${TINES_INTERNAL_LAPACKE_TARGET}>
#   $<BUILD_INTERFACE:${TINES_INTERNAL_LAPACK_TARGET}>
# )
TARGET_LINK_LIBRARIES(tines
  PUBLIC 
  ${TINES_INTERNAL_KOKKOS_TARGET}
  ${TINES_INTERNAL_OPENBLAS_TARGET}
  ${TINES_INTERNAL_LAPACKE_TARGET}
  ${TINES_INTERNAL_LAPACK_TARGET}
)

TARGET_INCLUDE_DIRECTORIES(tines
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sacado>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sacado/mpl>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sacado/experimental>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/linear-algebra>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/linear-algebra/internal>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/time-integration>  
  $<INSTALL_INTERFACE:${TINES_INSTALL_INCLUDE_PATH}>
)

INSTALL(TARGETS tines
  EXPORT tines-targets
  LIBRARY DESTINATION "${TINES_INSTALL_LIB_PATH}"
  ARCHIVE DESTINATION "${TINES_INSTALL_LIB_PATH}"
)

INSTALL(FILES ${TINES_HDR}
  DESTINATION "${TINES_INSTALL_INCLUDE_PATH}"
)

INSTALL(EXPORT tines-targets
  DESTINATION "${TINES_INSTALL_LIB_PATH}/cmake/Tines"
  NAMESPACE Tines::
)

INCLUDE(CMakePackageConfigHelpers)
CONFIGURE_PACKAGE_CONFIG_FILE("cmake/TinesConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/TinesConfig.cmake"
  INSTALL_DESTINATION "${TINES_INSTALL_LIB_PATH}/cmake/Tines"
)

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/TinesConfig.cmake"
  DESTINATION "${TINES_INSTALL_LIB_PATH}/cmake/Tines"
)
