/*

   Copyright( C) 1998 T. Scott Dattalo
   Copyright( C) 2006,2010,2013,2015 Roy R. Rankin

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or( at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdio>

#include "pic-ioports.h"
#include "processor.h"
#include "p16f88x.h"
#include "pir.h"
#include "14bit-tmrs.h"
#include "comparator.h"
#include "a2d_v2.h"
////#include "ctmu.h"
#include "clc.h"

//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

ComparatorModule::ComparatorModule(Processor *pCpu)
    : cmcon( pCpu,"cmcon" ),
      cmcon1( pCpu,"cmcon1" ),
      vrcon( pCpu,"vrcon" )
{
}

void ComparatorModule::initialize( PIR_SET *pir_set,
                                   PinModule *pin_vr0, PinModule *pin_cm0,
                                   PinModule *pin_cm1, PinModule *pin_cm2, PinModule *pin_cm3,
                                   PinModule *pin_cm4, PinModule *pin_cm5)
{
    //  cmcon = new CMCON;
    cmcon.assign_pir_set(pir_set);
    cmcon.setINpin(0, pin_cm0, "an0");
    cmcon.setINpin(1, pin_cm1, "an1");
    cmcon.setINpin(2, pin_cm2, "an2");
    cmcon.setINpin(3, pin_cm3, "an3");
    cmcon.setOUTpin(0, pin_cm4);
    cmcon.setOUTpin(1, pin_cm5);
    vrcon.setIOpin(pin_vr0);
    cmcon._vrcon = &vrcon;
    vrcon._cmcon = &cmcon;
}

//--------------------------------------------------

class CMSignalSource : public SignalControl
{
public:
    CMSignalSource(CMCON *_cmcon, int _index)
        : m_state('0'), m_cmcon(_cmcon), index(_index) { }
    ~CMSignalSource() { }

    virtual void release()      { m_cmcon->releasePin(index); }
    virtual char getState()     { return m_state; }
    void putState(bool new_val) { m_state = new_val?'1':'0'; }

private:
    char m_state;
    CMCON* m_cmcon;
    int  index;
};
//--------------------------------------------------

class CMxSignalSource : public PeripheralSignalSource
{
public:
    CMxSignalSource(PinModule *_pin, CMxCON0_base *_cmcon)
        : PeripheralSignalSource(_pin),  m_cmcon(_cmcon) { }

    ~CMxSignalSource() { }
    virtual void release() { m_cmcon->releasePin(); }

private:
    CMxCON0_base * m_cmcon;
};
//--------------------------------------------------

/*CM_stimulus::CM_stimulus(CMCON * arg, const char *cPname,double _Vth, double _Zth)
    : stimulus(cPname, _Vth, _Zth)
{
    _cmcon = arg;
}

CM_stimulus::~CM_stimulus()
{
}

void   CM_stimulus::set_nodeVoltage(double v)
{
    if( nodeVoltage != v )
    {
        nodeVoltage = v;
        Dprintf(("set_nodeVoltage %s _cmcon %p %s v=%.2f\n", name().c_str(), _cmcon, _cmcon->name().c_str(), v));
        _cmcon->get();  // recalculate comparator values
    }
}*/

/*
    Setup the configuration for the comparators. Must be called
    for each comparator and each mode(CN2:CM0) that can be used.
        il1 = input Vin- when CIS == 0
        ih1 = input Vin+ when CIS == 0
        il2 = input Vin- when CIS == 1
        ih2 = input Vin+ when CIS == 1

        if input == VREF, reference voltage is used.
*/
void CMCON::set_configuration( int comp, int mode, int il1, int ih1, int il2, int ih2, int out )
{
    if( comp > cMaxComparators || comp < 1 )
    {
        cout << "CMCON::set_configuration comp=" << comp << " out of range\n";
        return;
    }
    if( mode > cMaxConfigurations)
    {
        cout << "CMCON::set_configuration mode too large\n";
        return;
    }
    m_configuration_bits[comp-1][mode] =
           ( il1 << CFG_SHIFT*4) |
           ( ih1 << CFG_SHIFT*3) |
           ( il2 << CFG_SHIFT*2) |
           ( ih2 << CFG_SHIFT) |
            out;
}

//------------------------------------------------------------------------
CMCON::CMCON( Processor *pCpu, const char *pName )
    : SfrReg( pCpu, pName ), _vrcon(0),
      pir_set(0), m_tmrl(0), m_eccpas(0)
{
    value.put(0);
    cm_input[0]=cm_input[1]=cm_input[2]=cm_input[3]=0;
    cm_output[0] = cm_output[1] = 0;
    cm_input_pin[0]=cm_input_pin[1]=cm_input_pin[2]=cm_input_pin[3]=0;
    cm_an[0] = cm_an[1] = cm_an[2] = cm_an[3] = 0;
    cm_output_pin[0]=cm_output_pin[1]=0;
    cm_source[0]=cm_source[1]=0;
    cm_source_active[0]=cm_source_active[1] = false;
}

CMCON::~CMCON()
{
    uint mode = value.get() & 0x07;

    for( int i=0; i<2; i++ )
    {
        if( cm_source[i])
        {
            int cfg = m_configuration_bits[i][mode] & CFG_MASK;

            // Our Source active so port still defined if cm_output defined,
            // set default source.
            if( cfg == i && cm_output[i] && cm_source_active[i])
                cm_output[i]->setSource(0);
            delete cm_source[i];
        }
    }
    for( int i=0; i<4; i++ )
    {
        if( cm_input_pin[i]) free(cm_input_pin[i]);
        if( cm_an[i]) free(cm_an[i]);
    }
    if( cm_output_pin[0]) free(cm_output_pin[0]);
    if( cm_output_pin[1]) free(cm_output_pin[1]);
}

void CMCON::releasePin(int i)
{
    cm_source_active[i] = false;
}

void CMCON::setINpin(int i, PinModule *newPinModule, const char *an)
{
    if( newPinModule == NULL) return;
    cm_input[i] = newPinModule;
    cm_input_pin[i] = strdup( newPinModule->getPin().name_str.c_str());
    cm_an[i] = strdup(an);
}

void CMCON::setOUTpin(int i, PinModule *newPinModule)
{
    if( newPinModule == NULL ) return;
    cm_output[i] = newPinModule;
    cm_output_pin[i] = strdup( newPinModule->getPin().name_str.c_str());
}

void CMCON::assign_pir_set(PIR_SET *new_pir_set)
{
    pir_set = new_pir_set;
}

double CMCON::comp_voltage( int ind, int invert )
{
    double Voltage;
    const char *name;

    switch(ind)
    {
    case V06:
        Voltage = 0.6;
        name = "V0.6";
        break;

    case VREF:
        Voltage = _vrcon->get_Vref();
        name = "Vref";
        break;

    case NO_IN:
        Voltage = invert ? cpu->get_Vdd() : 0.;
        name = "No_IN";
        break;

    default:
        Voltage = cm_input[ind]->getPin().get_nodeVoltage();
        name = cm_input[ind]->getPin().name_str.c_str();
        break;
    }
    if( name )        // this is just to avoid a compiler warning
    {
        Dprintf(("CMCON::comp_voltage ind=%d IN%c %.2f %s\n", ind, invert?'-':'+', Voltage, name));
    }
    return Voltage;
}

//  read the comparator inputs and set C2OUT and C1OUT
//  as required. Also drive output pins if required.

uint CMCON::get()
{
    uint cmcon_val = value.get();
    int mode = cmcon_val & 0x07;
    int i;

    for( i=0; i<2; i++ )
    {
        double Vhigh;
        double Vlow;
        bool out_true;
        int out;
        int invert_bit =( i == 0) ? C1INV : C2INV;
        int output_bit =( i == 0) ? C1OUT : C2OUT;
        int shift =( cmcon_val & CIS) ? CFG_SHIFT : CFG_SHIFT*3;


        if( (m_configuration_bits[i][mode] & CFG_MASK) != ZERO)
        {
            Vhigh = comp_voltage(
                       ( m_configuration_bits[i][mode] >> shift) & CFG_MASK,
                        cmcon_val & invert_bit);
            Vlow = comp_voltage(
                       ( m_configuration_bits[i][mode] >>( shift + CFG_SHIFT)) & CFG_MASK,
                       ( cmcon_val & invert_bit) == 0);

            if( Vhigh > Vlow) out_true =( cmcon_val & invert_bit) ? false:true;
            else              out_true =( cmcon_val & invert_bit) ? true:false;

            if( out_true) cmcon_val |= output_bit;
            else          cmcon_val &= ~output_bit;

            if( (out = m_configuration_bits[i][mode] & CFG_MASK) < 2 )
            {
                cm_source[out]->putState( out_true );
                cm_output[out]->updatePinModule();
                update();
            }
        }
        else cmcon_val &= ~output_bit; // Don't care about inputs, register value 0
    }

    if( value.get() ^ cmcon_val ) // change of state
    {
        int diff = value.get() ^ cmcon_val;

        if( m_eccpas ) // Signal ECCPAS ?
        {
            if( diff & C1OUT) m_eccpas->c1_output(cmcon_val & C1OUT);
            if( diff & C2OUT) m_eccpas->c2_output(cmcon_val & C2OUT);
        }
        if( pir_set ) // Generate interupt ?
        {
            if( diff & C1OUT) pir_set->set_c1if();
            if( diff & C2OUT) pir_set->set_c2if();
        }
    }
    if( m_tmrl ) m_tmrl->compare_gate( (cmcon_val & C1OUT) == C1OUT );

    value.put( cmcon_val );
    return( cmcon_val );
}

void CMCON::put( uint new_value )
{
    uint mode = new_value & 0x7;
    uint in_mask = 0;
    uint out_mask = 0;
    uint configuration;
    int i;

    for( i=0; i<2; i++ ) // Determine used input and output pins
    {
        configuration = m_configuration_bits[i][mode];

        if( (configuration & CFG_MASK) < 2) out_mask |=( 1 <<( configuration & CFG_MASK));

        for( int j=0; j<4; j++)
        {
            configuration >>= CFG_SHIFT;
            if( (configuration & CFG_MASK) < 6 ) in_mask |=( 1 <<( configuration & CFG_MASK));
        }
    }

    for( i=0; i<2 && cm_output[i]; i++ ) // setup outputs
    {
        if( out_mask &( 1<<i))
        {
            char name[20];
            if(  ! cm_source[i]) cm_source[i] = new CMSignalSource(this, i);

            snprintf(name, sizeof(name), "c%dout", i + 1);

            cm_output[i]->setSource(cm_source[i]);
            cm_source_active[i] = true;
        }
        else if( cm_source_active[i]) cm_output[i]->setSource(0);

    }

    for( i=0; i<4 ; i++ ) // setup inputs
    {
        if( cm_input[i] )
        {
            // rewrite GUI name as required
            if( in_mask &( 1 << i) ) cm_input[i]->AnalogReq( this, true, cm_an[i] );
            else
            {
                cm_input[i]->AnalogReq( this, false, cm_input[i]->getPin().name_str.c_str());
            }
        }
    }
    if( !cm_output[1] ) new_value &= 0x1f; // if only one comparator,  mask C2INV
    value.put(new_value);

    get();        // update comparator values
}
//------------------------------------------------------------------------

CMCON1::CMCON1(Processor *pCpu, const char *pName )
    : SfrReg(pCpu, pName ), m_tmrl(0), valid_bits(0x3)
{
}
CMCON1::~CMCON1() {}
void CMCON1::put(uint new_value)
{
    assert(m_tmrl);
    m_tmrl->set_T1GSS(new_value & T1GSS);

    value.put(new_value & valid_bits);
}
//------------------------------------------------------------------------

SRCON::SRCON(Processor *pCpu, const char *pName )
    : SfrReg(pCpu, pName )
{
    writable_bits = SR1 | SR0 | C1SEN | C2REN | FVREN;
    SR_Q = false;
}

void SRCON::put(uint new_value)
{
    // PULSR and PULSS should be only settable using bsf
    // it is not clear if these bits read anything other than 0,
    // but this is what I am assuming RRR
    if( new_value & PULSR)                  SR_Q = false;
    else if( new_value & PULSS && ! reset)  SR_Q = true;

    value.put(new_value & writable_bits);
}

CMxCON0_base::CMxCON0_base(Processor *pCpu, const char *pName, uint _cm, ComparatorModule2 *cmModule)
    : SfrReg(pCpu, pName ),
      cm_output(0),
      m_cm2con1(0), m_srcon(0),
      IntSrc(0),
      cm(_cm), m_cmModule(cmModule), cm_source(0), cm_source_active(false)
{
    value.put(0);
    cm_input[0]=cm_input[1]=cm_input[2]=cm_input[3]=cm_input[4]=0;
}

CMxCON0_base::~CMxCON0_base()
{
    if( cm_source_active && cm_output) cm_output->setSource(0);
    if( cm_source) delete cm_source;
    if( IntSrc) delete IntSrc;
}

uint CMxCON0_base::get() // evaluate inputs and determine output
{
    bool output;

    if( !is_on() )
    {
        // need to test what happens in a real device RRR
        //output = out_invert()?true:false;
        output = false;
    }
    else
    {
        double Vpos = get_Vpos();
        double Vneg = get_Vneg();
        output = output_high();
        if( fabs(Vpos - Vneg) > get_hysteresis())
        {
            output = Vpos > Vneg;
            if( out_invert()) output = !output;
        }
        Dprintf(("%s ON Vpos %.2f Vneg %.2f output %d invert %d\n", name().c_str(), Vpos, Vneg, output, out_invert()));
    }
    set_output(output);
    return(value.get());
}

//--------------------------------------------------
//      Voltage reference
//--------------------------------------------------

VRCON::VRCON(Processor *pCpu, const char *pName )
    : SfrReg(pCpu, pName ),
      vr_PinModule(0),
      pin_name(0)
{
    valid_bits = VR0|VR1|VR2|VR3|VRR|VROE|VREN;
    value.put(0);
}

VRCON::~VRCON()
{
    free( pin_name );
}

void VRCON::setIOpin( PinModule* newPinModule)
{
    if( newPinModule == NULL) return;
    vr_PinModule = newPinModule;
    pin_name = strdup( newPinModule->getPin().name_str.c_str() );
}

double VRCON::get_Vref()
{
    /// Moved to VRCON::put
    /*uint new_value = value.get();

    Vref_high = cpu->get_Vdd();
    Vref_low = 0.0;

    vr_Rhigh =( 8 +( 16 -( new_value & 0x0f))) * 2000.;
    vr_Rlow  =( new_value & 0x0f) * 2000.;
    
    if( !(new_value & VRR)) vr_Rlow += 16000.;  // High range ?

    vr_Vref =( Vref_high - Vref_low) * vr_Rlow /( vr_Rhigh + vr_Rlow) + Vref_low;*/

    return( vr_Vref );
}

void VRCON::put( uint new_value )
{
    new_value &= valid_bits;
    uint old_value = value.get();
    uint diff = new_value ^ old_value;

    if( !diff ) return;

    if( !vr_PinModule ) new_value &= ~VROE; // if no PinModule clear VROE bit
    value.put( new_value );

    if( new_value & VREN )         // Vreference enable set
    {
        ///get_Vref();
        Vref_high = cpu->get_Vdd();
        Vref_low = 0.0;

        vr_Rhigh =( 8 +( 16 -( new_value & 0x0f))) * 2000.;
        vr_Rlow  =( new_value & 0x0f) * 2000.;

        if( !(new_value & VRR)) vr_Rlow += 16000.;  // High range ?

        vr_Vref =( Vref_high - Vref_low) * vr_Rlow /( vr_Rhigh + vr_Rlow) + Vref_low;

        if( vr_PinModule )
        {
            if( new_value & VROE ) outputToPin( true ); // output voltage to pin
            else                                        // not outputing voltage to pin
            {
                if( diff & 0x2f ) _cmcon->get();   // did value of vreference change ?

                outputToPin( false );
            }
        }
        else        // output pin not defined
        {
            if( diff & 0x2f ) _cmcon->get();   // did value of vreference change ?
        }
    }
    else if( vr_PinModule  ) outputToPin( false ); // vref disable
}

void VRCON::outputToPin( bool enabled )
{
    PICComponentPin* picPin = vr_PinModule->getPin().m_picPin;

    double vddAdmit = 0;
    double gndAdmit = 0;

    if( enabled )
    {
        vddAdmit = 1/vr_Rhigh;
        gndAdmit = 1/vr_Rlow;
    }
    picPin->setExtraSource( vddAdmit, gndAdmit );
}


//--------------------------------------------------
//      Voltage reference
//--------------------------------------------------
class P16F631;

VRCON_2::VRCON_2( Processor* pCpu, const char *pName  )
    : SfrReg( pCpu, pName  )
{
    value.put(0);

    m_v06Ref = 0.0;
    m_cvRef  = 0.0;
}

VRCON_2::~VRCON_2()
{
}

void VRCON_2::put( uint new_vrcon )
{
    uint old_value = value.get();
    uint diff = new_vrcon ^ old_value;

    if( !diff ) return;

    value.put( new_vrcon );

    if( diff & VP6EN )                  // Turn 0.6 V reference on or off ?
    {
        if( new_vrcon & VP6EN ) m_v06Ref = 0.6; //vr_06v->set_Vth( 0.6 );
        else                    m_v06Ref = 0.0; //vr_06v->set_Vth( 0.0 );
    }

    if( diff &( C1VREN | C2VREN | VRR | VR3 | VR2 | VR1 | VR0) )
    {
        double vdd = cpu->get_Vdd();
        double cvr = (double)(new_vrcon & 0x0f);

        if( !(new_vrcon & VRR) ) m_cvRef = vdd/4 + cvr*vdd/32; // High range
        else                     m_cvRef = cvr*vdd/24;
    }
}

CMxCON0::CMxCON0( Processor *pCpu, const char *pName, uint _cm, ComparatorModule2* cmModule )
    : CMxCON0_base( pCpu, pName, _cm, cmModule )
{
}
CMxCON0::~CMxCON0()
{
}

void CMxCON0::put( uint new_value )
{
    uint old_value = value.get();
    uint diff =( new_value ^ old_value) & mValidBits;

    // assume masked bits are read-only
    new_value =( new_value & mValidBits) |( old_value & ~mValidBits);

    value.put(new_value);

    if( diff == 0 )
    {
        get();
        return;
    }

    if( diff & CxOE )
    {
        cm_output = m_cmModule->cmxcon1[cm]->output_pin();
        if( new_value & CxOE )
        {
            char name[20];
            if( !cm_source ) cm_source = new CMxSignalSource(cm_output, this);

            snprintf(name, sizeof(name), "c%uout", cm + 1);
            assert(cm_output);

            cm_output->setSource(cm_source);
            cm_source_active = true;
        }
        else if( cm_source_active)        // Enable output enable turned off
        {
            cm_output->setSource(0);
        }
    }
    get();
}
double CMxCON0::get_hysteresis()
{
    double ret = 0.;
    if( value.get() & CxHYS ) ret = 0.05;
    
    return ret;
}

double CMxCON0::get_Vpos() { return m_cmModule->cmxcon1[cm]->get_Vpos(); }
double CMxCON0::get_Vneg() { return m_cmModule->cmxcon1[cm]->get_Vneg(); }

void CMxCON0::set_output(bool output)
{
    uint cmxcon0 = value.get();
    bool old_out = cmxcon0 & CxOUT;

    if(output) cmxcon0 |= CxOUT;
    else       cmxcon0 &= ~CxOUT;
    
    Dprintf(("cm%u POL %d output %d cmxcon0=%x old_out %d\n", cm+1,( bool)(cmxcon0 & CxPOL), output, cmxcon0, old_out));
    value.put(cmxcon0);
    m_cmModule->set_cmout(cm, output);
    
    if( cmxcon0 & CxOE )
    {
        cm_source->putState(output?'1':'0');
        m_cmModule->cmxcon1[cm]->output_pin()->updatePinModule();
    }
    if( old_out != output ) // state change
    {
        // Positive going edge, set interrupt ?
        if( output &&( m_cmModule->cmxcon1[cm]->value.get() & CMxCON1::CxINTP) )
            IntSrc->Trigger();

        // Negative going edge, set interrupt ?
        if( !output &&( m_cmModule->cmxcon1[cm]->value.get() & CMxCON1::CxINTN) )
            IntSrc->Trigger();
    }
}

void CMxCON0_V2::put(uint new_value)
{
    uint old_value = value.get();
    uint diff =( new_value ^ old_value) & mValidBits;

    value.put(new_value);

    if( diff == 0 )// assume masked bits are read-only
    {
        get();
        return;
    }
    if(( diff & CxON) && !(new_value & CxON) ) // turning off
    {
        cm_output = m_cmModule->cmxcon1[cm]->output_pin(cm);

        cm_output->setSource(0);
        // remove stimulus from input pins
        m_cmModule->cmxcon1[0]->setPinStimulus(0, POS+cm*2);
        m_cmModule->cmxcon1[0]->setPinStimulus(0, NEG+cm*2);
        return;
    }

    if( diff & CxOE )
    {
        cm_output = m_cmModule->cmxcon1[cm]->output_pin(cm);
        if(new_value & CxOE)
        {
            char name[20];
            if(  ! cm_source) cm_source = new CMxSignalSource(cm_output, this);

            snprintf(name, sizeof(name), "c%uout", cm + 1);
            assert(cm_output);

            cm_output->setSource(cm_source);
            cm_source_active = true;
        }
        else if( cm_source_active)        // Enable output enable turned off
        {
            cm_output->setSource(0);
        }
    }
    get();
}

void CMxCON0_V2::set_output(bool output)
{
    uint cmxcon0 = value.get();
    uint cmxcon1 = m_cmModule->cmxcon1[cm]->value.get();
    bool old_out = cmxcon0 & CxOUT;

    if(output)
    {
        cmxcon0 |= CxOUT;
        cmxcon1 |=( (cm==0)? CM2CON1_V2::MC1OUT : CM2CON1_V2::MC2OUT);
    }
    else
    {
        cmxcon0 &= ~CxOUT;
        cmxcon1 &= ~((cm==0)? CM2CON1_V2::MC1OUT : CM2CON1_V2::MC2OUT);
    }
    Dprintf(("cm%u POL %d output %d cmxcon0=%x old_out %d\n", cm+1,( bool)(cmxcon0 & CxPOL), output, cmxcon0, old_out));
    value.put(cmxcon0);
    m_cmModule->cmxcon1[cm]->value.put(cmxcon1);
    m_cmModule->set_cmout(cm, output);
    if( cmxcon0 & CxOE)
    {
        cm_source->putState(output?'1':'0');
        m_cmModule->cmxcon1[cm]->output_pin(cm)->updatePinModule();
    }
    if( old_out != output) // state change
    {
        m_cmModule->cmxcon1[cm]->tmr_gate(cm, output);
        // Positive going edge, set interrupt ?
        if( output)
            IntSrc->Trigger();
    }
}

double CMxCON0_V2::get_hysteresis()
{
    double hyst_volt = 0.;

    if( m_cmModule->cmxcon1[cm]->hyst_active(cm) )
    {
        hyst_volt =  0.05; // assume 50 mv hysteresis
    }

    return hyst_volt;
}

CMxCON0_V2::CMxCON0_V2(Processor *pCpu, const char *pName , uint _cm, ComparatorModule2 *cmModule)
    : CMxCON0_base(pCpu, pName , _cm, cmModule)
{
}

CMxCON0_V2::~CMxCON0_V2()
{
}

double CMxCON0_V2::get_Vpos()
{
    return m_cmModule->cmxcon1[cm]->get_Vpos(cm, value.get());
}

double CMxCON0_V2::get_Vneg()
{
    return m_cmModule->cmxcon1[cm]->get_Vneg(cm, value.get());
}

//-------------------------------------------------------
//-------------------------------------------------------

CM2CON1_V4::CM2CON1_V4( Processor* pCpu, const char* pName , uint _cm, ComparatorModule2* cmModule )
    : CM2CON1_V3( pCpu, pName , _cm, cmModule )
    , m_vrcon(0)
{
}

CM2CON1_V4::~CM2CON1_V4()
{
}

void CM2CON1_V4::put( uint new_value )
{
    value.put( new_value & mValidBits );

    if( m_cmModule->tmr1l[0] )
        m_cmModule->tmr1l[0]->set_T1GSS((new_value & T1GSS) == T1GSS);
}

double CM2CON1_V4::get_Vpos( uint cm, uint cmxcon0 )
{
    double Voltage = 0.0;

    assert( m_vrcon );

    if( cmxcon0 & CMxCON0_V2::CxR) // use Vref defined in cm2con1
    {
        if( (cm == 0 &&( m_vrcon->value.get() & VRCON_2::C1VREN))
         || (cm == 1 &&( m_vrcon->value.get() & VRCON_2::C2VREN)) )
        {
            Voltage = m_vrcon->m_cvRef; //Voltage = cpu->CVREF->get_nodeVoltage();
        }
        else Voltage = m_vrcon->m_v06Ref; //cpu->V06REF->get_nodeVoltage();
    }
    else                // use CM1IN+ or CM2IN+
    {
        if( !stimulus_pin[POS]) setPinStimulus(cm_inputPos[cm], POS);

        Voltage = cm_inputPos[cm]->getPin().get_nodeVoltage();
    }
    return Voltage;
}

//-------------------------------------------------------
//-------------------------------------------------------

void CM2CON1_V3::put(uint new_value)
{
    uint old_value = value.get();

    value.put(new_value & mValidBits);
    if( (new_value ^ old_value) & C1RSEL) m_cmModule->cmxcon0[0]->get();
    if( (new_value ^ old_value) & C2RSEL) m_cmModule->cmxcon0[1]->get();

    if( m_cmModule->tmr1l[0]) m_cmModule->tmr1l[0]->set_T1GSS((new_value & T1GSS) == T1GSS);
}

double CM2CON1_V3::get_Vpos( uint cm, uint cmxcon0 )
{
    double Voltage = 0.0;
    uint cmxcon1 = value.get();

    assert( m_vrcon );

    if( cmxcon0 & CMxCON0_V2::CxR) // use Vref defined in cm2con1
    {
        if( (cm == 0 &&( cmxcon1 & C1RSEL)) |
               ( cm == 1 &&( cmxcon1 & C2RSEL)))
        {
            Voltage =  m_vrcon->get_Vref();
            Dprintf(("%s cm%u Vref %.2f\n", __FUNCTION__, cm+1, Voltage));
        }
        else
        {
            Voltage =  0.6;
            Dprintf(("%s cm%u Absref %.2f\n", __FUNCTION__, cm+1, Voltage));
        }
    }
    else                // use CM1IN+ or CM2IN+
    {
        if( stimulus_pin[POS] != cm_inputPos[cm]) setPinStimulus(cm_inputPos[cm], POS);

        Voltage =  cm_inputPos[cm]->getPin().get_nodeVoltage();
        Dprintf(("%s cm%u %s %.2f\n", __FUNCTION__, cm+1, cm_inputPos[cm]->getPin().name().c_str(), Voltage));
    }
    return Voltage;
}

double CM2CON1_V3::get_Vneg(uint cm, uint cmxcon0)
{
    uint cxNchan = cmxcon0 &( CMxCON0_V2::CxCH0 | CMxCON0_V2::CxCH1);
    if( stimulus_pin[NEG] != cm_inputNeg[cxNchan]) setPinStimulus(cm_inputNeg[cxNchan], NEG);

    Dprintf(("%s cm%u pin %u %s %.2f\n", __FUNCTION__, cm+1, cxNchan, cm_inputNeg[cxNchan]->getPin().name().c_str(), cm_inputNeg[cxNchan]->getPin().get_nodeVoltage()));
    return cm_inputNeg[cxNchan]->getPin().get_nodeVoltage();
}

//*************************************************************
// CM2CON1_V2

void CM2CON1_V2::put(uint new_value)
{
    uint old_value = value.get();
    new_value &= mValidBits;
    uint diff = old_value ^ new_value;

    value.put(new_value);
    if( diff &( C1RSEL | C1HYS)) m_cmModule->cmxcon0[0]->get();
    if( diff &( C2RSEL | C2HYS)) m_cmModule->cmxcon0[1]->get();
}
void CM2CON1_V3::tmr_gate(uint cm, bool output)
{
    if( cm == 1 && m_cmModule->tmr1l[0]) //CM2
    {
        Dprintf(("CM2CON1_V3::tmr_gate cm%u output=%d\n", cm+1, output));
        m_cmModule->tmr1l[0]->compare_gate(output);
    }
}

CM2CON1_V2::CM2CON1_V2(Processor *pCpu, const char *pName, ComparatorModule2 * cmModule):
    CMxCON1_base(pCpu, pName , 0, cmModule),
    ////ctmu_stim(0),
    ctmu_attached(false)
{
    assert(m_cmModule->cmxcon0[1]);

    ctmu_stimulus_pin = 0;
}

CM2CON1_V2::~CM2CON1_V2()
{
}

double CM2CON1_V2::get_Vpos(uint cm, uint cmxcon0)
{
    double Voltage = 0.0;
    uint cmxcon1 = value.get();

    if( cmxcon0 & CMxCON0_V2::CxR) // use Vref defined in cm2con1
    {
        if(( cm == 0 &&( cmxcon1 & C1RSEL))|( cm == 1 &&( cmxcon1 & C2RSEL)))
        {
            Voltage =  m_cmModule->FVR_voltage;
            Dprintf(("%s cm%u FVR %.2f\n", __FUNCTION__, cm+1, Voltage));
        }
        else
        {
            Voltage =  m_cmModule->DAC_voltage;
            Dprintf(("%s cm%u DAC %.2f\n", __FUNCTION__, cm+1, Voltage));
        }
    }
    else                // use CM1IN+ or CM2IN+
    {
        if( stimulus_pin[POS+cm*2] != cm_inputPos[cm]) setPinStimulus(cm_inputPos[cm], POS+cm*2);

        Voltage =  cm_inputPos[cm]->getPin().get_nodeVoltage();
        Dprintf(("%s cm%u %s %.2f\n", __FUNCTION__, cm+1, cm_inputPos[cm]->getPin().name().c_str(), Voltage));
    }
    return Voltage;
}

double CM2CON1_V2::get_Vneg( uint cm, uint cmxcon0 )
{
    uint cxNchan = cmxcon0 &( CMxCON0_V2::CxCH0 | CMxCON0_V2::CxCH1);
    if( stimulus_pin[NEG+cm*2] != cm_inputNeg[cxNchan])
        setPinStimulus( cm_inputNeg[cxNchan], NEG+cm*2 );

     ////if( cm_inputNeg[cxNchan]->getPin().snode)
       ////  cm_inputNeg[cxNchan]->getPin().snode->update();

    Dprintf(("%s cm%u pin %u %s %.2f\n", __FUNCTION__, cm+1, cxNchan, cm_inputNeg[cxNchan]->getPin().name().c_str(), cm_inputNeg[cxNchan]->getPin().get_nodeVoltage()));
    return cm_inputNeg[cxNchan]->getPin().get_nodeVoltage();
}

bool CM2CON1_V2::hyst_active(uint cm)
{
    bool hyst = false;

    if     ( cm == 0) hyst = value.get() & C1HYS;
    else if( cm == 1) hyst = value.get() & C2HYS;

    return hyst;
}
void CM2CON1_V2::tmr_gate( uint cm, bool output)
{
    Dprintf(("CM2CON1_V2::tmr_gate cm%u output %d\n", cm+1, output));
    for( int i=0; i<3; i++ )
    {
        if( m_cmModule->t1gcon[i])
        {
            if     ( cm == 0) m_cmModule->t1gcon[i]->CM1_gate(output); // CM1
            else if( cm == 1) m_cmModule->t1gcon[i]->CM2_gate(output); //CM2
        }
    }
}
////
/*void CM2CON1_V2::set_ctmu_stim( stimulus*_ctmu_stim, CTMU *_ctmu_module )
{
    if( _ctmu_stim)
    {
        if( !m_cmModule->ctmu_module) m_cmModule->ctmu_module = _ctmu_module;
        ctmu_stim = _ctmu_stim;
        attach_ctmu_stim();
    }
    else
    {
        detach_ctmu_stim();
        ctmu_stim = 0;
    }
}*/

void CM2CON1_V2::attach_ctmu_stim()
{
    if( !cm_inputNeg[1])
    {
        fprintf(stderr, "ERROR CM2CON1_V2::attach_ctmu_stim C12IN1- not defined\n");
        return;
    }
     ////if( !(cm_inputNeg[1]->getPin().snode))
    {
         ////printf("Warning CM2CON1_V2::attach_ctmu_stim %s has no node attached CTMU will not work properly\n", cm_inputNeg[1]->getPin().name_str.c_str());
        return;
    }
    ////if( ctmu_stim)
    {
         ////cm_inputNeg[1]->getPin().snode->attach_stimulus(ctmu_stim);
         ////cm_inputNeg[1]->getPin().snode->update();
        ////ctmu_attached = true;
    }
}

void CM2CON1_V2::detach_ctmu_stim()
{
    if( ctmu_attached )
    {
         ////cm_inputNeg[1]->getPin().snode->detach_stimulus(ctmu_stim);
         ////cm_inputNeg[1]->getPin().snode->update();
        ctmu_attached = false;
    }
}

CMxCON1::CMxCON1(Processor *pCpu, const char *pName , uint _cm, ComparatorModule2 *cmModule)
    : CMxCON1_base(pCpu, pName , _cm, cmModule)
{
}

CMxCON1::~CMxCON1()
{
}

CMxCON1_base::CMxCON1_base(Processor *pCpu, const char *pName , uint _cm, ComparatorModule2 *cmModule)
    : SfrReg(pCpu, pName ),
      cm(_cm), m_cmModule(cmModule)
{
    assert(m_cmModule->cmxcon0[cm]);
    ////cm_stimulus[NEG] = new CM_stimulus((CMCON *)m_cmModule->cmxcon0[cm], "cm_stimulus_-", 0, 1e12);
    ////cm_stimulus[POS] = new CM_stimulus((CMCON *)m_cmModule->cmxcon0[cm], "cm_stimulus_+", 0, 1e12);

    for( int i=0; i<5; i++ ) cm_inputNeg[i] = 0;
    
    for( int i=0; i<2; i++ )
    {
        stimulus_pin[i] = 0;
        stimulus_pin[i+2] = 0;
        cm_inputPos[i] = 0;
        cm_output[i] = 0;
    }
    ctmu_stimulus_pin = 0;
}

CMxCON1_base::~CMxCON1_base()
{
    ////delete cm_stimulus[NEG];
    ////delete cm_stimulus[POS];
}

double CMxCON1::get_Vneg( uint arg, uint arg2 )
{
    uint cxNchan = value.get() & CxNMASK;
    if( !stimulus_pin[NEG]) setPinStimulus(cm_inputNeg[cxNchan], NEG);
     ////if( cm_inputNeg[cxNchan]->getPin().snode)
      ////   cm_inputNeg[cxNchan]->getPin().snode->update();

    Dprintf(("%s pin %u %s %.2f\n", __FUNCTION__, cxNchan, cm_inputNeg[cxNchan]->getPin().name().c_str(), cm_inputNeg[cxNchan]->getPin().get_nodeVoltage()));
    return cm_inputNeg[cxNchan]->getPin().get_nodeVoltage();
}

double CMxCON1::get_Vpos( uint arg, uint arg2 )
{
    uint cxPchan = (value.get() & CxPMASK) >> 3;
    double        Voltage;

    switch( cxPchan )
    {
    case 0:
        if( stimulus_pin[POS] != cm_inputPos[cxPchan])
            setPinStimulus(cm_inputPos[cxPchan], POS);
        Voltage = cm_inputPos[cxPchan]->getPin().get_nodeVoltage();
        Dprintf(("%s %s %s v=%.2f\n", name().c_str(), __FUNCTION__, cm_inputPos[cxPchan]->getPin().name().c_str(), Voltage));
        break;

    case 2:
        Voltage = m_cmModule->DAC_voltage;
        Dprintf(("%s %s %s v=%.2f\n", name().c_str(), __FUNCTION__, "DAC", Voltage));
        break;

    case 4:
        Voltage = m_cmModule->FVR_voltage;
        Dprintf(("%s %s %s v=%.2f\n", name().c_str(), __FUNCTION__, "FVR", Voltage));
        break;

    default:
        printf("CMxCON1::get_Vpos unexpected Pchan %x\n", cxPchan);
    case 6:
        Voltage = 0.;
        Dprintf(("%s %s %s v=%.2f\n", name().c_str(), __FUNCTION__, "AGND", Voltage));
        break;
    }
    return Voltage;
}

// Attach a stimulus to an input pin so that changes
// in the pin voltage can be reflected in the comparator output.
//
// pin may be 0 in which case a current stimulus, if any, will be detached
// pol is either the enum POS or NEG
//
void CMxCON1_base::setPinStimulus( PinModule* pin, int pol )
{
    if( pin == stimulus_pin[pol] ) return;

    ////
    /*if( stimulus_pin[pol])
    {
       ( stimulus_pin[pol]->getPin().snode)->detach_stimulus(cm_stimulus[pol]);
        stimulus_pin[pol] = 0;
    }
    if( pin && pin->getPin().snode)
    {
        stimulus_pin[pol] = pin;
       ( stimulus_pin[pol]->getPin().snode)->attach_stimulus(cm_stimulus[pol]);
    }*/
}

void CMxCON1::put(uint new_value)
{
    uint old_value = value.get();
    new_value &= mValidBits;
    uint diff = old_value ^ new_value;

    value.put(new_value);

    if( (diff & CxNMASK) || !stimulus_pin[NEG])
    {
        uint cxNchan = new_value & CxNMASK;

        setPinStimulus( cm_inputNeg[cxNchan], NEG );

    }
    if( (diff & CxPMASK) || !stimulus_pin[POS])
    {
        uint cxPchan =( new_value & CxPMASK) >> 3;

        if( cxPchan == 0) setPinStimulus(cm_inputPos[cxPchan], POS);
        else if( stimulus_pin[POS]) setPinStimulus(0, POS);
    }
    m_cmModule->run_get(cm);
}

void CMxCON1_base::set_OUTpin( PinModule *pin_cm0, PinModule *pin_cm1 )
{
    cm_output[0] = pin_cm0;
    cm_output[1] = pin_cm1;
}

void CMxCON1_base::set_INpinNeg(PinModule *pin_cm0, PinModule *pin_cm1, PinModule *pin_cm2,  PinModule *pin_cm3,  PinModule *pin_cm4)
{
    cm_inputNeg[0] = pin_cm0;
    cm_inputNeg[1] = pin_cm1;
    cm_inputNeg[2] = pin_cm2;
    cm_inputNeg[3] = pin_cm3;
    cm_inputNeg[4] = pin_cm4;
}

void CMxCON1_base::set_INpinPos(PinModule *pin_cm0, PinModule *pin_cm1)
{
    cm_inputPos[0] = pin_cm0;
    cm_inputPos[1] = pin_cm1;
}

ComparatorModule2::ComparatorModule2(Processor *pCpu)
{
    for(int i = 0; i < 4; i++)
    {
        cmxcon0[i] = 0;
        cmxcon1[i] = 0;
    }
    cmout = 0;
    t1gcon[0] = t1gcon[1] = t1gcon[2] = 0;
    tmr1l[0] = tmr1l[1] = tmr1l[2] = 0;
    eccpas[0] = eccpas[1] = eccpas[2] = 0;
    sr_module = 0;
    ctmu_module = 0;
    for( int i=0; i<4; i++ ) m_clc[i] = 0;
}

ComparatorModule2::~ComparatorModule2()
{
    for( int i = 0; i < 4; i++ )
    {
        if( cmxcon0[i]) delete cmxcon0[i];
        if( cmxcon1[i]) delete cmxcon1[i];
        
        if( i < 3 && cmxcon1[i] == cmxcon1[i+1])
            cmxcon1[i+1] = 0;
    }
    if( cmout) delete cmout;
}

// this function sets the bits in the CMOUT register and also
// sends the state to the T1GCON class if t1gcon is defined
//
void ComparatorModule2::set_cmout(uint bit, bool value)
{
    int i;

    if( cmout)
    {
        if( value ) cmout->value.put(cmout->value.get() |( 1<<bit));
        else        cmout->value.put(cmout->value.get() &~(1<<bit));
    }

    for( i=0; i<4; i++)
        if( m_clc[i]) m_clc[i]->CxOUT_sync(value, bit);

    switch(bit)
    {
    case 0:                //CM1
        for(i=0; i < 3; i++)
        {
            if( t1gcon[i]) t1gcon[i]->CM1_gate(value);
            if( eccpas[i]) eccpas[i]->c1_output(value);
        }
        if( sr_module) sr_module->syncC1out(value);
        break;

    case 1:                //CM2
        for(i=0; i < 3; i++)
        {
            if( t1gcon[i]) t1gcon[i]->CM2_gate(value);
            if( eccpas[i]) eccpas[i]->c2_output(value);
        }
        if( sr_module)   sr_module->syncC2out(value);
        ////if( ctmu_module) ctmu_module->syncC2out(value);
        break;

    default:        //Do nothing other CMs
        break;
    }
}

void ComparatorModule2::set_DAC_volt(double _volt)
{
    DAC_voltage = _volt;
    for( int i=0; i < 4; i++) if( cmxcon0[i]) cmxcon0[i]->get();
}

void ComparatorModule2::set_FVR_volt(double _volt)
{
    FVR_voltage = _volt;
    Dprintf(("ComparatorModule2::set_FVR_volt %.2f\n", FVR_voltage));

    for( int i=0; i < 4; i++) if( cmxcon0[i]) cmxcon0[i]->get();
}

void ComparatorModule2::set_if( uint cm ) // set interrupt for comparator cm
{
    switch(cm)
    {
    case 0:
        pir_set->set_c1if();
        break;

    case 1:
        pir_set->set_c2if();
        break;

    case 2:
        pir_set->set_c3if();
        break;

    case 3:
        pir_set->set_c4if();
        break;
    }
}
