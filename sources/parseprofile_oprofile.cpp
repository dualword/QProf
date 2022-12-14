/***************************************************************************
                          parseprofile_pose.cpp  -  description
                             -------------------
    begin                : Wed Jul 10 2002
    copyright            : (C) 2002 by Colin Desmond
    email                : colin.desmond@btopenworld.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "./includes/parseprofile_oprofile.h"

#include <stdlib.h>

#include "./includes/cprofileinfo.h"
#include "./includes/qprofwidget.h"

#include <QTextStream>
#include <QVector>
#include <QRegExp>
#include <QHash>

CParseProfile_oprofile::CParseProfile_oprofile (QTextStream& t, QVector<CProfileInfo>& profile)
{
    /*
     * parse a profile results file generated by the PalmOS Emulator
     *
     */

    int line = 0;
    int numEntries = 0;
    int cgCount = 0;
    QString s;

    // because of the way POSE results are shown, we have to keep a dictionnary
    // of indexes -> CProfileInfo*, and a list of call maps index -> parent index
    QHash<QString, CProfileInfo*> functions;// (257);
    SPoseCallGraph* callGraph = (SPoseCallGraph *) malloc (256 * sizeof (SPoseCallGraph));

    // mapping between indexes and profile ptrs. because of the number of indexes
    // one typically encounters in a profile results file, use of a dictionnary
    // leads to very slow parsing. This is why I use the array below...
    int indexes = 8192;
    CProfileInfo **indexToProfile = (CProfileInfo **) malloc (indexes * sizeof (CProfileInfo *));

    for (int i = 0; i < indexes; i++) {
        indexToProfile[i] = NULL;
    }

    t.setCodec ("Latin1");

    while (!t.atEnd ()) {
        if (++line == 0) {
            continue;    // skip first line, it only contains field descriptor
        }

        s = t.readLine ();

        if (s.length() == 0) {
            continue;
        }

        // split the line fields
        QStringList fields;
        fields = s.split ("\t");

        if (fields.isEmpty ()) {
            continue;
        }

        for (uint i = 0; i < fields.count(); i++) {
            fields[i] = fields[i].trimmed();
        }

        if (fields.count() != 15) {
            line--;
            continue;
        }

        // gather the index of this entry
        int ind = fields[0].toInt();

        if (ind > 512000) {
            // uh ? this is probably a parsing problem!
            line--;
            continue;
        }

        // first look if we have a dictionnary entry for this function
        bool created = false;
        CProfileInfo *p = functions[fields[3]];

        if (p == NULL) {
            // nope: create a new one
            p = new CProfileInfo;
            functions.insert (fields[3].toLatin1(), p);
            p->ind  = numEntries;
            p->name = fields[3];
            created = true;
        }

        // add entry to the indexes dictionary
        if (ind >= indexes) {
            int n = ((ind / 8192) + 1) * 8192;
            indexToProfile = (CProfileInfo **) realloc (indexToProfile, n * sizeof (CProfileInfo *));

            // @@@ TODO: test and report memory error here
            for (int i = indexes; i < n; i++) {
                indexToProfile[i] = NULL;
            }

            indexes = n;
        }

        indexToProfile[ind] = p;

        // add caller to the callers list
        if (cgCount && (cgCount & 0xff) == 0) {
            callGraph = (SPoseCallGraph *) realloc (callGraph, (cgCount + 256) * sizeof (SPoseCallGraph));
        }

        callGraph[cgCount].index = ind;
        callGraph[cgCount++].parent = fields[1].toInt ();

        p->cumPercent       += fields[10].toFloat ();
        p->cumSeconds       += fields[9].toFloat () / 1000.0;       // value given in milliseconds
        p->selfSeconds      += fields[6].toFloat () / 1000.0;       // value given in milliseconds
        p->calls            += fields[4].toLong ();
        p->custom.pose.selfCycles += fields[5].toLong ();
        p->custom.pose.cumCycles += fields[8].toLong ();

        // @@@ TODO: check and fix this
        float v = fields[11].toFloat ();

        if (v > p->totalMsPerCall) {
            p->totalMsPerCall = v;
        }

        p->totalMsPerCall   += fields[5].toFloat ();

        // p->simplifiedName will be updated in postProcessProfile()
        p->recursive        = false;
        p->object           = QProfWidget::getClassName (p->name);
        p->multipleSignatures = false;                              // will be updated in postProcessProfile()

        if (created) {
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

            profile.append(*p);
//             profile.resize (numEntries + 1);
//             profile.insert (numEntries++, *p);
        }
    }

    // post-process call-graph data
    for (int i = 0; i < cgCount; i++) {
        int father = callGraph[i].parent;
        int child  = callGraph[i].index;

        if (father != -1) {
            CProfileInfo *pFather = indexToProfile [father];
            CProfileInfo *pChild  = indexToProfile [child];

            // these errors should not happen in a well-formed profile result,
            // but who knows...
            if (pFather == NULL) {
                fprintf (stderr, "qprof: pFather==NULL: No profile entry for index %d!\n", father);
                continue;
            }

            if (pChild == NULL) {
                fprintf (stderr, "qprof: pChild==NULL: No profile entry for index %d!\n", child);
                continue;
            }

            if (pFather == pChild) {
                pFather->recursive = true;
            } else {
                if (pFather->called.count() == 0 || pFather->called.indexOf (pChild) == -1) {
                    int n = pFather->called.count ();
                    pFather->called.append (pChild);
                }

                if (pChild->callers.count() == 0 || pChild->callers.indexOf (pFather) == -1) {
                    int n = pChild->callers.count ();
                    pChild->callers.append (pFather);
                }
            }
        }
    }

    free (indexToProfile);
    free (callGraph);
}

bool CParseProfile_oprofile::valid() const
{
    return mValid;
}
