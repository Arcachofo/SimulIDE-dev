/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SEVENSEGMENT_H
#define SEVENSEGMENT_H

#include "component.h"
#include "e-element.h"
#include "ledsmd.h"

class LibraryItem;

class MAINMODULE_EXPORT SevenSegment : public Component, public eElement
{
    public:
        SevenSegment( QObject* parent, QString type, QString id );
        ~SevenSegment();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();

        QString colorStr();
        void setColorStr( QString color );

        int numDisplays() { return m_numDisplays; }
        void setNumDisplays( int dispNumber );

        bool verticalPins() { return m_verticalPins; }
        void setVerticalPins( bool v );
        
        bool isComCathode() { return m_commonCathode; }
        void setComCathode( bool isCommonCathode );
        
        double threshold() { return m_threshold; }
        void   setThreshold( double threshold );
        
        double maxCurrent() { return m_maxCurrent; }
        void   setMaxCurrent( double current );

        double resistance() { return m_resistance; }
        void   setResistance( double res );

        virtual void stamp() override;

        virtual QStringList getEnumUids( QString ) override;
        virtual QStringList getEnumNames( QString ) override;

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget ) override;

    private:
        void createDisplay( int n );
        void deleteDisplay( int n );
        void resizeData( int displays );

        bool m_commonCathode;
        bool m_verticalPins;
        
        int  m_numDisplays;
        double m_threshold;
        double m_maxCurrent;
        double m_resistance;

        std::vector<Pin*>   m_commonPin;
        std::vector<ePin*>  m_cathodePin;
        std::vector<ePin*>  m_anodePin;
        std::vector<LedSmd*> m_segment;
        eNode* m_enode[8];
};

#endif

