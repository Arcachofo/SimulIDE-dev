/***************************************************************************
 *   Copyright (C) 2020 by Jan K. S.                                       *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

/*   Modified 2020 by Santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         */

#ifndef KY023_H
#define KY023_H

#include "e-element.h"
#include "component.h"
#include "joystickwidget.h"

class LibraryItem;
class IoPin;
class QToolButton;

class MAINMODULE_EXPORT KY023 : public Component, public eElement
{
    public:
        KY023( QObject* parent, QString type, QString id );
        ~KY023();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual void updateStep() override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
        
    public slots:
        void onbuttonpressed();
        void onbuttonreleased();
        
    private:
        JoystickWidget m_joystickW;
        QGraphicsProxyWidget* m_proxy;
        
        QToolButton* m_button;
        QGraphicsProxyWidget* m_proxy_button;
        
        bool m_changed;
        bool m_closed;

        IoPin* m_vrx;
        IoPin* m_vry;
        IoPin* m_sw;
};

#endif

