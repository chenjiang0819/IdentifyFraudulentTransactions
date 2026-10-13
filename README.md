# Transaction Limit and Fraud Detection

## Overview

A handwritten C coursework program that reads card limits and transaction records, then reports per-transaction daily and individual-limit violations.

## Original Context

Original handwritten implementation by Chen Jiang; the staged input/output structure and algorithms remain recognizable. The preserved pre-maintenance snapshot is `f055d0f9fd621c8e24081ba32d8c3b14523e6877`;
its most recent original commit is dated 2022-01-13. This date describes Git evidence,
not necessarily the complete implementation period.

## What I Implemented

Card-table loading, recursive binary search, linked-list transaction staging, day-aware spending accumulation, and four-stage formatted reporting.

## Technical Approach

Cards are sorted for recursive identifier lookup; transaction records are accumulated in an explicit linked list before extraction into reporting records. The maintenance keeps that architecture and the original four-stage output format.

Per-card date state and cumulative amounts implement the daily-limit model. Supplied educational fixtures remain the compatibility reference, with byte-equivalent text output checked by regression tests.

## Build / Run

Run `make`, then `./main < test0.txt`. Run `make test` and `make sanitize`; tests require Python 3 standard library only.

## Testing

- Apple Clang 21: normal -O2 build and regression suite passed.
- AddressSanitizer + UndefinedBehaviorSanitizer regression suite passed on macOS.
- GCC 14.3 in a network-disabled Linux container: strict -Werror sanitizer checks passed, including LeakSanitizer defaults.
- Critical make test checks are rerun on the merged default branch before pushing it.
- make test: five test methods passed, including both supplied fixtures, malformed/duplicate/unknown-card cases, 1,000 transactions, and repeated execution.
- Strict -O0 -Werror build and the same five test methods passed.
- Evidence: fraud_before_fixture0.log, fraud_before_empty.log, fraud_after_tests.log, fraud_linux_tests.log.

## 2026 Maintenance

- Bounded card/transaction parsing, validated integer/calendar fields, and clear EOF/separator errors.
- Corrected recursive-search bounds and per-card day resets; preserved unsorted input-card support.
- Released consumed list nodes, handled empty transaction sets safely, and removed the student-number comment while keeping authorship.
- Added Makefile build/test/sanitizer targets and deterministic regression cases.

The later work is AI-assisted maintenance, separate from the original implementation.
The exact original snapshot remains on `archive/handwritten-original`.
See [AI_PROJECT_REVIEW.md](AI_PROJECT_REVIEW.md) for a technical review.

## Known Limitations

- Transactions for a given card must not move backward across calendar dates; such input is rejected.
- This is an educational rule-based limit detector, not a banking integration or a statistical fraud model.
- Allocation fault injection and real financial data were not tested.
