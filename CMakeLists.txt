cmake_minimum_required (VERSION 3.12.0)

# HaraliCU Version
set (HaraliCU_MAJOR    0)
set (HaraliCU_MINOR    0)
set (HaraliCU_REVISION 1)
set (HaraliCU_VERSION ${HaraliCU_MAJOR}.${HaraliCU_MINOR}.${HaraliCU_REVISION})

project (HaraliCU LANGUAGES CXX VERSION ${HaraliCU_VERSION} DESCRIPTION "Extracting Haralick Features using CUDA for GPU-acceleration")

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_PROJECT_HOMEPAGE_URL "https://github.com/Nico-Curti/HaraliCU")

add_definitions (-DMAJOR=${HaraliCU_MAJOR} -DMINOR=${HaraliCU_MINOR} -DREVISION=${HaraliCU_REVISION})

enable_language(CXX)

#################################################################
#                         PARSE OPTIONS                         #
#################################################################

find_package (OpenCV REQUIRED COMPONENTS core highgui imgproc)
include_directories (${OpenCV_INCLUDE_DIRS})
list(APPEND linked_libs "opencv_core")
list(APPEND linked_libs "opencv_highgui")
list(APPEND linked_libs "opencv_imgproc")

include(CheckLanguage)

check_language(CUDA)

if ( CMAKE_CUDA_COMPILER )
  enable_language(CUDA)
  message(STATUS "Selected CMAKE_CUDA_ARCHITECTURES: ${CMAKE_CUDA_ARCHITECTURES}")

  find_package(CUDA REQUIRED)

  if ("${CMAKE_CUDA_ARCHITECTURES}" STREQUAL "")
    message (STATUS "Default CMAKE_CUDA_ARCHITECTURES not found. Build the whole set of available ones.")

    cuda_select_nvcc_arch_flags(CUDA_ARCH_FLAGS ${CUDA_ARCHITECTURES})

    foreach (ARCH ${CUDA_ALL_GPU_ARCHITECTURES})
      string (REPLACE "." "" TRUE_ARCH ${ARCH})
      list (APPEND AVAILABLE_GPU_ARCHS ${TRUE_ARCH})
    endforeach ()

    message(STATUS "Available CMAKE_CUDA_ARCHITECTURES: ${AVAILABLE_GPU_ARCHS}")

  else ()

    cuda_select_nvcc_arch_flags(CUDA_ARCH_FLAGS ${CUDA_ARCHITECTURES})

  endif ()

  message(STATUS "Building with CUDA flags: " "${CUDA_ARCH_FLAGS}")

endif()


#################################################################
#                         COMPILE OPTIONS                       #
#################################################################


#################################################################
#                         SETTING VARIABLES                     #
#################################################################

if ( NOT APPLE )
  set (CMAKE_SKIP_BUILD_RPATH             FALSE )
  set (CMAKE_BUILD_WITH_INSTALL_RPATH     FALSE )
  set (CMAKE_INSTALL_RPATH_USE_LINK_PATH  TRUE  )
endif()

# make sure that the default is a RELEASE
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if ( CMAKE_COMPILER_IS_GNUCXX )
  set(ADDITIONAL_CXX_FLAGS "-Wall -Wextra -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -Wno-narrowing -Wno-unused-function -Wno-pedantic -Wno-deprecated-declarations -Wno-write-strings")
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    string (REGEX REPLACE "-O[0123]" "-Og" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG" )
    set (CMAKE_CXX_FLAGS_RELEASE "")
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    string (REGEX REPLACE "-O3" "-Ofast" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG" )
    set (CMAKE_CXX_FLAGS_DEBUG "")
  endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(ADDITIONAL_CXX_FLAGS "-Wno-deprecated -Wno-writable-strings")
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    string (REGEX REPLACE "-O0" "-Og" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    string (REGEX REPLACE "-O0" "-Og" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG" )
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG" )
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    string (REGEX REPLACE "-O3" "-Ofast" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    string (REGEX REPLACE "-O3" "-Ofast" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG" )
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG" )
  endif()
endif()

if (MSVC)
  add_definitions (-D_CRT_RAND_S)
  add_definitions (-DNOMINMAX)
  add_definitions (-D_USE_MATH_DEFINES)
  add_definitions (-D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
  set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /DCMAKE_BUILD /DG_DISABLE_SINGLE_INCLUDES")
  set(ADDITIONAL_CXX_FLAGS "/wd4013 /wd4018 /wd4028 /wd4047 /wd4068 /wd4090 /wd4101 /wd4113 /wd4133 /wd4190 /wd4244 /wd4267 /wd4305 /wd4477 /wd4996 /wd4819")
  set(CMAKE_CXX_FLAGS "${ADDITIONAL_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")
  string(REGEX REPLACE "/O2" "/Ox" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
else ()
  set(ADDITIONAL_CXX_FLAGS "-Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -Wno-deprecated-declarations -Wno-write-strings")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DCMAKE_BUILD -DG_DISABLE_SINGLE_INCLUDES")
endif()

if (MSVC)
  set(CUDA_HOST_COMPILER_FLAGS "-Wno-deprecated-declarations -Xcompiler=\"${ADDITIONAL_CXX_FLAGS_COMMA_SEPARATED}\"")
else()
  set(CUDA_HOST_COMPILER_FLAGS "--compiler-options \" ${ADDITIONAL_CXX_FLAGS} -fPIC -fopenmp -Ofast \"")
endif()

string(REPLACE ";" " " CUDA_ARCH_FLAGS_SPACE_SEPARATED "${CUDA_ARCH_FLAGS}")
set(CMAKE_CUDA_FLAGS "${CUDA_ARCH_FLAGS_SPACE_SEPARATED} ${CUDA_HOST_COMPILER_FLAGS} ${CMAKE_CUDA_FLAGS}")
message(STATUS "CMAKE_CUDA_FLAGS: ${CMAKE_CUDA_FLAGS}")


include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if (COMPILER_SUPPORTS_MARCH_NATIVE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "Install prefix" FORCE)
endif()


#################################################################
#                         SETTING DIRECTORIES                   #
#################################################################

set(CPU_DIR       ${CMAKE_SOURCE_DIR}/CPU/src            CACHE PATH "Path where find cpp files"                        )
set(CPU_INC_DIR   ${CMAKE_SOURCE_DIR}/CPU/src            CACHE PATH "Path where find cpp header files"                        )
set(CUDA_INC_DIR  ${CMAKE_SOURCE_DIR}/GPU/src            CACHE PATH "Path where find CUDA header files"                     )
set(CUDA_DIR      ${CMAKE_SOURCE_DIR}/GPU/src            CACHE PATH "Path where find CUDA files"                       )
set(LIB_DIR       ${CMAKE_SOURCE_DIR}/lib                CACHE PATH "Path where lib will be installed"            FORCE)
set(INST_DIR      ${CMAKE_SOURCE_DIR}/share/HaraliCU     CACHE PATH "Path where cmake configs will be installed"  FORCE)
set(OUT_DIR       ${CMAKE_SOURCE_DIR}/bin                CACHE PATH "Path where outputs will be installed"        FORCE)

set(INSTALL_INCLUDE_DIR  "${CMAKE_CURRENT_LIST_DIR}/share/include/HaraliCU"  CACHE PATH "Path where headers will be installed")
set(INSTALL_CMAKE_DIR    "${CMAKE_CURRENT_LIST_DIR}/share/HaraliCU"          CACHE PATH "Path where cmake configs will be installed")

set(CMAKE_DEBUG_POSTFIX d)

set(HaraliCUlib_gpu HaraliCU_gpu)
set(HaraliCUlib_cpu HaraliCU_cpu)

file(GLOB HEADER_CPU    "${CPU_INC_DIR}/*.h")
file(GLOB SRC_CPU       "${CPU_DIR}/*.cpp")
file(GLOB HEADER_CUDA   "${CUDA_INC_DIR}/*.h")
file(GLOB SRC_CUDA      "${CUDA_DIR}/*.cu")
file(GLOB SRC_CPP_CUDA  "${CUDA_DIR}/*.cpp")

list(REMOVE_ITEM SRC_CPU
  ${CPU_DIR}/Main.cpp
)
list(REMOVE_ITEM SRC_CPP_CUDA
  ${CUDA_DIR}/Main.cpp
)

set(HaraliCU_INSTALL_INCLUDE_DIR ${INSTALL_INCLUDE_DIR})

configure_file(${CMAKE_SOURCE_DIR}/HaraliCU.pc.in ${CMAKE_SOURCE_DIR}/HaraliCU.pc @ONLY)
message(STATUS "Pkg-config generated")

# allow the export of LD_LIBRARY_PATH env variable
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

string (REPLACE " " ";" CMAKE_CUDA_FLAGS_LIST ${CUDA_ARCH_FLAGS_SPACE_SEPARATED})

#################################################################
#                          SUMMARY                              #
#################################################################

message(STATUS ""                                                                    )
message(STATUS "================ HaraliCU configuration Summary ==============="     )
message(STATUS "   HaraliCU version: ${HaraliCU_VERSION}"                            )
message(STATUS ""                                                                    )
message(STATUS "   Build type : ${CMAKE_BUILD_TYPE}"                                 )
message(STATUS "   C++ :"                                                            )
message(STATUS "      C++ Compiler : ${CMAKE_CXX_COMPILER}"                          )
message(STATUS "      C++ flags    :"                                                )
foreach(FLAG ${CMAKE_CXX_FLAGS_LIST})
  message(STATUS "                    * ${FLAG}"                                     )
endforeach(FLAG)
message(STATUS "      CUDA flags    :"                                               )
foreach(FLAG ${CMAKE_CUDA_FLAGS_LIST})
  message(STATUS "                    * ${FLAG}"                                     )
endforeach(FLAG)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "      C++ Debug flags:"                                            )
  foreach(FLAG ${CMAKE_CXX_FLAGS_DEBUG})
    message(STATUS "                    * ${FLAG}"                                   )
  endforeach(FLAG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "      C++ Release flags  :"                                        )
  foreach(FLAG ${CMAKE_CXX_FLAGS_RELEASE})
    message(STATUS "                    * ${FLAG}"                                   )
  endforeach(FLAG)
endif()
message(STATUS ""                                                                    )
message(STATUS ""                                                                    )


#################################################################
#                         MAIN RULES                            #
#################################################################

add_library(${HaraliCUlib_gpu} SHARED ${HEADER_CUDA} ${SRC_CUDA} ${SRC_CPP_CUDA})
set_property(TARGET ${HaraliCUlib_gpu} PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(${HaraliCUlib_gpu}
                           PUBLIC
                             $<INSTALL_INTERFACE:${HaraliCU_INSTALL_INCLUDE_DIR}>
                           )
set_target_properties(${HaraliCUlib_gpu} PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
target_include_directories(${HaraliCUlib_gpu} PUBLIC ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
target_link_libraries(${HaraliCUlib_gpu} PRIVATE cuda cudart cudadevrt ${linked_libs})
set_target_properties(${HaraliCUlib_gpu} PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)
set_target_properties(${HaraliCUlib_gpu} PROPERTIES CUDA_ARCHITECTURES "${AVAILABLE_GPU_ARCHS}")
set_target_properties(${HaraliCUlib_gpu} PROPERTIES PUBLIC_HEADER "${HEADER_CUDA}")


add_executable(HaraliCU ${CUDA_DIR}/Main.cpp)
target_include_directories(HaraliCU PUBLIC ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
target_link_libraries(HaraliCU PRIVATE cuda cudart ${HaraliCUlib_gpu})
set_target_properties(HaraliCU PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)
set_target_properties(HaraliCU PROPERTIES CUDA_ARCHITECTURES "${AVAILABLE_GPU_ARCHS}")
install(TARGETS HaraliCU       DESTINATION ${OUT_DIR})




add_library(${HaraliCUlib_cpu} SHARED ${HEADER_CPU} ${SRC_CPU})
set_target_properties(${HaraliCUlib_cpu} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(${HaraliCUlib_cpu} 
                           PUBLIC 
                             $<INSTALL_INTERFACE:${HaraliCU_INSTALL_INCLUDE_DIR}>
                          )
target_include_directories(${HaraliCUlib_cpu} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${HaraliCUlib_cpu} PUBLIC ${linked_libs})
set_target_properties(${HaraliCUlib_cpu} PROPERTIES CUDA_ARCHITECTURES "${AVAILABLE_GPU_ARCHS}")
target_compile_definitions(${HaraliCUlib_cpu} PRIVATE -D_USE_MATH_DEFINES)
set_target_properties(${HaraliCUlib_cpu} PROPERTIES PUBLIC_HEADER "${HEADER}")


add_executable(HaraliCPU ${CPU_DIR}/Main.cpp)
target_link_libraries(HaraliCPU PRIVATE ${linked_libs} ${HaraliCUlib_cpu})
set_target_properties(HaraliCPU PROPERTIES CUDA_ARCHITECTURES "${AVAILABLE_GPU_ARCHS}")
install(TARGETS HaraliCPU       DESTINATION ${OUT_DIR})

if (MSVC)

  MACRO (COPY_DLL trgt libname)
          ADD_CUSTOM_COMMAND (
          TARGET ${trgt}
          POST_BUILD
          COMMAND ${CMAKE_COMMAND}
          ARGS -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${libname}.dll" ${OUT_DIR}
      )
  ENDMACRO (COPY_DLL)

  COPY_DLL (HaraliCU opencv_core)
  COPY_DLL (HaraliCU opencv_highgui)
  COPY_DLL (HaraliCU opencv_imgproc)

endif ()

#################################################################
#                          INSTALLERS                           #
#################################################################

install(TARGETS ${HaraliCUlib_gpu}  EXPORT HaraliCUTargets
                                    RUNTIME DESTINATION "${OUT_DIR}"
                                    LIBRARY DESTINATION "${LIB_DIR}"
                                    ARCHIVE DESTINATION "${LIB_DIR}"
                                    PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}"
                                    COMPONENT dev
                                    )

install(EXPORT HaraliCUTargets
  FILE HaraliCUTargets.cmake
  NAMESPACE HaraliCU::GPU::
  DESTINATION "${INSTALL_CMAKE_DIR}"
)

# Export the package for use from the build-tree (this registers the build-tree with a global CMake-registry)
export(PACKAGE HaraliCU)

# Create the HaraliCUConfig.cmake
# First of all we compute the relative path between the cmake config file and the include path
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(HaraliCUConfig.cmake.in "${PROJECT_BINARY_DIR}/HaraliCUConfig.cmake" @ONLY)
set(CONF_INCLUDE_DIRS "\${HaraliCU_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(HaraliCUConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HaraliCUConfig.cmake" @ONLY)

# Create the HaraliCUConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/HaraliCUConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HaraliCUConfig.cmake"
  "${PROJECT_BINARY_DIR}/HaraliCUConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}"
)

install(TARGETS ${HaraliCUlib_cpu}  EXPORT HaraliCUTargets
                                    RUNTIME DESTINATION "${OUT_DIR}"
                                    LIBRARY DESTINATION "${LIB_DIR}"
                                    ARCHIVE DESTINATION "${LIB_DIR}"
                                    PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}"
                                    COMPONENT dev
                                    )

install(EXPORT HaraliCUTargets
  FILE HaraliCUTargets.cmake
  NAMESPACE HaraliCU::CPU::
  DESTINATION "${INSTALL_CMAKE_DIR}"
)

# Export the package for use from the build-tree (this registers the build-tree with a global CMake-registry)
export(PACKAGE HaraliCPU)

# Create the HaraliCPUConfig.cmake
# First of all we compute the relative path between the cmake config file and the include path
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(HaraliCPUConfig.cmake.in "${PROJECT_BINARY_DIR}/HaraliCPUConfig.cmake" @ONLY)
set(CONF_INCLUDE_DIRS "\${HaraliCU_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(HaraliCPUConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HaraliCPUConfig.cmake" @ONLY)

# Create the HaraliCUConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/HaraliCUConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HaraliCPUConfig.cmake"
  "${PROJECT_BINARY_DIR}/HaraliCUConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}"
)
