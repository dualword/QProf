/*
 * popup_menu.h
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

#ifndef __POPUPWIDGET_H__
#define __POPUPWIDGET_H__

#include <QtCore>
#include <QWidget>
#include <QFrame>
#include <QMouseEvent>
#include <QToolButton>


class TitleBar : public QWidget
{
        Q_OBJECT
    public:
        TitleBar(QWidget *parent);

    public slots:
        void showSmall();
        void showMaxRestore();
    protected:
        void mousePressEvent(QMouseEvent *me) ;
        void mouseMoveEvent(QMouseEvent *me);

    private:
        QToolButton *minimize;
        QToolButton *maximize;
        QToolButton *close;
        QPixmap restorePix, maxPix;
        bool maxNormal;
        QPoint startPos;
        QPoint clickPos;
};


class Frame : public QFrame
{
    public:

        Frame(QWidget *parent);

        // Allows you to access the content area of the frame
        // where widgets and layouts can be added
        QWidget *contentWidget() const;
        TitleBar *titleBar() const ;
        void mousePressEvent(QMouseEvent *e);
        void mouseMoveEvent(QMouseEvent *e);
        void mouseReleaseEvent(QMouseEvent *e);

    private:
        TitleBar *m_titleBar;
        QWidget *m_content;
        QPoint m_old_pos;
        bool m_mouse_down;
        bool left, right, bottom;
};

#endif