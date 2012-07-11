/***************************************************************************
 *   Copyright (C) 2012 by Eduard Kalinowski      <karbofos@ymail.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#define DONATE_STR "https://www.paypal.com/cgi-bin/webscr?cmd=_xclick&business=eduard_kalinowski@yahoo.de"

#include <QDialog>
#include "ui_aboutForm.h"

class aboutForm : public QDialog, private Ui::aboutDialog
{
    Q_OBJECT

public:
    aboutForm(QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~aboutForm();

public slots:

protected:

protected slots:
    void onPayPal(void);
};

#endif

