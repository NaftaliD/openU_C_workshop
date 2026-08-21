# Notes

A few judgment calls used in this code for cases the spec doesn't fully spell out.

## Macro & label names


For macro we apply the rule must start with a letter, then letters/digits/underscores, max 31 characters.
For label we apply the same rule with '_' forbidden.

## Macro name vs. symbol collisions

A symbol can't share a name with a macro. This is enforced for every kind
of symbol definition (labels and `.extern` alike).

## A label with nothing after it is an error

`FOO:` alone on a line is rejected (`ERR_LABEL_WITHOUT_STATEMENT`), not
treated as blank.

## Pre-assembler: recoverable vs. unrecoverable errors

Unlike Pass One, the pre-assembler only keeps scanning past errors that
don't corrupt macro-tracking state (`in_macro`, the body being collected).

- Recoverable: line too long, stray `mcroend`, duplicate macro name.
- Unrecoverable: nested/malformed `mcro`/`mcroend`, allocation failures.

Invalid/reserved macro names are also treated as unrecoverable, though a
cheap recovery (collect the body anyway, skip registering it) exists if
ever needed.

## Error line numbers

Error messages cite `.am` line numbers, not `.as` ones (the forum allows either).

## Line length

The 80-character max line length is validated in the pre-assembler
(`macro_processor.c`), on each raw line as it's read from the `.as` file,
not deferred to Pass One. By the time Pass One reads the `.am` file, every
line is already guaranteed to be within the limit.

## Building

`make` puts object files in `build_artifacts/` and leaves `assembler` (the
deliverable) at the top level. `make clean` removes both; `make distclean`
also clears out `.am` / `.ob` / `.ent` / `.ext` / `.log` files left from test
runs.

## Debug log

`common.h` has `IS_DEBUG` (currently 1), which writes a step-by-step trace to
`tests/assembler.log`. Set it to 0 to turn that off.

## Status

The full pipeline is implemented: macro expansion (pre-assembler), Pass One,
Pass Two, and output file generation (.ob/.ent/.ext).
