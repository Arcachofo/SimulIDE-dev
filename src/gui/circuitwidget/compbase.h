/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef COMPBASE_H
#define COMPBASE_H

#include <QObject>
#include <QString>
#include <QPointF>
#include <QHash>

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
class QDomElement;
class QGraphicsItemGroup;

class MAINMODULE_EXPORT CompBase : public QObject
{
    public:
        CompBase( QObject* parent, QString type, QString id );
        ~CompBase();

        void addPropGroup( propGroup pg, bool list=true );
        void remPropGroup( QString name );
        void addProperty( QString group, ComProperty* p );
        void removeProperty( QString prop );
        QList<propGroup>* properties() { return &m_propGroups; } // Circuit needs pointer bcos properties can change (ex: loadHex changes Config)

        virtual QStringList getEnumUids( QString )  { return m_enumUids; }
        virtual QStringList getEnumNames( QString ) { return m_enumNames; }
        int getEnumIndex( QString prop );

        virtual bool    setPropStr( QString prop, QString val );
        virtual QString getPropStr( QString prop );

        virtual QString toString();

        QString getUid() { return m_id; }
        void setUid( QString uid ) { m_id = uid; }

        QString itemType()  { return m_type; }
        void setItemType( QString ) {;}

        virtual void setValLabelText( QString ){;}

        virtual QString showProp(){return "";}
        virtual void setShowProp( QString ){;}

    protected:
        QString m_id;
        QString m_type;

        bool m_isMainComp;

        QGraphicsItemGroup* m_group;

        PropDialog* m_propDialog;
        QList<propGroup> m_propGroups;
        QHash<QString, ComProperty*> m_propHash;

        QStringList m_enumUids;
        QStringList m_enumNames;
};

#endif
