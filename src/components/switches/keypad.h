/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef KEYPAD_H
#define KEYPAD_H

#include "e-element.h"
#include "push_base.h"

class LibraryItem;

class KeyPad : public Component, public eElement
{
    public:
        KeyPad( QString type, QString id );
        ~KeyPad();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        int rows() { return m_rows; }
        void setRows( int rows );
        
        int cols() { return m_cols; }
        void setCols( int cols );
        
        QString keyLabels() { return m_keyLabels; }
        void setKeyLabels( QString keyLabels );
        
        virtual void stamp() override;
        virtual void remove() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        virtual void setflip() override;
        void setupButtons();

        int m_rows;
        int m_cols;
        
        QString m_keyLabels;
        
        QList<PushBase*> m_buttons;
};

#endif
