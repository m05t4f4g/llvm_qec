// include/QEC/QECOps.h
#ifndef QEC_OPS_H
#define QEC_OPS_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"

#include "QEC/QECDialect.h"
#include "QEC/QECTypes.h"

#define GET_OP_CLASSES
#include "QEC/QECOps.h.inc"       // <-- .h.inc

#endif // QEC_OPS_H