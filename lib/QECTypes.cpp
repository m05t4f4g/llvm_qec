#include "QEC/QECTypes.h"
#include "QEC/QECDialect.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

#define GET_TYPEDEF_CLASSES
#include "QEC/QECTypes.cpp.inc"    // definitions live only here
