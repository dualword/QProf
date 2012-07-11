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
#include <QStringList>
#include <QRegExp>
#include <QHash>


class PositionSpec
{
public:
    PositionSpec() {
        fromLine = 0, toLine = 0, fromAddr = 0, toAddr = 0;
    }
    PositionSpec(uint l1, uint l2, unsigned long long a1, unsigned long long a2) {
        fromLine = l1, toLine = l2, fromAddr = a1, toAddr = a2;
    }

    bool isLineRegion() const {
        return (fromLine != toLine);
    }
    bool isAddrRegion() const {
        return (fromAddr != toAddr);
    }

    uint fromLine, toLine;
    unsigned long long fromAddr, toAddr;
};

class CParseProfile_callgrind : public CParseProfile
{
public:
    CParseProfile_callgrind(QTextStream& t, QVector<CProfileInfo>& profile);
    ~CParseProfile_callgrind() {};

    bool valid () const;

    enum lineType { SelfCost, CallCost, BoringJump, CondJump };

private:
    CParseProfile_callgrind(); // init
    bool parsePosition(const QString& s, PositionSpec& newPos);
    bool parse_part();
    bool parse();
    bool parse_header_line();
    bool parse_part_detail();
    bool parse_description();
    bool parse_event_specification();
    bool parse_cost_line_def();
    bool parse_cost_summary();
    bool parse_body_line();
    bool parse_cost_lines();
    bool parse_association_spec();
    bool parse_position_spec();
    bool parse_empty();
    bool parse_comment();
    bool parse_key(const QString& key );
    bool parse_keys(const QStringList& keyList);//??
    CProfileInfo* make_function(void);
    CProfileInfo* make_CalledFunction(void);
//         CProfileInfo* get_function_entries();
//         CProfileInfo* get_call_entries();

private:
    QHash<QString, QString> _position_table_map;
    QHash<QString, QString> _position_map;
    QHash<QString, QString> _keys;
    CProfileInfo **indexToProfile;
    QRegExp _call_re;
    QString cost_events;
    QRegExp _position_re;
    int num_positions;
    QString cost_positions;
    QRegExp _cost_re;
    QRegExp _key_re;
    QString  libName;
    QString  fileName;
    QString  function;
    // current position
    lineType nextLineType;
//         PositionSpec currentPos;
    QString  calledLibName;
    QString  calledFileName;
    QString  calledFunction;
    QString  currentJumpToFile;
    QString  currentJumpToFunction;
    bool hasLineInfo;
    bool hasAddrInfo;
    long long int summary;
//         int     last_positions[8];
//         int     events[8];
    QTextStream *strm;
    QString line;
    QVector<CProfileInfo>* workCProfile;


//         QStringList _detail_keys;
//     position_ids;
//     positions;
//
// //         # Numeric positions
//     int num_positions;
//     cost_positions;
//     last_positions;

//         # Events
    long int num_events;
//     cost_events;

    typedef struct {                    // structure holding call-graph data for PalmOS Emulator results
        int     index;
        int     parent;
    } SCallgrindCallGraph;

    QVector<SCallgrindCallGraph> callGraphBlock;

};

#endif
