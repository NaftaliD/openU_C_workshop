/*
 * pass_two.h
 * Second pass: resolves labels and finishes every operation's binary
 * encoding, writing .ob/.ent/.ext directly as each line becomes ready
 */

#ifndef PASS_TWO_H
#define PASS_TWO_H

#include "assembler_context.h"

/**
 * Runs Pass Two, writing lines to each file as they're ready.
 * ent_filename/ext_filename are only created if something is actually
 * written to them. Any error removes every file this call created.
 * @param ob_filename path to write the object file to
 * @param ent_filename path to write entry symbols to, if any exist
 * @param ext_filename path to write external symbol usages to, if any exist
 * @param context the assembler context, fully populated by Pass One
 * @return TRUE on success, FALSE if ob_filename could not be opened for writing
 */
Boolean runPassTwo(const char *ob_filename, const char *ent_filename,
                    const char *ext_filename, AssemblerContext *context);

#endif /* PASS_TWO_H */
