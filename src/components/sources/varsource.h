/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VARSOURCE_H
#define VARSOURCE_H

#include "component.h"
#include "sourcewidget.h"
#include "e-element.h"

class VarSource : public Component, public eElement
{
    public:
        VarSource( QString type, QString id );
        ~VarSource();

        double getVal() { return m_outValue; }
        void setVal( double val );

        double maxValue() { return m_maxValue; }
        void setMaxValue( double v );

        double minValue() { return m_minValue; }
        void setMinValue( double v );

        bool running();
        void setRunning( bool r );

        virtual void initialize() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    public slots:
        void onbuttonclicked();
        void dialChanged( int val );

    protected:
        void updateButton();
        void updtValue( double v );

        virtual void setflip() override;
        
        double m_maxValue;
        double m_minValue;
        double m_outValue;

        QString m_unit;
        
        SourceWidget m_voltw;

        CustomButton* m_button;
        QGraphicsProxyWidget* m_proxy;
};

#endif

