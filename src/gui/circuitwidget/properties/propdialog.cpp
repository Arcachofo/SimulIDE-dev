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
#include "labelval.h"
#include "propval.h"

#include "comproperty.h"

PropDialog::PropDialog( QWidget* parent, QString help )
          : QDialog( parent )
{
    setupUi( this );
    //this->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );

    m_component = NULL;

    m_helpExpanded = false;
    helpText->setVisible( false );
    mainLayout->removeWidget( helpText );
    helpText->setText( help );
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
    QList<propGroup> groups = comp->propeties();

    for( propGroup group : groups )
    {
        if( (group.name.startsWith("Comp"))
         || (group.name == "Board")
         || (group.name.startsWith("Hidden") ) ) continue;

        QList<ComProperty*> propList = group.propList;
        if( !propList.isEmpty() )
        {
            index++;
            QWidget* propWidget = new QWidget( tabList );
            propWidget->setLayout( new QVBoxLayout( propWidget ));
            propWidget->layout()->setSpacing( 9 );
            propWidget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

            for( ComProperty* prop : propList )
            {
                if( prop->name() == "" ) // Just a label
                {
                    LabelVal* mp = new LabelVal( this );
                    mp->setLabelVal( prop->capt() );
                    propWidget->layout()->addWidget( mp );
                    continue;
                }
                PropVal* mp = prop->getWidget( this );
                if( !mp ) continue;

                mp->setup();
                m_propList.append( mp );
                propWidget->layout()->addWidget( mp );
            }
            tabList->addTab( propWidget, group.name );
    }   }
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

void PropDialog::on_tabList_currentChanged( int )
{
    if( !m_component ) return;

    if( !helpText->isVisible() )
    {
        QWidget* widget = tabList->currentWidget();
        this->setMaximumHeight( widget->minimumSizeHint().height()+150 );
    }
    this->adjustSize();
    this->setMaximumHeight( 800 );
}

void PropDialog::on_helpButton_clicked()
{
    m_helpExpanded = !m_helpExpanded;
    if( m_helpExpanded ) mainLayout->addWidget( helpText );
    else                 mainLayout->removeWidget( helpText );
    helpText->setVisible( m_helpExpanded );

    this->adjustSize();
}

void PropDialog::updtValues()
{ for( PropVal* prop : m_propList ) prop->updtValues(); }
