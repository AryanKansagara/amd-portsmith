# Portsmith

A runnable C++17 advisory scanner for migration assumptions in CUDA-like source. AMD builds CPUs and GPUs; HIP and ROCm provide its GPU programming stack. HIPIFY already converts APIs. This project checks a small explicit set of remaining assumptions and produces exact source positions; it is not another translator or proof of port correctness.

## Run

Requires GNU Make and a C++17 compiler; no packages, credentials or hardware drivers.

```sh
make test
make demo
./build/portsmith scan kernel.cu
```

The CLI writes machine-readable JSON. Findings use zero-based byte offsets and one-based lines/columns; tabs count as one byte, LF starts a new line, and CR counts as a byte. There is no filename interpolation in output. Malformed supported literals/comments fail with nonzero status. Inputs are bounded to 1 MiB.

## Implemented

The lexer separates code from line/block comments, ordinary strings/characters, escaped quotes, prefixed strings, and C++ raw strings with custom delimiters. It reports these deliberately narrow rules:

| Rule | Token pattern | Follow-up experiment |
| --- | --- | --- |
| Hard-coded warp division | `/ 32` or `% 32` | Test active width, partial groups and non-power-of-two sizes |
| Hard-coded lane mask | `& 31` | Compare lane mapping at different widths |
| Hard-coded warp shift | `>> 5` | Compare group indexing with target warp size |
| NVIDIA architecture guard | exact `__CUDA_ARCH__` identifier | Review target-specific fallback/build path |
| Inline assembly review | `asm`/`__asm__` followed by `(` or `volatile` | Inspect instruction dialect and portable alternative |

Comments between tokens do not hide a match. Comments and literals containing those same words are negative controls. Inline assembly is flagged at the keyword, while its string content is excluded from ordinary token rules.

## Actual verification

`make test` passes **12 groups / 114 assertions**, including five rule categories, exact CRLF/tab positions, benign lookalikes, prefixed/raw/escaped literals, malformed source and deterministic location shifts. See `test-results.txt` and `demo.json` for the local run. GitHub Actions runs the same checks; inspect Actions for the latest hosted result.

The bounded lexer rejects backslash-newline splicing rather than misreporting continuation comments. It does not implement a preprocessor, macro expansion, conditional compilation, complete C++ parsing, trigraph processing, arbitrary numeric literal semantics or Clang AST analysis. Disabled preprocessor branches are still scanned. The decimal patterns above can be ordinary arithmetic, so findings require human review. Other spellings and more complex expressions may be missed. An empty finding list does not establish portability.

## Sources and next step

- [AMD HIP porting guide](https://rocmdocs.amd.com/projects/HIP/en/latest/how-to/hip_porting_guide.html)
- [AMD HIP FAQ](https://rocmdocs.amd.com/projects/HIP/en/latest/faq.html)

These sources establish existing HIP/porting capabilities and limitations; the proposed assurance workflow is an independent inference, not an AMD roadmap claim. Next: a compile-database/Clang adapter plus native numerical fixture runner, followed by actual HIP/CUDA comparisons on declared devices. **No AMD or NVIDIA execution, numerical equivalence, performance or deployment has been validated.**
