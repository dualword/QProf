#include <QAbstractItemView>

// #include "./includes/qprofwidget.h"
#include "./includes/plotterbase.h"
#include "./includes/axisbase.h"

// class QProfWidget;

namespace QSint
{


PlotterBase::PlotterBase(QWidget *parent) :
    QWidget(parent)
{
    m_axisX = new AxisBase(Qt::Horizontal, this);
    m_axisY = new AxisBase(Qt::Vertical, this);

    setModel(0);
}


void PlotterBase::setBorderPen(const QPen &pen)
{
    m_pen = pen;
}

void PlotterBase::setBackground(const QBrush &brush)
{
    m_bg = brush;
}

void PlotterBase::setForeground(const QBrush &brush)
{
    m_fg = brush;
}

QRect PlotterBase::dataRect()
{
    QRect p_rect(rect());

    if (m_axisX) {
        p_rect.setBottom(p_rect.bottom() - m_axisX->offset());
    }

    if (m_axisY) {
        p_rect.setLeft(p_rect.left() + m_axisY->offset());
    }

    return p_rect;
}


void PlotterBase::setModel(QAbstractItemModel *model)
{
    m_model = model;

    if (m_axisX) {
        m_axisX->setModel(model);
    }

    if (m_axisY) {
        m_axisY->setModel(model);
    }

    if (m_model) {
        setMouseTracking(true);
        connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(repaint()));
        connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation, int, int)), this, SLOT(repaint()));
        connect(m_model, SIGNAL(columnsInserted(const QModelIndex &, int, int)), this, SLOT(repaint()));
        connect(m_model, SIGNAL(columnsRemoved(const QModelIndex &, int, int)), this, SLOT(repaint()));
        connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(repaint()));
        connect(m_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(repaint()));
    }
}


bool PlotterBase::event(QEvent *event)
{

    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QPoint p = helpEvent->pos();

        QModelIndex index;

        if (indexAt(p, index) == true) {
            QToolTip::showText(helpEvent->globalPos(), index.data(Qt::ToolTipRole).toString() );
        } else {
            QToolTip::hideText();
            event->ignore();
        }

        return true;
    }

    if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *helpEvent = static_cast<QMouseEvent *>(event);
        QPoint p = helpEvent->pos();
        QString fileName;

        if (horItemNameAt(p, fileName) == true) {
//             qDebug() << fileName << "clicked";
            emit selectName(fileName);
//             QToolTip::showText(helpEvent->globalPos(), index.data(Qt::ToolTipRole).toString() );
        } else {
//             QToolTip::hideText();
            event->ignore();
        }

        return true;
    }

    return QWidget::event(event);
}


void PlotterBase::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    drawBackground(p);

    drawAxis(p);

    drawContent(p);

    drawForeground(p);
}

void PlotterBase::drawBackground(QPainter &p)
{
    p.fillRect(rect(), m_bg);
}

void PlotterBase::drawForeground(QPainter &p)
{
    p.setOpacity(1);
    p.setPen(m_pen);
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

void PlotterBase::drawAxis(QPainter &p)
{
    if (m_axisX) {
        m_axisX->draw(p);
    }

    if (m_axisY) {
        m_axisY->draw(p);
    }
}


} // namespace
