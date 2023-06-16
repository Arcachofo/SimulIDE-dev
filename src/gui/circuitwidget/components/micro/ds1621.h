/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef DS1621_H
#define DS1621_H

#include "twimodule.h"
#include "iocomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT DS1621 : public IoComponent, public TwiModule
{
    public:
        DS1621( QObject* parent, QString type, QString id );
        ~DS1621();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        double tempInc() { return m_tempInc; }
        void setTempInc( double inc ) { m_tempInc = trim( inc ); }

        double temp() { return m_temp; }
        void setTemp( double temp ) { m_temp = temp; }

        virtual void stamp() override;
        virtual void runEvent() override;
        virtual void voltChanged() override;

        virtual void readByte() override;
        virtual void writeByte() override;
        //virtual void startWrite() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    public slots:
        void upbuttonclicked();
        void downbuttonclicked();

    private:
        double trim( double data ) { return (double)((int)(data*10))/10; }
        void doConvert();

        bool m_convert;
        bool m_oneShot;
        bool m_outPol;

        int m_writeByte;
        uint8_t m_command;

        uint8_t m_config;
        uint8_t m_tempCount;
        uint8_t m_tempSlope;

        double m_tempInc;
        double m_temp;
        int8_t m_tempReg[2];

        double m_Th;
        int8_t m_ThReg[2];

        double m_Tl;
        int8_t m_TlReg[2];

        QFont m_font;
};

#endif

