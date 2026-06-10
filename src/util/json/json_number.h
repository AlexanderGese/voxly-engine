#ifndef UTIL_JSON_NUMBER_H
#define UTIL_JSON_NUMBER_H

// number lexing + formatting. split out because both the parser (reading) and
// the writer (printing back) need to agree on exactly what a json number looks
// like, and i didnt want that grammar smeared across two files.

#include <stddef.h>

#include "../strbuf.h"
#include "json_types.h"

// scan a json number starting at s[0..n). on success returns the number of
// bytes consumed, writes the parsed value to *out and sets *is_int when the
// source had no '.' / 'e' (so we can round-trip integers cleanly). returns 0 if
// the leading bytes arent a valid json number (the caller turns that into
// JSON_ERR_BADNUMBER).
size_t json_number_scan(const char *s, size_t n, double *out, int *is_int);

// format a number into out. when is_int and the value fits a 64-bit integer we
// print it with no decimal point; otherwise we use the shortest round-trippable
// %.17g and then trim it. keeps configs from turning every 1 into 1.0.
void json_number_format(strbuf *out, double v, int is_int);

#endif
