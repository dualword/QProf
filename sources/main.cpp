/*
 * main.cpp
 *
 * Copyright (c) 2012 Eduard Kalinowski <karbofos@ymail.com>
 * Copyright (c) 2000 Florent Pillet <florent.pillet@wanadoo.fr>
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

#include <QApplication>
#include <QLocale>
#include <QString>
#include <QtGui>
#include <QSplashScreen>

// #include "./includes/qprof.h"
#include "./includes/Log.h"
#include "./includes/qprofwidget.h"
#include "./includes/constants.h"


static const char *description = "Execution profile results analysis utility";


int main(int argc, char **argv)
{
#if 0
    KAboutData aboutData(
        PACKAGE, I18N_NOOP("QProf"),
        VERSION, description, KAboutData::License_GPL,
        "(c) 2000-2002, Florent Pillet & Colin Desmond\n",
        NULL,
        "http://kprof.sourceforge.net/",
        "fpillet,cdesmond@users.sourceforge.net");

//     KCmdLineArgs::init (argc, argv, &aboutData);
//     KCmdLineArgs::addCmdLineOptions( options );

    aboutData.addAuthor("Florent Pillet", 0, "florent.pillet@wanadoo.fr ");
    aboutData.addAuthor("Colin Desmond", 0, "colin.desmond@btopenworld.com");

    QApplication app;

    Log::Init("TRC", 80000);
    RUN("create the application");

    if( app.isRestored() ) { //SessionManagement
        RUN("restore the top level widget");
        RESTORE(QProfTopLevel);
    } else {
        RUN("create the top level widget");
        QProfTopLevel *ktl = new QProfTopLevel(0, "QProf main");
        CHECK_PTR(ktl);
        RUN("show the top level widget");
        ktl->show();

    }

    RUN("start the application");
    int i = app.exec();
    RUN("finish the application");

    return i;
#else

    QApplication app(argc, argv);
#if 0
    QPixmap logo ( ":/images/splash.png" );
    QPainter painter ( &logo );
    painter.setFont ( QFont ( "Arial", 15 ) );

    QSplashScreen* splash = new QSplashScreen ( logo );
    splash->show();
    splash->showMessage ( QObject::tr ( PROGRAM_FULL_NAME ), Qt::AlignLeft | Qt::AlignBottom );
#endif
    QProfWidget * mw;
    mw = new QProfWidget(0, Qt::Window );
//     mw->setLangGUI();
    mw->setWindowTitle(QString(PROGRAM_NAME));

    mw->show();
    return app.exec();
#endif
}

