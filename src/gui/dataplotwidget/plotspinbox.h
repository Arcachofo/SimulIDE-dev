/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PLOTSPINBOX_H
#define PLOTSPINBOX_H

#include <QDoubleSpinBox>

class MAINMODULE_EXPORT PlotSpinBox : public QDoubleSpinBox
{
    public:
        PlotSpinBox( QWidget* parent );
        ~PlotSpinBox();

        QValidator::State validate( QString& text, int& n ) const override;

    private:
        QDoubleSpinBox* m_this;
};
#endif
