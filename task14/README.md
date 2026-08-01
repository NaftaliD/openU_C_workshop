# Notes

A few conventions used in this code that the spec doesn't spell out, in case
they look like judgment calls while reading.

## Macro names

The spec never gives syntax rules for macro names, only that they can't be a
reserved word. We apply the same rule as label names: must start with a
letter, then letters/digits only, no underscores, max 31 characters.

## Macro name vs. symbol collisions

A symbol can't share a name with a macro. This is enforced for every kind
of symbol definition -- labels and `.extern` alike -- not just labels.

## A label with nothing after it is an error

`FOO:` alone on a line is rejected (`ERR_LABEL_WITHOUT_STATEMENT`), not
treated as blank -- there's no instruction/directive to tell Pass One
whether it should be CODE- or DATA-typed.

## Pre-assembler stops at the first error, Pass One doesn't

Pass One collects every error in the file before deciding whether to abort
(each line parses independently, so one bad line can't corrupt how later
lines are read). The pre-assembler (`expandMacros`) deliberately does
*not* do this -- it stops scanning at the first error. Macro-tracking state
(inside a definition or not, whose body is being collected) is cumulative
across lines, so one real problem (e.g. a missing `mcroend`) would make
every subsequent line get misinterpreted if we kept going, producing a
cascade of confusing secondary errors instead of the one real one.

## Line length

The 80-character max line length is validated in the pre-assembler
(`macro_processor.c`), on each raw line as it's read from the `.as` file --
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

Macro expansion (pre-assembler) and Pass One (grammar + driver) are done.
Pass Two and the output file generator (.ob/.ent/.ext) are not implemented
yet.
