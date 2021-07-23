/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2016 by santiago González                               *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "op_amp.h"
#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "iopin.h"

static const char* OpAmp_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Power Pins")
};

Component* OpAmp::construct( QObject* parent, QString type, QString id )
{
        return new OpAmp( parent, type, id );
}

LibraryItem* OpAmp::libraryItem()
{
    return new LibraryItem(
        tr( "OpAmp" ),
        tr( "Active" ),
        "opamp.png",
        "opAmp",
        OpAmp::construct );
}

OpAmp::OpAmp( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id )
{
    Q_UNUSED( OpAmp_properties );
    
    m_area = QRect( -18, -8*2, 36, 8*2*2 );
    setLabelPos(-16,-32, 0);
    
    m_pin.resize( 5 );

    m_inputP = new IoPin( 180, QPoint(-16-8,-8), id+"-inputNinv", 0, this, input );
    m_pin[0] = m_inputP;
    m_pin[0]->setLabelText( "+" );
    m_pin[0]->setLabelColor( QColor( 0, 0, 0 ) );

    m_inputN = new IoPin( 180, QPoint(-16-8, 8), id+"-inputInv", 1, this, input );
    m_pin[1] = m_inputN;
    m_pin[1]->setLabelText( " -" );
    m_pin[1]->setLabelColor( QColor( 0, 0, 0 ) );

    m_output = new IoPin(   0, QPoint( 16+8, 0), id+"-output",   2, this, source );
    m_pin[2] = m_output;

    m_pin[3] = new Pin(  90, QPoint(0,-16), id+"-powerPos", 3, this );
    m_pin[3]->setLabelText( "+" );
    m_pin[3]->setLabelColor( QColor( 0, 0, 0 ) );
    m_pin[4] = new Pin( 270, QPoint(0, 16), id+"-powerNeg", 4, this );
    m_pin[4]->setLabelText( "-" );
    m_pin[4]->setLabelColor( QColor( 0, 0, 0 ) );
    
    setPowerPins( false );

    m_gain = 1000;
    m_outImp = cero_doub;
    m_voltPosDef = 5;
    m_voltNegDef = 0;
}
OpAmp::~OpAmp(){}

QList<propGroup_t> OpAmp::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Gain", tr("Gain"),""} );
    mainGroup.propList.append( {"Out_Imped", tr("Output Impedance"),"Ω"} );

    propGroup_t supGroup { tr("Supply") };
    supGroup.propList.append( {"Volt_Pos", tr("V+"),"V"} );
    supGroup.propList.append( {"Volt_Neg", tr("V-"),"V"} );
    supGroup.propList.append( {"Power_Pins", tr("Supply Pins"),""} );
    return {mainGroup, supGroup};
}

void OpAmp::initialize()
{
    m_accuracy = Simulator::self()->NLaccuracy();

    m_lastOut = 0;
    m_lastIn  = 0;
    m_k = 1e-6/m_gain;
    m_firstStep = true;
}

void OpAmp::stamp()
{
    if( m_inputP->isConnected() ) m_inputP->getEnode()->addToNoLinList(this);
    if( m_inputN->isConnected() ) m_inputN->getEnode()->addToNoLinList(this);
    if( m_output->isConnected() ) m_output->getEnode()->addToNoLinList(this);
}

void OpAmp::voltChanged() // Called when any pin node change volt
{
    if( m_powerPins )
    {
        m_voltPos = m_pin[3]->getVolt();
        m_voltNeg = m_pin[4]->getVolt();
    }
    else
    {
        m_voltPos = m_voltPosDef;
        m_voltNeg = m_voltNegDef;
    }
    double vd = m_inputP->getVolt()-m_inputN->getVolt();
    if( m_firstStep && fabs(m_lastIn-vd) < m_accuracy )
    {
        m_converged = true;
        m_firstStep = true;
        return;
    }

    double out = vd * m_gain;
    if     ( out > m_voltPos ) out = m_voltPos;
    else if( out < m_voltNeg ) out = m_voltNeg;

    if( fabs(out-m_lastOut) < m_accuracy )
    {
        m_converged = true;
        m_firstStep = true;
        return;
    }
    m_converged = false;

    if( m_firstStep )                  // First step after a convergence
    {
        double dOut = -1e-6;           // Do a tiny step to se what happens
        if( vd>0 ) dOut = 1e-6;

        out = m_lastOut + dOut;
        m_firstStep = false;
    }
    else {
        if( m_lastIn != vd ) // We problably are in a close loop configuration
        {
            double dIn  = fabs(m_lastIn-vd); // Input diff with last step
            out = (m_lastOut*dIn + vd*1e-6)/(dIn + m_k); // Guess next converging output:
        }
        m_firstStep = true;
    }
    if     ( out >= m_voltPos ) out = m_voltPos;
    else if( out <= m_voltNeg ) out = m_voltNeg;

    m_lastIn  = vd;
    m_lastOut = out;

    m_output->stampCurrent( out/m_outImp );
}

void OpAmp::setOutImp( double imp )
{
    if( imp < cero_doub ) imp = cero_doub;
    m_outImp = imp;
    m_output->setImp( imp );
}

void OpAmp::setPowerPins( bool set )
{
    m_pin[3]->setEnabled( set );
    m_pin[3]->setVisible( set );
    m_pin[4]->setEnabled( set );
    m_pin[4]->setVisible( set );
    
    m_powerPins = set;
}

QPainterPath OpAmp::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    
    points << QPointF(-16,-16 )
           << QPointF(-16, 16 )
           << QPointF( 16,  1 )
           << QPointF( 16, -1 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void OpAmp::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(2);
    p->setPen(pen);

    static const QPointF points[4] = {
        QPointF(-16,-16 ),
        QPointF(-16, 16 ),
        QPointF( 16,  1 ),
        QPointF( 16, -1 )            };

    p->drawPolygon(points, 4);
}

#include "moc_op_amp.cpp"
