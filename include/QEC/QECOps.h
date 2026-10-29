#ifndef QEC_OPS_H
#define QEC_OPS_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"   // OpAsmSetValueNameFn / OpAsmOpInterface

#include "QEC/QECDialect.h"
#include "QEC/QECTypes.h"                // op decls reference QubitType

#define GET_OP_CLASSES
#include "QEC/QECOps.h.inc"              // declarations only — never .cpp.inc here

#endif // QEC_OPS_H