/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "chip.h"

class Tunnel;
class LibraryItem;

class SubCircuit : public Chip
{
    struct subcData_t
    {
        QString subcType;
        QString circuit;
        QMap<QString, QString> packageList;
    };

    public:
        SubCircuit( QString type, QString id, QString device );
        ~SubCircuit();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void remove() override;

        virtual void setLogicSymbol( bool ls ) override;

        QList<Tunnel*> getPinTunnels() { return m_pinTunnels.values(); }
        Component* getMainComp( QString uid="" );

        virtual QString toString() override;

        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

 static void clearLocalData() { s_localDevices.clear(); } // Clear Data for Subcircuits in Circuit folder

    protected:
        void loadSubCircuitFile( QString file );
        void loadSubCircuit( QString doc );

        void addMainCompsMenu( QMenu* menu );

        void openCircuit();

        Pin* findPin( QString pinId );

        virtual Pin* addPin( QString id, QString type, QString label,
                            int pos, int xpos, int ypos, int angle, int length=8, double space=0 ) override;

        virtual Pin* updatePin( QString id, QString type, QString label,
                                int xpos, int ypos, int angle, int length=8, double space=0 ) override;

        QMap<QString, Component*> m_mainComponents;
 static QString s_subcDir;      // Subcircuit Path

        QList<Component*>      m_compList;
        QList<Tunnel*>         m_subcTunnels;
        QMap<QString, Tunnel*> m_pinTunnels;

 static QStringList s_graphProps;
        void loadGraphProps();

 static QMap<QString, subcData_t> s_globalDevices;  // Data for Subcircuits (global)
 static QMap<QString, subcData_t> s_localDevices;   // Data for Subcircuits in Circuit folder (cleared at Circuit close)
};
