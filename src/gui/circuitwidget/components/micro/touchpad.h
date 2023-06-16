/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include "e-element.h"
#include "component.h"
#include "e-resistor.h"
#include "touchpadwidget.h"
#include "pin.h"

class LibraryItem;
class Pin;

class MAINMODULE_EXPORT TouchPad : public Component, public eElement
{
    public:
        TouchPad( QObject* parent, QString type, QString id );
        ~TouchPad();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;

        int width() { return m_width; }
        void setWidth( int w );

        int height() { return m_height; }
        void setHeight( int h );

        bool transparent() { return m_transparent; }
        void setTransparent( bool t );

        double RxMin() { return m_RxMin; }
        void setRxMin( double min );

        double RxMax() { return m_RxMax; }
        void setRxMax( double max );

        double RyMin() { return m_RyMin; }
        void setRyMin( double min );

        double RyMax() { return m_RyMax; }
        void setRyMax( double max );

        virtual QPainterPath shape() const override;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        void updateSize();

        bool m_transparent;

        int m_width;
        int m_height;
        int m_xPos;
        int m_yPos;

        double m_RxMin;
        double m_RxMax;
        double m_RyMin;
        double m_RyMax;

        eNode* m_eNodeX;
        eNode* m_eNodeY;

        eResistor m_resXA;
        eResistor m_resXB;
        eResistor m_resYA;
        eResistor m_resYB;
        eResistor m_resTouch;

        Pin* m_vrx_p;
        Pin* m_vry_p;
        Pin* m_vrx_m;
        Pin* m_vry_m;

        ePin m_ePinXA;
        ePin m_ePinXB;
        ePin m_ePinYA;
        ePin m_ePinYB;
        ePin m_ePinTA;
        ePin m_ePinTB;

        TouchPadWidget m_touchpadW;
        QGraphicsProxyWidget* m_proxy;
};

#endif
