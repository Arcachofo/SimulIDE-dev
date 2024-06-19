/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef KEYPAD_H
#define KEYPAD_H

#include "e-element.h"
#include "push_base.h"
#include "e-diode.h"
class LibraryItem;

class KeyPad : public Component, public eElement
{
    public:
        KeyPad( QString type, QString id );
        ~KeyPad();
		void initialize();

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
		
		bool has_diodes() {return m_has_diodes; }
		void set_has_diodes(bool d);
		bool direction(){ return m_direction; }
		void setDirection(bool dir);
       
	   	virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        virtual void setflip() override;
        void setupButtons(int prev_rows, int prev_cols);

        int m_rows;
        int m_cols;
        
		bool m_has_diodes;
		bool m_direction;
        
		QString m_keyLabels;
        
        QList<PushBase*> m_buttons;
        QList<eDiode*> m_diodes;
        QList<eNode*> m_enodes;
};

#endif
