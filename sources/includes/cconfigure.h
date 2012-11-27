
#ifndef _CCONFIGURE_H_
#define _CCONFIGURE_H_

#include <QColor>
// #include <QColorDialog.h>
#include <QWidget>


/**
 * This class holds all the configuration data for KProf
 * Colin Desmond
 **/
class CConfigure
{
//         Q_OBJECT
    public:
        CConfigure(QWidget *parent = 0L);
        ~CConfigure();

        const QColor& highColour() const;

        void chooseGraphHighColour();

    private:
        QWidget* parentWindow;

        QColor graphHighColour;
};

#endif
