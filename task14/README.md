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

Macro expansion (pre-assembler) is done. Pass One, Pass Two and the output
file generator (.ob/.ent/.ext) are not implemented yet.
