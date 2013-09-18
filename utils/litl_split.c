/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/**
 *  \file utils/litl_split.c
 *  \brief litl_split A utility for disassembling archives of traces into
 *  separate regular trace files
 *
 *  \authors
 *    Developers are: \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "litl_split.h"

static char *__archive_name = "";
static char *__output_dir = "";

static void __usage(int argc __attribute__((unused)), char **argv) {
  fprintf(stderr, "Usage: %s [-f archive_traces] [-d output_dir] \n", argv[0]);
  printf("       -?, -h:    Display this help and exit\n");
}

static void __parse_args(int argc, char **argv) {
  int i;

  for (i = 1; i < argc; i++) {
    if ((strcmp(argv[i], "-f") == 0)) {
      __archive_name = argv[++i];
    } else if ((strcmp(argv[i], "-d") == 0)) {
      __output_dir = argv[++i];
    } else if ((strcmp(argv[i], "-h") || strcmp(argv[i], "-?")) == 0) {
      __usage(argc, argv);
      exit(-1);
    } else if (argv[i][0] == '-') {
      fprintf(stderr, "Unknown option %s\n", argv[i]);
      __usage(argc, argv);
      exit(-1);
    }
  }

  if (strcmp(__archive_name, "") == 0) {
    __usage(argc, argv);
    exit(-1);
  } else if (strcmp(__output_dir, "") == 0) {
    __usage(argc, argv);
    exit(-1);
  }
}

int main(int argc, char **argv) {

  // parse the arguments passed to this program
  __parse_args(argc, argv);

  // split the archive
  litl_split_archive(__archive_name, __output_dir);

  return EXIT_SUCCESS;
}
