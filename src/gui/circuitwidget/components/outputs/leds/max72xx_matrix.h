/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MAX72XX_MATRIX_H
#define MAX72XX_MATRIX_H

#include "logiccomponent.h"

class LibraryItem;
class IoPin;
class Pin;

class MAINMODULE_EXPORT Max72xx_matrix : public LogicComponent
{
    public:
        Max72xx_matrix( QObject* parent, QString type, QString id );
        ~Max72xx_matrix();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        QString colorStr() { return m_enumUids.at((int)m_ledColor ); }
        void setColorStr( QString color );

        int numDisplays() { return m_numDisplays; }
        void setNumDisplays( int dispNumber );

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void updateStep() override;

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        void proccessCommand();

        int m_numDisplays;

        int  m_ram[16][8];
        int  m_intensity[16];
        int  m_decodemode;
        int  m_scanlimit;
        bool m_shutdown;
        bool m_test;

        int m_rxReg;        // Received value
        int m_inBit;        // How many bits have we read since last value
        int m_inDisplay;    // Current display

        int m_ledColor;
        QColor m_colors[7];

        //Inputs
        Pin* m_pinCS;
        Pin* m_pinDin;
        IoPin* m_pinSck;
};

#endif
