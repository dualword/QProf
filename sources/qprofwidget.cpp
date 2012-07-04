/*
 * qprofwidget.cpp
 *
 *
 * $Id: ported from kprofwidget.cpp,v 1.56 2004/07/03 06:03:50 bab Exp $
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

#include <cstdlib>
#include <cstring>
#include <assert.h>

#include <QButtonGroup>
#include <QUuid>
#include <QPrintDialog>
#include <QTextStream>
#include <QHBoxLayout>
#include <QFrame>
#include <QVBoxLayout>
#include <QFontDialog>
#include <QEventTransition>
#include <QLayout>
#include <QMessageBox>
#include <QRadioButton>
#include <QPrinter>
#include <QVector>
#include <QLabel>
#include <QDebug>
#include <QWhatsThis>
#include <QFile>
#include <QDir>

#include <QApplication>
#include <QAction>
#include <QActionGroup>
#include <QSettings>
#include <QLocale>
#include <QToolButton>
#include <QMouseEvent>
#include <QMenu>
#include <QFileDialog>
#include <QIcon>
#include <QTextBrowser>
#include <QUrl>

// icons
#include "./resource/lo32-app-qprof.xpm"

#include "./includes/constants.h"
#include "./includes/qprofwidget.h"
#include "./includes/cprofileviewitem.h"
#include "./includes/ctidyup.h"
// #include "./includes/popup_menu.h"

#include "./includes/dotCallGraph.h"
// #include "./includes/vcgCallGraph.h"
#include "./includes/aboutform.h"
#include "./includes/clientsidemap.h"
#include "./includes/parseprofile_gprof.h"
#include "./includes/parseprofile_fnccheck.h"
#include "./includes/parseprofile_pose.h"
#include "./includes/qproffile.h"

#include "./includes/Log.h"

#ifdef HAVE_LIBQTREEMAP
#include <qtreemap.h>
#include <qtreemapwindow.h>
#include <qlistviewtreemap.h>
#include <qlistviewtreemapwindow.h>
#endif




/*
 * Some globals we need - one of these days I'll have to synthesize this
 * in a config class or something...
 *
 */

short QProfWidget::sLastFileFormat = FORMAT_GPROF;
bool  QProfWidget::sDiffMode = false;


QProfWidget::QProfWidget (QWidget* parent, Qt::WindowFlags flags)
    :   QMainWindow (parent, flags)
{

    setupUi ( this );

    qsrand( time (NULL));
    int randomZahl = qrand();
    sLastFileFormat = FORMAT_GPROF;

    mAbbrevTemplates = false;

    initColFields();

    processName = "/tmp/QProf_" + QString::number(randomZahl) + "/";
    QDir().mkdir(processName);

    recentGroup = new QActionGroup(this);

    connect (recentGroup, SIGNAL (triggered(QAction*)), this, SLOT (openRecentFile(QAction*)));

    setWindowIcon ( QIcon(appIcon));

    prepareProfileView (mFlat, false, sLastFileFormat);
    actionOpen->setIcon(QIcon::fromTheme("document-open"));
    actionCompare->setIcon(QIcon::fromTheme("edit-copy"));
    actionPrint->setIcon(QIcon::fromTheme("printer"));

    actionQuit->setIcon(QIcon::fromTheme("application-exit"));
    actionOpen_Recent->setIcon(QIcon::fromTheme("document-open-recent"));

    connect (actionOpen, SIGNAL (triggered ()), this, SLOT (openResultsFile ()));
    connect (actionQuit, SIGNAL (triggered ()), this, SLOT (quit ()));
//     connect (actionOpen_Recent, SIGNAL (triggered()), this, SLOT (openRecentFile()));

//      (*actionCompare).setDisabled(true);
    connect (actionCompare, SIGNAL (triggered ()), this, SLOT (compareFile ()));

    connect (action_Generate_Call_Graph, SIGNAL (triggered ()), this, SLOT (generateCallGraph ()));

#ifndef HAVE_LIBQTREEMAP
    (*action_Display_TreeMap_View).setDisabled(true);
#else
    connect (action_Display_TreeMap_View, SIGNAL (triggered ()), this, SLOT (displayTreeMapView ()));
#endif

    connect (actionAbbreviate_C_Templates, SIGNAL (toggled(bool)), this, SLOT (toggleTemplateAbbrev (bool)));
    connect (actionSelect_Font, SIGNAL (triggered ()), this, SLOT (selectListFont ()));
    connect (action_Configure_KProf, SIGNAL (triggered ()), this, SLOT (configure ()));
//     actionAbout_programm->setIcon(QIcon::fromTheme("about"));
    connect (actionAbout_programm, SIGNAL (triggered ()), this, SLOT (about ()));
//     actionAbout_Qt->setIcon(QIcon::fromTheme("about-qt"));
    connect (actionAbout_Qt, SIGNAL (triggered ()), this, SLOT (aboutQt ()));


    mFlat->setContextMenuPolicy(Qt::CustomContextMenu);
    connect (mFlat, SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (profileEntryRightClick(const  QPoint&)));

    prepareProfileView (mHier, true, sLastFileFormat);
    mFlat->setContextMenuPolicy(Qt::CustomContextMenu);
    connect (mHier, SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (profileEntryRightClick(const  QPoint&)));

    prepareProfileView (mObjs, true, sLastFileFormat);
    mObjs->setContextMenuPolicy(Qt::CustomContextMenu);
    connect (mObjs, SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (profileEntryRightClick(const  QPoint&)));

    actionAbbreviate_C_Templates->setCheckable(true);

    // add some help on items
//     QWhatsThis::add (flatFilter,
//                      tr (  "Type text in this field to filter the display "
//                            "and only show the functions/methods whose name match the text."));

    mFlat->setWhatsThis(tr ("This is the <I>flat view</I>.\n\n"
                            "It displays all functions and method 'flat'. Click on a column header "
                            "to sort the list on this column (click a second time to reverse the "
                            "sort order)."));
    mHier->setWhatsThis(tr ("This is the <I>hierarchical view</I>.\n\n"
                            "It displays each function/method like a tree to let you see the other "
                            "functions that it calls. For that reason, each function may appear several "
                            "times in the list."));
    mObjs->setWhatsThis(tr ("This is the <I>object view</I>.\n\n"
                            "It displays C++ methods grouped by object name."));
    mCallTree->setWhatsThis(tr ("This is a graphical representation of the call tree. Click on methods"
                                "to display them on the methods tab."));
    mMethod->setWhatsThis(tr ("This is a clickable view of the method selected in the Graph view"));

    createToolBars();

    mColorConfigure = new CConfigure(this);

    openCommandLineFiles();

    loadSettings ();
}


void QProfWidget::quit()
{
    applySettings();
    qApp->quit();
}


QProfWidget::~QProfWidget ()
{
    CTidyUp* tidy = new CTidyUp(processName);
    tidy->removeDir();
    applySettings ();
}

void QProfWidget::aboutQt()
{
    QMessageBox::aboutQt ( this, PROGRAM_NAME );
}


void QProfWidget::about()
{
    aboutForm aboutProgram;
    aboutProgram.exec();
}


void QProfWidget::toggleTemplateAbbrev (bool stste)
{
    mAbbrevTemplates = stste;// mAbbrevTemplates ? false : true;
    actionAbbreviate_C_Templates->setChecked (mAbbrevTemplates);

    if (mProfile.count ()) {
        postProcessProfile (false); // regenerate simplified names
        mFlat->clear ();        // rebuild lists to make sure refresh is done
        mHier->clear ();
        mObjs->clear ();

        fillFlatProfileList ();
        fillObjsProfileList ();
        fillHierProfileList ();
    }
}



void QProfWidget::selectListFont ()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QFont("Times", 16), this);

    if (ok) {
        mFlat->setFont (font);
        mHier->setFont (font);
        mObjs->setFont (font);

        sListFont = font;// Font sListFont;
        // font is set to the font the user selected
//         settingsChanged();
    }
}

void QProfWidget::configure()
{
    mColorConfigure->chooseGraphHighColour();
}


void QProfWidget::prepareProfileView (QTreeWidget *view, bool rootIsDecorated, short profiler)
{
    QStringList* vPtr;
    view->clear();

    view->setColumnCount(0);

    switch (profiler) {
        case FORMAT_GPROF:

            if (sDiffMode) {
                vPtr = &prof_diff_columns;
            } else {
                vPtr = &prof_columns;
            }

            break;

        case FORMAT_FNCCHECK:

            if (sDiffMode) {
                vPtr = &func_diff_columns;
            } else {
                vPtr = &func_columns;
            }

            break;

        case FORMAT_POSE:

            if (sDiffMode) {
                vPtr = &pose_diff_columns;
            } else {
                vPtr = &pose_columns;
            }

            break;
    }

    view->setHeaderLabels(*vPtr);
    view->setColumnCount((*vPtr).count());

    view->setAllColumnsShowFocus (true);
    view->setSortingEnabled(true);
    view->setRootIsDecorated (rootIsDecorated);

    view->setColumnWidth (0, 300); // column 0
    view->setColumnWidth (1, 25);  // column 1

    for (int i = 2; i < (*vPtr).count(); i++) {
        view->resizeColumnToContents(i);
    }
}


void QProfWidget::settingsChanged ()
{
    applySettings ();
//     loadSettings ();
}

void QProfWidget::applySettings ()
{
    QSettings settings("KarboSoft", "QProfiler");

    settings.setValue ("AbbreviateTemplates", mAbbrevTemplates);
    settings.setValue  ("FontName", sListFont.family());
    settings.setValue  ("FontSize", sListFont.pointSize());
    settings.setValue  ("LastFileFormat", sLastFileFormat);
    settings.setValue ( "pos", pos() );
    settings.setValue ( "size", size() );

    settings.beginWriteArray("RecentFiles");

    for (int i = 0; (i < recentList.size()) && (i < 10); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("File", recentList.at(i));
    }

    settings.endArray();
}

void QProfWidget::loadSettings ()
{
    QSettings settings("KarboSoft", "QProfiler");

    //Load settings for C++ Templates
    mAbbrevTemplates = settings.value ("AbbreviateTemplates", true).toBool();
    actionAbbreviate_C_Templates->setChecked (mAbbrevTemplates);

    //Load old Font-Settings
    QString FontName = settings.value ("FontName", "").toString();
    sListFont = QFont(FontName);
    int     FontSize = settings.value ("FontSize", 0).toInt();

    if (!FontName.isEmpty() && FontSize > 0) {
//         qDebug("stored Font was %s", FontName.data());
        sListFont.setFamily(FontName);
        sListFont.setPointSize(FontSize);
    } else {
//         qDebug("set Font to menuFont=%s", QApplication::font().toString().data());
        sListFont = QApplication::font();
    }

    QPoint pos = settings.value ( "pos", QPoint ( 200, 200 ) ).toPoint();
    QSize size = settings.value ( "size", QSize ( 800, 400 ) ).toSize();
    resize ( size );
    move ( pos );

    mFlat->setFont(sListFont);
    mHier->setFont(sListFont);
    mObjs->setFont(sListFont);

    //Load last FileFormat settings
    sLastFileFormat = (short int)settings.value ("LastFileFormat", 1).toInt();
//  qDebug("LastFileFormat was=%d",sLastFileFormat);

    // loop for the recent file list

    int len = settings.beginReadArray("RecentFiles");

//     recMenu = actionOpen_Recent->addMenu(tr("&Recent files"));

    disconnect (recentGroup, SIGNAL (triggered(QAction*)), this, SLOT (openRecentFile(QAction*)));
    actRecentSelect.clear();

    if (len == 0) {
        return;
    }

    recentMenu = new QMenu();

    delete recentGroup;
    recentGroup = new QActionGroup(this);

    for (int i = 0; i < len; ++i) {
        settings.setArrayIndex(i);
        QString name;

        name = settings.value("File").toString();
        QAction *tmpAction = new QAction(name, recentGroup);
        recentGroup->addAction(tmpAction);
        recentMenu->addAction(tmpAction);

        actRecentSelect.push_back(tmpAction);
        recentList << name;
    }

    actionOpen_Recent->setMenu(recentMenu);

    connect (recentGroup, SIGNAL (triggered(QAction*)), this, SLOT (openRecentFile(QAction*)));
    settings.endArray();
}


void QProfWidget::openRecentFile (QAction* act)
{
    QUrl* url;
    QString tmpStr;

    tmpStr = act->text();

    url = new QUrl(tmpStr);

    QString filename = url->path ();
    QString protocol = url->scheme ();

    if (protocol == "file-gprof") {
        openFile (filename, FORMAT_GPROF, false);
    } else if (protocol == "file-fnccheck") {
        openFile (filename, FORMAT_FNCCHECK, false);
    } else if (protocol == "file-pose") {
        openFile (filename, FORMAT_POSE, false);
    } else {
        QMessageBox::warning (this, tr ("Unknown format selected"), tr ("Please select existing format"));
//         openFile (filename, (QProfWidget::short) - 1);
    }
}


void QProfWidget::compareFile ()
{
    // here we do not customize the file open dialog since the compared
    // file should be the same type
    //QWidget * parent = 0, const QString & caption = QString(), const QString & dir = QString(), const QString & filter = QString(), QString * selectedFilter = 0, Options options = 0
    QString f = QFileDialog::getOpenFileName (this,  tr ("Select a profiling results file to compare..."), mCurDir.absolutePath() );

    if (f.isEmpty ()) {
        return;
    }

    openFile (f, sLastFileFormat, true);
}


void QProfWidget::openResultsFile ()
{
    // customize the Open File dialog: we add
    // a few widgets at the end which allow the user
    // to give us a hint at which profiler the results
    // file comes from (GNU gprof, Function Check, Palm OS Emulator)
    QFileDialog fd (this, tr ("Select a profiling results file"), mCurDir.absolutePath());
    fd.setOption(QFileDialog::DontUseNativeDialog, true);


    QGridLayout* mainLayout = dynamic_cast<QGridLayout*>(fd.layout());
    assert(mainLayout->columnCount() == 3);
    assert(mainLayout->rowCount()    == 4);

    QWidget*     container      = new QWidget();
    QHBoxLayout* hlayout        = new QHBoxLayout();
    QLabel*      txtLabel = new QLabel("Text File Format:");

    container->setLayout(hlayout);
    hlayout->setAlignment(Qt::AlignLeft);

    QButtonGroup *bgroup = new QButtonGroup (&fd);
    bgroup->setExclusive(true);// setRadioButtonExclusive (true);
    QRadioButton *fmtGPROF = new QRadioButton (tr ("GNU gprof  "));
    bgroup->addButton(fmtGPROF);
    hlayout->addWidget(fmtGPROF);

    QRadioButton *fmtFNCCHECK = new QRadioButton (tr ("Function Check  "));
    bgroup->addButton(fmtFNCCHECK);
    hlayout->addWidget(fmtFNCCHECK);

    QRadioButton *fmtPOSE = new QRadioButton (tr ("Palm OS Emulator"));
    fmtPOSE->setDisabled(true);
    bgroup->addButton(fmtPOSE);
    hlayout->addWidget(fmtPOSE);

    // reset format button to last used format
    if (sLastFileFormat == FORMAT_GPROF && !fmtGPROF->isChecked()) {
        fmtGPROF->toggle ();
    } else if (sLastFileFormat == FORMAT_FNCCHECK && !fmtFNCCHECK->isChecked ()) {
        fmtFNCCHECK->toggle ();
    } else if (!fmtPOSE->isChecked ()) {
        fmtPOSE->toggle ();
    }

    hlayout->setContentsMargins(0, 0, 0, 0); // Removes unwanted spacing

    // Shifting relevant child widgets one row down.
    int rowCount = mainLayout->rowCount();
    QLayoutItem* x00 = mainLayout->itemAtPosition(mainLayout->rowCount() - 2, 0);
    QLayoutItem* x10 = mainLayout->itemAtPosition(mainLayout->rowCount() - 1, 0);
    QLayoutItem* x01 = mainLayout->itemAtPosition(mainLayout->rowCount() - 2, 1);
    QLayoutItem* x11 = mainLayout->itemAtPosition(mainLayout->rowCount() - 1, 1);
    QLayoutItem* x02 = mainLayout->itemAtPosition(mainLayout->rowCount() - 1, 2);
    assert(x00);
    assert(x01);
    assert(x10);
    assert(x11);
    assert(x02);

    mainLayout->addWidget(x00->widget(), rowCount - 1, 0, 1, 1);
    mainLayout->addWidget(x10->widget(), rowCount,   0, 1, 1);
    mainLayout->addWidget(x01->widget(), rowCount - 1, 1, 1, 1);
    mainLayout->addWidget(x11->widget(), rowCount,   1, 1, 1);
    mainLayout->addWidget(x02->widget(), rowCount - 1, 2, 2, 1);

    // Adding the widgets in the now empty row.
    rowCount        = mainLayout->rowCount();
    mainLayout->addWidget(txtLabel, rowCount - 3, 0, 1, 1 );
    mainLayout->addWidget(container,      rowCount - 3, 1, 1, 1);

    // Setting the proper tab-order
    QLayoutItem* tmp  = mainLayout->itemAtPosition(mainLayout->rowCount() - 2, 1);
    QLayoutItem* tmp2 = mainLayout->itemAtPosition(mainLayout->rowCount() - 1, 1);
    assert(tmp);
    assert(tmp2);

    fd.exec();

    if (fd.selectedFiles().count() != 1) {
        return;
    }

    QString filename = fd.selectedFiles().at(0);//.selectedFile();

    if (!filename.isEmpty()) {
        sLastFileFormat =   fmtGPROF->isChecked () ? FORMAT_GPROF :
                            fmtFNCCHECK->isChecked () ? FORMAT_FNCCHECK :
                            FORMAT_POSE;

        //print a debug message
        switch(sLastFileFormat) {
            case    FORMAT_GPROF:
                qDebug("Suppose Fileformat is \"GNU gprof\"");
                break;
            case    FORMAT_FNCCHECK:
                qDebug("Suppose Fileformat is \"Function Check\"");
                break;
            case    FORMAT_POSE:
                qDebug("Suppose Fileformat is \"PalmOS Emulator\"");
                break;
        }

        //open the file
        openFile (filename, sLastFileFormat, false);
    }
}

void QProfWidget::openCommandLineFiles ()
{
    QStringList args;
    args = QCoreApplication::arguments();

    if (args.size() <= 1 ) {
        return;
    }

    QString fileName = "";
    short prof = FORMAT_GPROF;
    bool argsToUse = parseArguments(args, fileName, prof);
    args.clear();

    //If the file name has been set in the command line arguments
    //then open the file with the chosen profiler.
    if (argsToUse && (fileName != "")) {
        openFile(fileName, prof, false);
    }
}


bool QProfWidget::parseArguments(const QStringList & args, QString& fileName, short& prof)
{
    bool success = false;

    for (int i = 1; i < args.count(); i++) {
        QString arg = args.at(i);

        if (arg == "-f") {
            fileName = args.at(++i);
            success = true;
        }

        if(arg == "-p") {
            QString profiler = args.at(++i);

            if (profiler == "gprof") {
                prof = FORMAT_GPROF;
                success = true;
            } else if (profiler == "fnccheck") {
                prof = FORMAT_FNCCHECK;
                success = true;
            } /*else if (profiler == "pose") {

                prof = FORMAT_POSE;
                success = true;
            } */else {
//                 usage();
                success = false;
            }
        }
    }

    return success;
}

void QProfWidget::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(actionOpen);
    fileToolBar->addAction(actionCompare);
    fileToolBar->addAction(actionPrint);

    filterToolBar = addToolBar(tr("Filter"));

    QLabel *lab =  new QLabel("Filter (flat profile): ");
    QLineEdit *flatFilter = new QLineEdit(filterToolBar);
    QSize sz = flatFilter->baseSize();
    flatFilter->setMaximumWidth(300);

    filterToolBar->addWidget(lab);
//     filterToolBar->addWidget (new QSpacer(10, 10));
    QAction *action = filterToolBar->addWidget(flatFilter);

    connect (flatFilter, SIGNAL (textChanged (const QString &)), this, SLOT (flatProfileFilterChanged (const QString &)));
}


void QProfWidget::openFile (const QString &filename, short format, bool compare)
{
    bool isExec = false;

    if (filename.isEmpty ()) {
        return;
    }

    // if the file is an executable file, generate the profiling information
    // directly from gprof and the gmon.out file
    QFileInfo finfo (filename);

    if (finfo.isExecutable ()) {
        isExec = true;

        // prepare the "gmon.out" filename
        QString outfile = finfo.dir().absolutePath() + "/gmon.out";
        QFileInfo gmonfinfo (outfile);

        if (!gmonfinfo.exists ()) {
            outfile = finfo.dir().absolutePath() + "/fnccheck.out";
            QFileInfo fnccheckinfo (outfile);

            if (!fnccheckinfo.exists ()) {
                QMessageBox::critical (this, tr ("File not found"), tr ("Can't find any profiling output file\n(gmon.out or fnccheck.out)"));

                return;
            } else {
                format = FORMAT_FNCCHECK;
            }
        } else {
            format = FORMAT_GPROF;
        }

        QStringList arguments;
        QString progName;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("LC_ALL", "C");
        gprofApplication.setProcessEnvironment(env);// .in; .setEnvironment("LC_ALL", "C");

        if (format == FORMAT_GPROF) {
            // GNU gprof analysis of gmon.out file
            // exec "gprof -b filename gmon.out"
            QStringList arguments;
            arguments << "-b" << filename << outfile;
            progName = "gprof";
            gprofApplication.setProcessEnvironment(env);
        } else {
            // Function Check analysis of fnccheck.out file
            // exec "fncdump +calls -no-decoration filename"
            progName = "fncdump";
            arguments << "+calls" << "-no-decoration" << "-sfile" << outfile << filename;
            gprofApplication.setProcessEnvironment(env);
        }

        mGProfStdout = "";
        mGProfStderr = "";
        connect (&gprofApplication, SIGNAL (receivedStdout ()), this, SLOT (gprofStdout ()));
        connect (&gprofApplication, SIGNAL (receivedStderr ()), this, SLOT (gprofStderr ()));

        gprofApplication.execute (progName, arguments);//// QProcess::Block, QProcess::AllOutput);

        if (!gprofApplication.exitCode() || gprofApplication.exitStatus ()) {
            QString text = tr ("Could not generate the profile data.\n");
            text += ("\nFile: " + filename);

            if (gprofApplication.exitCode () && gprofApplication.exitStatus ()) {
                QString s = QString(tr("Error ") + QString::number(gprofApplication.exitCode()) + tr( " was returned.\n") );
                text += s;
            }

            if (!mGProfStderr.isEmpty ()) {
                text += tr ("It returned the following error message(s):\n");
                text += mGProfStderr;
            } else if (!mGProfStdout.isEmpty ()) {
                text += tr ("Following output was displayed:\n");
                text += mGProfStdout;

            }

            QMessageBox::critical (this, tr ("gprof exited with error(s)"), text);
            return;
        }

        mFlat->clear ();
        mHier->clear ();
        mObjs->clear ();

        // if we are going to compare results, save the previous results and
        // remove any previously deleted entry
        sDiffMode = compare;

        if (compare) {
            mPreviousProfile = mProfile;

            for (uint i = mPreviousProfile.count(); i > 0; ) {
                if (mPreviousProfile[--i].deleted) {
                    mPreviousProfile.remove (i);
                }

                mPreviousProfile[i].previous = NULL;
            }
        }

        mProfile.clear ();

        // parse profile data
        QTextStream t (&mGProfStdout, QIODevice::ReadOnly);

        CParseProfile* profile = 0;

        if (format == FORMAT_GPROF) {
            profile = new CParseProfile_gprof(t, mProfile);
        } else {
            profile = new CParseProfile_fnccheck(t, mProfile);
        }

        if (!profile->valid()) {
            QMessageBox::critical(this, tr("Error"), tr("This fnccheck file is not in the correct format"));

            return;
        }
    } else {
        // if user tried to open gmon.out, have him select the executable instead, then recurse to use
        // the executable file
        if (finfo.fileName() == "gmon.out") {
            QMessageBox::critical (this, tr ("Opening gmon.out not allowed"), tr ("File 'gmon.out' is the result of the execution of an application\nwith gprof(1) profiling turned on.\nYou can not open it as such: either open the application itself\nor open a text results file generated with 'gprof -b application-name'"));

            return;
        } else if (finfo.fileName() == "fnccheck.out") {
            QMessageBox::critical (this, tr ("Opening fnccheck.out not allowed"), tr ("File 'fnccheck.out' is the result of the execution of an application\nwith Function Check profiling turned on.\nYou can not open it as such: either open the application itself\nor open a text results file generated with 'fncdump +calls application-name'"));

            return;
        }

        mFlat->clear ();
        mHier->clear ();
        mObjs->clear ();

        // if we are going to compare results, save the previous results and
        // remove any previously deleted entry
        sDiffMode = compare;

        if (compare) {
            qDebug() << "compare";
            mPreviousProfile = mProfile;

            for (uint i = mPreviousProfile.count(); i > 0; ) {
                if (mPreviousProfile[--i].deleted) {
                    mPreviousProfile.remove (i);
                }

                mPreviousProfile[i].previous = NULL;
            }
        }

        mProfile.clear ();

        // parse profile data
        QFile inpf (filename);

        if (!inpf.open (QIODevice::ReadOnly)) {
            return;
        }

        QTextStream t (&inpf);

        if (format == FORMAT_GPROF) {
            CParseProfile_gprof (t, mProfile);
        } else if (format == FORMAT_FNCCHECK) {
            CParseProfile_fnccheck (t, mProfile);
        } else {
            CParseProfile_pose (t, mProfile);
        }
    }

    // post-process the parsed data
    postProcessProfile (compare);

    prepareProfileView (mFlat, false, sLastFileFormat);
    prepareProfileView (mHier, true, sLastFileFormat);
    prepareProfileView (mObjs, true, sLastFileFormat);

    prepareHtmlPart(mCallTree);

    //For the time being dump all the method html
    //files to the tmp directory.
    for (unsigned int i = 0; i < mProfile.size (); i++) {
        (mProfile[i]).dumpHtml(processName);
    }

    // fill lists
    fillFlatProfileList ();
    fillHierProfileList ();
    fillObjsProfileList ();

    // make sure we add the recent file (this also changes the window title)
    QUrl url (filename);
    url.setScheme (isExec ? "file" : sLastFileFormat == FORMAT_GPROF ? "file-gprof" : sLastFileFormat == FORMAT_FNCCHECK ? "file-fnccheck" : "file-pose");
//     qDebug() << url;
    addRecentFile (url);

    // update the current directory
    mCurDir = finfo.dir ();
}


void QProfWidget::addRecentFile(const QUrl &url)
{
    if (recentList.indexOf(url.toString()) == -1) {
        recentList.push_front(url.toString());
    }
}

void QProfWidget::gprofStdout ()
{
//     gprofApplication.setReadChannel( QProcess::StandardOutput );
    mGProfStdout += gprofApplication.readAllStandardOutput();
}



void QProfWidget::gprofStderr ()
{
//     gprofApplication.setReadChannel( QProcess::StandardError );
    mGProfStderr += gprofApplication.readAllStandardOutput();
}

//Handle the standard output from the GraphViz command
void QProfWidget::graphVizStdout ()
{
//     graphApplication.setReadChannel( QProcess::StandardOutput );
    mGraphVizStdout += graphApplication.readAllStandardOutput();//String::fromLocal8Bit(buffer, buflen);
}

//Handle the standard errors from the GraphViz command
void QProfWidget::graphVizStderr ()
{
//     graphApplication.setReadChannel( QProcess::StandardError );
    mGraphVizStderr += graphApplication.readAllStandardError();//::fromLocal8Bit(buffer, buflen);
}

//Handle the standard output from the GraphViz command
void QProfWidget::graphVizDispStdout ()
{
//     displayApplication.setReadChannel( QProcess::StandardOutput );
    mGraphVizDispStdout += displayApplication.readAllStandardOutput();//::fromLocal8Bit(buffer, buflen);
}

//Handle the standard errors from the GraphViz command
void QProfWidget::graphVizDispStderr ()
{
//     displayApplication.setReadChannel( QProcess::StandardError);
    mGraphVizDispStderr += displayApplication.readAllStandardError();//::fromLocal8Bit(buffer, buflen);
}


void QProfWidget::postProcessProfile (bool compare)
{
    // once we have read a profile information file, we can post-process
    // the data. First, we need to create the list of classes that were
    // found.
    // After that, we check every function/method to see if it
    // has multiple signatures. We mark entries with multiple signatures
    // so that we can display the arguments only when needed

    mClasses.resize (0);
    uint i, j;

    for (i = 0; i < mProfile.count (); i++) {
        // fill the class list
        if (!mProfile[i].object.isEmpty ()) {
            uint k;

            for (k = 0; k < mClasses.count (); k++) {
                if (mClasses[k].compare (mProfile[i].object) == 0) {
                    break;
                }
            }

            if (k == mClasses.count ()) {
//                 mClasses.resize (mClasses.count () + 1);
                mClasses.append (mProfile[i].object);
            }
        }

        // check for multiple signatures
        for (j = i + 1; j < mProfile.count(); j++) {
            if (mProfile[i].multipleSignatures) {
                continue;
            }

            if (mProfile[j].multipleSignatures == false &&
                    mProfile[j].object == mProfile[i].object &&
                    mProfile[j].method == mProfile[i].method) {
                mProfile[i].multipleSignatures = true;
                mProfile[j].multipleSignatures = true;
            }
        }

        //construct the HTML name
        QString htmlName = mProfile[i].object;
        htmlName.replace(QRegExp("<"), "[");
        htmlName.replace(QRegExp(">"), "]");
        mProfile[i].htmlName = htmlName;


        // construct the function/method's simplified name
        if (mProfile[i].multipleSignatures) {
            mProfile[i].simplifiedName   = removeTemplates (mProfile[i].name);
        } else if (mProfile[i].object.isEmpty ()) {
            mProfile[i].simplifiedName = removeTemplates (mProfile[i].method);
        } else {
            mProfile[i].simplifiedName = removeTemplates (mProfile[i].object) + "::" + removeTemplates (mProfile[i].method);
        }
    }

    // profile results comparison: link new entry with previous entry, add deleted entries
    // to the list (marking them "deleted"). To mark entries that we have already seen,
    // set their "output" flag to true. This is temporary, just for the duration of
    // the code below.
    if (compare == false) {
        return;
    }

    for (i = 0; i < mPreviousProfile.count(); i++) {
        mPreviousProfile[i].output = false;    // reset all "output" flags
    }

    for (i = 0; i < mProfile.count(); i++) {
        for (j = 0; j < mPreviousProfile.count(); j++) {
            if (mPreviousProfile[j].output == false && mProfile[i].name == mPreviousProfile[j].name) {
                mProfile[i].previous = &mPreviousProfile[j];
                mPreviousProfile[j].output = true;
                break;
            }
        }
    }

    for (j = mPreviousProfile.count(); j > 0;) {
        if (mPreviousProfile[--j].output == false) {
            // this item was deleted, add it to the new list and mark it 'deleted'
            mPreviousProfile[j].deleted = true;
            mProfile.append ( mPreviousProfile[j]);
        }
    }
}


void QProfWidget::initColFields()
{
    comm_columns << col_function_t << col_recursive_t << col_count_t << col_total_t << col_totalPercent_t << col_self_t << col_totalMsPerCall_t;
    prof_columns << comm_columns << col_selfMsPerCall_t;
    func_columns << comm_columns << col_minMsPerCall_t << col_maxMsPerCall_t;
    pose_columns << comm_columns << col_selfCycles_t << col_cumCycles_t;

    comm_diff_columns << diff_col_function_t << diff_col_status_t << diff_col_count_t << diff_col_new_count_t << diff_col_total_t << diff_col_new_total_t
                      << diff_col_totalPercent_t << diff_col_new_totalPercent_t << diff_col_self_t << diff_col_new_self_t << diff_col_totalMsPerCall_t << diff_col_new_totalMsPerCall_t;
    prof_diff_columns << comm_diff_columns << diff_col_selfMsPerCall_t << diff_col_new_selfMsPerCall_t;
    func_diff_columns << comm_diff_columns << diff_col_minMsPerCall_t << diff_col_new_minMsPerCall_t << diff_col_maxMsPerCall_t;
    pose_diff_columns << comm_diff_columns << diff_col_selfCycles_t << diff_col_new_selfCycles_t << diff_col_cumCycles_t << diff_col_new_cumCycles_t;
}


void QProfWidget::hideFlatProfileList ()
{
    for(int i = 0; i < mFlat->topLevelItemCount(); ++i) {
        mFlat->topLevelItem(i)->setHidden(false);
    }

    if (mFlatFilter.length() == 0) {
        return;
    }

    for(int i = 0; i < mFlat->topLevelItemCount(); ++i) {
        if (mFlat->topLevelItem(i)->text(0).indexOf(QRegExp(mFlatFilter)) == -1) {
            mFlat->topLevelItem(i)->setHidden(true);
        } else {
            mFlat->topLevelItem(i)->setHidden(false);
        }
    }

    for (int i = 2; i < mFlat->columnCount(); i++) {
        mFlat->resizeColumnToContents(i);
    }
}


void QProfWidget::fillFlatProfileList ()
{
//     bool filter = ((mFlatFilter.isEmpty() == false) && (mFlatFilter.length() > 0));

    for (unsigned int i = 0; i < mProfile.size (); i++) {
//         if (filter && !mProfile[i].name.contains (mFlatFilter)) {
//             continue;
//         }

        CProfileViewItem *item =  new CProfileViewItem (mFlat, &mProfile[i]);
    }

    for (int i = 2; i < mFlat->columnCount(); i++) {
        mFlat->resizeColumnToContents(i);
    }
}


void QProfWidget::fillHierProfileList ()
{
    for (unsigned int i = 0; i < mProfile.size(); i++) {
        CProfileViewItem *item = new CProfileViewItem (mHier, &mProfile[i]);
        QVector<CProfileInfo *> addedEntries;// = new QVector<CProfileInfo *>[mProfile.size ()];

        if (&mProfile[i] == NULL) {
            break;
        }

        int count = 1;

        fillHierarchy (item, &mProfile[i], addedEntries, count);
    }

    for (int i = 2; i < mHier->columnCount(); i++) {
        mHier->resizeColumnToContents(i);
    }
}


void QProfWidget::fillHierarchy ( CProfileViewItem *item, CProfileInfo *parent, QVector<CProfileInfo *> &addedEntries, int &count)
{
    for (uint i = 0; i < parent->called.count (); i++) {
        // skip items already added to avoid recursion
        if (addedEntries.indexOf (parent->called[i]) != -1) {
            continue;
        }

        CProfileInfo * tmpProfile = new CProfileInfo();
        tmpProfile = parent->called[i];
        addedEntries.append(tmpProfile);
        CProfileViewItem *newItem = new CProfileViewItem (item, parent->called[i]);

        fillHierarchy (newItem, parent->called[i], addedEntries, count);
    }

    selectItemInView (item, parent, false);
}

void QProfWidget::fillObjsProfileList ()
{
    // create all toplevel elements and their descendants
    for (uint i = 0; i < mClasses.count (); i++) {
        CProfileViewItem *parent = new CProfileViewItem (mObjs, NULL);
        parent->setText(0, mClasses[i]);

        for (uint j = 0; j < mProfile.count (); j++) {
            if (mProfile[j].object == &mClasses[i]) {
                new CProfileViewItem (parent, &mProfile[j]);
            }
        }
    }

    for (int i = 2; i < mObjs->columnCount(); i++) {
        mObjs->resizeColumnToContents(i);
    }
}

void QProfWidget::profileEntryRightClick (const QPoint & iPoint)
{
    QTreeWidget * wd = (QTreeWidget *)sender();
    QTreeWidgetItem* listItem;

    if(wd->topLevelItemCount() < 1 ) {
        return;
    }

    listItem = wd->currentItem();

    if (!listItem) {
        return;
    }

    CProfileViewItem *item = (CProfileViewItem *) listItem;
    CProfileInfo *info = item->getProfile();

    if (info == NULL) {
        return;             // in objs profile, happens on class name lines
    }

    QVector<CProfileInfo *> itemProf;
    itemProf.resize (info->callers.count () + info->called.count ());
    uint n = 0;

    // if there are no callers nor called functions, return
    if (itemProf.size() == 0) {
        return;
    }

    QPoint globalPos = wd->mapToGlobal(iPoint);
    QWidget* popup = new QWidget(this, Qt::Popup);
    popup->move(globalPos);

    QVBoxLayout *layout = new QVBoxLayout;

    if (info->callers.count () > 0) {
        QLabel* lab1 = new QLabel("Called by:");
        lab1->setAlignment(Qt::AlignCenter);
        lab1->setFrameStyle(QFrame::Panel);
        layout->addWidget(lab1);

        for (uint i = 0; i < info->callers.count (); i++) {
            CProfileInfo *p = info->callers[i];
            QLabel *lab = new QLabel(p->name);
            layout->addWidget(lab);
            itemProf[n++] = p;
        }
    }

    if (info->called.count () > 0) {
        QLabel* lab2 = new QLabel("Calls:");
        lab2->setAlignment(Qt::AlignCenter);
        lab2->setFrameStyle(QFrame::Panel);
        layout->addWidget(lab2);

        for (uint i = 0; i < info->called.count (); i++) {
            CProfileInfo *p = info->called[i];

            QLabel *lab = new QLabel(p->name);
            layout->addWidget(lab);
            itemProf[n++] = p;
        }
    }

    popup->setLayout(layout);
    popup->show();
}

#if 0
// function for the popup window
void QProfWidget::selectProfileItem (CProfileInfo *info)
{
    // synchronize the three views by selecting the
    // same item in all lists
#if 0
    selectItemInView (mFlat, info, false);
    selectItemInView (mHier, info, false);
    selectItemInView (mObjs, info, true);
#endif
}
#endif

// to open the called subroutines in tree structure
void QProfWidget::selectItemInView (QTreeWidgetItem *item, CProfileInfo *info, bool examineSubs)
{
    QTreeWidget *view = item->treeWidget();

    while (item) {
        if (((CProfileViewItem *)item)->getProfile () == info) {

            view->clearSelection ();

            for (QTreeWidgetItem *parent = item->parent(); parent; parent = parent->parent()) {
                parent->setExpanded(true);
            }

            view->setItemHidden (item, false);
            view->setItemSelected (item, true);

            return;
        }

        QTreeWidgetItem *parent = item->parent();
        QTreeWidgetItem *nextSibling;

        if(parent) {
            nextSibling = parent->child(parent->indexOfChild(item) + 1);
        } else {
            QTreeWidget *treeWidget = item->treeWidget();
            nextSibling = treeWidget->topLevelItem(treeWidget->indexOfTopLevelItem(item) + 1);
        }

        if (examineSubs && item->child(0)) {
            item = item->child(0);
        } else if (nextSibling ) {
            item = nextSibling;
        } else if (examineSubs) {
            item = item->parent();

            if (item) {
                item = nextSibling;
            }
        } else {
            break;
        }
    }
}


void QProfWidget::flatProfileFilterChanged (const QString &filter)
{
//     mFlat->clear ();
    mFlatFilter = filter;
    hideFlatProfileList ();
}

void QProfWidget::doPrint ()
{
    int cur;
    QTreeWidget *view;

    cur = mTabs->currentIndex();

    switch (cur) {
        case 0:
            view = mFlat;
            break;
        case 1:
            view = mHier;
            break;
        default:
            view = mObjs;
            break;
    }

    QString s;
    s = "<HTML><HEAD><META http-equiv=\"content-type\" content=\"text/html\" charset=\"iso-8859-1\"></HEAD>";
    s += "<BODY bgcolor=\"#FFFFFF\">";
    s += "<TABLE border=\"0\" cellspacing=\"2\" cellpadding=\"1\">";

    QTreeWidgetItem *item = view->headerItem();
    int cols = view->columnCount();
    int row = 0;

    s += "<BR><BR><THEAD><TR>";     // two BRs to alleviate for margin problems

    for (int i = 0; i < cols; i++)
        if (i == 0) {
            s += "<TH align=\"left\"><B>" + item->text(i) + "</B></TH>";
        } else {
            s += "<TH align=\"right\"><B>" + item->text(i) + "</B></TH>";
        }

    s += "</TR></THEAD><TBODY>";

    while (item) {
        CProfileViewItem *pitem = (CProfileViewItem *) item;
        QString sitem = "<TR valign=\"top\">";

        for (int i = 0; i < cols; i++)
            if (i == 0) {
                sitem += "<TD align=\"left\">" + pitem->text(i) + "</TD>";
            } else {
                sitem += "<TD align=\"right\">" + pitem->text(i) + "</TD>";
            }

        sitem += "</TR>";
        s += sitem;
        item = view->itemBelow(item);//->nextSibling();
    }

    s += "</TBODY></TABLE></BODY></HTML>";

    QPrinter printer;

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));

    if (dialog->exec() != QDialog::Accepted) {
        return;
    }

    QTextDocument* part = new QTextDocument();
    part->setHtml(s);
    part->print(&printer);

    delete part;
}


void QProfWidget::generateCallGraph ()
{
    int cur;
    cur = mTabs->currentIndex();
    // Display the call-graph format selection dialog and generate a
    // call graph
    CCallGraph* dialog = new CCallGraph();
//     setupUi(dialog);

    if (dialog->exec ()) {
        bool currentSelectionOnly = dialog->mSelectedFunction->isChecked ();

        QTreeWidgetItem *selectedItem = NULL;

        if (currentSelectionOnly) {
            selectedItem = (cur == 0 ? mFlat->currentItem() :
                            cur == 1 ? mHier->currentItem() :
                            mObjs->currentItem());

            if (selectedItem == NULL) {
                QMessageBox::critical (this, tr ("Selection Empty"), tr ("To export the current selection's call-graph,\nyou must select an item in the profile view.") );
                return;
            }
        }

        if (dialog->mSaveFile->isChecked() ) {
            QString dotfile = QFileDialog::getSaveFileName ( this,  tr ("Save call-graph as..."), tr("*.dot|GraphViz files"));

            if (dotfile.isEmpty ()) {
                return;
            }

            QFile file (dotfile);

            if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                QMessageBox::critical (this, tr ("File Error"), tr ("File could not be opened for writing."));

                return;
            }

            if (currentSelectionOnly) {
                for (uint i = 0; i < mProfile.count(); i++) {
                    mProfile[i].output = false;
                }

                CProfileInfo *info = ((CProfileViewItem *) selectedItem)->getProfile ();

                if (info == NULL) {
                    // probably a parent item in the object profile view;
                    // in this case, mark all objects of the same class for output
                    QTreeWidgetItem *childItem = selectedItem->child (0);

                    if (childItem) {
                        info = ((CProfileViewItem *) childItem)->getProfile ();
                    }

                    if (info == NULL) {
                        QMessageBox::critical (this, tr ("Internal Error"), tr ("Could not find any function or class to export."));

                        return;
                    }

                    QString className = info->object;

                    for (uint i = 0; i < mProfile.count(); i++) {
                        if (mProfile[i].output == false && mProfile[i].object == info->object) {
                            markForOutput (&mProfile[i]);
                        }
                    }
                } else {
                    markForOutput (info);
                }
            }

            // graph generation
            DotCallGraph dotCallGraph(file, currentSelectionOnly, false, mProfile, processName, mColorConfigure->highColour());

            file.close ();
        } else {
            //Generate a temporary file
            QFile file;

            file.setFileName(".graphViz_temp");

            file.open(QIODevice::ReadWrite);

            if (!file.exists()) {
                QMessageBox::critical (this, tr ("Internal Error"), tr ("Could not open a temporary file for writing."));
                return;
            }

            // graph generation
            DotCallGraph dotCallGraph(file, currentSelectionOnly, true, mProfile, processName, mColorConfigure->highColour());

            file.close ();

            QString graphApplicationName = "";
            QStringList graphParams;

            graphApplicationName = "dot";
            graphParams << file.fileName() << "-Timap" ;
//                 displayApplication << "dot" << file.fileName() << "-Tjpg" << "-o" << ".graphViz.jpg";
            graphApplication.setProcessChannelMode(QProcess::MergedChannels);

            connect (&graphApplication, SIGNAL (readyReadStandardOutput ()), this, SLOT (graphVizStdout ()));
            connect (&graphApplication, SIGNAL (readyReadStandardError()), this, SLOT (graphVizStderr ()));

            graphApplication.execute(graphApplicationName, graphParams);
            graphParams.clear();
            graphParams << file.fileName() << "-Tjpg" << "-o" << ".graphViz.jpg";
            displayApplication.setProcessChannelMode(QProcess::MergedChannels);
            displayApplication.execute(graphApplicationName, graphParams);

            QFile mapFile;
            QTextStream text (&mGraphVizStdout, QIODevice::ReadOnly);

            mapFile.setFileName("./.qprof.html");
            mapFile.open(QIODevice::ReadWrite);
            ClientSideMap(text, mapFile, processName);

            if (graphApplication.exitCode () || graphApplication.exitStatus ()) {
                QString text = graphApplicationName + tr (" could not display the data.\n");
                text += ("\nFile: " + file.fileName());

                if (graphApplication.exitCode () && graphApplication.exitStatus ()) {
                    QString s = QString(tr ("Error") + QString::number(graphApplication.exitStatus()) + tr(" was returned.\n"));
                    text += s;
                }

                QMessageBox::critical (this, tr ("Exited with error(s)"), text );
                return;
            }
        }
    }
}

void QProfWidget::markForOutput (CProfileInfo *p)
{
    // if true, we already passed this item; avoid entering a recursive loop
    if (p->output) {
        return;
    }

    p->output = true;

    for (uint i = 0; i < p->called.count(); i++) {
        markForOutput (p->called[i]);
    }
}

QString QProfWidget::getClassName (const QString &name)
{
    // extract the class name from a complete method prototype
    int args = name.indexOf ('(');

    if (args != -1) {
        // remove extra spaces before '(' (should not happen more than once..)
        while (--args > 0 && (name[args] == ' ' || name[args] == '\t')) {
            ;
        }

        if (args <= 0) {

            return QString ("");
        }

        // if there is a function template as member, make sure we
        // properly skip the template
        if (name[args] == '>') {
            int depth = 1;

            while (--args > 0 && depth) {
                if (name[args] == '>') {
                    depth++;
                } else if (name[args] == '<') {
                    depth--;
                }
            }

            // remove extra spaces before '(' (should not happen more than once..)
            while (--args > 0 && (name[args] == ' ' || name[args] == '\t')) {
                ;
            }

            if (args <= 0) {

                return QString ("");
            }
        }

        while (args > 0) {
            if (name[args] == '>') {
                // end of another template: this is definitely
                // not a class name

                return "";
            }

            if (name[args] == ':' && name[args - 1] == ':') {
                args--;
                break;
            }

            args--;
        }

        if (args <= 0) {

            return QString ("");
        }

        // TODO: remove return type by analyzing the class name token

        return name.left (args);
    }

    return QString ("");
}

QString QProfWidget::removeTemplates (const QString &name)
{
    if (mAbbrevTemplates == false) {
        return name;
    }

    // remove the templates from inside a name, leaving only
    // the <...> and return the converted name
    QString s (name);
    int tmpl = -1;
    int depth = 0;

    for (uint i = 0; i < s.length(); i++) {
        if (s[i] == '<') {
            if (depth++ == 0) {
                tmpl = i;
            }
        } else if (s[i] == '>') {
            if (depth == 0) {
                continue;
            }

            if (--depth == 0) {
                s = s.replace (tmpl + 1, i - tmpl - 1, "...");
                i = tmpl + 4;
                tmpl = -1;
            }
        }
    }

    return s;
}

/** If the QTreeMapView library is present, display the call tree in this format */
void QProfWidget::displayTreeMapView()
{
#ifdef HAVE_LIBQTREEMAP

    // create options for the treemap
    mTreemapOptions = new QTreeMapOptions ();
    mTreemapOptions->calc_nodesize = CALCNODE_ALWAYS;

    // open up the treemap widget
    mObjTreemap = new QTreeWidgetTreeMapWindow (QProfWidget::col_function, QProfWidget::col_totalPercent);
    mObjTreemap->makeWidgets ();
    mObjTreemap->makeColumnMenu (mObjs);
    mObjTreemap->getArea()->setOptions (mTreemapOptions);
    mObjTreemap->getArea()->setTreeMap ((Object *)mObjs->firstChild ());
    mObjTreemap->setWindowTitle (tr ("QProf Object Profile"));

    mHierTreemap = new QTreeWidgetTreeMapWindow (QProfWidget::col_function, QProfWidget::col_totalPercent);
    mHierTreemap->makeWidgets ();
    mHierTreemap->makeColumnMenu (mHier);
    mHierTreemap->getArea()->setOptions (mTreemapOptions);
    mHierTreemap->getArea()->setTreeMap ((Object *)mHier->firstChild());
    mHierTreemap->setWindowTitle (tr ("QProf Hierarchy Profile"));

#endif
}

//This method takes the call graph and turns it into a client side image map
//in the html viewer
void QProfWidget::prepareHtmlPart(QTextBrowser* part)
{
    QFile* file = new QFile();

    file->setFileName(processName + "graphViz_temp");

    if (file->exists()) {
        file->remove();
    }

    file->open(QIODevice::WriteOnly | QIODevice::Text);

    DotCallGraph dotCallGraph(*file, true, true, mProfile, processName, mColorConfigure->highColour());
    file->close ();


    QString  graphAppName, dispAppName;
    QStringList graphAppParams, dispAppParams;

    graphAppName = "dot";
    graphAppParams << file->fileName() << "-Timap" ;
    QString fileName(processName + "graphViz.jpg");
    QFile::remove(fileName);

    dispAppName = "dot";
    dispAppParams << file->fileName() << "-Tjpg" << "-o" << fileName;

    graphApplication.setProcessChannelMode(QProcess::MergedChannels);
    displayApplication.setProcessChannelMode(QProcess::MergedChannels);

    connect (&graphApplication, SIGNAL (readyReadStandardOutput ()), this, SLOT (graphVizStdout ()));
    connect (&graphApplication, SIGNAL (readyReadStandardError ()), this, SLOT (graphVizStderr ()));

    connect (&displayApplication, SIGNAL (readyReadStandardOutput ()), this, SLOT (graphVizDispStdout ()));
    connect (&displayApplication, SIGNAL (readyReadStandardError ()), this, SLOT (graphVizDispStderr ()));

    graphApplication.execute(graphAppName, graphAppParams);
    displayApplication.execute(dispAppName, dispAppParams);

    if (graphApplication.exitCode () || graphApplication.exitStatus ()) {
        QString text = tr ("dot could not display the data (1).\n");
        text += ("File: " + file->fileName());

        if (graphApplication.exitCode () && graphApplication.exitStatus ()) {

            QString s = QString(tr ("Error") + QString::number(graphApplication.exitStatus()) + tr(" was returned.\n"));
            text += s;
            text += mGraphVizDispStderr;
        }

        QMessageBox::critical (this, tr ("Exited with error(s)"), text );
        return;
    }

    if (displayApplication.exitCode () || displayApplication.exitStatus ()) {
        QString text = tr (" dot could not display the data (2).\n");
        text += ("File: " + file->fileName());

        if (displayApplication.exitCode () && displayApplication.exitStatus ()) {
            QString s = QString(tr ("Error") + QString::number(displayApplication.exitStatus()) + tr(" was returned.\n"));
            text += s;
        }

        QMessageBox::critical (this, tr ("Exited with error(s)"), text);
        return;
    }

    file->close();
    //delete file;
    file = NULL;

    if (QFile::exists(processName + "graphViz.jpg") == false) {
        part->setHtml("<HTML><BODY>\nApplication <B>dot</B> is not installed!<BR>Please install the package <B>graphviz</B></BODY></HTML>\n");
        return;
    }


    QFile mapFile;
    qDebug() << "mgraphviz" << mGraphVizStdout.length();
    QTextStream text (&mGraphVizStdout, QIODevice::ReadOnly);
    mapFile.setFileName(processName + "qprof.html");

    if(mapFile.exists()) {
        mapFile.remove();
    }

    mapFile.open(QIODevice::ReadWrite);

    ClientSideMap(text, mapFile, processName);

    mapFile.close();
    part->setSource(QUrl("file://" + processName + "qprof.html"));
}


