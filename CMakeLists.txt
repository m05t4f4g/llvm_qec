cmake_minimum_required(VERSION 3.20)
project(llvm_qec LANGUAGES CXX C)

# ---------------------------------------------------------------------------
# Find LLVM / MLIR
# ---------------------------------------------------------------------------
set(LLVM_DIR "/usr/lib/llvm-19/lib/cmake/llvm")
set(MLIR_DIR "/usr/lib/llvm-19/lib/cmake/mlir")

find_package(LLVM 19.1 REQUIRED CONFIG)
find_package(MLIR REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")

# ---------------------------------------------------------------------------
# Load LLVM/MLIR CMake utilities (ORDER MATTERS)
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(AddLLVM)          # defines get_subproject_title, add_llvm_executable, ...
include(TableGen)         # provides tablegen macros (mlir_tablegen comes from AddMLIR)
include(AddMLIR)          # provides mlir_tablegen, add_mlir_dialect, add_mlir_library, ...

# ---------------------------------------------------------------------------
# Compiler / global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
include_directories(
        ${LLVM_INCLUDE_DIRS}
        ${MLIR_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
)

# ---------------------------------------------------------------------------
# TableGen: generate dialect / op files
# ---------------------------------------------------------------------------
set(LLVM_TARGET_DEFINITIONS ${CMAKE_CURRENT_SOURCE_DIR}/tablegen/QECDialect.td)

# Make sure the output directory exists BEFORE mlir_tablegen runs
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/QEC)

# Dialect declarations  ->  QECDialect.h.inc
mlir_tablegen(include/QEC/QECDialect.h.inc -gen-dialect-decls)
# Dialect definitions   ->  QECDialect.cpp.inc  (needed by QECDialect.cpp)
mlir_tablegen(include/QEC/QECDialect.cpp.inc -gen-dialect-defs)

# Op declarations       ->  QECOps.h.inc
mlir_tablegen(include/QEC/QECOps.h.inc -gen-op-decls)
# Op definitions        ->  QECOps.cpp.inc
mlir_tablegen(include/QEC/QECOps.cpp.inc -gen-op-defs)

# Type declarations     ->  QECTypes.h.inc
mlir_tablegen(include/QEC/QECTypes.h.inc -gen-typedef-decls)
# Type definitions      ->  QECTypes.cpp.inc
mlir_tablegen(include/QEC/QECTypes.cpp.inc -gen-typedef-defs)

add_public_tablegen_target(MLIRQECIncGen)

# ---------------------------------------------------------------------------
# Dialect library
# ---------------------------------------------------------------------------
add_mlir_dialect_library(MLIRQEC
        lib/QECDialect.cpp
        lib/QECOps.cpp
        lib/QECTypes.cpp

        ADDITIONAL_HEADER_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/QEC

        DEPENDS
        MLIRQECIncGen

        LINK_LIBS PUBLIC
        MLIRIR
        MLIRSupport
        )

# Explicitly attach include directories to the library target
target_include_directories(MLIRQEC PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

# ---------------------------------------------------------------------------
# qec-opt driver
# ---------------------------------------------------------------------------
add_mlir_tool(qec-opt tools/qec-opt.cpp)

target_link_libraries(qec-opt PRIVATE
        MLIRQEC
        MLIROptLib
        MLIRFuncDialect        # provides func.func
        MLIRArithDialect       # provides arith.constant / arith ops
        MLIRIR
        MLIRSupport
        )

# Ensure qec-opt also has direct access to the include roots
target_include_directories(qec-opt PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
        )

