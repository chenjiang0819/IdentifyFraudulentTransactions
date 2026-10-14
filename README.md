# Transaction Limit and Fraud Detection

A handwritten C coursework program that reads card limits and transaction records, then reports per-transaction daily and individual-limit violations.

Chen Jiang’s University of Melbourne computing coursework.

University: University of Melbourne  

## Implementation

Card-table loading, recursive binary search, linked-list transaction staging, day-aware spending accumulation, and four-stage formatted reporting.

## Technical approach

Cards are sorted for recursive identifier lookup; transaction records are accumulated in an explicit linked list before extraction into reporting records. The maintenance keeps that architecture and the original four-stage output format.

Per-card date state and cumulative amounts implement the daily-limit model. Supplied educational fixtures remain the compatibility reference, with byte-equivalent text output checked by regression tests.

## Run

Run `make`, then `./main < test0.txt`. Run `make test` and `make sanitize`; tests require Python 3 standard library only.

## Checks and results

- Apple Clang 21: normal -O2 build and regression suite passed.
- AddressSanitizer + UndefinedBehaviorSanitizer regression suite passed on macOS.
- GCC 14.3 in a network-disabled Linux container: strict -Werror sanitizer checks passed, including LeakSanitizer defaults.
- make test: five test methods passed, including both supplied fixtures, malformed/duplicate/unknown-card cases, 1,000 transactions, and repeated execution.
- Strict -O0 -Werror build and the same five test methods passed.

## 2026 maintenance

- Bounded card/transaction parsing, validated integer/calendar fields, and clear EOF/separator errors.
- Corrected recursive-search bounds and per-card day resets; preserved unsorted input-card support.
- Released consumed list nodes, handled empty transaction sets safely, and removed the student-number comment while keeping authorship.
- Added Makefile build/test/sanitizer targets and deterministic regression cases.

These changes are 2026 AI-assisted maintenance of the original student implementation.

## Usage notes

- Transactions for a given card must not move backward across calendar dates; such input is rejected.
- This is an educational rule-based limit detector, not a banking integration or a statistical fraud model.
