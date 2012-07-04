/*
 * consts.h
 *
 * $Id: consts.h,v 0.9 2012
 *
 * Copyright (c) 2000-2001 Florent Pillet <fpillet@users.sourceforge.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 * Requires the K Desktop Environment 2.0 (KDE 2.0) libraries or later,
 * available at no cost at http://www.kde.org/
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

#ifndef __KPROFCONSTS_H__
#define __KPROFCONSTS_H__

#define PROGRAM_NAME "QProfiler v.0.9.1"
#define PROGRAM_VERSION "<b>v.0.9.1 (5 july 2012)</b>"


#define col_function_t                "Function/Method"
#define col_recursive_t               "R"
#define col_count_t                   "Count"
#define col_total_t                   "Total (s)"
#define col_totalPercent_t            "%"
#define col_self_t                    "Self (s)"
#define col_totalMsPerCall_t          "Total ms/call"
// gprof specific columns
#define col_selfMsPerCall_t           "Self ms/call"
// Function Check specific columns
#define col_minMsPerCall_t            "Min. ms/call"
#define col_maxMsPerCall_t            "Max. ms/call"
// POSE specific columns
#define col_selfCycles_t              "Self Cycles"
#define col_cumCycles_t               "Total Cycles"

#define diff_col_function_t           "Function/Method"
#define diff_col_status_t             "Remarks"
#define diff_col_count_t              "Old Count"
#define diff_col_new_count_t          "New Count"
#define diff_col_total_t              "Old Total (s)"
#define diff_col_new_total_t          "New Total (s)"
#define diff_col_totalPercent_t       "Old %"
#define diff_col_new_totalPercent_t   "New %"
#define diff_col_self_t               "Old Self (s)"
#define diff_col_new_self_t           "New Self (s)"
#define diff_col_totalMsPerCall_t     "Old Total ms/call"
#define diff_col_new_totalMsPerCall_t "New Total ms/call"  // last column common to all formats
// gprof specific columns
#define diff_col_selfMsPerCall_t     "Old Self ms/call"
#define diff_col_new_selfMsPerCall_t "New Self ms/call"
// Function Check specific columns
#define diff_col_minMsPerCall_t      "Old Min. ms/call"
#define diff_col_new_minMsPerCall_t  "New Min. ms/call"
#define diff_col_maxMsPerCall_t      "Old Max. ms/call"
#define diff_col_new_maxMsPerCall_t  "New Max. ms/call"
// POSE specific columns
#define diff_col_selfCycles_t        "Old Self Cycles"
#define diff_col_new_selfCycles_t    "New Self Cycles"
#define diff_col_cumCycles_t         "Old Total Cycles"
#define diff_col_new_cumCycles_t     "New Total Cycles"

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
// POSE specific columns
#define col_selfCycles      7
#define col_cumCycles     8

#define diff_col_function       0
#define diff_col_status         1
#define diff_col_count          2
#define diff_col_new_count      3
#define diff_col_total          4
#define diff_col_new_total      5
#define diff_col_totalPercent   6
#define diff_col_new_totalPercent 7
#define diff_col_self           8
#define diff_col_new_self       9
#define diff_col_totalMsPerCall 10
#define diff_col_new_totalMsPerCall 11  // last column common to all formats
// gprof specific columns
#define diff_col_selfMsPerCall  12
#define diff_col_new_selfMsPerCall 13
// Function Check specific columns
#define diff_col_minMsPerCall   12
#define diff_col_new_minMsPerCall 13
#define diff_col_maxMsPerCall   14
#define diff_col_new_maxMsPerCall 15
// POSE specific columns
#define diff_col_selfCycles     12
#define diff_col_new_selfCycles 13
#define diff_col_cumCycles      14
#define diff_col_new_cumCycles 15


#endif