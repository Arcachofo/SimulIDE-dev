/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SEVENSEGMENTBCD_H
#define SEVENSEGMENTBCD_H

#include "bcdbase.h"

class LibraryItem;

class MAINMODULE_EXPORT SevenSegmentBCD : public BcdBase
{
    public:
        SevenSegmentBCD( QString type, QString id );
        ~SevenSegmentBCD();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        bool isShowEnablePin() { return m_showEnablePin; }
        void setShowEnablePin( bool v );

        bool isShowDotPin() { return m_showDotPin; }
        void setShowDotPin( bool v );

        virtual void setLinked( bool l ) override;
        virtual void setLinkedValue( double v, int i=0  ) override;

        void voltChanged() override;
        std::vector<Pin *> getPins() override;
        void remove() override;
        void stamp() override;

        void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        int m_intensity;

        bool m_showEnablePin;
        bool m_showDotPin;

        IoPin *m_dotPin;
        IoPin *m_enablePin;
};

#endif
