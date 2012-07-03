/***************************************************************************
                          clientsidemap.cpp  -  description
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
#include "./includes/clientsidemap.h"
#endif

#include <QString>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QDebug>
//Added by qt3to4:
#include <QTextOStream>

#include <iostream>
using namespace std;

//This class maps a Server Side Image Map onto a
//Client Side Image Map
ClientSideMap::ClientSideMap(QTextStream& serverSideMap, QFile& file, const QString& tempDir)
{
//     QByteArray map;
    QTextStream stream (&file);//map,  QIODevice::WriteOnly);

    stream << "<HTML><BODY>" << endl;
    stream << "<IMG SRC=\"" << tempDir << "graphViz.jpg" << "\" USEMAP=\"#qprof\">" << endl;

    stream << "<MAP NAME=\"qprof\">" << endl;

    QString line;
    int numLines;
    serverSideMap.setCodec("Latin1");

    QString shape;
    QString name;
    QString coordOne;
    QString coordTwo;
    QString N;
    QString E;
    QString S;
    QString W;
    numLines = 0;
    line = serverSideMap.readLine();   //first line is a comment
    line = serverSideMap.readLine();   //second line is redundant
    line = serverSideMap.readLine();   //third line is a comment

    while (!serverSideMap.atEnd()) {
        line = serverSideMap.readLine();

        if (line.length() == 0) {
            continue;
        }

        if ( line[0] != 35) {
            int beginPos = line.indexOf(" ", 0);
            int endPos = line.indexOf(".html", 0);
            QString goodName = line.mid(beginPos + 1, endPos - beginPos);
            goodName = goodName.replace(QRegExp(" "), "_");
            goodName.append("html");

            line = line.replace( QRegExp(" \\["), "[" );
            line = line.replace( QRegExp(" del"), "_del");
            line = line.replace( QRegExp(", "), ",");
            line = line.replace( QRegExp("( "), "(");
            line = line.replace( QRegExp(") "), ")");

            shape = line.section(' ', 0, 0);       //First element is the shape of the object
            name = line. section('.html', 1, 1);      //Second is the name of the file to be referenced
            coordOne = line.section(' ', -2, -2); //Third is the first coordinate
            coordTwo = line.section(' ', -1, -1); //Fourth is the last coordinate
            N = coordOne.section(',', 0, 0);
            W = coordOne.section(',', 1, 1);
            S = coordTwo.section(',', 0, 0);
            E = coordTwo.section(',', 1, 1);
            stream << "<AREA SHAPE=\"RECT\" COORDS = \"" << N << "," << W << "," << S << "," << E << "\" HREF = \"" << goodName << "\" />" << endl;
            numLines++;
        }
    }

    stream << "</MAP>" << endl;
    stream << "</BODY></HTML>" << endl;
}


