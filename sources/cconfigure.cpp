
#include "./includes/cconfigure.h"

#include <QWidget>
#include <QColorDialog>

CConfigure::CConfigure(QWidget *parent)
{
    parentWindow = parent;
    QString defaultColour("red");
    graphHighColour.setNamedColor(defaultColour);
}


CConfigure::~CConfigure()
{

}

void CConfigure::chooseGraphHighColour()
{
    QColor result = QColorDialog::getColor(graphHighColour, parentWindow);
}

const QColor& CConfigure::highColour() const
{
    return graphHighColour;
}
