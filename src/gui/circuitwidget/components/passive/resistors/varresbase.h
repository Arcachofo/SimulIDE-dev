/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef VARRESBASE_H
#define VARRESBASE_H

#include "e-resistor.h"
#include "comp2pin.h"
#include "dialwidget.h"

class MAINMODULE_EXPORT VarResBase : public Comp2Pin, public eResistor
{
    Q_OBJECT
    public:
        VarResBase( QObject* parent, QString type, QString id );
        ~VarResBase();

        double maxVal()  { return m_maxVal; }
        virtual void setMaxVal( double max );

        double minVal() { return m_minVal; }
        virtual void setMinVal( double min );

        double getVal() { return m_value; }
        void setVal( double val );

        double getStep() { return m_step; }
        void setStep( double step ) { if( step<0 ) step=0;m_step = step ; }

        virtual void initialize() override;


    public slots:
        void dialChanged( int val );

    protected:
        void updtValue();

        double m_value;
        double m_minVal;
        double m_maxVal;
        double m_step;

        DialWidget m_dialW;
        QDial*     m_dial;
        QGraphicsProxyWidget* m_proxy;
};

#endif
