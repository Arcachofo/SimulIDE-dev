/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SWITCH_BASE_H
#define SWITCH_BASE_H

#include "mech_contact.h"

class QToolButton;
class QGraphicsProxyWidget;

class MAINMODULE_EXPORT SwitchBase : public MechContact
{
    Q_OBJECT
    public:
        SwitchBase( QObject* parent, QString type, QString id );
        ~SwitchBase();

        virtual void updateStep() override;

        virtual void setHidden( bool hid, bool hidLabel=false ) override;

        QString key() { return m_key; }
        void setKey( QString key );
        
        QToolButton* button() { return m_button; }

    public slots:
        virtual void onbuttonclicked();
        virtual void keyEvent( QString key, bool pressed ){;}

    protected:
        QString m_key;
        
        QToolButton*          m_button;
        QGraphicsProxyWidget* m_proxy;
};

#endif
