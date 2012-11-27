/*
 * consts.h
 *
 * Copyright (c) 2012 Eduard Kalinowski <karbofos@ymail.com>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __QPROFCONSTS_H__
#define __QPROFCONSTS_H__

#define PROGRAM_NAME "QProf v.1.2.2"
#define PROGRAM_VERSION "<b>v.1.2.2 (26 aug 2012)</b>"


#define col_function_t                "Function/Method"
#define col_recursive_t               "R"
#define col_count_t                   "Calls"
#define col_total_t                   "Total (s)"
#define col_totalPercent_t            "%"
#define col_self_t                    "Self (s)"
#define col_totalMsPerCall_t          "Total ms/call"
// gprof specific columns
#define col_selfMsPerCall_t           "Self ms/call"
// Function Check specific columns
#define col_minMsPerCall_t            "Min. ms/call"
#define col_maxMsPerCall_t            "Max. ms/call"
// CALLGRIND , POSE specific columns
#define col_selfCycles_t              "Self Cycles"
#define col_cumCycles_t               "Total Cycles"
// CALLGRIND specific columns
// #define col_selfCycles_t              "Self Cycles"
// #define col_cumCycles_t               "Total Cycles"


#define col_function        0
#define col_recursive       1
#define col_count           2
#define col_total           3
#define col_totalPercent    4
#define col_self            5
#define col_totalMsPerCall  6           // last column common to all formats
// gprof specific columns
#define col_selfMsPerCall   7
// Function Check specific columns
#define col_minMsPerCall    7
#define col_maxMsPerCall    8
// POSE, CALLGRIND specific columns
#define col_selfCycles      7
#define col_cumCycles     8


#endif