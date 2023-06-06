/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CUSTOMSlider_H
#define CUSTOMSlider_H

#include <QSlider>

class CustomSlider : public QSlider
{
    Q_OBJECT
    public:
        CustomSlider( QWidget* parent=NULL );

    private:
        virtual void paintEvent( QPaintEvent* ) override;
};
#endif
