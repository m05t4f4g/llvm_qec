#ifndef QEC_PASSES_H
#define QEC_PASSES_H

namespace mlir::qec {

    void registerQECLinearityPass();
    void registerQECCancelHHPass();
    void registerQECCancelCNOTPass();
    void registerQECFuse1QPass();
    void registerQECCNOTConjugationPass();
    void registerQECSchedulePass();
    void registerQECToQIRPass();


} // namespace mlir::qec

#endif // QEC_PASSES_H