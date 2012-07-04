/*
 * kprofwidget.h
 *
 * $Id: kprofwidget.h,v 1.35 2004/06/19 05:18:55 bab Exp $
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

#ifndef __QPROFWIDGET_H__
#define __QPROFWIDGET_H__

#include <QVector>
#include <QList>
#include <QString>
#include <QToolButton>
#include <QLineEdit>
#include <QPrinter>
#include <QActionGroup>
#include <QTextStream>
#include <QDir>
#include <QtGui/QApplication>
#include <QToolBar>
#include <QTreeWidget>
#include <QTabWidget>
#include <QUrl>
#include <QProcess>

#include "cprofileinfo.h"
#include "cconfigure.h"
#include "ui_mainForm.h"
#include "ui_call-graph.h"

#include <QFont>
#include <QFile>
class CProfileViewItem;

#define PROGRAM_NAME "QProfiler v.0.9.0"
#define PROGRAM_VERSION "<b>v.0.9.0 (5 july 2012)</b>"


#ifdef HAVE_LIBQTREEMAP
class QTreeWidgetTreeMapWindow;
class QTreeMapOptions;
#endif

#define FORMAT_GPROF    1                   // GNU gprof
#define FORMAT_FNCCHECK 2            // Function Check
#define FORMAT_POSE     3                // PalmOS Emulator


class CCallGraph : public QDialog, public Ui_CCallGraph
{
        Q_OBJECT

    public:
};



class QProfWidget : public QMainWindow, public Ui_MainWindow
{
        Q_OBJECT

    public:
        QFont            sListFont;  // font used to draw list entries
        static short     sLastFileFormat;    // format of the last opened file
        static bool      sDiffMode;  // true if performing a diff. Used by CProfileViewItem

    protected:
        QProcess graphApplication;
        QProcess displayApplication;
        QProcess gprofApplication;

        QVector<CProfileInfo>   mProfile;   // profile information read from file
        QVector<CProfileInfo>   mPreviousProfile;   // when comparing, keep previous profile information here
        QVector<QString>            mClasses;   // list of distinct class names found in the profile information

        QStringList   comm_columns;
        QStringList   comm_diff_columns;
        QStringList   prof_columns;
        QStringList   prof_diff_columns;
        QStringList   func_columns;
        QStringList   func_diff_columns;
        QStringList   pose_columns;
        QStringList   pose_diff_columns;
        QStringList   recentList;
        QActionGroup* recentGroup;
        QMenu*        recentMenu;
        QVector<QAction*> actRecentSelect;

        QString       mGProfStdout;   // stdout from gprof command
        QString       mGProfStderr;   // stderr from gprof command
        QString       mGraphVizStdout;//  Stdout from the graphViz command
        QString       mGraphVizStderr; // Stderr from the graphViz command
        QString       mGraphVizDispStdout;//  Stdout from the graphViz command
        QString       mGraphVizDispStderr; // Stderr from the graphViz command
        QString       mFlatFilter;    // filter string for flat profile view
        QFont         mListFont;      // font used to draw the text
        bool          mAbbrevTemplates; // if true, templates are "abbreviates" (i.e. become <...>)
        QDir          mCurDir;        // current directory
        QToolBar*     fileToolBar;
        QToolBar*     filterToolBar;

#ifdef HAVE_LIBQTREEMAP
        QTreeMapOptions*        mTreemapOptions;
        QTreeWidgetTreeMapWindow* mObjTreemap;
        QTreeWidgetTreeMapWindow* mHierTreemap;
#endif

    public:
        QProfWidget (QWidget *parent = NULL, Qt::WindowFlags flags = 0);
        ~QProfWidget ();
        static QString getClassName (const QString& name);

    public slots:
        void settingsChanged ();
        void loadSettings ();
        void applySettings ();

        void openResultsFile ();
        void compareFile ();
        void openRecentFile (QAction* act);
        void openCommandLineFiles ();
        void doPrint ();

        void profileEntryRightClick (const QPoint & iPoint);
        void flatProfileFilterChanged (const QString &filter);
        void generateCallGraph ();
        void displayTreeMapView();
        void aboutQt();
        void about();
        void quit();
        void toggleTemplateAbbrev (bool state);
        void selectListFont ();
        void configure();

    protected slots:
        void gprofStdout ();
        void gprofStderr ();
        void graphVizStdout ();
        void graphVizStderr ();
        void graphVizDispStdout ();
        void graphVizDispStderr ();
        /*
            signals:
                void addRecentFile (const QUrl&);*/

    private:
        void initColFields();
        void createToolBars();
        void openFile (const QString &filename, short format, bool compare = false);
        void prepareProfileView (QTreeWidget *view, bool rootIsDecorated, short profiler);
        void postProcessProfile (bool compare);
        void prepareHtmlPart(QTextBrowser* part);
        bool parseArguments(const QStringList &args, QString& fileName, short& prof);
        void addRecentFile (const QUrl&);
        void fillFlatProfileList ();
        void fillHierProfileList ();
        void fillHierarchy (CProfileViewItem *item, CProfileInfo *parent, QVector<CProfileInfo *> &addedEntries, int &count);
        void fillObjsProfileList ();

//         void selectProfileItem (CProfileInfo *info);
        void selectItemInView (QTreeWidgetItem *view, CProfileInfo *info, bool examineSubs);

        void markForOutput (CProfileInfo *info);

        QString removeTemplates (const QString& name);

        QString processName;
        CConfigure* mColorConfigure;
};

#endif
