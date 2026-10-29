#include "QEC/QECDialect.h"
#include "QEC/QECOps.h"
#include "QEC/QECTypes.h"

#include "mlir/IR/DialectImplementation.h"

// Generated dialect definitions: constructor + parseType/printType.
#include "QEC/QECDialect.cpp.inc"

using namespace mlir;
using namespace mlir::qec;

void QECDialect::initialize() {
    addOperations<
#define GET_OP_LIST
#include "QEC/QECOps.cpp.inc"
    >();

    addTypes<
#define GET_TYPEDEF_LIST
#include "QEC/QECTypes.cpp.inc"
    >();
}