//Added by qt3to4:
#include <QTextStream>
/***************************************************************************
                          clientsidemap.h  -  description
                             -------------------
    begin                : Mon Jul 8 2002
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
#ifndef _CLIENTSIDEMAP_H_
#define _CLIENTSIDEMAP_H_

#include <QFile>
#include <QTextStream>
#include <QString>

class ClientSideMap
{
//           Q_OBJECT
    public:
        ClientSideMap(QTextStream& serverSideMap, QFile& file, const QString& tempDir);
        ~ClientSideMap() {};

    private:
        ClientSideMap();
};

#endif
