/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "infowidget.h"
#include "mainwindow.h"
#include "mcu.h"

InfoWidget* InfoWidget::m_pSelf = nullptr;

InfoWidget::InfoWidget( QWidget* parent )
          : QWidget( parent )
{
    setupUi( this );
    m_pSelf = this;

    QFont font( MainWindow::self()->defaultFontName() , 10, QFont::Bold );
    double fontScale = MainWindow::self()->fontScale();
    font.setPixelSize( 12*fontScale );

    simTimeLabel->setFont( font );
    simTime->setFont( font );
    targetSpeedLabel->setFont( font );
    targetSpeed->setFont( font );
    realSpeedLabel->setFont( font );
    realSpeed->setFont( font );
    simLoadLabel->setFont( font );
    simLoadVal->setFont( font );
    guiLoadLabel->setFont( font );
    guiLoadVal->setFont( font );
    overLoadLabel->setFont( font );
    overLoadVal->setFont( font );
    fpsLabel->setFont( font );
    fpsVal->setFont( font );
    mainMcuLabel->setFont( font );
    mainMcu->setFont( font );
    mainMcuName->setFont( font );

    overLoadLabel->setVisible( false );
    overLoadVal->setVisible( false );
}

void InfoWidget::setTargetSpeed( double s )
{
    QString speed = QString::number( s,'f', 2 )+" %";
    if( s < 100 ) speed = "0"+speed;
    if( s < 10 )  speed = "0"+speed;
    targetSpeed->setText("  "+speed );
    updtMcu();
}

void InfoWidget::updtMcu()
{
    if( Mcu::self() )
    {
        QString device = Mcu::self()->device();
        QString freq = QString::number( eMcu::self()->freq()*1e-6 );
        mainMcu->setText( "  "+device+" at "+freq+" MHz" );

        QString name;
        Mcu* mcu = Mcu::self();
        if( mcu->isMainComp() && mcu->parentItem() )
        {
            Component* comp = static_cast<Component*>( mcu->parentItem() );
            name = comp->idLabel();
        }
        else name = mcu->idLabel();
        mainMcuName->setText( "  "+name );
    }else{
        mainMcu->setText( "  ---" );
        mainMcuName->setText( "  ---" );
    }
    mainMcuLabel->setVisible( Mcu::self() );
    mainMcu->setVisible( Mcu::self() );
    mainMcuName->setVisible( Mcu::self() );
}

void InfoWidget::setRate( double rate, double simLoad, double guiLoad, int fps )
{
    updtMcu();
    if( rate < 0 )
    {
        if( rate == -1 ) realSpeed->setText( "  "+tr("Speed: Debugger") );
        else             realSpeed->setText( "  "+tr("Circuit ERROR!!!") );
    }else{
        //if( (load > 150) || (load < 0) ) load = 0;
        double speed = rate/100;
        QString Srate = QString::number( speed,'f', 2 );
        while( Srate.size() < 6 ) Srate = "0"+Srate;

        if( simLoad > 101 )
        {
            int overLoad = simLoad-100;
            simLoad = 100;

            QString Oload = QString::number( overLoad,'f', 2 );
            while( Oload.size() < 6 ) Oload = "0"+Oload;

            overLoadVal->setText( "  "+Oload+" %    ");
            overLoadLabel->setVisible( true );
            overLoadVal->setVisible( true );
        }else{
            overLoadLabel->setVisible( false );
            overLoadVal->setVisible( false );
        }
        QString Sload = QString::number( simLoad,'f', 2 );
        while( Sload.size() < 6 ) Sload = "0"+Sload;

        QString Gload = QString::number( guiLoad,'f', 2 );
        while( Gload.size() < 6 ) Gload = "0"+Gload;

        QString FPS = QString::number( fps );

        realSpeed->setText( "  "+Srate+" %" );
        simLoadVal->setText( "  "+Sload+" %    ");
        guiLoadVal->setText( "  "+Gload+" %    ");
        fpsVal->setText( "  "+FPS );
}   }

void InfoWidget::setCircTime( uint64_t tStep )
{
    double step = tStep/1e6;
    int hours = step/3600e6;
    step -= hours*3600e6;
    int mins  = step/60e6;
    step -= mins*60e6;
    int secs  = step/1e6;
    step -= secs*1e6;
    int mSecs = step/1e3;
    step -= mSecs*1e3;
    int uSecs = step;
    step -= uSecs;
    int nSecs = step*1e3;
    step -= nSecs/1e3;
    step += 1e-7;
    int pSecs = step*1e6;

    QString strH = QString::number( hours );
    if( strH.length() < 2 ) strH = "0"+strH;
    QString strM = QString::number( mins );
    if( strM.length() < 2 ) strM = "0"+strM;
    QString strS = QString::number( secs );
    if( strS.length() < 2 ) strS = "0"+strS;
    QString strMS = QString::number( mSecs );
    while( strMS.length() < 3 ) strMS = "0"+strMS;
    QString strUS = QString::number( uSecs );
    while( strUS.length() < 3 ) strUS = "0"+strUS;
    QString strNS = QString::number( nSecs );
    while( strNS.length() < 3 ) strNS = "0"+strNS;
    QString strPS = QString::number( pSecs );
    while( strPS.length() < 3 ) strPS = "0"+strPS;

    simTime->setText( strH+":"+strM+":"+strS+" s  "
                     +strMS+" ms  "+strUS+" µs  "+strNS+" ns  "+strPS+" ps " );
}
