/***************************************************************************
                          parseprofile.cpp  -  description
                             -------------------
    begin                : Tue Jul 9 2002
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

#include "./includes/parseprofile.h"

#include "./includes/cprofileinfo.h"

#include <QString>
#include <QVector>
#include <QDebug>

CParseProfile::CParseProfile()
{
    mValid = true;
}

void CParseProfile::processCallGraphBlock (const QVector<SCallGraphEntry> &data,
        QVector<CProfileInfo>& profile)
{
    // process a call graph block. The list of entries is stored in the
    // order they are encountered, which is very important because the
    // order matters (see the gprof manual)

    // first, locate the primary entry
    CProfileInfo *primary = NULL;
    uint i;

    for (i = 0; i < data.count(); i++) {
        if (data[i].primary) {
            primary = locateProfileEntry (data[i].name, profile);
            break;
        }
    }

    // make sure primary is not NULL !
    if (primary == NULL) {
        // @@@ should perform better error reporting here
        if (i != data.count ())
            qDebug()  << "missing flat profile entry for '" << data[i].name.toLatin1()
                      << "' (line " << data[i].line << ")";

        return;
    }

    // store callers, called funcs and info about primary entry
    bool beforePrimary = true;

    for (i = 0; i < data.count (); i++) {
        CProfileInfo *p = locateProfileEntry (data[i].name, profile);
// qDebug() << i << data[i].name;
        if (p == NULL) {
            break;
        }

        if (data[i].primary) {
            if (data[i].recursive) {
                p->recursive = true;
            }

            beforePrimary = false;
            continue;
        }

        if (p) {
            if (beforePrimary) {
                if (primary->callers.count() == 0 || primary->callers.indexOf(p) == -1) {
                    primary->callers.append (p);
                }
            } else {
                if (primary->called.count() == 0 || primary->called.indexOf(p) == -1) {
                    primary->called.append ( p);
                }
            }
        }
    }
//     qDebug() << primary->name << primary->called.count();
}

CProfileInfo *CParseProfile::locateProfileEntry (const QString& name, QVector<CProfileInfo>& profile)
{
    // find an entry in our profile table based on function name
    for (uint j = 0; j < profile.count (); j++) {
        if (profile[j].name == name) {
            return &profile[j];
        }
    }

    return NULL;
}
