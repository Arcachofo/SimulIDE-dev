/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CHIP_H
#define CHIP_H

#include "component.h"
#include "e-element.h"

class QDomElement;

class Chip : public Component, public eElement
{
    public:
        Chip( QString type, QString id );
        ~Chip();

        enum subcType_t{
            None=0,
            Logic,
            Board,
            Shield,
            Module
        };

        virtual bool setPropStr( QString prop, QString val ) override;

        QString package() { return m_package; }
        virtual void setPackage( QString package );
        
        bool logicSymbol() { return m_isLS; }
        virtual void setLogicSymbol( bool ls );

        virtual void setBckGndData( QString data );
        virtual void setBackground( QString bck ) override;

        QString name() { return m_name; }
        void setName( QString name );

        void setPinStr( QString pin );

        subcType_t subcType() { return m_subcType; }

        int pkgWidth() { return m_width; }

        void setBackData( std::vector<std::vector<int>>* d ) { m_backData = d; }

        virtual void setflip() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

 static QString convertPackage( QString domText );

    protected:
        virtual Pin* addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle, int length=8, int space=0 ){return NULL;}

        virtual Pin* updatePin( QString id, QString type, QString label,
                                int pos, int xpos, int ypos, int angle, int space=0  ){return NULL;}

        virtual void addNewPin( QString id, QString type, QString label,
                        int pos, int xpos, int ypos, int angle, int length=8, int space=0 );

        void initPackage(  QString pkgStr  );

        virtual void findHelp() override;

        int m_width;
        int m_height;
        
        bool m_isLS;
        bool m_initialized;

        subcType_t m_subcType;

        QColor m_lsColor;
        QColor m_icColor;

        QString m_name;
        QString m_dataFile;

        QMap<QString, QString> m_packageList;

        QString m_package;
        QList<Pin*> m_ncPins;
        QList<Pin*> m_tempPins;
        QMap<QString, Pin*> m_pinMap;

        std::vector<std::vector<int>>* m_backData;

        QGraphicsTextItem m_label;
};

#endif
