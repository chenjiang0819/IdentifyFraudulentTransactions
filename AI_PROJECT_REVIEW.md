# AI Project Review

## Project Summary

A handwritten C coursework program that reads card limits and transaction records, then reports per-transaction daily and individual-limit violations.

Original handwritten implementation by Chen Jiang; the staged input/output structure and algorithms remain recognizable.

## Original Engineering Strengths

The project makes the complete data path inspectable: fixed-size card records support lookup, transaction nodes preserve input order, and the final reporting stages expose both aggregate card information and individual results. This is useful evidence of reasoning about several data representations within one small C program.

The original limit model combines independent daily and per-transaction conditions into four explicit statuses. Its linked-list ownership and recursive search provide concrete low-level mechanisms to discuss, rather than delegating the entire task to a framework.

## Technical Highlights

Cards are sorted for recursive identifier lookup; transaction records are accumulated in an explicit linked list before extraction into reporting records. The maintenance keeps that architecture and the original four-stage output format.

Per-card date state and cumulative amounts implement the daily-limit model. Supplied educational fixtures remain the compatibility reference, with byte-equivalent text output checked by regression tests.

## 2026 Maintenance Improvements

- Bounded card/transaction parsing, validated integer/calendar fields, and clear EOF/separator errors.
- Corrected recursive-search bounds and per-card day resets; preserved unsorted input-card support.
- Released consumed list nodes, handled empty transaction sets safely, and removed the student-number comment while keeping authorship.
- Added Makefile build/test/sanitizer targets and deterministic regression cases.

The changes retain the project's original mechanisms and strengthen their execution
boundaries and verification contract. They do not substitute a new project concept.

## Verification

- Apple Clang 21: normal -O2 build and regression suite passed.
- AddressSanitizer + UndefinedBehaviorSanitizer regression suite passed on macOS.
- GCC 14.3 in a network-disabled Linux container: strict -Werror sanitizer checks passed, including LeakSanitizer defaults.
- Critical make test checks are rerun on the merged default branch before pushing it.
- make test: five test methods passed, including both supplied fixtures, malformed/duplicate/unknown-card cases, 1,000 transactions, and repeated execution.
- Strict -O0 -Werror build and the same five test methods passed.
- Evidence: fraud_before_fixture0.log, fraud_before_empty.log, fraud_after_tests.log, fraud_linux_tests.log.

Verification is limited to the commands and cases listed above; passing these checks is not a claim of universal correctness.

## Skills Demonstrated

C parsing, recursive search, linked-list ownership, numeric boundaries, domain-state modeling, compatibility tests, and sanitizer-driven debugging.

## Engineering Assessment

The original project is a concrete demonstration of implementing lookup, ownership, and domain rules directly. The maintained version gives those ideas a reproducible build and a stronger boundary contract while retaining the original reporting design.

## Maintenance Transparency

The original implementation is preserved in `archive/handwritten-original` at
`f055d0f9fd621c8e24081ba32d8c3b14523e6877`. The reviewed maintained version contains a later AI-assisted engineering
maintenance pass focused on verification, robustness, testing, and documentation
while retaining the original project approach.
