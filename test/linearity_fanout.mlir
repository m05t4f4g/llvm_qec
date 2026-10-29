// Two violations of linearity. Running this with --qec-linearity
// MUST emit errors and exit non-zero.
//
//   func @block_arg_fanout : %q (block argument) used twice
//   func @op_result_fanout : %q (result of qec.alloc) used twice
//
// Run with:  qec-opt --qec-linearity test/linearity_fanout.mlir

func.func @block_arg_fanout(%q: !qec.qubit) -> (i1, i1) {
  %b1 = qec.measure %q : !qec.qubit
  %b2 = qec.measure %q : !qec.qubit      // ERROR: %q has 2 uses
  return %b1, %b2 : i1, i1
}

func.func @op_result_fanout() -> (i1, i1) {
  %q  = qec.alloc : !qec.qubit
  %b1 = qec.measure %q : !qec.qubit
  %b2 = qec.measure %q : !qec.qubit      // ERROR: %q has 2 uses
  return %b1, %b2 : i1, i1
}