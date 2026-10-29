#include "QEC/QECTypes.h"
#include "QEC/QECDialect.h"

#include "mlir/IR/Builders.h"                // defines mlir::Builder
#include "mlir/IR/DialectImplementation.h"   // AsmParser / AsmPrinter
#include "llvm/ADT/TypeSwitch.h"             // llvm::TypeSwitch

// Generated type definitions.
#define GET_TYPEDEF_CLASSES
#include "QEC/QECTypes.cpp.inc"