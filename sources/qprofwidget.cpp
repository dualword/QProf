/*
 * qprofwidget.cpp
 *
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
 *
 *
 *
 *            Information about version changes. Developer of project E. Kalinowski
 *
 * 0.9.0      06 jul 2012    first release. complete ported from kprof project
 * 0.9.4      12 jul 2012    debug fixing, autodetection of file format: callgrind, gprof, funccheck
 * 1.0.0      06 aug 2012    overview page added, removed "compare" functionality, select of file.
 *                           loading of multoply files, autodetection of ELF format, if executable
 * 1.1.0      07 aug 2012    scaling for seconds enabled, overview page
 *                           ToolTip information on overview page for displaying of function/method names
 * 1.1.1      08 aug 2012    Q3Support removed completely grom call-graph.ui
 * 1.2.0      09 aug 2012    onDblClick added to select of file on overview page
 * 1.2.1      12 aug 2012    additional popup window for help about colorizing
 * 1.2.2      19 aug 2012    bugfixes callgrind
 *
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
#include <QPainter>
#include <QSettings>
#include <QLocale>
#include <QPushButton>
#include <QToolButton>
#include <QMouseEvent>
#include <QMenu>
#include <QFileDialog>
#include <QIcon>
#include <QTextBrowser>
#include <QUrl>

// icons
#include "./resource/lo32-app-qprof.h"

#include "./includes/constants.h"
#include "./includes/qprofwidget.h"
#include "./includes/cprofileviewitem.h"
#include "./includes/ctidyup.h"
// #include "./includes/popup_menu.h"

#include "./includes/dotCallGraph.h"
// #include "./includes/vcgCallGraph.h"
#include "./includes/aboutform.h"
#include "./includes/colorform.h"
#include "./includes/clientsidemap.h"
#include "./includes/parseprofile_gprof.h"
#include "./includes/parseprofile_fnccheck.h"
#include "./includes/parseprofile_callgrind.h"
#include "./includes/parseprofile_pose.h"
// #include "./includes/qproffile.h"

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


QProfWidget::QProfWidget (QWidget* parent, Qt::WindowFlags flags)
    :   QMainWindow (parent, flags)
{

    setupUi ( this );

    qsrand( time (NULL));
    int randomZahl = qrand();
    sLastFileFormat = FORMAT_GPROF;

    mAbbrevTemplates = false;

    itemModel = NULL;

    initColFields();

    processName = "/tmp/QProf_" + QString::number(randomZahl) + "/";
    QDir().mkdir(processName);

    recentGroup = new QActionGroup(this);

    connect (recentGroup, SIGNAL (triggered(QAction*)), this, SLOT (openRecentFile(QAction*)));

    selectGroup = new QActionGroup(this);

    connect (selectGroup, SIGNAL (triggered(QAction*)), this, SLOT (selectFile(QAction*)));

    setWindowIcon ( QIcon(appIcon));

    prepareProfileView (mFlat, false, sLastFileFormat);
    actionOpen->setIcon(QIcon::fromTheme("document-open"));
    actionAdditional->setIcon(QIcon::fromTheme("edit-copy"));
    actionPrint->setIcon(QIcon::fromTheme("printer"));

    selectedProfileNum = 0;
    pInfo.resize(0);

    percentDiag = true;

    actionQuit->setIcon(QIcon::fromTheme("application-exit"));
    actionOpen_Recent->setIcon(QIcon::fromTheme("document-open-recent"));

    connect (actionOpen, SIGNAL (triggered ()), this, SLOT (openDialogFile ()));
    connect (actionQuit, SIGNAL (triggered ()), this, SLOT (quit ()));
//     connect (actionOpen_Recent, SIGNAL (triggered()), this, SLOT (openRecentFile()));

//      (*actionCompare).setDisabled(true);
    connect (actionAdditional, SIGNAL (triggered ()), this, SLOT (additionalFiles ()));

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

    connect (mBarPlot, SIGNAL (selectName(const QString&)), this, SLOT (selectFileName(const QString&)));

    connect (radioButton, SIGNAL (clicked()), this, SLOT (changeDiagram ()));
    connect (pushColor, SIGNAL (clicked()), this, SLOT (colorCoding ()));
    connect (radioButton_2 , SIGNAL (clicked()), this, SLOT (changeDiagram ()));
//     connect(mCallTree, SIGNAL(openURLRequestDelayed( const QUrls &)), this, SLOT(openURLRequestDelayed( const QUrls &)));

//     connect(mMethod, SIGNAL(openURLRequestDelayed( const QUrls &)), this, SLOT(openURLRequestDelayed( const QUrls &)));

    mFlat->setContextMenuPolicy(Qt::CustomContextMenu);
    connect (mFlat, SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (profileEntryRightClick(const  QPoint&)));

    prepareProfileView (mHier, true, sLastFileFormat);
    mFlat->setContextMenuPolicy(Qt::CustomContextMenu);
    connect (mHier, SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (profileEntryRightClick(const  QPoint&)));

    prepareProfileView (mObjs, true, sLastFileFormat);
    mObjs->setContextMenuPolicy(Qt::CustomContextMenu);
    connect (mObjs, SIGNAL (customContextMenuRequested(const QPoint&)), this, SLOT (profileEntryRightClick(const  QPoint&)));

    actionAbbreviate_C_Templates->setCheckable(true);

    radioButton->setChecked(true);

    // add some help on items
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
    mBarPlot->setWhatsThis(tr ("This is a overview page of loaded files"));

    createToolBars();

    mColorConfigure = new CConfigure(this);

    openCommandLineFiles();

    loadSettings ();

    filelist.clear();

    fillOverviewProfileList ();
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

void QProfWidget::colorCoding()
{
    colorForm colorPopUp;
    colorPopUp.exec();
}

void QProfWidget::about()
{
    aboutForm aboutProgram;
    aboutProgram.exec();
}

void QProfWidget::changeDiagram()
{
    if (radioButton->isChecked() == true) {
        percentDiag = true;
    } else {
        percentDiag = false;
    }

//     qDebug() << "change" << percentDiag;
    fillOverviewProfileList ();
}


int QProfWidget::fileDetection(const QString &fname)
{
    QFile file;
    char ba[512];
    int num;
    int ret;

    file.setFileName(fname);
    ret = -1;

    if (file.open(QIODevice::ReadOnly) == false) {
        return ret;
    }

    QDataStream in(&file);    // read the data serialized from the file
    in.readRawData(ba, 512);
    file.close();

    // is it Executable Linux Format?
    if (ba[1] == 'E' && ba[2] == 'L' && ba[3] == 'F') {
        return FORMAT_ELF;
    }

    for (int i = 0; i < 128; i++) {
        if ( ba[i] == 0x0 || ba[i] > 0x7f) {
            return -2;
        }
    }

    if (file.open(QIODevice::ReadWrite) == false) {
        return ret;
    }

    // directly from gprof and the gmon.out file
    QFileInfo finfo (fname);

    if (finfo.isExecutable ()) {
        return ret;
    }

    QString line;
    QTextStream t (&file);

    num = 0;

    while (!t.atEnd()) {
        line = t.readLine(256);

        if (line.indexOf("callgrind") >= 0) {
            ret = FORMAT_CALLGRIND;
            break;
        }

        if (line.indexOf("Flat profile") >= 0) {
            ret = FORMAT_GPROF;
            break;
        }

        if ((line.indexOf("local") >= 0) && (line.indexOf("total") >= 0)) {
            ret = FORMAT_FNCCHECK;
            break;
        }

        num++;

        if (num > 10) {
            break;
        }
    }

    file.close();

    return ret;
}


void QProfWidget::toggleTemplateAbbrev (bool stste)
{
    mAbbrevTemplates = stste;// mAbbrevTemplates ? false : true;
    actionAbbreviate_C_Templates->setChecked (mAbbrevTemplates);

    if(pInfo.size() == 0) {
        return;
    }

    if (pInfo[selectedProfileNum].mProfile.count ()) {
        postProcessProfile (); // regenerate simplified names
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


//Captures the URL clicked signal from the call tree HTML widget and
//opens the required URL in the method widget.
void QProfWidget::openURLRequestDelayed( const QUrl &url)
{
    mMethod->setSource(url);
//     mMethod->openURL(url);
}

void QProfWidget::prepareProfileView (QTreeWidget *view, bool rootIsDecorated, short profiler)
{
    QStringList* vPtr;
    view->clear();

    view->setColumnCount(0);

    switch (profiler) {
        case FORMAT_GPROF:
            vPtr = &prof_columns;
            break;

        case FORMAT_FNCCHECK:
            vPtr = &func_columns;
            break;

        case FORMAT_POSE:
            vPtr = &pose_columns;
            break;

        case FORMAT_CALLGRIND:
            vPtr = &pose_columns;
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
    QSettings settings("KarboSoft", "QProf");

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
    QSettings settings("KarboSoft", "QProf");

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

    // loop for the recent file list
    int len = settings.beginReadArray("RecentFiles");

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

void QProfWidget::selectFileName(const QString& name)
{
//     qDebug() << name;

    for(int num = 0; num < actFileSelect.count(); ++num) {
//         qDebug() << (*actFileSelect.at(num)).text();
        if ((*actFileSelect.at(num)).text().indexOf(name) >= 0) {
            selectedProfileNum = num;
            actFileSelect.at(num)->setChecked (true);

            emit selectFile(selectGroup->actions().at(num));
            break;
        }
    }

    mTabs->setCurrentIndex(1); // next tab
}

void QProfWidget::selectFile (QAction* act)
{
    QString tmpStr;
    int num;
    tmpStr = act->text();

    disconnect (selectGroup, SIGNAL (triggered(QAction*)), this, SLOT (selectFile(QAction*)));

    selectedProfileNum = 0;

    for(num = 0; num < actFileSelect.count(); ++num) {
        if (actFileSelect.at(num)->isChecked() == true) {
            selectedProfileNum = num;
            break;
        }
    }

//     qDebug() << "triggered" << selectedProfileNum << num;
    mFlat->clear ();
    mHier->clear ();
    mObjs->clear ();

    // post-process the parsed data
    postProcessProfile ();

    prepareProfileView (mFlat, false, sLastFileFormat);
    prepareProfileView (mHier, true, sLastFileFormat);
    prepareProfileView (mObjs, true, sLastFileFormat);

#if 0
//     prepareHtmlPart(mCallTree);

    //For the time being dump all the method html
    //files to the tmp directory.
    for (unsigned int i = 0; i < pInfo[selectedProfileNum].mProfile.size (); i++) {
        (pInfo[selectedProfileNum].mProfile[i]).dumpHtml(processName);
    }

#endif

    // fill lists
    fillFlatProfileList ();
    fillHierProfileList ();
    fillObjsProfileList ();

    connect (selectGroup, SIGNAL (triggered(QAction*)), this, SLOT (selectFile(QAction*)));
}


void QProfWidget::openRecentFile (QAction* act)
{
    QUrl* url;
    QString tmpStr;

    tmpStr = act->text();

    url = new QUrl(tmpStr);

    QString filename = url->path ();
    QString protocol = url->scheme ();

    openFile (filename);
}



void QProfWidget::rebuildSelectGroup()
{
    disconnect (selectGroup, SIGNAL (triggered(QAction*)), this, SLOT (selectFile(QAction*)));
    actFileSelect.clear();

    menuSelect->clear();

    delete selectGroup;
    selectGroup = new QActionGroup(this);

    for (int i = 0; i < filelist.count(); ++i) {
        QString name;
        name = filelist.at(i);

        QAction *tmpAction = new QAction(name, selectGroup);
        selectGroup->addAction(tmpAction);
        tmpAction->setCheckable(true);
        menuSelect->addAction(tmpAction);

        actFileSelect.push_back(tmpAction);
        recentList << name;
    }

    actionOpen_Recent->setMenu(menuSelect);

    connect (selectGroup, SIGNAL (triggered(QAction*)), this, SLOT (selectFile(QAction*)));
}

void QProfWidget::openCommandLineFiles ()
{
    QStringList args;
    args = QCoreApplication::arguments();

    if (args.size() <= 1 ) {
        return;
    }

    QString fileName = "";
//     short prof = FORMAT_GPROF;
    bool argsToUse = parseArguments(args, fileName);
    args.clear();

    //If the file name has been set in the command line arguments
    //then open the file with the chosen profiler.
    if (argsToUse && (fileName != "")) {
        openFile(fileName);
    }
}


bool QProfWidget::parseArguments(const QStringList & args, QString& fileName)
{
    bool success = false;

    for (int i = 1; i < args.count(); i++) {
        QString arg = args.at(i);

        if (arg == "-f") {
            fileName = args.at(++i);
            success = true;
        }
    }

    return success;
}


void QProfWidget::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(actionOpen);
    fileToolBar->addAction(actionAdditional);
    fileToolBar->addAction(actionPrint);

    filterToolBar = addToolBar(tr("Filter"));

    QLabel *lab =  new QLabel("Filter (flat profile): ");
    QLineEdit *flatFilter = new QLineEdit(filterToolBar);
    QSize sz = flatFilter->baseSize();
    flatFilter->setMaximumWidth(300);
    flatFilter->setWhatsThis( tr (  "Type text in this field to filter the display "
                                    "and only show the functions/methods whose name match the text."));

    filterToolBar->addWidget(lab);

    QAction *action = filterToolBar->addWidget(flatFilter);

    connect (flatFilter, SIGNAL (textChanged (const QString &)), this, SLOT (flatProfileFilterChanged (const QString &)));
}


void QProfWidget::openDialogFile ()
{
    int addPos = 0;

    // customize the Open File dialog: we add
    // a few widgets at the end which allow the user
    // to give us a hint at which profiler the results
    // file comes from (GNU gprof, Function Check, Palm OS Emulator)
    if (filelist.count() > 0) {
        if (QMessageBox::question(this, "Clean", "Do you want to clean the existing list?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            selectedProfileNum = 0;
            filelist.clear();
            pInfo.resize(0);
        } else {
            addPos = pInfo.size();
        }
    }

    QFileDialog fd (this, tr ("Select a profiling results file(s)"), mCurDir.absolutePath());
    fd.setFileMode(QFileDialog::ExistingFiles);
    fd.setOption(QFileDialog::DontUseNativeDialog, true);

    fd.exec();
    QStringList fl;
    int cnt = fd.selectedFiles().count();

    if ( cnt >= 1) {
        fl = fd.selectedFiles();
        fl.sort();
        openFileList(fl);
    }

    rebuildSelectGroup();

    if (filelist.count() > 0) {
        selectedProfileNum = addPos;
        selectGroup->actions().at(addPos)->setChecked (true);

        emit selectFile(selectGroup->actions().at(addPos));
    }
}


void QProfWidget::additionalFiles ()
{
    QFileDialog fd (this, tr ("Select a profiling results file(s) for adding"), mCurDir.absolutePath());
    fd.setFileMode(QFileDialog::ExistingFiles);
    fd.setOption(QFileDialog::DontUseNativeDialog, true);

    fd.exec();
    QStringList fl = fd.selectedFiles();
    int pos = filelist.count();

    if (fl.count() > 0) {
        openFileList(fl);
        rebuildSelectGroup();

        selectedProfileNum = pos;
        selectGroup->actions().at(pos)->setChecked(true);

        emit selectFile(selectGroup->actions().at(pos));
    }
}


void QProfWidget::openFileList (const QStringList &filenames)
{
    int sz = filenames.count();

    for (int i = 0; i < sz; ++i) {
        selectedProfileNum = i;
        openFile (filenames[i]);
    }

    selectedProfileNum = 0;

    fillOverviewProfileList();
}

void QProfWidget::openFile (const QString &filename)
{
    bool isExec = false;
    int currentpInfoSize = pInfo.size();
    short format;

    if (filename.isEmpty ()) {
        return;
    }

    sLastFileFormat = fileDetection(filename);

    if (sLastFileFormat == -2) {
        QMessageBox::critical (this, tr ("File is binary"), tr ("Wrong output file format\nDetected binary information"));
        return;
    }

    if (sLastFileFormat == FORMAT_ELF) {
        QMessageBox::critical (this, tr ("File is ELF"), tr ("Wrong file was selected\nDetected ELF binary"));
        return;
    }

    if(filelist.indexOf(filename) != -1) { // exists
        return;
    }


    format = sLastFileFormat;

    // if the file is an executable file, generate the profiling information
    // directly from gprof and the gmon.out file
    QFileInfo finfo (filename);

    // if user tried to open gmon.out, have him select the executable instead, then recurse to use
    // the executable file
    if (format != FORMAT_GPROF && format != FORMAT_FNCCHECK && format != FORMAT_CALLGRIND ) {
        QMessageBox::critical (this, tr ("Opening of file not allowed"), tr ("This file is in binary format\nPlease convert the file in text format\n"));
        return;
    }

    mFlat->clear ();
    mHier->clear ();
    mObjs->clear ();

    // if we are going to compare results, save the previous results and
    // remove any previously deleted entry

    pInfo.resize(currentpInfoSize + 1);

    pInfo[currentpInfoSize].mProfile.clear ();

    // parse profile data
    QFile inpf (filename);

    if (!inpf.open (QIODevice::ReadOnly)) {
        return;
    }

    QTextStream t (&inpf);

    if (format == FORMAT_GPROF) {
        CParseProfile_gprof (t, pInfo[currentpInfoSize].mProfile);
    } else if (format == FORMAT_FNCCHECK) {
        CParseProfile_fnccheck (t, pInfo[currentpInfoSize].mProfile);
    } else if (format == FORMAT_CALLGRIND) {
        CParseProfile_callgrind (t, pInfo[currentpInfoSize].mProfile);
    } else {
        CParseProfile_pose (t, pInfo[currentpInfoSize].mProfile);
    }


    filelist << filename;

    // post-process the parsed data
    postProcessProfile ();

    prepareProfileView (mFlat, false, sLastFileFormat);
    prepareProfileView (mHier, true, sLastFileFormat);
    prepareProfileView (mObjs, true, sLastFileFormat);

    prepareHtmlPart(mCallTree);

    //For the time being dump all the method html
    //files to the tmp directory.
    for (unsigned int i = 0; i < pInfo[currentpInfoSize].mProfile.size (); i++) {
        (pInfo[currentpInfoSize].mProfile[i]).dumpHtml(processName);
    }

    // fill lists
    selectedProfileNum = currentpInfoSize;

    fillFlatProfileList ();
    fillHierProfileList ();
    fillObjsProfileList ();

    // make sure we add the recent file (this also changes the window title)
    QUrl url (filename);
    QString schem;

    if (isExec == true ) {
        schem = "file";
    } else if (sLastFileFormat == FORMAT_GPROF) {
        schem = "file-gprof";
    } else if (sLastFileFormat == FORMAT_FNCCHECK) {
        schem = "file-fnccheck";
    } else if (sLastFileFormat == FORMAT_CALLGRIND) {
        schem = "file-callgrind";
    } else {
        schem = "file-pose";
    }

    url.setScheme (schem);

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


void QProfWidget::postProcessProfile ()
{
    // once we have read a profile information file, we can post-process
    // the data. First, we need to create the list of classes that were
    // found.
    // After that, we check every function/method to see if it
    // has multiple signatures. We mark entries with multiple signatures
    // so that we can display the arguments only when needed
    if (pInfo.size() == 0) {
        return;
    }

    pInfo[selectedProfileNum].mClasses.resize (0);
    uint i, j;

    for (i = 0; i < pInfo[selectedProfileNum].mProfile.count (); i++) {
        // fill the class list
        if (!pInfo[selectedProfileNum].mProfile[i].object.isEmpty ()) {
            uint k;

            for (k = 0; k < pInfo[selectedProfileNum].mClasses.count (); k++) {
                if (pInfo[selectedProfileNum].mClasses[k].compare (pInfo[selectedProfileNum].mProfile[i].object) == 0) {
                    break;
                }
            }

            if (k == pInfo[selectedProfileNum].mClasses.count ()) {
//                 mClasses.resize (mClasses.count () + 1);
                pInfo[selectedProfileNum].mClasses.append (pInfo[selectedProfileNum].mProfile[i].object);
            }
        }

        // check for multiple signatures
        for (j = i + 1; j < pInfo[selectedProfileNum].mProfile.count(); j++) {
            if (pInfo[selectedProfileNum].mProfile[i].multipleSignatures) {
                continue;
            }

            if (pInfo[selectedProfileNum].mProfile[j].multipleSignatures == false &&
                    pInfo[selectedProfileNum].mProfile[j].object == pInfo[selectedProfileNum].mProfile[i].object &&
                    pInfo[selectedProfileNum].mProfile[j].method == pInfo[selectedProfileNum].mProfile[i].method) {
                pInfo[selectedProfileNum].mProfile[i].multipleSignatures = true;
                pInfo[selectedProfileNum].mProfile[j].multipleSignatures = true;
            }
        }

        //construct the HTML name
        QString htmlName = pInfo[selectedProfileNum].mProfile[i].object;
        htmlName.replace(QRegExp("<"), "[");
        htmlName.replace(QRegExp(">"), "]");
        pInfo[selectedProfileNum].mProfile[i].htmlName = htmlName;


        // construct the function/method's simplified name
        if (pInfo[selectedProfileNum].mProfile[i].multipleSignatures) {
            pInfo[selectedProfileNum].mProfile[i].simplifiedName = removeTemplates (pInfo[selectedProfileNum].mProfile[i].name);
        } else if (pInfo[selectedProfileNum].mProfile[i].object.isEmpty ()) {
            pInfo[selectedProfileNum].mProfile[i].simplifiedName = removeTemplates (pInfo[selectedProfileNum].mProfile[i].method);
        } else {
            pInfo[selectedProfileNum].mProfile[i].simplifiedName = removeTemplates (pInfo[selectedProfileNum].mProfile[i].object) + "::" + removeTemplates (pInfo[selectedProfileNum].mProfile[i].method);
        }
    }
}


void QProfWidget::initColFields()
{
    comm_columns << col_function_t << col_recursive_t << col_count_t << col_total_t << col_totalPercent_t << col_self_t << col_totalMsPerCall_t;
    prof_columns << comm_columns << col_selfMsPerCall_t;
    func_columns << comm_columns << col_minMsPerCall_t << col_maxMsPerCall_t;
    pose_columns << comm_columns << col_selfCycles_t << col_cumCycles_t;
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
    flatItems.clear();

    for (unsigned int i = 0; i < pInfo[selectedProfileNum].mProfile.size (); i++) {
//         if (filter && !mProfile[i].name.contains (mFlatFilter)) {
//             continue;
//         }

        CProfileViewItem *item =  new CProfileViewItem (mFlat, &pInfo[selectedProfileNum].mProfile[i]);
        flatItems.push_back(item);
    }

    for (int i = 2; i < mFlat->columnCount(); i++) {
        mFlat->resizeColumnToContents(i);
    }
}


void QProfWidget::fillHierProfileList ()
{
    hierItems.clear();

    for (unsigned int i = 0; i < pInfo[selectedProfileNum].mProfile.count(); i++) {
        CProfileViewItem *item = new CProfileViewItem (mHier, &pInfo[selectedProfileNum].mProfile[i]);
        hierItems.push_back(item);
        QVector<CProfileInfo *> addedEntries;// = new QVector<CProfileInfo *>[mProfile.size ()];

        if (&pInfo[selectedProfileNum].mProfile[i] == NULL) {
            break;
        }

        int count = 1;

        fillHierarchy (item, &pInfo[selectedProfileNum].mProfile[i], addedEntries, count);
    }

    for (int i = 2; i < mHier->columnCount(); i++) {
        mHier->resizeColumnToContents(i);
    }
}


void QProfWidget::fillHierarchy ( CProfileViewItem *item, CProfileInfo *parent, QVector<CProfileInfo *> &addedEntries, int &count)
{
    for (uint i = 0; i < parent->called.count(); ++i) {
        if (parent->called[i] == NULL) {
            break;
        }

//         qDebug() << parent->called[i]->name;

        // skip items already added to avoid recursion
        if (addedEntries.indexOf (parent->called[i]) != -1) {
            continue;
        }

        CProfileInfo * tmpProfile = new CProfileInfo();
        tmpProfile = parent->called[i];

        addedEntries.append(tmpProfile);
        CProfileViewItem *newItem = new CProfileViewItem (item, parent->called[i]);
        hierItems.push_back(newItem);

        fillHierarchy (newItem, parent->called[i], addedEntries, count);
    }

    selectItemInView (item, parent, false);
}

void QProfWidget::fillObjsProfileList ()
{
    objItems.clear();

    // create all toplevel elements and their descendants
    for (uint i = 0; i < pInfo[selectedProfileNum].mClasses.count (); i++) {
        CProfileViewItem *parent = new CProfileViewItem (mObjs, NULL);
        objItems.push_back(parent);

        parent->setText(0, pInfo[selectedProfileNum].mClasses[i]);

        for (uint j = 0; j < pInfo[selectedProfileNum].mProfile.count (); j++) {
            if (pInfo[selectedProfileNum].mProfile[j].object == &pInfo[selectedProfileNum].mClasses[i]) {
                CProfileViewItem *item = new CProfileViewItem (parent, &pInfo[selectedProfileNum].mProfile[j]);
                objItems.push_back(item);
            }
        }
    }

    for (int i = 2; i < mObjs->columnCount(); i++) {
        mObjs->resizeColumnToContents(i);
    }
}


void QProfWidget::fillOverviewProfileList ()
{
    int fCount = filelist.count();
    mBarPlot->axisY()->setRanges(0, 100);
    mBarPlot->axisY()->setTicks(2, 10);
    mBarPlot->axisY()->setPen(QPen(Qt::darkGray));
    mBarPlot->axisY()->setMinorTicksPen(QPen(Qt::gray));
    mBarPlot->axisY()->setMajorTicksPen(QPen(Qt::darkGray));
    //mBarPlot->axisY()->setMinorGridPen(QPen(Qt::gray));
    mBarPlot->axisY()->setMajorGridPen(QPen(Qt::lightGray));
    mBarPlot->axisY()->setTextColor(Qt::black);

    mBarPlot->axisX()->setPen(QPen(Qt::darkGray));
    mBarPlot->axisX()->setMinorTicksPen(QPen(Qt::gray));
    mBarPlot->axisX()->setMajorTicksPen(QPen(Qt::darkGray));
    mBarPlot->axisX()->setMajorGridPen(QPen(Qt::lightGray));
    mBarPlot->axisX()->setTextColor(Qt::black);

    mBarPlot->setBarSize(32, 128);
    mBarPlot->setBarOpacity(0.5);
// mBarPlot->setAlternatingRowColors(true);
//     mBarPlot->setContextMenuPolicy(Qt::CustomContextMenu);

    QLinearGradient bg(0, 0, 0, 1);
    bg.setCoordinateMode(QGradient::ObjectBoundingMode);
    bg.setColorAt(1, Qt::white);
    bg.setColorAt(0.5, QColor(0xccccff));
    bg.setColorAt(0, Qt::white);
    mBarPlot->setBackground(QBrush(bg));

    QStringList names;
    names = filelist;

    for (QStringList::iterator in = names.begin(); in != names.end(); ++in) {
        int pos;
        pos = (*in).lastIndexOf("/"); // short form name

        if ( pos > 0) {
            (*in) = (*in).mid(pos + 1);
        }
    }

    if (fCount > 0) {
        float maxVal;
        maxVal = 0.0;

        if (itemModel != NULL) {
            delete itemModel;
            itemModel = NULL;
        }

        itemModel = new QStandardItemModel(MAX_ROWS, names.count(), this); // raws, columns
//         itemModel->setContextMenuPolicy(Qt::CustomContextMenu);
        itemModel->setHorizontalHeaderLabels(names);

        for (int j = 0; j < fCount; j++) { // columns
            float vol;
            int i;
            float sumInFile = 0.0;

            for (i = 0; i < pInfo[j].mProfile.count(); i++) { // raws
                if (percentDiag == true) {
                    vol = pInfo[j].mProfile[i].cumPercent;
                } else {
                    vol = pInfo[j].mProfile[i].selfSeconds;
                    sumInFile += vol;
                }
            }

            if (percentDiag == false) {
                if (maxVal < sumInFile) {
                    maxVal = sumInFile;
                }

//                 qDebug() << "max seconds, sum:" << maxVal;
            }
        }

        for (int j = 0; j < fCount; j++) { // columns
            float vol;
            int i;

            for (i = 0; i < MAX_ROWS; i++) { // raws
                itemModel->setData(itemModel->index(i, j), 0); // init
            }

            i = 0;

            for (int b = 0; (b < pInfo[j].mProfile.count()) && (i < MAX_ROWS); b++) {
                if (percentDiag == true) {
                    vol = pInfo[j].mProfile[b].cumPercent;
                } else {
                    vol = pInfo[j].mProfile[b].selfSeconds;
                }

                if (vol > 0) {
                    float percent = pInfo[j].mProfile[b].cumPercent;
                    const QModelIndex index(itemModel->index(i, j));

                    if (percent < 1.0) {
                        itemModel->setData(index, Qt::white, Qt::BackgroundRole);
                    }

                    if (percent >= 1.0 && percent < 5.0) {
                        itemModel->setData(index, Qt::green, Qt::BackgroundRole);
                    }

                    if (percent >= 5.0 && percent < 10.0) {
                        itemModel->setData(index, Qt::darkGreen, Qt::BackgroundRole);
                    }

                    if (percent >= 10.0 && percent < 15.0) {
                        itemModel->setData(index, Qt::blue, Qt::BackgroundRole);
                    }

                    if (percent >= 15.0 && percent < 25.0) {
                        itemModel->setData(index, Qt::darkBlue, Qt::BackgroundRole);
                    }

                    if (percent >= 25.0 && percent < 35.0) {
                        itemModel->setData(index, Qt::yellow, Qt::BackgroundRole);
                    }

                    if (percent >= 35.0 && percent < 45.0) {
                        itemModel->setData(index, Qt::darkYellow, Qt::BackgroundRole);
                    }

                    if (percent >= 45.0 && percent < 65.0) {
                        itemModel->setData(index, Qt::red, Qt::BackgroundRole);
                    }

                    if (percent >= 65.0) {
                        itemModel->setData(index, Qt::darkRed, Qt::BackgroundRole);
                    }

                    itemModel->setData(index, pInfo[j].mProfile[b].name, Qt::ToolTipRole);
                    itemModel->setData(index, vol);
//                     itemModel->item(i, j)->setToolTip(pInfo[j].mProfile[b].name);
                    i++;
                }
            }
        }

        if (percentDiag == false) {
            if (maxVal < 1.0) {
                mBarPlot->axisY()->setRanges(0, 1);
            } else {
                mBarPlot->axisY()->setRanges(0, maxVal);
            }

            mBarPlot->setBarScale(0.5);
        }

        mBarPlot->setModel(itemModel);
    }

    mBarPlot->repaint();
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
                for (uint i = 0; i < pInfo[selectedProfileNum].mProfile.count(); i++) {
                    pInfo[selectedProfileNum].mProfile[i].output = false;
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

                    for (uint i = 0; i < pInfo[selectedProfileNum].mProfile.count(); i++) {
                        if (pInfo[selectedProfileNum].mProfile[i].output == false && pInfo[selectedProfileNum].mProfile[i].object == info->object) {
                            markForOutput (&pInfo[selectedProfileNum].mProfile[i]);
                        }
                    }
                } else {
                    markForOutput (info);
                }
            }

            // graph generation
            DotCallGraph dotCallGraph(file, currentSelectionOnly, false, pInfo[selectedProfileNum].mProfile, processName, mColorConfigure->highColour());

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
            DotCallGraph dotCallGraph(file, currentSelectionOnly, true, pInfo[selectedProfileNum].mProfile, processName, mColorConfigure->highColour());

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

    DotCallGraph dotCallGraph(*file, true, true, pInfo[selectedProfileNum].mProfile, processName, mColorConfigure->highColour());
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


