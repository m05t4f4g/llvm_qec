#ifndef QEC_TYPES_H
#define QEC_TYPES_H

#include "mlir/IR/Dialect.h"
#include "mlir/IR/Types.h"
#include "mlir/IR/TypeSupport.h"

#define GET_TYPEDEF_CLASSES
#include "QEC/QECTypes.h.inc"     // declarations only

#endif // QEC_TYPES_H