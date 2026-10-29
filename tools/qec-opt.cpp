// tools/qec-opt.cpp
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllDialects.h"   // keep if you want the header, but we won't call registerAllDialects
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include "QEC/QECDialect.h"

int main(int argc, char **argv) {
    mlir::DialectRegistry registry;

    // Register only the dialects this tool actually needs.
    registry.insert<mlir::qec::QECDialect,
            mlir::func::FuncDialect,
            mlir::arith::ArithDialect>();

    // Do NOT call mlir::registerAllDialects(registry) here.

    return mlir::asMainReturnCode(
            mlir::MlirOptMain(argc, argv, "QEC Dialect Driver", registry));
}