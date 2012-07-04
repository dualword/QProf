/*
 * kprof.cpp
 *
 * Copyright (c) 2012 Eduard Kalinowski <karbofos@ymail.com>
 * Copyright (c) 2000-2001 Florent Pillet <fpillet@users.sourceforge.net>
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

#include <qicon.h>

#include <QApplication>
#include <QMainWindow>
#include <QSettings>
#include <QLocale>
#include <QAction>
// #include <kstdaction.h>
#include <QIcon>
#include "./includes/Log.h"

#include "./includes/qprof.h"
#include "./includes/qprofwidget.h"

QProfTopLevel::QProfTopLevel (QWidget *parent, const char *name)
    :   QMainWindow (parent, name)
{
//     BEGIN;
    kapp = parent;

    mProf = new QProfWidget (this, "qprof");
    CHECK_PTR(mProf);

    setupActions ();
    createGUI ("kprofui.rc");
    setCentralWidget (mProf);

    // load the recent files list
    QSettings *config = kapp->config ();
    KRecentFilesAction *recent = (KRecentFilesAction *) actionCollection()->action (KStdAction::stdName (KStdAction::OpenRecent));
    recent->loadEntries (config);

    connect (mProf, SIGNAL (addRecentFile(const QUrl&)), this, SLOT(addRecentFile(const QUrl&)));

    loadSettings ();
    applySettings ();

    RUN("process command line args");
    mProf->openCommandLineFiles ();

//     END;
}

void KProfTopLevel::loadSettings ()
{
//     BEGIN;
    QSettings &config = *kapp->config;
    config.setGroup ("KProfiler");

    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(840, 640)).toSize();
    resize(size);
    move(pos);

    mProf->loadSettings ();
//     END;
}

void KProfTopLevel::applySettings ()
{
//     BEGIN;
    QSettings &config = *kapp->config ();
    config.setGroup ("KProfiler");
    config.writeEntry ("Width", width ());
    config.writeEntry ("Height", height ());
    mProf->applySettings ();
    config.sync ();
//     END;
}

void KProfTopLevel::setupActions ()
{
//     BEGIN;
    KStdAction::open (mProf, SLOT(openResultsFile()), actionOpen());
    KStdAction::openRecent (mProf, SLOT(openRecentFile(const QUrl&)), actionOpenRecent());
    mCompareFile = new QAction (tr ("Compare With..."), 0, mProf, SLOT (compareFile ()), actionCompare(), "compare_file");
    mCompareFile->setEnabled (false);
    KStdAction::print (mProf, SLOT(doPrint()), actionPrint());
    KStdAction::quit (this, SLOT(close()), actionQuit ());

    KStdAction::showToolbar (this, SLOT(toggleToolBar()), actionShowToolbar());

    mToggleTemplateAbbrev = new QAction (tr ("Abbreviate C++ &Templates"), 0, mProf, SLOT (toggleTemplateAbbrev ()), actionCollection(), "toggle_template_abbreviations");
    mSelectFont = new QAction (tr ("Select Font..."), 0, mProf, SLOT (selectListFont ()), actionSelectFont(), "select_list_font");
    mGenCallGraphAction = new QAction (tr ("&Generate Call Graph..."), 0, mProf, SLOT (generateCallGraph ()), actionGenCallGraph(), "generate_call_graph");
#ifdef HAVE_LIBQTREEMAP
    mDisplayTreeMapAction = new QAction(tr("&Display TreeMap View"), 0, mProf, SLOT (displayTreeMapView()), actionDisplayTreeMap(), "display_tree_map_view");
#endif
    mConfigure = new QAction(tr("&Configure KProf"), 0, mProf, SLOT(configure()), actionConfigure(), "configure_kprof");

//     END;
}

void KProfTopLevel::toggleToolBar ()
{
//     BEGIN;

    if (toolBar()->isVisible ()) {
        toolBar()->hide ();
    } else {
        toolBar()->show ();
    }

//     END;
}

KProfTopLevel::~KProfTopLevel ()
{
//     BEGIN;
//     END;
}

bool KProfTopLevel::queryExit( void )
{
//     BEGIN;
    QSettings *config = kapp->config ();
    KRecentFilesAction *recent = (KRecentFilesAction *) actionCollection()->action (KStdAction::stdName (KStdAction::OpenRecent));
    recent->saveEntries (config);
    applySettings ();
//     END;
    return true;
}

void KProfTopLevel::addRecentFile (const QUrl& url)
{
    // this slot is called by kprofwidget when a file has been opened.
    // we store it in the recent files and also change the window title
//     BEGIN;
    KRecentFilesAction *recent = (KRecentFilesAction *) actionCollection()->action (KStdAction::stdName (KStdAction::OpenRecent));
    recent->addURL (url);

    setCaption (url.fileName ());
    mCompareFile->setEnabled (true);
//     END;
}

// #include "kprof.moc"
