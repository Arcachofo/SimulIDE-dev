/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DIALED_H
#define DIALED_H

#include "component.h"
#include "e-element.h"
#include "dialwidget.h"

class MAINMODULE_EXPORT Dialed : public Component
{
    public:
        Dialed( QObject* parent, QString type, QString id );
        ~Dialed();

        QList<ComProperty*> dialProps();

        bool slider() { return m_slider; }
        void setSlider( bool s );

        double scale() { return m_dialW.scale(); }
        void setScale( double s ) { m_dialW.setScale(s); updateProxy(); }

        virtual void setLinked( bool l ) override;
        virtual void setLinkedValue( int v, int i=0 ) override;

    public slots:
        virtual void dialChanged( int );

    protected:
        virtual void updateProxy(){;}

        bool m_needUpdate;
        bool m_slider;

        DialWidget m_dialW;
        QGraphicsProxyWidget* m_proxy;
};

#endif
