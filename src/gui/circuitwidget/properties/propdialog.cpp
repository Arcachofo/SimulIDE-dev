/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "propdialog.h"
#include "component.h"
#include "circuit.h"
#include "labelval.h"
//#include "propval.h"

#include "numval.h"
#include "stringval.h"
#include "textval.h"
#include "enumval.h"
#include "boolval.h"
#include "mainwindow.h"

#include "comproperty.h"

PropDialog::PropDialog( QWidget* parent, QString help )
          : QDialog( parent )
{
    setupUi( this );
    //this->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );

    m_component = NULL;

    m_scale = MainWindow::self()->fontScale();
    m_minW  = 350*m_scale;
    m_minH  = 100*m_scale;

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

    int w=0, index=0;
    QList<propGroup>* groups = comp->properties();

    for( propGroup group : *groups )
    {
        if( group.flags & groupHidden ) continue;

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
                QString type = prop->type();
                PropVal* mp = NULL;

                if     ( type == "double"  ) mp = new NumVal( this, comp, prop );
                else if( type == "uint"    ) mp = new NumVal( this, comp, prop );
                else if( type == "int"     ) mp = new NumVal( this, comp, prop );
                else if( type == "string"  ) mp = new StringVal( this, comp, prop );
                else if( type == "textEdit") mp = new TextVal( this, comp, prop );
                else if( type == "enum"    ) mp = new EnumVal( this, comp, prop );
                else if( type == "bool"    ) mp = new BoolVal( this, comp, prop );

                if( !mp ) continue;

                mp->setup();
                w = mp->width();
                mp->setMinimumWidth( w );
                m_propList.append( mp );
                propWidget->layout()->addWidget( mp );
            }
            propWidget->setMinimumHeight( propList.size()*30);
            propWidget->setMinimumWidth( w+40 );
            tabList->addTab( propWidget, group.name );
    }   }
    if( tabList->count() == 0 ) tabList->setVisible( false ); // Hide tab widget if empty
    adjustWidgets();
}

void PropDialog::showProp( QString name, bool show )
{
    for( PropVal* prop : m_propList )
    {
        if( prop->propName() != name ) continue;
        prop->setHidden( !show );
        break;
    }
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
    updtValues();
    adjustWidgets();
}

void PropDialog::on_helpButton_clicked()
{
    m_helpExpanded = !m_helpExpanded;
    if( m_helpExpanded ) mainLayout->addWidget( helpText );
    else                 mainLayout->removeWidget( helpText );
    helpText->setVisible( m_helpExpanded );

    adjustWidgets();
}

void PropDialog::adjustWidgets()
{
    int h = 0;
    int w = 0;
    QWidget* widget = tabList->currentWidget();
    if( widget )
    {
        h = widget->minimumHeight()+150*m_scale;
        w = widget->minimumWidth()+25*m_scale;
    }
    if( h < m_minH ) h = m_minH;
    if( w < m_minW ) w = m_minW;

    if( helpText->isVisible() )
    {
        helpText-> setFixedWidth( helpText->width() );
        w += helpText->width()+6;
    }
    this->setMinimumHeight( h );
    this->setMaximumHeight( h+100 );

    this->setMinimumWidth( w );
    this->setMaximumWidth( w+50 );

    this->adjustSize();
}

void PropDialog::updtValues()
{
    for( PropVal* prop : m_propList ) prop->updtValues();
    m_component->updtValues();
}

void PropDialog::changed()
{
    Circuit::self()->setChanged();
}
