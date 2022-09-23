/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VARSOURCE_H
#define VARSOURCE_H

#include "component.h"
#include "voltwidget.h"
#include "e-element.h"

class MAINMODULE_EXPORT VarSource : public Component, public eElement
{
    Q_OBJECT
    public:
        VarSource( QObject* parent, QString type, QString id );
        ~VarSource();

        double getVal() { return m_outValue; }
        void setVal( double val );

        double maxValue() { return m_maxValue; }
        void setMaxValue( double v ) { m_maxValue = v;}

        virtual void initialize() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void onbuttonclicked();
        void valueChanged( int val );

    protected:
        void updateButton();
        void updtValue( double v );

        virtual void setflip() override;
        
        double m_maxValue;
        double m_outValue;

        QString m_unit;
        
        VoltWidget m_voltw;

        QPushButton* m_button;
        QDial* m_dial;
        QGraphicsProxyWidget* m_proxy;
};

#endif

