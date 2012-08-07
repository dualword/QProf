/*
 * qprofwidget.h
 *
 * Copyright (c) 2012 Eduard Kalinowski <karbofos@ymail.com>
 * Copyright (c) 2000-2001 Florent Pillet <fpillet@users.sourceforge.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
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

#include <QtGui>
#include <QVector>
#include <QList>
#include <QString>
#include <QToolButton>
#include <QLineEdit>
#include <QPrinter>
#include <QActionGroup>
#include <QPaintEvent>
#include <QTextBrowser>
#include <QTextStream>
#include <QDir>
#include <QtGui/QApplication>
#include <QToolBar>
#include <QTreeWidget>
#include <QTabWidget>
#include <QUrl>
#include <QProcess>

#include "axisbase.h"
#include "plotterbase.h"
#include "barchartplotter.h"

#include "cprofileinfo.h"
#include "cconfigure.h"
#include "ui_mainForm.h"
#include "ui_call-graph.h"

#include <QFont>
#include <QFile>
class CProfileViewItem;

#define MAX_ROWS 50

#ifdef HAVE_LIBQTREEMAP
class QTreeWidgetTreeMapWindow;
class QTreeMapOptions;
#endif

#define FORMAT_GPROF       1            // GNU gprof
#define FORMAT_FNCCHECK    2            // Function Check
#define FORMAT_POSE        3            // PalmOS Emulator
#define FORMAT_CALLGRIND   4            // Callgrind
#define FORMAT_ELF         32           // linux elf format


class CCallGraph : public QDialog, public Ui_CCallGraph
{
    Q_OBJECT

public:
};

namespace Ui {
class QProfWidget;
}

class QProfWidget : public QMainWindow, public Ui_MainWindow
{
    Q_OBJECT

public:
    QFont            sListFont;  // font used to draw list entries
    static short     sLastFileFormat;    // format of the last opened file
//     static bool      sDiffMode;  // true if performing a diff. Used by CProfileViewItem

protected:
    QProcess graphApplication;
    QProcess displayApplication;
    QProcess gprofApplication;
    int selectedProfileNum;
    struct profInfo {
        QVector<CProfileInfo>   mProfile;   // profile information read from file
//         QVector<CProfileInfo>   mPreviousProfile;   // when comparing, keep previous profile information here
        QVector<QString>        mClasses;   // list of distinct class names found in the profile information
    };
    QVector<profInfo> pInfo; // for all opened files

    QStringList   comm_columns;
    QStringList   prof_columns;
    QStringList   func_columns;
    QStringList   pose_columns;
    QStringList   recentList;
    QActionGroup* recentGroup;
    QActionGroup* selectGroup;
    QMenu*        recentMenu;
    QVector<CProfileViewItem*> flatItems;
    QVector<CProfileViewItem*> hierItems;
    QVector<CProfileViewItem*> objItems;
//     QMenu*        selectMenu;
    QVector<QAction*> actRecentSelect;
    QVector<QAction*> actFileSelect;

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
    void additionalFiles ();

    void openDialogFile ();
    void openRecentFile (QAction* act);
    void selectFile (QAction* act);
    void openCommandLineFiles ();
    void doPrint ();

    void profileEntryRightClick (const QPoint & iPoint);
    void flatProfileFilterChanged (const QString &filter);
    void generateCallGraph ();
    void displayTreeMapView();
    void aboutQt();
    void changeDiagram ();
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
    void rebuildSelectGroup();
    int  fileDetection(const QString &fname);
    void openFileList (const QStringList &filename/*, bool compare*/);
    void openFile (const QString &filename/*, bool compare = false*/);
    void prepareProfileView (QTreeWidget *view, bool rootIsDecorated, short profiler);
    void postProcessProfile ();
    void prepareHtmlPart(QTextBrowser* part);
    bool parseArguments(const QStringList &args, QString& fileName);
    void addRecentFile (const QUrl&);
    void openURLRequestDelayed( const QUrl &url);
    void fillFlatProfileList ();
    void hideFlatProfileList ();
    void fillHierProfileList ();
    void fillHierarchy (CProfileViewItem *item, CProfileInfo *parent, QVector<CProfileInfo *> &addedEntries, int &count);
    void fillObjsProfileList ();
    void fillOverviewProfileList ();

//         void selectProfileItem (CProfileInfo *info);
    void selectItemInView (QTreeWidgetItem *view, CProfileInfo *info, bool examineSubs);

    void markForOutput (CProfileInfo *info);

    QString removeTemplates (const QString& name);

    QString processName;
    CConfigure* mColorConfigure;

private:
    QStringList filelist;
    bool percentDiag;
    QStandardItemModel *itemModel;

};

#endif
