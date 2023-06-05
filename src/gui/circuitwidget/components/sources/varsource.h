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
    public:
        VarSource( QObject* parent, QString type, QString id );
        ~VarSource();

        double getVal() { return m_outValue; }
        void setVal( double val );

        double maxValue() { return m_maxValue; }
        void setMaxValue( double v );

        bool running();
        void setRunning( bool r );

        virtual void initialize() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    public slots:
        void onbuttonclicked();
        void dialChanged( int val );

    protected:
        void updateButton();
        void updtValue( double v );

        virtual void setflip() override;
        
        double m_maxValue;
        double m_outValue;

        QString m_unit;
        
        VoltWidget m_voltw;

        CustomButton* m_button;
        QGraphicsProxyWidget* m_proxy;
};

#endif

