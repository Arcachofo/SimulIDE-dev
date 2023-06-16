/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FIXEDVOLT_H
#define FIXEDVOLT_H

#include <QToolButton>
#include <QGraphicsProxyWidget>

#include "component.h"
#include "e-element.h"

class IoPin;
class LibraryItem;
class CustomButton;

class MAINMODULE_EXPORT FixedVolt : public Component, public eElement
{
    public:
        FixedVolt( QObject* parent, QString type, QString id );
        ~FixedVolt();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void updateStep() override;

        bool out();
        virtual void setOut( bool out );

        double volt() { return m_voltage; }
        void setVolt( double v );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    public slots:
        virtual void onbuttonclicked();

    protected:
        void updateOutput();

        double m_voltage;

        IoPin* m_outpin;

        CustomButton* m_button;
        QGraphicsProxyWidget* m_proxy;
};

#endif
