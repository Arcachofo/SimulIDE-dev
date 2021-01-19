/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "propdialog.h"
#include "component.h"
#include "doubleval.h"
#include "intval.h"
#include "boolval.h"
#include "stringval.h"
#include "enumval.h"
#include "labelval.h"
#include "colorval.h"

PropDialog::PropDialog( QWidget* parent )
          : QDialog( parent )
{
    setupUi(this);
    this->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );

    m_component = NULL;

    helpText->setVisible( false );
    mainLayout->removeWidget( helpText );
    //this->adjustSize();
}

void PropDialog::setComponent( Component* comp )
{
    this->setWindowTitle( "Uid: "+comp->objectName() );
    type->setText( "Type: "+comp->itemType() );
    labelBox->setText( comp->idLabel() );
    tabList->clear();
    m_component = comp;
    showLabel->setChecked( comp->showId() );

    int index = 0;
    QList<propGroup_t> groups = comp->propGroups();

    for( propGroup_t group : groups )
    {
        QList<property_t> propList = group.propList;
        if( !propList.isEmpty() )
        {
            index++;
            QWidget* propWidget = new QWidget( tabList );
            propWidget->setLayout( new QVBoxLayout( propWidget ));

            for( property_t prop : propList )
            {
                QString propName = prop.name;
                QString unit     = prop.unit;

                const char* name = propName.toStdString().c_str();

                if( propName == "")
                {
                    LabelVal* mp = new LabelVal( this );
                    mp->setLabelVal( prop.caption );
                    propWidget->layout()->addWidget( mp );
                    continue;
                }
                QVariant value = comp->property( name );
                if( !value.isValid() ) continue;

                QVariant::Type type = value.type();

                if( unit == "enum")
                {
                    EnumVal* mp = new EnumVal( this );
                    mp->setPropName( propName, prop.caption );
                    mp->setup( comp );
                    propWidget->layout()->addWidget( mp );
                }
                else if( type == QVariant::Double )
                {
                    DoubleVal* mp = new DoubleVal( this );
                    mp->setPropName( propName, prop.caption );
                    mp->setup( comp, unit );
                    propWidget->layout()->addWidget( mp );
                }
                else if( type == QVariant::Int
                      || type == QVariant::ULongLong  )
                {
                    IntVal* mp = new IntVal( this );
                    mp->setPropName( propName, prop.caption );
                    mp->setup( comp, unit );
                    propWidget->layout()->addWidget( mp );
                }
                else if( type == QVariant::Bool )
                {
                    BoolVal* mp = new BoolVal( this );
                    mp->setPropName( propName, prop.caption );
                    mp->setup( comp );
                    propWidget->layout()->addWidget( mp );
                }
                else if( type == QVariant::String )
                {
                    StringVal* mp = new StringVal( this );
                    mp->setPropName( propName, prop.caption );
                    mp->setup( comp );
                    propWidget->layout()->addWidget( mp );
                }
                else if( type == QVariant::Color )
                {
                    ColorVal* mp = new ColorVal( this );
                    mp->setPropName( propName, prop.caption );
                    mp->setup( comp );
                    propWidget->layout()->addWidget( mp );
                }
            }
            tabList->addTab( propWidget, group.name );
        }
    }
    if( tabList->count() == 0 ) tabList->setVisible( false ); // Hide tab widget if empty
    this->adjustSize();
}

void PropDialog::on_labelBox_editingFinished()
{
    m_component->setIdLabel( labelBox->text() );
}

void PropDialog::on_showLabel_toggled( bool checked )
{
    m_component->setShowId( checked );
}

void PropDialog::on_tabList_currentChanged(int)
{
    if( !m_component ) return;
    this->adjustSize();
}
