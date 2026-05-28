/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "component.h"
#include "e-element.h"

class Chip : public Component, public eElement
{
    public:
        Chip( QString type, QString id, QString device=""  );
        ~Chip();

        bool propNotFound( QString prop, QString val ) override;

        bool isBoard() { return m_isBoard; }

        QString package() { return m_package; }
        virtual void setPackage( QString package );
        
        bool logicSymbol() { return m_isLS; }
        virtual void setLogicSymbol( bool ls );

        virtual void setBckGndData( QString data );
        void setBackground( QString bck ) override;

        bool customColor() { return m_customColor; }
        virtual void setCustomColor( bool c ){ m_customColor = c; }

        QString pkgColorStr() { return m_pkgColor.name(); }
        void setPkgColorStr( QString color );

        bool border() { return m_border; }
        void setBorder( bool b ) { m_border = b; update(); }

        QString name() { return m_name; }
        void setName( QString name );

        //subcType_t subcType() { return m_subcType; }
        QString subcTypeStr() { return m_subcType; }
        virtual void setSubcTypeStr( QString s ){ m_subcType = s; }

        int pkgWidth() { return m_width; }
        void setWidth( int w );
        void setHeight( int h );

        void setBackData( std::vector<std::vector<int>>* d );

        void setMargins( int top, int bottom, int right, int left );
        void setMargins( QString margins );
        QString getMargins() { return m_margins; }

        void setflip() override;

        void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

 static QMap<QString, QString> getPackages( QString compText );
 static QString convertPackage( QString pkgText );
 static QString cleanPinName( QString name );
 static QString getDevice( QString id );
 static QString s_subcType;

    protected:
        virtual Pin* addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle, int length=8, double space=0 ){return nullptr;}

        virtual Pin* updatePin( QString id, QString type, QString label,
                                int pos, int xpos, int ypos, int angle, double space=0  ){return nullptr;}

        virtual void addNewPin( QString id, QString type, QString label,
                        int pos, int xpos, int ypos, int angle, int length=8, double space=0 );

        void initPackage(  QString pkgStr  );

        void setPinStr( QVector<propStr_t> properties );

        virtual void embeedBackground( QString pixmapPath );

        void findHelp() override;

        void updateColor();

        int m_width;
        int m_height;
        int m_topMargin;
        int m_bottomMargin;
        int m_rightMargin;
        int m_leftMargin;
        QString m_margins;
        
        bool m_isLS;
        bool m_initialized;
        bool m_customColor;
        bool m_isBoard;
        bool m_border;
        bool m_hasBckGndData;

        QString m_subcType;

        QColor m_lsColor;
        QColor m_icColor;
        QColor m_pkgColor;

        QString m_name;
        QString m_device;
        QString m_package;
        QString m_dataFile;

        QMap<QString, QString> m_packageList;

        QList<Pin*> m_ncPins;
        QList<Pin*> m_tempPins;
        QMap<QString, Pin*> m_pinMap;

        std::vector<std::vector<int>>* m_backData;

        QGraphicsTextItem m_label;
};
