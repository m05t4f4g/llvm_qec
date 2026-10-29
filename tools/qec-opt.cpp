// tools/qec-opt.cpp
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

#include "QEC/QECDialect.h"
#include "QEC/QECPasses.h"

int main(int argc, char **argv) {
    mlir::DialectRegistry registry;
    registry.insert<mlir::qec::QECDialect,
            mlir::func::FuncDialect,
            mlir::arith::ArithDialect>();

    mlir::qec::registerQECLinearityPass();
    mlir::qec::registerQECCancelHHPass();
    mlir::qec::registerQECCancelCNOTPass();
    mlir::qec::registerQECFuse1QPass();
    mlir::qec::registerQECCNOTConjugationPass();
    mlir::qec::registerQECSchedulePass();
    mlir::qec::registerQECDeadGateElimPass();
    mlir::qec::registerQECSchedulerPass();
    mlir::qec::registerQECDecomposeU3Pass();
//    mlir::qec::registerQECRoutePass();

    return mlir::asMainReturnCode(
            mlir::MlirOptMain(argc, argv, "QEC Dialect Driver", registry));
}