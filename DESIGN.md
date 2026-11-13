# QEC Dialect — Design Notes

## Purpose

A minimal MLIR dialect for representing and optimizing quantum
error-correction circuits. The goal is a **complete vertical slice** —
parse, verify, optimize, emit — not a production compiler. Every design
choice below is made with "demonstrate the pattern" ahead of "scale".

Scope explicitly excludes: surface-code layout, syndrome extraction
scheduling, magic-state distillation, fault-tolerant routing. Those are
research problems; this project stops at the last well-understood stage.

## Dialect design

### One type: `!qec.qubit`

Qubits are linear (no-cloning is physical law). The type carries no
parameters — no index, no basis, no error-rate metadata. All of that is
better handled by a later pass (layout assignment, noise model
annotation) than baked into the type. Keeping the type monomorphic means
every pass can pattern-match on `isa<QubitType>(v.getType())` without
enumerating variants.

### Ops: 13, chosen as a *complete* gate set, not a *minimal* one

- `alloc` / `measure` are the classical/quantum boundary. Every circuit
  starts and ends with them; without them the linearity verifier has no
  anchor. `measure` produces `i1`, not `!qec.qubit` — this is what makes
  it a *side-effect* (the qubit's state is destroyed), which DCE must
  preserve.
- `h`, `s`, `sdg`, `t`, `tdg`, `x`, `y`, `z` are the named Clifford+T
  primitives. They exist so the IR looks like a real circuit to a human
  reader. Internally every one of them is a `U3` specialization.
- `u3(θ, φ, λ)` is the *universal* 1-qubit gate. Fusion targets it
  because *every* 1-qubit unitary is expressible as a `U3`; the named
  gates are just the cases where `(θ, φ, λ)` collapses to nice values.
  Round-tripping (`fuse-1q` then `decompose-u3`) proves this.
- `cnot` is the two-qubit entangler. It has 2 operands and 2 results
  (control, target → control', target') so that the SSA graph preserves
  linearity through the op without threading tricks.
- `barrier` and `swap` are *scheduling* primitives, not gates. They carry
  qubits from input to output unchanged, but their presence in the IR
  constrains reordering. `swap` is a `qec-route` output; `barrier` is a
  `qec-schedule` output.

### Assembly formats: declarative, no custom C++

Every op uses `assemblyFormat = "..."`. This means ODS generates
`parse`/`print` from the format string. The trade-off: we can't put
"pretty" hints (like `cnot c, t` instead of `cnot(c, t)`) into the
format, but we get zero hand-written parser code and the format *is* the
documentation. For a dialect this small, the win dominates.

The type `QubitType` declares `assemblyFormat = ""` — a parameterless
type that prints as `!qec.qubit` with nothing after the mnemonic.

## Pass architecture

Three distinct categories, each with a different shape. This taxonomy
matters because it dictates *where* a check belongs and *how* to test it.

### Verifiers (`--qec-linearity`)

A verifier **fails the build** when an invariant is violated; it never
modifies IR. Linearity (no-cloning, no-dropping) is the one quantum
invariant that ODS cannot express as an op trait, because it is a
*global* property of the SSA graph — each qubit value must have exactly
one use, but "use" includes block arguments and function returns, which
ODS has no way to inspect.

A verifier belongs after every pass that could break the invariant.
In our pipeline we run `--qec-linearity` last, but it could equally run
after each pass; the check is O(instructions) and cheap.

### Rewriters (`cancel-hh`, `cancel-cnot`, `fuse-1q`, `dead-gate-elim`,
### `cnot-conjugation`, `decompose-u3`)

Each uses `RewritePattern` + `applyPatternsAndFoldGreedily`. The
fixpoint driver handles chains (`H·H·H·H` → zero ops), so each pattern
only needs to express *one* local rewrite. This is the whole point of
the pattern-rewriter abstraction: you write the physics, the driver
handles the graph.

Design rule: **rewriters preserve linearity by construction.** Every
rewrite either removes ops (`cancel-*`), merges two ops into one
(`fuse-1q`), or replaces an op's operands (`cnot-conjugation`). None of
them create new uses of an existing qubit, so `--qec-linearity` cannot
fail on a rewriter's output. This is why the pipeline never has to
interleave verifiers between rewriters.

### Transformers (`scheduler`, `schedule`, `route`, `dce`)

These are passes, not patterns. They work at the block level (reorder,
insert, delete) rather than the op level.

- `scheduler` calls `Operation::moveBefore(sentinel)`. It moves only
  *independent* 1-qubit gates — two gates on the same wire are never
  reordered, because that would change the circuit's semantics. Reordering
  independent gates is safe because quantum operations commute iff their
  qubit sets are disjoint.
- `schedule` and `route` use `OpBuilder::create<X>` + `replaceAllUsesExcept`.
  This pattern is critical: after inserting the new op, every *other* use
  of the old SSA value must be redirected to the new output. The
  `Except` variant skips the new op's own operand, which is still the
  old value.
- `dce` is deliberately excluded from the `RewritePattern` machinery.
  It's a loop-to-fixpoint over the module, erasing ops whose results are
  all unused. It runs *after* everything else because it's cleanup, not
  optimization.

## Backend design

Both backends use the exact same 50-line pattern:

```cpp
DenseMap<Value, unsigned> qubitIndex;   // SSA value → physical wire number