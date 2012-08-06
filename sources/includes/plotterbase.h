#ifndef PLOTTERBASE_H
#define PLOTTERBASE_H

#include <QtGui>


namespace QSint
{


class AxisBase;


class PlotterBase : public QWidget
{
    Q_OBJECT
public:
    explicit PlotterBase(QWidget *parent = 0);


    void setBorderPen(const QPen &pen);
    inline const QPen& borderPen() const {
        return m_pen;
    }

    void setBackground(const QBrush &brush);
    void setForeground(const QBrush &brush);

    inline const QBrush& background() const {
        return m_bg;
    }
    inline const QBrush& foreground() const {
        return m_fg;
    }

    QRect dataRect();


    inline AxisBase* axisX() const {
        return m_axisX;
    }
    inline AxisBase* axisY() const {
        return m_axisY;
    }


    void setModel(QAbstractItemModel *model);
    inline QAbstractItemModel* model() const {
        return m_model;
    }


signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent *);
//     virtual void mouseMoveEvent( QMouseEvent* );
//     virtual bool event(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

//     virtual void showEvent(QShowEvent *event);
    virtual void drawBackground(QPainter &p);
    virtual void drawForeground(QPainter &p);
    virtual void drawAxis(QPainter &p);
    virtual void drawContent(QPainter &p) = 0;

    AxisBase *m_axisX;
    AxisBase *m_axisY;

    QAbstractItemModel *m_model;

    QBrush m_bg;
    QBrush m_fg;
    QPen m_pen;
};


} // namespace

#endif // PLOTTERBASE_H
