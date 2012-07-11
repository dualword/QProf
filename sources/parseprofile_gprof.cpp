/***************************************************************************
                          parseprofile_gprof.cpp  -  description
                             -------------------
    begin                : Tue Jul 9 2002
    copyright            : (C) 2002 by Colin Desmond
    email                : colin.desmond@btopenworld.com

    ported to Qt4        : Eduard Kalinowski
    email                : eduard_kalinowski@yahoo.de

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "./includes/parseprofile_gprof.h"

#include "./includes/cprofileinfo.h"
#include "./includes/qprofwidget.h"

#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QRegExp>


CParseProfile_gprof::CParseProfile_gprof (QTextStream& t, QVector<CProfileInfo>& profile)
{
    /*
     * parse a GNU gprof output generated with -b (brief)
     *
     */

    // regular expressions we use while parsing
    QRegExp indexRegExp (" *\\[\\d+\\] *$");
    QRegExp floatRegExp ("^[0-9]*\\.[0-9]+$");
    QRegExp countRegExp ("^[0-9]+[/\\+]?[0-9]*$");
    QRegExp dashesRegExp ("^\\-+");
    QRegExp recurCountRegExp (" *<cycle \\d+.*> *$");

    // while parsing, we temporarily store all entries of a call graph block
    // in an array
    QVector<SCallGraphEntry> callGraphBlock;
//     callGraphBlock..setAutoDelete (true);
    callGraphBlock.resize (32);

    int state = SEARCH_FLAT_PROFILE;
    long line = 0;
    QString s;

    t.setCodec ("Latin1");

    while (!t.atEnd ()) {
        line++;
        s = t.readLine ();

        if (s.length() == 0) {
            continue;
        }

        if (s[0] == 12) {
            if (state == PROCESS_CALL_GRAPH) {
                processCallGraphBlock (callGraphBlock, profile);
                break;
            }

            if (state == PROCESS_FLAT_PROFILE) {
                state = SEARCH_CALL_GRAPH;
            }

            continue;
        }

        s = s.simplified();

        // remove <cycle ...> and [n] from the end of the line
        if (state == PROCESS_FLAT_PROFILE || state == PROCESS_CALL_GRAPH) {
            int pos = indexRegExp.indexIn(s, 0);

            if (pos != -1) {
                s = s.left (pos);
            }

            pos = recurCountRegExp.indexIn (s, 0);

            if (pos != -1) {
                s = s.left (pos);
            }
        }

        // split the line in tab-delimited fields
        QStringList fields = s.split (" ", QString::SkipEmptyParts);

        if (fields.isEmpty ()) {
            continue;
        }

        if (fields.count() > 1 && (state == PROCESS_FLAT_PROFILE || state == PROCESS_CALL_GRAPH)) {
            // the split did also split the function name & args. restore them so that they
            // are only one field
            uint join;

            for (join = 0; join < fields.count (); join++) {
                QChar c = fields[join][0];

                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                    break;
                }
            }

            while (join < (fields.count () - 1)) {
                fields[join] += " " + fields[join + 1];
                fields.removeAll (fields.at (join + 1));
            }
        }

        switch (state) {
            /*
             * look for beginning of flat profile
             *
             */
        case SEARCH_FLAT_PROFILE:

            if (fields[0] == "time" && fields[1] == "seconds" && fields[2] == "seconds") {
                state = PROCESS_FLAT_PROFILE;
            }

            break;

            /*
             * analyze flat profile entry
             *
             */
        case PROCESS_FLAT_PROFILE: {
            CProfileInfo *p = new CProfileInfo;
            bool isFloat;
            p->ind = profile.count ();

            if (fields[0].length() < 1) {
                delete p;
                break;
            }

            p->cumPercent = fields[0].toFloat (&isFloat);

            if (isFloat == false) {
                delete p;
                break;
            }

            p->cumSeconds = fields[1].toFloat (&isFloat);

            if (isFloat == false) {
                delete p;
                break;
            }

            p->selfSeconds = fields[2].toFloat (&isFloat);

            if (isFloat == false) {
                delete p;
                break;
            }

            if (fields[3][0].isDigit ()) {
                p->calls            = fields[3].toLong ();
                p->custom.gprof.selfMsPerCall   = fields[4].toFloat ();
                p->totalMsPerCall   = fields[5].toFloat ();
                p->name             = fields[6];
            } else {
                // if the profile was generated with -z, uncalled functions
                // have empty "calls", "selfTsPerCall" and "totalTsPerCall" fields
                p->calls            = 0;
                p->custom.gprof.selfMsPerCall = 0;
                p->totalMsPerCall   = 0;
                p->name             = fields[3];
            }

            // p->simplifiedName will be updated in postProcessProfile()
            p->recursive        = false;
            p->object           = QProfWidget::getClassName (p->name);
            p->multipleSignatures = false;              // will be updated in postProcessProfile()

            int argsoff = p->name.indexOf ('(');

            if (argsoff != -1) {
                p->method = p->name.mid (p->object.length(), argsoff - p->object.length());
                p->arguments  = p->name.right (p->name.length() - argsoff);
            } else {
                p->method = p->name.right (p->name.length() - p->object.length());
            }

            if (p->method.startsWith ("::")) {
                p->method.remove (0, 2);
            }

//                 int j = profile.size ();
            profile.append (*p);
//                 profile.insert (j, *p);
            break;
        }

        /*
         * look for call graph
         *
         */
        case SEARCH_CALL_GRAPH:

            if (fields[0] == "index" && fields[1] == "%") {
                state = PROCESS_CALL_GRAPH;
            }

            break;

            /*
             * analyze call graph entry
             *
             */
        case PROCESS_CALL_GRAPH: {
            // if we reach a dashes line, we finalize the previous call graph block
            // by analyzing the block and updating callers, called & recursive
            // information
            if (dashesRegExp.indexIn (fields[0], 0) == 0) {
                processCallGraphBlock (callGraphBlock, profile);
                callGraphBlock.resize (0);
                break;
            }

            QString count;
            SCallGraphEntry *e = new SCallGraphEntry;
            uint field = 0;

            e->line = line;
            e->primary = false;
            e->recursive = false;

            // detect the primary line in the call graph
            if (indexRegExp.indexIn (fields[0], 0) == 0) {
                e->primary = true;
                field++;
            }

            // gather other values (we have to do some guessing to get them right)
            while (field < fields.count ()) {
                if (countRegExp.indexIn (fields[field], 0) == 0) {
                    e->recursive = fields[field].indexOf ('+') != -1;
                } else if (floatRegExp.indexIn (fields[field], 0) == -1) {
                    e->name = fields[field];
                }

                field++;
            }

            // if we got a call graph block without a primary function name,
            // drop it completely.
            if (e->name == NULL || e->name.length() == 0) {
                delete e;
                break;
            }

            if (e->primary == true && count.indexOf ('+') != -1) {
                e->recursive = true;
            }

            callGraphBlock.append(*e);
            break;
        }
        }
    }
}

bool CParseProfile_gprof::valid() const
{
    return mValid;
}
