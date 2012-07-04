/*
 * cprofileviewitem.h
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

#ifndef __CPROFILEVIEWITEM_H__
#define __CPROFILEVIEWITEM_H__

#include <QTreeWidget>
#include <QRegExp>

class CProfileInfo;


class CProfileViewItem : public QTreeWidgetItem
{
//         Q_OBJECT
    protected:
        CProfileInfo*       mProfile;

    public:
        CProfileViewItem (QTreeWidget *parent,           CProfileInfo *profile);
        CProfileViewItem (QTreeWidgetItem *parent,   CProfileInfo *profile);
        CProfileViewItem (QTreeWidget *parent,           QTreeWidgetItem *after, CProfileInfo *profile);
        CProfileViewItem (QTreeWidgetItem *parent,   QTreeWidgetItem *after, CProfileInfo *profile);
        virtual ~CProfileViewItem ();

        virtual void paintCell (QPainter * p, const QColor &cg, int column, int width, int align);
        virtual QString getText (int column) const;
        virtual QString key (int column, bool ascending) const;

        inline CProfileInfo* getProfile () const {
            return mProfile;
        }

    private:
        void setRecursiveIcon ();
        void setTextInformation();
        static QString formatFloat (float n, int precision);
        static QString formatSpeedDiff (float newSpeed, float oldSpeed);
};

#endif
