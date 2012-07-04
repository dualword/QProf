//Added by qt3to4:
#include <QTextStream>
/***************************************************************************
                          parseprofile_pose.h  -  description
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
#ifndef _CPARSEPROFILE_CALLGRIND_H_
#define _CPARSEPROFILE_CALLGRIND_H_

#ifndef _PARSEPROFILE_H_
#include "parseprofile.h"
#endif

class CProfileInfo;
#include <QString>

class CParseProfile_callgrind : public CParseProfile
{
    public:
        CParseProfile_callgrind(QTextStream& t, QVector<CProfileInfo>& profile);
        ~CParseProfile_callgrind() {};

        bool valid () const;

    private:
        CParseProfile_callgrind();

        typedef struct {                    // structure holding call-graph data for PalmOS Emulator results
            int     index;
            int     parent;
        } SPoseCallGraph;

};

#endif
