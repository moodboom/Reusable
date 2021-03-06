//
// File: tpie_tempnam.C
// Author: 
// Created: 02/02/02
//
#include <versions.H>
VERSION(tpie_tempnam_C,"$Id: tpie_tempnam.C,v 1.1 2008/07/28 17:08:02 m Exp $");

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lib_config.H"
#include <tpie_tempnam.H>

// Defined below.
char *tpie_mktemp(char *str);

/* like tempnam, but consults environment in an order we like; note
 * that the returned pointer is to static storage, so this function is
 * not re-entrant. */
char *tpie_tempnam(char *base, const char* dir) {
  char *base_dir;
  static char tmp_path[BUFSIZ];
  char *path;

  if (dir == NULL) {
    // get the dir
    base_dir = getenv(AMI_SINGLE_DEVICE_ENV);
    if (base_dir == NULL) {
      base_dir = getenv(TMPDIR_ENV);
      if (base_dir == NULL) {
	base_dir = TMP_DIR;
      }
    }
    sprintf(tmp_path, TPIE_OS_TEMPNAMESTR, base_dir, base);
  } else {
    sprintf(tmp_path, TPIE_OS_TEMPNAMESTR, dir, base);
  }

  path = tpie_mktemp(tmp_path);    
  return path;
}

char *tpie_mktemp(char *str) {
  const char chars[] = 
  { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  const int chars_count = 62;
  static int counter = time(NULL) % (chars_count * chars_count); 
  int pos = strlen(str) - 6;

  str[pos++] = chars[counter/chars_count];
  str[pos++] = chars[counter%chars_count];

  str[pos++] = chars[TPIE_OS_RANDOM() % chars_count];
  str[pos++] = chars[TPIE_OS_RANDOM() % chars_count];
  str[pos++] = chars[TPIE_OS_RANDOM() % chars_count];
  str[pos]   = chars[TPIE_OS_RANDOM() % chars_count];

  counter = (counter + 1) % (chars_count * chars_count);
  return str;
}
