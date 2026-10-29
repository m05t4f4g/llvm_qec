#ifndef QEC_PASSES_H
#define QEC_PASSES_H

namespace mlir::qec {

    void registerQECLinearityPass();
    void registerQECCancelHHPass();
    void registerQECCancelCNOTPass();
    void registerQECFuse1QPass();
    void registerQECCNOTConjugationPass();
    void registerQECSchedulePass();
    void registerQECDeadGateElimPass();
    void registerQECSchedulerPass();
    void registerQECDecomposeU3Pass();
    void registerQECRoutePass();
    void registerQECDCEPass();



} // namespace mlir::qec

#endif // QEC_PASSES_H