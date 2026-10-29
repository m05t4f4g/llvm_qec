// tools/qec-translate.cpp
#include "mlir/Support/LogicalResult.h"
#include "mlir/Tools/mlir-translate/MlirTranslateMain.h"

#include "QEC/QECToQASM.h"

int main(int argc, char **argv) {
    mlir::qec::registerQECToQASMTranslation();

    // mlirTranslateMain returns LogicalResult; convert to a process
    // exit code inline (0 = success, 1 = failure).
    return mlir::failed(mlir::mlirTranslateMain(argc, argv, "QEC Translator"))
           ? 1
           : 0;
}