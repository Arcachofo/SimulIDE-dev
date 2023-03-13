/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SWITCHDIP_H
#define SWITCHDIP_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class QPushButton;
class QGraphicsProxyWidget;

class MAINMODULE_EXPORT SwitchDip : public Component, public eElement
{
    public:
        SwitchDip( QObject* parent, QString type, QString id );
        ~SwitchDip();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        int  size() { return m_size; }
        void setSize( int size );

        int  state() { return m_state; }
        void setState( int state );

        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void remove() override;

        void createSwitches( int c );
        void deleteSwitches( int d );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void onbuttonclicked();

    private:
        QList<QPushButton*> m_buttons;
        QList<QGraphicsProxyWidget*> m_proxys;

        int m_size;
        int m_state;
};

#endif
