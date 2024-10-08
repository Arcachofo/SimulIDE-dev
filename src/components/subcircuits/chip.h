/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CHIP_H
#define CHIP_H

#include "component.h"
#include "e-element.h"

class Chip : public Component, public eElement
{
    friend class EmbedCircuit;

    public:
        Chip( QString type, QString id );
        ~Chip();

        virtual bool setPropStr( QString prop, QString val ) override;

        bool isBoard() { return m_isBoard; }

        QString package() { return m_package; }
        virtual void setPackage( QString package );
        
        bool logicSymbol() { return m_isLS; }
        virtual void setLogicSymbol( bool ls );

        virtual void setBckGndData( QString data );
        virtual void setBackground( QString bck ) override;

        QString name() { return m_name; }
        void setName( QString name );

        //subcType_t subcType() { return m_subcType; }
        QString subcTypeStr() { return m_subcType; }
        virtual void setSubcTypeStr( QString s ){ m_subcType = s; }

        int pkgWidth() { return m_width; }

        void setBackData( std::vector<std::vector<int>>* d ) { m_backData = d; }  // Used by ScriptCpu

        virtual void setflip() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

 static QMap<QString, QString> getPackages( QString compText );
 static QString convertPackage( QString pkgText );
 static QString cleanPinName( QString name );
 static QString s_subcType;

    protected:
        virtual Pin* addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle, int length=8, int space=0 ){return NULL;}

        virtual Pin* updatePin( QString id, QString type, QString label,
                                int pos, int xpos, int ypos, int angle, int space=0  ){return NULL;}

        virtual void addNewPin( QString id, QString type, QString label,
                        int pos, int xpos, int ypos, int angle, int length=8, int space=0 );

        void initPackage(  QString pkgStr  );

        void setPinStr( QVector<propStr_t> properties );

        virtual void findHelp() override;

        int m_width;
        int m_height;
        
        bool m_isLS;
        bool m_initialized;
        bool m_customColor;
        bool m_isBoard;

        QString m_subcType;

        QColor m_lsColor;
        QColor m_icColor;

        QString m_name;
        QString m_package;
        QString m_dataFile;

        QMap<QString, QString> m_packageList;

        QList<Pin*> m_ncPins;
        QList<Pin*> m_tempPins;
        QMap<QString, Pin*> m_pinMap;

        std::vector<std::vector<int>>* m_backData; // Used by ScriptCpu

        QGraphicsTextItem m_label;
};

#endif
