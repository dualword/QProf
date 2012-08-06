/*
 * cprofileviewitem.cpp
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

#include <stdlib.h>
#include <math.h>
#include <QTreeWidget>
#include <QString>
#include <QPainter>
#include <QLocale>
#include <QDebug>
#include <QIcon>

#include "./includes/qprofwidget.h"
#include "./includes/constants.h"
#include "./includes/cprofileinfo.h"
#include "./includes/cprofileviewitem.h"

CProfileViewItem::CProfileViewItem (QTreeWidget *parent, CProfileInfo *profile)
    :   QTreeWidgetItem (parent),
        mProfile (profile)
{
    setTextInformation();
}

CProfileViewItem::CProfileViewItem (QTreeWidgetItem *parent, CProfileInfo *profile)
    :   QTreeWidgetItem (parent),
        mProfile (profile)
{
    setTextInformation();
}

CProfileViewItem::CProfileViewItem (QTreeWidget *parent, QTreeWidgetItem *after, CProfileInfo *profile)
    :   QTreeWidgetItem (parent, after),
        mProfile (profile)
{
    setTextInformation();
}


CProfileViewItem::CProfileViewItem (QTreeWidgetItem *parent, QTreeWidgetItem *after, CProfileInfo *profile)
    :   QTreeWidgetItem (parent, after),
        mProfile (profile)
{
    setTextInformation();
}

CProfileViewItem::~CProfileViewItem ()
{
}

void CProfileViewItem::setTextInformation()
{
    if (mProfile == NULL) {
        return;
    }

    this->setText(col_function, mProfile->name);

    switch(QProfWidget::sLastFileFormat) {
    case    FORMAT_GPROF:
        for (int i = col_count; i <= col_selfMsPerCall; i++) {
            this->setData(i, Qt::DisplayRole, getText(i));
        }
        break;

    case    FORMAT_FNCCHECK:
        for (int i = col_count; i <= col_maxMsPerCall; i++) {
            this->setData(i, Qt::DisplayRole, getText(i));
        }
        break;

    case    FORMAT_POSE:
        for (int i = col_count; i <= col_cumCycles; i++) {
            this->setData(i, Qt::DisplayRole, getText(i));
        }
        break;

    case    FORMAT_CALLGRIND:
        for (int i = col_count; i <= col_cumCycles; i++) {
            this->setData(i, Qt::DisplayRole, getText(i));
        }
        break;

    default:
        break;
    }

    setRecursiveIcon ();
}

void CProfileViewItem::setRecursiveIcon ()
{
    if (mProfile && mProfile->recursive) {
        // here we do not need to test diff mode because col_function is
        // always the first column
        QIcon undoicon = QIcon::fromTheme("edit-undo");
        this->setIcon(col_function + 1, undoicon);
    }
}

QString CProfileViewItem::formatFloat (float n, int precision)
{
    // format a float with parameterized precision
    char buffer[32], format[16];
    sprintf (format, "%%.0%df", precision);
    sprintf (buffer, format, n);
    return QString (buffer);
}

QString CProfileViewItem::getText (int column) const
{
    if (mProfile == NULL) {
        // we are a top level element of the object view: just return the
        // name, being the class name
        CProfileViewItem *child = (CProfileViewItem *) this->child (0);
        return (child == NULL || column != col_function) ? QString("") : child->mProfile->object;
    }

    switch (column) {
    case col_function: {
        CProfileViewItem *p = dynamic_cast<CProfileViewItem *> (parent ());

        if (p && p->mProfile == NULL) {
            // we are in a method of an object in the object
            if (mProfile->multipleSignatures) {
                return mProfile->name.right (mProfile->name.length () - mProfile->object.length() - 2);
            }

            return mProfile->method;
        }

        return mProfile->simplifiedName;
    }

    case col_count:
        return QString::number (mProfile->calls);

    case col_total:
        return formatFloat (mProfile->cumSeconds, 3);

    case col_totalPercent:
        return formatFloat (mProfile->cumPercent, 3);

    case col_self:
        return formatFloat (mProfile->selfSeconds, 3);

    case col_totalMsPerCall:
        return formatFloat (mProfile->totalMsPerCall, 3);

    default:

        if (QProfWidget::sLastFileFormat == FORMAT_GPROF) {
            if (column == col_selfMsPerCall) {
                return formatFloat (mProfile->custom.gprof.selfMsPerCall, 3);
            }
        } else if (QProfWidget::sLastFileFormat == FORMAT_FNCCHECK) {
            if (column == col_minMsPerCall) {
                return formatFloat (mProfile->custom.fnccheck.minMsPerCall, 3);
            }

            if (column == col_maxMsPerCall) {
                return formatFloat (mProfile->custom.fnccheck.maxMsPerCall, 3);
            }
        } else if (QProfWidget::sLastFileFormat == FORMAT_CALLGRIND) {
            if (column == col_selfCycles) {
                return QString::number (mProfile->custom.callgrind.selfSamples);
            }

            if (column == col_cumCycles) {
                return QString::number (mProfile->custom.callgrind.cumSamples);
            }
        } else if (QProfWidget::sLastFileFormat == FORMAT_POSE) {
            if (column == col_selfCycles) {
                return QString::number (mProfile->custom.pose.selfCycles);
            }

            if (column == col_cumCycles) {
                return QString::number (mProfile->custom.pose.cumCycles);
            }
        }

        return "";
    }
}

QString CProfileViewItem::key (int column, bool) const
{
    QString s;

    if (mProfile == NULL) {
        // we are a top level element of the object view: just return the
        // name, being the class name
        CProfileViewItem *child = (CProfileViewItem *) this->child (0);

        if (child == NULL || column != col_function) {
            return "";
        }

        return child->mProfile->object;
    }

    switch (column) {
    case col_function:
        s = mProfile->name;
        break;

    case col_count:
        s.sprintf ("%014ld", mProfile->calls);
        break;

    case col_total:
        s.sprintf ("%014ld", (long) (mProfile->cumSeconds * 100.0));
        break;

    case col_totalPercent:
        s.sprintf ("%05ld",  (long) (mProfile->cumPercent * 100.0));
        break;

    case col_self:
        s.sprintf ("%014ld", (long) (mProfile->selfSeconds * 100.0));
        break;

    case col_totalMsPerCall:
        s.sprintf ("%014ld", (long) (mProfile->totalMsPerCall * 100.0));
        break;

    default:

        if (QProfWidget::sLastFileFormat == FORMAT_GPROF) {
            if (column == col_selfMsPerCall) {
                s.sprintf ("%014ld", (long) (mProfile->custom.gprof.selfMsPerCall * 100.0));
            }
        } else if (QProfWidget::sLastFileFormat == FORMAT_FNCCHECK) {
            if (column == col_minMsPerCall) {
                s.sprintf ("%014ld", (long) (mProfile->custom.fnccheck.minMsPerCall * 100.0));
            } else if (column == col_maxMsPerCall) {
                s.sprintf ("%014ld", (long) (mProfile->custom.fnccheck.maxMsPerCall * 100.0));
            }
        } else if (QProfWidget::sLastFileFormat == FORMAT_CALLGRIND) {
            if (column == col_selfCycles) {
                s.sprintf ("%014ld", mProfile->custom.callgrind.selfSamples);
            } else if (column == col_cumCycles) {
                s.sprintf ("%014ld", mProfile->custom.callgrind.cumSamples);
            }
        } else if (QProfWidget::sLastFileFormat == FORMAT_POSE) {
            if (column == col_selfCycles) {
                s.sprintf ("%014ld", mProfile->custom.pose.selfCycles);
            } else if (column == col_cumCycles) {
                s.sprintf ("%014ld", mProfile->custom.pose.cumCycles);
            }
        }

        break;
    }
    return s;
}

void CProfileViewItem::paintCell (QPainter * p, const QColor & cg, int column, int width, int align)
{
    // call original drawing method
    this->paintCell (p, cg, column, width, align);

    if (p == NULL) {
        return;
    }

    // if in diff mode, paint a border line on the right of the cell:
    // - light for the edge of a "previous" cell
    // - dark for the edge of a "new" cell
    // this gives a display where old and new entries are grouped by blocks of two
    p->save ();
    QColor gray (125, 125, 125);

    if (column == col_function ) {
        p->setPen (QPen (gray, 1, Qt::SolidLine));
    } else {
        bool solid = false;
        p->setPen (QPen (gray, 1, solid ? Qt::SolidLine : Qt::DotLine));
    }

    p->drawLine (width - 1, 0, width - 1, 20);
    p->restore ();
}

