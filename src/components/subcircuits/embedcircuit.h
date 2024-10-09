/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EMBEDCIRCUIT_H
#define EMBEDCIRCUIT_H

#include <QMap>

class Component;
class Tunnel;
class Pin;
class QMenu;
class Chip;

class EmbedCircuit
{
    public:
        EmbedCircuit( QString name, QString id, Chip* component );
        ~EmbedCircuit();

        QList<Tunnel*> getPinTunnels() { return m_pinTunnels.values(); }
        Component* getMainComp( QString uid="" );

        QString toString();

    protected:
        void loadSubCircuitFile( QString file );
        void loadSubCircuit( QString doc );

        Pin* addPin( QString id, QString type, QString label,
                     int pos, int xpos, int ypos, int angle, int length=8, int space=0 );

        Pin* updatePin( QString id, QString type, QString label,
                        int xpos, int ypos, int angle, int length=8, int space=0 );

        Pin* findPin( QString pinId );
        void addMainCompsMenu( QMenu* menu );

        Chip* m_component;

        QString m_ecName;
        QString m_ecId;

        QMap<QString, Component*> m_mainComponents;
        static QString m_subcDir;      // Subcircuit Path

        QList<Component*>      m_compList;
        QList<Tunnel*>         m_subcTunnels;
        QMap<QString, Tunnel*> m_pinTunnels;

        void loadGraphProps();
 static QStringList s_graphProps;
};
#endif
