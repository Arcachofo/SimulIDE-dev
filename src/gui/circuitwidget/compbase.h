/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMPBASE_H
#define COMPBASE_H

#include <QString>
#include <QPointF>
#include <QMap>
#include <QCoreApplication>

#include "proputils.h"

#define simulideTr(comp_name,str) QCoreApplication::translate(comp_name,str)

class ComProperty;

enum groupFlags{
    groupHidden = 1,
    groupNoCopy = 1<<1,
};

struct propGroup{
    QString name;
    QList<ComProperty*> propList;
    int flags;
};

class PropDialog;

class CompBase
{
    public:
        CompBase( QString type, QString id );
        virtual ~CompBase();

        propGroup* getPropGroup( QString name );
        void addPropGroup( propGroup pg, bool list=true );
        void remPropGroup( QString name );
        void addProperty( QString group, ComProperty* p, bool list=true );
        void remProperty( QString prop );
        QList<propGroup>* properties() { return &m_propGroups; } // Circuit needs pointer bcos properties can change (ex: loadHex changes Config)

        virtual void loadProperties( QVector<propStr_t> p );

        virtual bool    setPropStr( QString prop, QString val );
        virtual QString getPropStr( QString prop );

        virtual QString toString();

        QString getUid() { return m_id; }
        void setUid( QString uid ) { m_id = uid; }

        QString itemType()  { return m_type; }
        void setItemType( QString ) {;}

        virtual bool isHidden() { return false;}

        virtual void setup(){;} // Called after all properties are set

    protected:
        QString m_id;
        QString m_type;
        QString m_help;

        PropDialog* m_propDialog;
        QList<propGroup> m_propGroups;
        QMap<QString, ComProperty*> m_propMap;
};

#endif
