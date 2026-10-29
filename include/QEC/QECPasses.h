#ifndef QEC_PASSES_H
#define QEC_PASSES_H

namespace mlir::qec {

    void registerQECLinearityPass();
    void registerQECCancelHHPass();
    void registerQECCancelCNOTPass();


} // namespace mlir::qec

#endif // QEC_PASSES_H