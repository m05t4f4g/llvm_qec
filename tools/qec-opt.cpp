// tools/qec-opt.cpp
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

// Only the dialects this tool actually needs. If you later use more
// built-in dialects in your tests, add them to the registry + CMakeLists.
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include "QEC/QECDialect.h"

int main(int argc, char **argv) {
    mlir::DialectRegistry registry;

    registry.insert<mlir::qec::QECDialect,
            mlir::func::FuncDialect,
            mlir::arith::ArithDialect>();

    return mlir::asMainReturnCode(
            mlir::MlirOptMain(argc, argv, "QEC Dialect Driver", registry));
}