#ifndef QEC_PASSES_H
#define QEC_PASSES_H

namespace mlir::qec {

    void registerQECLinearityPass();
    void registerQECCancelHHPass();
    void registerQECCancelCNOTPass();
    void registerQECFuse1QPass();
    void registerQECCNOTConjugationPass();
    void registerQECSchedulePass();
    void registerQECDeadGateElimPass();       // Part 1
    void registerQECSchedulerPass();          // Part 2
    void registerQECDecomposeU3Pass();        // Part 3
    void registerQECRoutePass();              // Part 4



} // namespace mlir::qec

#endif // QEC_PASSES_H