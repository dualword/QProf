#include "./includes/barchartplotter.h"
#include "./includes/axisbase.h"


namespace QSint
{


BarChartPlotter::BarChartPlotter(QWidget *parent) :
    PlotterBase(parent)
{
    setBarType(Stacked);

    m_axisX->setType(AxisBase::AxisModel);

    setBarSize(-INT_MAX, INT_MAX);
    setBarScale(0.5);
    setBarOpacity(1.0);
}

void BarChartPlotter::setBarType(BarChartType type)
{
    m_type = type;
}

void BarChartPlotter::setBarSize(int min, int max)
{
    m_barsize_min = qMax(min, 0);
    m_barsize_max = qMax(min, max);
}

void BarChartPlotter::setBarScale(double scale)
{
    if (scale < 0.1)
        m_scale = 0.1;
    else
        m_scale = qMin(scale, 1.0);
}

void BarChartPlotter::setBarOpacity(double value)
{
    if (value < 0.0)
        m_opacity = 0;
    else
        m_opacity = qMin(value, 1.0);
}

void BarChartPlotter::drawContent(QPainter &p)
{
    if (!m_model || !m_axisX || !m_axisY)
        return;

    int p_start, p_end;
    m_axisX->calculatePoints(p_start, p_end);

    int p_y = m_axisY->toView(0);
    p.drawLine(p_start, p_y, p_end, p_y);

    int count = m_model->columnCount();
    if (!count)
        return;

    int row_count = m_model->rowCount();
    if (!row_count)
        return;

    int p_offs = double(p_end - p_start) / count;

    int bar_size = p_offs * m_scale;

    if (bar_size > m_barsize_max)
        bar_size = qMin(m_barsize_max, p_offs);
    else if (bar_size < m_barsize_min)
        bar_size = qMin(m_barsize_min, p_offs);



    switch (m_type) {
    case Stacked:

        for (int i = 0; i < count; i++) {
            int p_d = p_start + p_offs*i + (p_offs-bar_size)/2;

            double acc_value = 0;
            int p_y = m_axisY->toView(0);

            double neg_value = 0;
            int p_ny = p_y;

            p.setOpacity(m_opacity);

            for (int j = 0; j < row_count; j++) {
                const QModelIndex index(m_model->index(j, i));

                QPen pen(qvariant_cast<QColor>(m_model->data(index, Qt::ForegroundRole)));
                QBrush brush(qvariant_cast<QBrush>(m_model->data(index, Qt::BackgroundRole)));

                double value = m_model->data(index).toDouble();

                if (value < 0) {
                    neg_value += value;

                    int p_h = m_axisY->toView(neg_value);

                    drawBarItem(p, QRect(p_d, p_ny, bar_size, p_h-p_ny), pen, brush, index, value);

                    p_ny = p_h;
                }
                else {
                    acc_value += value;

                    int p_h = m_axisY->toView(acc_value);

                    drawBarItem(p, QRect(p_d, p_h, bar_size, p_y-p_h), pen, brush, index, value);

                    p_y = p_h;
                }
            }
        }

        break;

    case Columns:
    {
        int single_bar_size = bar_size/row_count;
        if (!single_bar_size)
            return;

        for (int i = 0; i < count; i++) {
            int p_d = p_start + p_offs*i + (p_offs-bar_size)/2;

            int p_y = m_axisY->toView(0);

            p.setOpacity(m_opacity);

            for (int j = 0; j < row_count; j++) {
                const QModelIndex index(m_model->index(j, i));
                QPen pen(qvariant_cast<QColor>(m_model->data(index, Qt::ForegroundRole)));
                QBrush brush(qvariant_cast<QBrush>(m_model->data(index, Qt::BackgroundRole)));

                double value = m_model->data(index).toDouble();

                int p_h = m_axisY->toView(value);

                if (value < 0) {
                    drawBarItem(p, QRect(p_d, p_y, single_bar_size, p_h-p_y), pen, brush, index, value);
                }
                else {
                    drawBarItem(p, QRect(p_d, p_h, single_bar_size, p_y-p_h), pen, brush, index, value);
                }

                p_d += single_bar_size;
            }
        }

        break;
    } // Nearby

    } // switch
}

bool BarChartPlotter::horItemNameAt(const QPoint &p, QString &name )
{
    if (!m_model || !m_axisX || !m_axisY)
        return false;

    int p_start, p_end;
    m_axisX->calculatePoints(p_start, p_end);

//     int p_y = m_axisY->toView(0);

    int col_count = m_model->columnCount();
    if (!col_count)
        return false;

    int row_count = m_model->rowCount();
    if (!row_count)
        return false;

    int p_offs = double(p_end - p_start) / col_count;

    int bar_size = p_offs * m_scale;

    if (bar_size > m_barsize_max)
        bar_size = qMin(m_barsize_max, p_offs);
    else if (bar_size < m_barsize_min)
        bar_size = qMin(m_barsize_min, p_offs);

    int p_y = m_axisY->toView(m_axisY->rangeMininum());
    int p_h = m_axisY->toView(m_axisY->rangeMaximum());

    switch (m_type) {
    case Stacked:
    {
        for (int i = 0; i < col_count; i++) {
            int p_d = p_start + p_offs*i + (p_offs-bar_size)/2;

            if ((p.x() >= p_d) && (p.x() <= (p_d+bar_size))) {
                name =  m_model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
                return true;
            }
        }

        break;
    }
    case Columns:
    {
        int single_bar_size = bar_size/row_count;
        if (!single_bar_size)
            return false;

//         int p_y = m_axisY->toView(m_axisY->rangeMininum());
//         int p_h = m_axisY->toView(m_axisY->rangeMaximum());

        for (int i = 0; i < col_count; i++) {
            int p_d = p_start + p_offs*i + (p_offs-bar_size)/2;

            QRect r(p_d, p_y, single_bar_size, p_h);
            if (r.contains(p) == true) {
                name =  m_model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
                return true;
            }
        }

        break;
    } // Nearby

    } // switch
    return false;
}


bool BarChartPlotter::indexAt(const QPoint &p, QModelIndex &idx )
{
    if (!m_model || !m_axisX || !m_axisY)
        return false;

    int p_start, p_end;
    m_axisX->calculatePoints(p_start, p_end);

    int p_y = m_axisY->toView(0);

    int col_count = m_model->columnCount();
    if (!col_count)
        return false;

    int row_count = m_model->rowCount();
    if (!row_count)
        return false;

    int p_offs = double(p_end - p_start) / col_count;

    int bar_size = p_offs * m_scale;

    if (bar_size > m_barsize_max)
        bar_size = qMin(m_barsize_max, p_offs);
    else if (bar_size < m_barsize_min)
        bar_size = qMin(m_barsize_min, p_offs);



    switch (m_type) {
    case Stacked:

        for (int i = 0; i < col_count; i++) {
            int p_d = p_start + p_offs*i + (p_offs-bar_size)/2;

            double acc_value = 0;
            int p_y = m_axisY->toView(0);

            double neg_value = 0;
            int p_ny = p_y;

            for (int j = 0; j < row_count; j++) {
                const QModelIndex index(m_model->index(j, i));
//                 QPen pen(qvariant_cast<QColor>(m_model->data(index, Qt::ForegroundRole)));
//                 QBrush brush(qvariant_cast<QBrush>(m_model->data(index, Qt::BackgroundRole)));

                double value = m_model->data(index).toDouble();

                if (value < 0) {
                    neg_value += value;

                    int p_h = m_axisY->toView(neg_value);

                    QRect r(p_d, p_ny, bar_size, p_h-p_ny);
                    if (r.contains(p) == true) {
                        idx = index;
                        return true;
                    }

                    p_ny = p_h;
                }
                else {
                    acc_value += value;

                    int p_h = m_axisY->toView(acc_value);
                    QRect r(p_d, p_h, bar_size, p_y-p_h);
                    if (r.contains(p) == true) {
                        idx = index;
                        return true;
                    }

                    p_y = p_h;
                }
            }
        }

        break;

    case Columns:
    {
        int single_bar_size = bar_size/row_count;
        if (!single_bar_size)
            return false;

        for (int i = 0; i < col_count; i++) {
            int p_d = p_start + p_offs*i + (p_offs-bar_size)/2;

            int p_y = m_axisY->toView(0);

            for (int j = 0; j < row_count; j++) {
                const QModelIndex index(m_model->index(j, i));
//                 QPen pen(qvariant_cast<QColor>(m_model->data(index, Qt::ForegroundRole)));
//                 QBrush brush(qvariant_cast<QBrush>(m_model->data(index, Qt::BackgroundRole)));

                double value = m_model->data(index).toDouble();

                int p_h = m_axisY->toView(value);

                if (value < 0) {
                    QRect r(p_d, p_y, single_bar_size, p_h-p_y);
                    if (r.contains(p) == true) {
                        idx = index;
                        return true;
                    }
                }
                else {
                    QRect r(p_d, p_h, single_bar_size, p_y-p_h);
                    if (r.contains(p) == true) {
                        idx = index;
                        return true;
                    }
                }

                p_d += single_bar_size;
            }
        }

        break;
    } // Nearby

    } // switch
    return false;
}

void BarChartPlotter::drawBarItem(QPainter &p, QRect rect, QPen &pen, QBrush &brush,
                                  const QModelIndex &index, double value)
{
    p.setPen(pen);
    p.setBrush(brush);
    p.drawRect(rect);
}


} // namespace
