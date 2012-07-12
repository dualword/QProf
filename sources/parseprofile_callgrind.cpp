/***************************************************************************
                          parseprofile_pose.cpp  -  description
                             -------------------
    begin                : Jul 05 2012
    copyright            : (C) 2012 by Eduard Kalinowski
    email                : eduard_kalinowski@yahoo.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program == free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "./includes/parseprofile_callgrind.h"

#include <stdlib.h>

#include "./includes/cprofileinfo.h"
#include "./includes/qprofwidget.h"

#include <assert.h>
#include <QTextStream>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QRegExp>
#include <QHash>

CParseProfile_callgrind::CParseProfile_callgrind (QTextStream& t, QVector<CProfileInfo>& profile)
{
    /*
     * parse a profile results file generated by the PalmOS Emulator
     *
     */

    int line = 0;
    int numEntries = 0;
    int cgCount = 0;
    QString s;

    t.setCodec ("Latin1");

    strm = &t;
    workCProfile = &profile;

    // because of the way CALLGRIND results are shown, we have to keep a dictionnary
    // of indexes -> CProfileInfo*, and a list of call maps index -> parent index
    QHash<QString, CProfileInfo*> functions;// (257);


    _call_re = QRegExp("^calls=\\s*(\\d+)\\s+((\\d+|\\+\\d+|-\\d+|\\*)\\s+)+$");
    _position_re = QRegExp("^(?P<position>[cj]?(?:ob|fl|fi|fe|fn))=\\s*(?:\((?P<id>\\d+)\\))?(?:\\s*(?P<name>.+))?");

//     LineParser.__init__(self, infile);

    QString __subpos = QString("(0x[0-9a-fA-F]+|\\d+|\\+\\d+|-\\d+|\\*)");

    _cost_re = QRegExp("^" + __subpos + "( +" + __subpos + ")*" + "( +\\d+)*" + "$");
    _key_re = QRegExp("^(\\w+):");

//         # Textual positions
//     position_ids = {};
//     positions = {};

//         # Numeric positions
    num_positions = 1;
    cost_positions = "line";
//     last_positions.append(0); = 0;

//         # Events
    num_events = 0;
    cost_events = "";

    int indexes = 8192;
    indexToProfile = (CProfileInfo **) malloc (indexes * sizeof (CProfileInfo *));
    callGraphBlock.resize (32);

    for (int i = 0; i < indexes; i++) {
        indexToProfile[i] = NULL;
    }

    if (parse_cost_lines() == false) {
        qDebug() << "warning: unexpected line: " << line;
    }

}

long long CParseProfile_callgrind::extractId(const QString& ln) {
    int posId;
    long long id;
    posId = ln.indexOf(QRegExp("\((\\d+)\)"));
    if (posId == -1) {
        qDebug() << "id is wrong" << ln;
        return -1;
    }
    else {
        QString idStr = ln.mid(posId, ln.indexOf(")")-posId);
        id = idStr.toLongLong();
//         qDebug() << line << id;
    }
    return id;
}


bool CParseProfile_callgrind::parse_cost_lines()
{
    CProfileInfo *func;
    // current position
    nextLineType  = SelfCost;
    int lines = 0;
    // default if there is no "positions:" line
    hasLineInfo = true;
    hasAddrInfo = false;
//     long long actualId;

    while (!strm->atEnd()) {
        line =  strm->readLine();
        line = line.simplified();
        lines++;
//         if (lines > 1000)
//             return false;

        if (line.length() == 0)
            continue;

        char c = line.at(0).toLatin1();

        if (!c) {
            continue;
        }

        if (c <= '9') {

            if (c == '#') {
                continue;
            }

            // go through after big switch
        } else {
            switch(c) {
            case 'f':
                // fl=, fi=, fe=
                if (line.startsWith("fl=") || line.startsWith("fi=") || line.startsWith("fe=")) {
                    int pos = line.indexOf(" ");
                    actualFileId = extractId(line);
                    actualCalledFileId = actualFileId;
                    if (pos > 0) {
                        if (fileName.find(actualFileId) == fileName.end()) // not exististing
                            fileName.insert(actualFileId, line.mid( pos + 1));
                    }

                    continue;
                }

                // fn=
                if (line.startsWith("fn=")) {
                    int pos = line.indexOf(" ");
                    actualFuncId = extractId(line);

                    if (pos > 0) {
                        if (function.find(actualFuncId) == function.end()) // not exististing
                            function.insert(actualFuncId, line.mid( pos + 1));
                    }

                    func = make_function();

                    if (func == NULL) {
                        return false;
                    }

#if 0
                    // on a new function, update status
                    int progress = (int)(100.0 * file.current() / file.len() + .5);

                    if (progress != statusProgress) {
                        statusProgress = progress;

                        /* When this signal is connected, it most probably
                         * should lead to GUI update. Thus, when multiple
                         * "long operations" (like file loading) are in progress,
                         * this can temporarly switch to another operation.
                         */
                        loadProgress(statusProgress);
                    }

#endif
                    continue;
                }

                break;

            case 'c':
                // cob=
                if (line.startsWith("cob=")) {
                    int pos = line.indexOf(" ");
                    actualCalledLibId = extractId(line);
                    if (pos > 0) {
                        if (libName.find(actualCalledLibId) == libName.end()) // not exististing
                            libName.insert(actualCalledLibId, line.mid( pos + 1));
                    }
                    continue;
                }

                // cfi= / cfl=
                if (line.startsWith("cfl=") || line.startsWith("cfi=")) {
                    int pos = line.indexOf(" ");
                    actualCalledFileId = extractId(line);
                    if (pos > 0) {
                        if (fileName.find(actualCalledFileId) == fileName.end()) // not exististing
                            fileName.insert(actualCalledFileId, line.mid( pos + 1));
                    }
                    continue;
                }

                // cfn=
                if (line.startsWith("cfn=")) {
                    int pos = line.indexOf(" ");
                    actualCalledFuncId = extractId(line);

                    if (pos > 0) {
                        if (function.find(actualCalledFuncId) == function.end()) // not exististing
                            function.insert(actualCalledFuncId, line.mid( pos + 1));
                    }

                    func = make_CalledFunction();

                    continue;
                }

                // calls=
                if (line.startsWith("calls=")) {
                    // ignore long lines...
//                     line.stripUInt64(currentCallCount);
                    if (func != NULL) {
                        int pos = line.indexOf("=");
                        if (pos > 0) {
                            QString num = line.mid(pos + 1, line.indexOf((" "))-pos-1);
//                             qDebug() << "calls" << num;
                            func->calls = num.toULongLong();
                        }
//                         qDebug() << line << func->calls;
                    }

                    nextLineType = CallCost;
                    continue;
                }

                // cmd:
                if (line.startsWith("cmd:")) {
                    QString command = QString(line).trimmed();
                    // ignore
                    /*
                                        if (!_data->command().isEmpty() &&
                                                _data->command() != command) {

                                            error(QString("Redefined command, was '%1'").arg(_data->command()));
                                        }

                                        _data->setCommand(command);*/
                    continue;
                }

                // creator:
                if (line.startsWith("creator:")) {
                    // ignore ...
                    continue;
                }

                break;

            case 'j':
                // jcnd=
                if (line.startsWith("jcnd=")) {
                    bool valid;
//ignore
//                     valid = line.stripUInt64(jumpsFollowed) &&
//                             line.startsWith("/") &&
//                             line.stripUInt64(jumpsExecuted) &&
//                             parsePosition(line, targetPos);
//
//                     if (!valid) {
//                         error(QString("Invalid line after 'jcnd'"));
//                     } else {
                    nextLineType = CondJump;
//                     }

                    continue;
                }

                if (line.startsWith("jump=")) {
                    bool valid;
//ignore..
//                     valid = line.stripUInt64(jumpsExecuted) &&
//                             parsePosition(line, targetPos);
//
//                     if (!valid) {
//                         error(QString("Invalid line after 'jump'"));
//                     } else {
                    nextLineType = BoringJump;
//                     }

                    continue;
                }

                // jfi=
                if (line.startsWith("jfi=")) {
//                     currentJumpToFile = line.mid(line.indexOf(" ") + 1);
                    continue;
                }

                // jfn=
                if (line.startsWith("jfn=")) {
                    /*
                                        if (!currentJumpToFile) {
                                            // !=0 as functions needs file
                                            currentJumpToFile = line.mid(line.indexOf(" ")+1);
                                        }*/

//                     currentJumpToFunction = line.mid(line.indexOf(" ") + 1);
                    continue;
                }

                break;

            case 'o':
                // ob=
                if (line.startsWith("ob=")) {
                    int pos = line.indexOf(" ");
                    actualLibId = extractId(line);
                    actualCalledLibId = actualLibId;
                    if (pos > 0) {
                        if (libName.find(actualLibId) == libName.end()) // not exististing
                            libName.insert(actualLibId, line.mid( pos + 1));
                    }

                    continue;
                }

                break;

            case '#':
                continue;

            case 't':
                // totals:
                if (line.startsWith("totals:")) {
                    continue;
                }

                // thread:
                if (line.startsWith("thread:")) {
                    // ignored
//                     prepareNewPart();
//                     _part->setThreadID(QString(line).toInt());
                    continue;
                }

                // timeframe (BB):
                if (line.startsWith("timeframe (BB):")) {
                    // ignored
//                     _part->setTimeframe(line);
                    continue;
                }

                break;

            case 'd':
                // desc:
                if (line.startsWith("desc:")) {

//                     line.stripSurroundingSpaces();

                    // desc: Trigger:
//                     if (line.startsWith("Trigger:")) {
//                         _part->setTrigger(line);
//                     }

                    continue;
                }

                break;

            case 'e':
                // events:
                if (line.startsWith("events:")) {
//                     prepareNewPart();
//                     mapping = _data->eventTypes()->createMapping(line);
//                     _part->setEventMapping(mapping);
                    continue;
                }

                // event:<name>[=<formula>][:<long name>]
                if (line.startsWith("event:")) {
//                     line.stripSurroundingSpaces();

//                     FixString e, f, l;
//
//                     if (!line.stripName(e)) {
//                         error(QString("Invalid event"));
//                         continue;
//                     }
//
//                     line.stripSpaces();
//
//                     if (!line.stripFirst(c)) {
//                         continue;
//                     }
//
//                     if (c == '=') {
//                         f = line.stripUntil(':');
//                     }
//
//                     line.stripSpaces();
//
//                     // add to known cost types
//                     if (line.isEmpty()) {
//                         line = e;
//                     }
//
//                     EventType::add(new EventType(e, line, f));
                    continue;
                }

                break;

            case 'p':
                // part:
                if (line.startsWith("part:")) {
                    // ignore
//                     prepareNewPart();
//                     _part->setPartNumber(QString(line).toInt());
                    continue;
                }

                // pid:
                if (line.startsWith("pid:")) {
                    // ignore
//                     prepareNewPart();
//                     _part->setProcessID(QString(line).toInt());
                    continue;
                }

                // positions:
                if (line.startsWith("positions:")) {
//                     prepareNewPart();
//                     QString positions(line);
                    hasLineInfo = line.contains("line");
                    hasAddrInfo = line.contains("instr");
                    continue;
                }

                break;

            case 'v':
                // version:
                if (line.startsWith("version:")) {
                    // ignore for now
                    continue;
                }

                break;

            case 's':
                // summary:
                if (line.startsWith("summary:")) {
                    summary =  line.mid(line.indexOf(" ") + 1).toLongLong();
//                     if (!mapping) {
//                         error(QString("No event line found. Skipping file"));
//                         delete _part;
//                         return false;
//                     }
//
//                     _part->totals()->set(mapping, line);
                    continue;
                }

            case 'r':
                // rcalls= (deprecated)
                if (line.startsWith("rcalls=")) {
                    // ignored
                    // handle like normal calls: we need the sum of call count
                    // recursive cost is discarded in cycle detection
//                     line.stripUInt64(currentCallCount);
                    nextLineType = CallCost;
//
//                     warning(QString("Old file format using deprecated 'rcalls'"));
                    continue;
                }

                break;

            default:
                break;
            }

//             qDebug() << QString("Invalid line '%1'").arg(line);
//             continue;
        }

        if (func == NULL)
            continue;

        if (hasAddrInfo) { // instuctions only
            QString num;
            bool io;
            long long sampl;
//             int posHex = line.lastIndexOf(QRegExp("0x"));
            int pos = line.lastIndexOf(QRegExp("(\\d+)"));
            if (pos > 0)
                num = line .mid(pos);

            sampl = num.toLongLong(&io);
            if (io == false) {
                qDebug() << QString("Invalid line '%1'").arg(line);
                continue;
            }

            if (func != NULL) {
                func->custom.callgrind.selfSamples += sampl;
            }
        }

        if (hasLineInfo) { // lines of source code
            QString num;
            bool io;
            long long sampl;
//             int posHex = line.lastIndexOf(QRegExp("0x"));
            int pos = line.lastIndexOf(QRegExp("(\\d+)"));
            if (pos > 0 )
                num = line .mid(pos);

            sampl = num.toLongLong(&io);
            if (io == false) {
                qDebug() << QString("Invalid line '%1'").arg(line);
                continue;
            }

            if (func != NULL) {
                func->custom.callgrind.selfSamples += sampl;
            }
        }

    }

    return true;
}


CProfileInfo* CParseProfile_callgrind::make_function()
{
    //FIXME: module and fileName are not being tracked reliably
    //id = "|".join((module, fileName, name))
    CProfileInfo *p;
    long int num;
    int pos;

// existiert?
    for (num = 0; num < workCProfile->count(); ++num) {
        if (workCProfile->at(num).name == function[actualFuncId]) break;
//         num = workCProfile->indexOf(function);
    }

// qDebug() << num << actualFuncId;

    if ( num >= 0 && num < workCProfile->count()) {
        p = (CProfileInfo*)&workCProfile->at(num);
        return p;
    }

//     processCallGraphBlock (callGraphBlock, *workCProfile);

    p = new CProfileInfo;// Function(id, name);
    p->fileName = fileName[actualFileId];

//     SCallGraphEntry *e = new SCallGraphEntry;
//
//     e->line = 0;
//     e->primary = true;
//     e->recursive = false;

    if (libName[actualLibId].length() > 0) {
        p->libName = libName[actualLibId];
    }

    pos = function[actualFuncId].indexOf("::");

    if ( pos > 0) {
        p->object = function[actualFuncId].left(pos);
        p->method = function[actualFuncId].mid (pos + 2);
    }

    p->calls = 0;
    p->custom.callgrind.selfSamples = 0;
    p->name = function[actualFuncId];

    qDebug() << p->libName << p->fileName << p->name;

    p->called.clear();
    workCProfile->append(*p);

//-----------------

//     // gather other values (we have to do some guessing to get them right)
//     while (field < fields.count ()) {
//         if (countRegExp.indexIn (fields[field], 0) == 0) {
//             e->recursive = fields[field].indexOf ('+') != -1;
//         } else if (floatRegExp.indexIn (fields[field], 0) == -1) {
//             e->name = fields[field];
//         }
//
//         field++;
//     }

    // if we got a call graph block without a primary function name,
    // drop it completely.
//     if (e->name == NULL || e->name.length() == 0) {
//         delete e;
// //         break;
//     }
    /*
        if (e->primary == true && count.indexOf ('+') != -1) {
            e->recursive = true;
        }*/

//     callGraphBlock.append(*e);

    return p;
}

CProfileInfo* CParseProfile_callgrind::make_CalledFunction()
{
    //FIXME: module and fileName are not being tracked reliably
    //id = "|".join((module, fileName, name))
    CProfileInfo *p;
    CProfileInfo *f;
    long int num;
    int pos;

// existiert?
    for (num = 0; num < workCProfile->count(); ++num) {
        if (workCProfile->at(num).name == function[actualCalledFuncId]) break;
//         num = workCProfile->indexOf(function);
    }

//     qDebug() << "called:" << num << actualCalledFuncId << function[actualCalledFuncId];

    if ( num >= 0 && num < workCProfile->count()) {
        p = (CProfileInfo*)&workCProfile->at(num);
        return p;
    }

//     processCallGraphBlock (callGraphBlock, *workCProfile);

    p = new CProfileInfo;// Function(id, name);
    p->fileName = fileName[actualCalledFileId];

//     SCallGraphEntry *e = new SCallGraphEntry;
//
//     e->line = 0;
//     e->primary = false;
//     e->recursive = false;

    if (libName[actualCalledLibId].length() > 0) {
        p->libName = libName[actualCalledLibId];
    }

    pos = function[actualCalledFuncId].indexOf("::");

    if ( pos > 0) {
        p->object = function[actualCalledFuncId].left(pos);
        p->method = function[actualCalledFuncId].mid (pos + 2);
    }

    p->calls = 0;
    p->custom.callgrind.selfSamples = 0;
    p->name = function[actualCalledFuncId];

    p->called.clear();

    // was called from:
    for (num = 0; num < workCProfile->count(); ++num) {
        if (workCProfile->at(num).method == function[actualFuncId]) break;
//         num = workCProfile->indexOf(function);
    }

    f = NULL;

    if ( num >= 0 && num < workCProfile->count()) {
        *f = workCProfile->at(num);
//         return p;
    }

    if (f != NULL) {
        p->callers.append(f);
        f->called.append(p);
    }

    workCProfile->append(*p);

    return p;
}


bool CParseProfile_callgrind::valid() const
{
    return mValid;
}

