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

    if (QProfWidget::sDiffMode == false) {
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
            default:
                break;
        }

        setRecursiveIcon ();
    } else {
        switch(QProfWidget::sLastFileFormat) {
            case    FORMAT_GPROF:

                for (int i = diff_col_status; i <= diff_col_new_selfMsPerCall; i++) {
                    this->setData(i, Qt::DisplayRole, getText( i));
                }

                break;
            case    FORMAT_FNCCHECK:

                for (int i = diff_col_status; i <= diff_col_maxMsPerCall; i++) {
                    this->setData(i, Qt::DisplayRole, getText( i ));
                }

                break;
            case    FORMAT_POSE:

                for (int i = diff_col_status; i <= diff_col_new_cumCycles; i++) {
                    this->setData(diff_col_selfCycles, Qt::DisplayRole, getText( diff_col_selfCycles ));
                }

                break;
            default:
                break;
        }
    }
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

QString CProfileViewItem::formatSpeedDiff (float newSpeed, float oldSpeed)
{
    // given two values which are speeds (usually execution speeds),
    // return a string formated as "x% faster" or "x% slower", with special cases
    // like "< 1% faster" or "< 1% slower"

    QString s;

    // make sure we don't have values < 1.0 which make 1/x miscalculations
    if (newSpeed > 0.0 && oldSpeed > 0.0) {
        while (newSpeed < 1.0 || oldSpeed < 1.0) {
            newSpeed *= 10.0;
            oldSpeed *= 10.0;
        }

        double pct = (double)1.0 / ((double)oldSpeed / (double)newSpeed / (double)100.0);

        if (pct < 100.0) {
            pct = (double)100.0 - pct;

            if (pct < (double)1.0) {
                s = "< 1% faster";
            } else {
                s = QString( QString::number(pct).toInt() + "%% faster");
            }
        } else if (newSpeed < oldSpeed) {
            pct -= (double)100.0;

            if (pct < (double)1.0) {
                s = "< 1% slower";
            } else {
                s = QString( QString::number(pct).toInt() + "%% slower");
            }
        }
    }

    return s;
}

QString CProfileViewItem::getText (int column) const
{
    if (mProfile == NULL) {
        // we are a top level element of the object view: just return the
        // name, being the class name
        CProfileViewItem *child = (CProfileViewItem *) this->child (0);
        return (child == NULL || column != col_function) ? QString("") : child->mProfile->object;
    }

    if (QProfWidget::sDiffMode == false) {
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
    } else {
        // if diff-mode
        switch (column) {
            case diff_col_function: {
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

            case diff_col_status:

                if (mProfile->previous) {
                    // try generating a meaningful string about the
                    // changes that occured between two profiles of the same call
                    if (QProfWidget::sLastFileFormat == FORMAT_GPROF) {
                        return formatSpeedDiff (mProfile->custom.gprof.selfMsPerCall, mProfile->previous->custom.gprof.selfMsPerCall);
                    }

                    if (mProfile->calls && mProfile->previous->calls) {
                        float n = mProfile->selfSeconds;
                        float o = mProfile->previous->selfSeconds;

                        if (o > 0.0 && n > 0.0) {
                            while (o < 1.0 || n < 1.0) {
                                o *= 10.0;      // get values up a bit to minimize precision loss errors
                                n *= 10.0;
                            }

                            return formatSpeedDiff ((float)((double)n / (double)mProfile->calls), (float)((double)o / (double)mProfile->previous->calls));
                        }
                    }

                    return QString ("");
                }

                return mProfile->deleted ? "deleted" : "new";

            case diff_col_new_count:
                return QString::number (mProfile->calls);

            case diff_col_count:
                return mProfile->previous ? QString::number (mProfile->previous->calls) : QString ("");

            case diff_col_new_total:
                return formatFloat (mProfile->cumSeconds, 3);

            case diff_col_total:
                return mProfile->previous ? formatFloat (mProfile->previous->cumSeconds, 3) : QString ("");

            case diff_col_new_totalPercent:
                return formatFloat (mProfile->cumPercent, 3);

            case diff_col_totalPercent:
                return mProfile->previous ? formatFloat (mProfile->previous->cumPercent, 3) : QString ("");

            case diff_col_new_self:
                return formatFloat (mProfile->selfSeconds, 3);

            case diff_col_self:
                return mProfile->previous ? formatFloat (mProfile->previous->selfSeconds, 3) : QString ("");

            case diff_col_new_totalMsPerCall:
                return formatFloat (mProfile->totalMsPerCall, 3);

            case diff_col_totalMsPerCall:
                return mProfile->previous ? formatFloat (mProfile->previous->totalMsPerCall, 3) : QString ("");

            default:

                if (QProfWidget::sLastFileFormat == FORMAT_GPROF) {
                    if (column == diff_col_new_selfMsPerCall) {
                        return formatFloat (mProfile->custom.gprof.selfMsPerCall, 3);
                    }

                    if (column == diff_col_selfMsPerCall) {
                        return mProfile->previous ? formatFloat (mProfile->previous->custom.gprof.selfMsPerCall, 3) : QString ("");
                    }
                } else if (QProfWidget::sLastFileFormat == FORMAT_FNCCHECK) {
                    if (column == diff_col_new_minMsPerCall) {
                        return formatFloat (mProfile->custom.fnccheck.minMsPerCall, 3);
                    }

                    if (column == diff_col_minMsPerCall) {
                        return mProfile->previous ? formatFloat (mProfile->previous->custom.fnccheck.minMsPerCall, 3) : QString ("");
                    }

                    if (column == diff_col_new_maxMsPerCall) {
                        return formatFloat (mProfile->custom.fnccheck.maxMsPerCall, 3);
                    }

                    if (column == diff_col_maxMsPerCall) {
                        return mProfile->previous ? formatFloat (mProfile->previous->custom.fnccheck.maxMsPerCall, 3) : QString ("");
                    }
                } else if (QProfWidget::sLastFileFormat == FORMAT_CALLGRIND) {
                    if (column == diff_col_new_selfCycles) {
                        return QString::number (mProfile->custom.callgrind.selfSamples);
                    }

                    if (column == diff_col_selfCycles) {
                        return mProfile->previous ? QString::number (mProfile->previous->custom.callgrind.selfSamples) : QString ("");
                    }

                    if (column == diff_col_new_cumCycles) {
                        return QString::number (mProfile->custom.callgrind.cumSamples);
                    }

                    if (column == diff_col_cumCycles) {
                        return mProfile->previous ? QString::number (mProfile->previous->custom.callgrind.cumSamples) : QString ("");
                    }
                } else if (QProfWidget::sLastFileFormat == FORMAT_POSE) {
                    if (column == diff_col_new_selfCycles) {
                        return QString::number (mProfile->custom.pose.selfCycles);
                    }

                    if (column == diff_col_selfCycles) {
                        return mProfile->previous ? QString::number (mProfile->previous->custom.pose.selfCycles) : QString ("");
                    }

                    if (column == diff_col_new_cumCycles) {
                        return QString::number (mProfile->custom.pose.cumCycles);
                    }

                    if (column == diff_col_cumCycles) {
                        return mProfile->previous ? QString::number (mProfile->previous->custom.pose.cumCycles) : QString ("");
                    }
                }

                return QString ("");
        }
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

    if (QProfWidget::sDiffMode == false) {
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
    } else {
        switch (column) {
            case diff_col_function:
                s = mProfile->name;
                break;

            case diff_col_new_count:
                s.sprintf ("%014ld", mProfile->calls);
                break;

            case diff_col_count:

                if (mProfile->previous) {
                    s.sprintf ("%014ld", mProfile->previous->calls);
                }

                break;

            case diff_col_new_total:
                s.sprintf ("%014ld", (long) (mProfile->cumSeconds * 100.0));
                break;

            case diff_col_total:

                if (mProfile->previous) {
                    s.sprintf ("%014ld", (long) (mProfile->previous->cumSeconds * 100.0));
                }

                break;

            case diff_col_new_totalPercent:
                s.sprintf ("%05ld",  (long) (mProfile->cumPercent * 100.0));
                break;

            case diff_col_totalPercent:

                if (mProfile->previous) {
                    s.sprintf ("%05ld",  (long) (mProfile->previous->cumPercent * 100.0));
                }

                break;

            case diff_col_new_self:
                s.sprintf ("%014ld", (long) (mProfile->selfSeconds * 100.0));
                break;

            case diff_col_self:

                if (mProfile->previous) {
                    s.sprintf ("%014ld", (long) (mProfile->previous->selfSeconds * 100.0));
                }

                break;

            case diff_col_new_totalMsPerCall:
                s.sprintf ("%014ld", (long) (mProfile->totalMsPerCall * 100.0));
                break;

            case diff_col_totalMsPerCall:

                if (mProfile->previous) {
                    s.sprintf ("%014ld", (long) (mProfile->previous->totalMsPerCall * 100.0));
                }

                break;

            default:

                if (QProfWidget::sLastFileFormat == FORMAT_GPROF) {
                    if (column == diff_col_new_selfMsPerCall) {
                        s.sprintf ("%014ld", (long) (mProfile->custom.gprof.selfMsPerCall * 100.0));
                    }

                    if (mProfile->previous && column == diff_col_selfMsPerCall) {
                        s.sprintf ("%014ld", (long) (mProfile->previous->custom.gprof.selfMsPerCall * 100.0));
                    }
                } else if (QProfWidget::sLastFileFormat == FORMAT_FNCCHECK) {
                    if (column == diff_col_new_minMsPerCall) {
                        s.sprintf ("%014ld", (long) (mProfile->custom.fnccheck.minMsPerCall * 100.0));
                    } else if (column == diff_col_new_maxMsPerCall) {
                        s.sprintf ("%014ld", (long) (mProfile->custom.fnccheck.maxMsPerCall * 100.0));
                    } else if (mProfile->previous) {
                        if (column == diff_col_minMsPerCall) {
                            s.sprintf ("%014ld", (long) (mProfile->previous->custom.fnccheck.minMsPerCall * 100.0));
                        } else if (column == diff_col_maxMsPerCall) {
                            s.sprintf ("%014ld", (long) (mProfile->previous->custom.fnccheck.maxMsPerCall * 100.0));
                        }
                    }
                } else if (QProfWidget::sLastFileFormat == FORMAT_CALLGRIND) {
                    if (column == diff_col_new_selfCycles) {
                        s.sprintf ("%014ld", mProfile->custom.callgrind.selfSamples);
                    } else if (column == diff_col_new_cumCycles) {
                        s.sprintf ("%014ld", mProfile->custom.callgrind.cumSamples);
                    } else if (mProfile->previous) {
                        if (column == diff_col_selfCycles) {
                            s.sprintf ("%014ld", mProfile->previous->custom.callgrind.selfSamples);
                        } else if (column == diff_col_cumCycles) {
                            s.sprintf ("%014ld", mProfile->previous->custom.callgrind.cumSamples);
                        }
                    }
                } else if (QProfWidget::sLastFileFormat == FORMAT_POSE) {
                    if (column == diff_col_new_selfCycles) {
                        s.sprintf ("%014ld", mProfile->custom.pose.selfCycles);
                    } else if (column == diff_col_new_cumCycles) {
                        s.sprintf ("%014ld", mProfile->custom.pose.cumCycles);
                    } else if (mProfile->previous) {
                        if (column == diff_col_selfCycles) {
                            s.sprintf ("%014ld", mProfile->previous->custom.pose.selfCycles);
                        } else if (column == diff_col_cumCycles) {
                            s.sprintf ("%014ld", mProfile->previous->custom.pose.cumCycles);
                        }
                    }
                }

                break;
        }
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

    if (column == col_function || QProfWidget::sDiffMode == false) {
        p->setPen (QPen (gray, 1, Qt::SolidLine));
    } else {
        bool solid = false;

        if (column == diff_col_status ||
                column == diff_col_new_count ||
                column == diff_col_new_total ||
                column == diff_col_new_totalPercent ||
                column == diff_col_new_self ||
                column == diff_col_new_totalMsPerCall ||
                (QProfWidget::sLastFileFormat == FORMAT_GPROF && column == diff_col_new_selfMsPerCall) ||
                (QProfWidget::sLastFileFormat == FORMAT_FNCCHECK &&
                 (column == diff_col_new_minMsPerCall || column == diff_col_new_maxMsPerCall)) ||
                (QProfWidget::sLastFileFormat == FORMAT_POSE &&
                 (column == diff_col_new_selfCycles || column == diff_col_new_cumCycles)) ||
                (QProfWidget::sLastFileFormat == FORMAT_CALLGRIND &&
                 (column == diff_col_new_selfCycles || column == diff_col_new_cumCycles))) {
            solid = true;
        }

        p->setPen (QPen (gray, 1, solid ? Qt::SolidLine : Qt::DotLine));
    }

    p->drawLine (width - 1, 0, width - 1, 20);
    p->restore ();
}

