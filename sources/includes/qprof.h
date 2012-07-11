/*
 * kprof.h
 *
 * Copyright (c) 2012 Eduard Kalinowski <karbofos@ymail.com>
 * Copyright (c) 2000 Florent Pillet <florent.pillet@wanadoo.fr>
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

#ifndef __QPROF_H__
#define __QPROF_H__

#include <QMainWindow>

#include <QAction>
#include <QWidget>

class KProfTopLevel : public QMainWindow
{
        Q_OBJECT

    protected:
        QWidget*    mProf;
        QWidget*    kapp;
        QAction*    mToggleTemplateAbbrev;
        QAction*    mSelectFont;
        QAction*    mGenCallGraphAction;
        QAction*    mCompareFile;
        QAction*    mRunApplication;
        QAction*    mDisplayTreeMapAction;
        QAction*    mConfigure;

    public:
        KProfTopLevel (QWidget *parent = 0, const char *name = NULL);
        ~KProfTopLevel ();

        inline QAction* getToggleTemplateAbbrevAction () {
            return mToggleTemplateAbbrev;
        }

    protected slots:
        virtual bool queryExit ();
        void toggleToolBar ();
        void addRecentFile (const QUrl& url);

    private:
        void setupActions ();
        void loadSettings ();
        void applySettings ();
};

#endif
