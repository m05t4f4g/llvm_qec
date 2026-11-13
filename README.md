# llvm_qec — a QEC dialect for MLIR

A standalone MLIR dialect for representing and optimizing quantum
error-correction circuits, with two backends (OpenQASM 2.0 and QIR).

Built against **LLVM / MLIR 19**.

[![Build](https://github.com/m05t4f4g/llvm_qec/actions/workflows/build.yml/badge.svg)](https://github.com/m05t4f4g/llvm_qec/actions/workflows/build.yml)

## What's in the box

- **Dialect**: `!qec.qubit` type + 13 ops (`alloc`, `h`, `s`, `sdg`, `t`,
  `tdg`, `x`, `y`, `z`, `u3`, `cnot`, `measure`, `barrier`, `swap`)
  with declarative assembly formats.
- **Verifier**: `--qec-linearity` — no-cloning / no-dropping, the one
  invariant unique to quantum IRs.
- **Optimizations**: `--qec-cancel-hh`, `--qec-cancel-cnot`,
  `--qec-dead-gate-elim`, `--qec-fuse-1q`, `--qec-cnot-conjugation`,
  `--qec-decompose-u3`.
- **Transformers**: `--qec-scheduler`, `--qec-schedule`, `--qec-route`,
  `--qec-dce`.
- **Backends**: `qec-translate --qec-to-qasm` (OpenQASM 2.0),
  `qec-to-qir --qec-to-qir` (QIR — validated by `llvm-as`).

## Requirements

- LLVM / MLIR 19 (headers + libraries)
- CMake ≥ 3.20
- A C++17 compiler (GCC 13+ / Clang 17+ tested)
- `llvm-as-19` (only needed to run the smoke test)

On Ubuntu 22.04+:

```bash
sudo apt install llvm-19-dev libmlir-19-dev mlir-19-tools clang-19