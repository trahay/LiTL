/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/**
 *  \file fut.h
 *  \brief fut Provides mapping of the LiTL APIs into the FxT APIs for easier
 *  usage of LiTL in FxT-related applications
 *
 *  \authors
 *    Developers are: \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#ifndef FUT_H_
#define FUT_H_

#include "fxt.h"

/*  "how" parameter values, analagous to "how" parameters to FKT */
#define FUT_ENABLE                  0xCE03      /* for enabling probes with 1's in keymask */
#define FUT_DISABLE                 0xCE04      /* for disabling probes with 1's in keymask */
#define FUT_SETMASK                 0xCE05      /* for enabling 1's, disabling 0's in keymask */

/*  Simple keymasks */
#define FUT_KEYMASK0                0x00000001
#define FUT_KEYMASK1                0x00000002
#define FUT_KEYMASK2                0x00000004
#define FUT_KEYMASK3                0x00000008
#define FUT_KEYMASK4                0x00000010
#define FUT_KEYMASK5                0x00000020
#define FUT_KEYMASK6                0x00000040
#define FUT_KEYMASK7                0x00000080
#define FUT_KEYMASK8                0x00000100
#define FUT_KEYMASK9                0x00000200
#define FUT_KEYMASK10               0x00000400
#define FUT_KEYMASK11               0x00000800
#define FUT_KEYMASK12               0x00001000
#define FUT_KEYMASK13               0x00002000
#define FUT_KEYMASK14               0x00004000
#define FUT_KEYMASK15               0x00008000
#define FUT_KEYMASK16               0x00010000
#define FUT_KEYMASK17               0x00020000
#define FUT_KEYMASK18               0x00040000
#define FUT_KEYMASK19               0x00080000
#define FUT_KEYMASK20               0x00100000
#define FUT_KEYMASK21               0x00200000
#define FUT_KEYMASK22               0x00400000
#define FUT_KEYMASK23               0x00800000
#define FUT_KEYMASK24               0x01000000
#define FUT_KEYMASK25               0x02000000
#define FUT_KEYMASK26               0x04000000
#define FUT_KEYMASK27               0x08000000
#define FUT_KEYMASK28               0x10000000
#define FUT_KEYMASK29               0x20000000
#define FUT_KEYMASK30               0x40000000
#define FUT_KEYMASK31               0x80000000
#define FUT_KEYMASKALL              0xffffffff

#define FUT_GCC_INSTRUMENT_KEYMASK  FUT_KEYMASK29

/*  Fixed parameters of the fut coding scheme */
#define FUT_GENERIC_EXIT_OFFSET     0x100   /* exit this much above entry */

#define FUT_UNPAIRED_LIMIT_CODE     0xf000  /* all unpaired codes above this limit */

/*  Codes for fut use */
#define FUT_SETUP_CODE              0xffff
#define FUT_KEYCHANGE_CODE          0xfffe
#define FUT_RESET_CODE              0xfffd
#define FUT_CALIBRATE0_CODE         0xfffc
#define FUT_CALIBRATE1_CODE         0xfffb
#define FUT_CALIBRATE2_CODE         0xfffa

#define FUT_THREAD_BIRTH_CODE       0xfff9
#define FUT_THREAD_DEATH_CODE       0xfff8
#define FUT_SET_THREAD_NAME_CODE    0xfff7

#define FUT_NEW_LWP_CODE            0xfff6

#define FUT_START_FLUSH_CODE        0xfff5
#define FUT_STOP_FLUSH_CODE         0xfff4

#define FUT_RQS_NEWLEVEL            0xffef
#define FUT_RQS_NEWLWPRQ            0xffee
#define FUT_RQS_NEWRQ               0xffed

#define FUT_SWITCH_TO_CODE          0x31a

#define FUT_MAIN_ENTRY_CODE         0x301
#define FUT_MAIN_EXIT_CODE          0x401

#endif /* FUT_H_ */
