// tools/qec-to-qir.cpp
#include "mlir/Support/LogicalResult.h"
#include "mlir/Tools/mlir-translate/MlirTranslateMain.h"

#include "QEC/QECToQIR.h"

int main(int argc, char **argv) {
    mlir::qec::registerQECToQIRTranslation();

    // mlirTranslateMain returns LogicalResult; convert to a process exit code.
    return mlir::failed(mlir::mlirTranslateMain(argc, argv, "QEC -> QIR"))
           ? 1 : 0;
}