/*
   Copyright( C) 2008,2015 Roy R Rankin
   Copyright( C) 2006 T. Scott Dattalo

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

#include "ioports.h"
#include "pic-processor.h"
#include "a2d_v2.h"

//#define p_cpu cpu

static PinModule InvalidAnInput;

//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif


ADCON0_V2::ADCON0_V2(Processor *pCpu, const char *pName)
    : SfrReg(pCpu, pName),
      adres(0), adresl(0), adcon1(0), adcon2(0), intcon(0), pir1v2(0),
      ad_state(AD_IDLE), channel_mask(15),
      ////ctmu_stim(0),
      active_stim(-1)
{
}

void ADCON0_V2::setA2DBits( uint nBits ) //  Set resolution of A2D converter
{
    m_A2DScale =( 1<<nBits) - 1;
    m_nBits = nBits;
}

void ADCON0_V2::start_conversion(void)
{
    uint64_t fc = cpu->currentCycle();

    Dprintf(("starting A/D conversion\n"));

    if(!(value.get() & ADON) )
    {
        stop_conversion();
        return;
    }

    // Get the A/D Conversion Clock Select bits
    //
    // This switch case will get the ADCS bits and set the Tad, or The A/D
    // converter clock period. Tad is the number of the oscillator periods
    //  rather instruction cycle periods.

    Tad = adcon2->get_tad();
    Tacq = adcon2->get_nacq();

    if( Tad == 0 ) // RC time source
    {
        if( cpu )
        {
            Tad =( m_RCtime*cpu->get_frequency());
            Tad = Tad < 2 ? 2 : Tad;
        }
        else Tad = 6;
    }

    if( Tacq == 0) fc += 1;    // if Tacq is 0,  go to acqusition on next clock cycle

    else  fc +=( Tacq * Tad ) / cpu->get_ClockPerInstr();

    if( ad_state != AD_IDLE )
    {
        // The A/D converter is either 'converting' or 'acquiring'
        // in either case, there is callback break that needs to be moved.

        stop_conversion();
        cpu->reassign_break( future_cycle, fc, this );
    }
    else cpu->setBreakAbs( fc, this );

    future_cycle = fc;
    ad_state = AD_ACQUIRING;
}

void ADCON0_V2::stop_conversion(void)
{
    ad_state = AD_IDLE;
}

void ADCON0_V2::put( uint new_value )
{
    uint old_value = value.get();
    // SET: Reflect it first!
    value.put(new_value);

    if( new_value & ADON ) // The A/D converter is being turned on( or maybe left on)
    {
        ////
        /*if( ctmu_stim) // deal with ctmu stimulus for channel change or ON/OFF
        {
            if( (old_value ^ new_value) &( ADON|CHS0|CHS1|CHS2|CHS3))
            {
                if     ( new_value & ADON)   attach_ctmu_stim();// A/D is on
                else if(!(new_value & ADON)) detach_ctmu_stim();// A/D is off
            }
        }*/
        if((new_value & ~old_value) & GO) // The 'GO' bit is being turned on, which is request to initiate and A/D conversion
            start_conversion();
    }
    else stop_conversion();
}
////
/*void ADCON0_V2::set_ctmu_stim( stimulus *_ctmu_stim )
{
    ctmu_stim = _ctmu_stim;
    if( value.get() & ADON ) attach_ctmu_stim();
}*/

void ADCON0_V2::put_conversion(void)
{
    double dRefSep = m_dSampledVrefHi - m_dSampledVrefLo;
    double dNormalizedVoltage;

    dNormalizedVoltage =( dRefSep>0.0) ?
               ( m_dSampledVoltage - m_dSampledVrefLo)/dRefSep : 0.0;
    dNormalizedVoltage = dNormalizedVoltage>1.0 ? 1.0 : dNormalizedVoltage;

    uint converted =( uint)(m_A2DScale*dNormalizedVoltage + 0.5);

    if(adresl)    // non-null for more than 8 bit conversion
    {
        if( adcon2->value.get() & ADCON2_V2::ADFM )
        {
            adresl->put( converted & 0xff );
            adres->put( (converted >> 8) & 0x3 );
        }
        else
        {
            adresl->put( (converted << 6) & 0xc0);
            adres->put( (converted >> 2) & 0xff);
        }
    }
    else  adres->put((converted ) & 0xff);
}

// ADCON0_V2 callback is called when the cycle counter hits the break point that
// was set in ADCON0_V2::put.

void ADCON0_V2::callback(void)
{
    int channel;

    switch(ad_state) // The a/d converter is simulated with a state machine.
    {
    case AD_IDLE:
        break;

    case AD_ACQUIRING:
        channel =( value.get() >> 2) & channel_mask;

        m_dSampledVoltage = adcon1->getChannelVoltage( channel );
        m_dSampledVrefHi  = adcon1->getVrefHi();
        m_dSampledVrefLo  = adcon1->getVrefLo();

        future_cycle = cpu->currentCycle() +( (m_nBits+1)*Tad )/cpu->get_ClockPerInstr();
        cpu->setBreakAbs( future_cycle, this );
        ad_state = AD_CONVERTING;
        break;

    case AD_CONVERTING:
        put_conversion();

        value.put(value.get() &( ~GO)); // Clear the GO/!DONE flag.
        set_interrupt();
        ad_state = AD_IDLE;
    }
}

//------------------------------------------------------
//
void ADCON0_V2::set_interrupt(void)
{
    pir1v2->set_adif();
    intcon->peripheral_interrupt();

}

void ADCON0_V2::detach_ctmu_stim()
{
    ////
    /*
    if( active_stim >=0 && ctmu_stim )
    {
        PinModule *pm=adcon1->get_A2Dpin( active_stim );
        if( pm && pm->getPin().snode && ctmu_stim)
        {
            pm->getPin().snode->detach_stimulus(ctmu_stim);
            pm->getPin().snode->update();
        }
    }*/
    active_stim = -1;
}
/* Move ctmu_stim onto currently selected A/D channel input pin.
   if channel has not changed, just return.
   Stimulus can only be attached if pin is connected to a node.
*/
void ADCON0_V2::attach_ctmu_stim()
{
    int channel =( value.get() >> 2) & channel_mask;
    if     ( active_stim == channel ) return;
    else if( active_stim >= 0 )       detach_ctmu_stim();

    PinModule *pm=adcon1->get_A2Dpin(channel);

    if( pm )
    {////
        /*if( !(pm->getPin().snode) )
        {
            printf("Warning ADCON0_V2::attach_ctmu_stim %s has no node attached CTMU will not work properly\n", pm->getPin().name_str.c_str());
        }
        else if( ctmu_stim )
        {
            pm->getPin().snode->attach_stimulus(ctmu_stim);
            pm->getPin().snode->update();
            active_stim = channel;
        }*/
    }
}


//------------------------------------------------------
// ADCON1
//
ADCON1_V2::ADCON1_V2(Processor *pCpu, const char *pName)
    : SfrReg(pCpu, pName ),
      m_AnalogPins(0), m_nAnalogChannels(0),
      mValidCfgBits(0), mCfgBitShift(0), m_vrefHiChan(-1),
      m_vrefLoChan(-1),  mIoMask(0), m_adcon0(0)
{
    for( int i=0; i<(int)cMaxConfigurations; i++)
        setChannelConfiguration(i, 0);
}
ADCON1_V2::~ADCON1_V2()
{
    delete [] m_AnalogPins;
}

void ADCON1_V2::put( uint new_value )
{
    uint new_mask = get_adc_configmask(new_value);
    uint diff = mIoMask ^ new_mask;

    Dprintf( ( "ADCON1_V2::put(  %02X ) - new_mask %02X\n", new_value, new_mask ));

    char newname[20];

    for(uint i = 0; i < m_nAnalogChannels; i++)
    {
        if( (diff &( 1 << i)) && m_AnalogPins[i] != &InvalidAnInput)
        {
            if( new_mask &( 1<<i))
            {
                snprintf(newname, sizeof(newname), "an%u", i);
                m_AnalogPins[i]->AnalogReq(this, true, newname);
            }
            else
            {
                m_AnalogPins[i]->AnalogReq(this, false, "?" /*m_AnalogPins[i]->getPin().name_str.c_str()*/);
            }
        }
    }
    mIoMask = new_mask;
    value.put(new_value);
}

/*
 * If A2D uses PCFG, call for each PCFG value( cfg 0 to 15) with
 * each set bit of bitMask indicating port is an analog port
 *( either A2D input port or Vref). Processors which use an A2D
 * method that uses ANSEL register will not call this.
 *
 * As an example, for the following Port Configuration Control bit:
 * PCFG   AN7   AN6   AN5   AN4   AN3   AN2   AN1   AN0
 * ----   ---- ----- -----  ----- ----- ----- ----- -----
 * 1100   D    D     D      A     Vref+ Vref- A     A
 *
 *  then call setChannelConfiguration with cfg = 12 , bitMask = 0x1f
 * */
void ADCON1_V2::setChannelConfiguration( uint cfg, uint bitMask )
{
    if( cfg < cMaxConfigurations) m_configuration_bits[cfg] = bitMask;
}

/*
 * Performs same function as setChannelConfiguration, but defines
 * all entiries in configuration table in one call.
 */

void ADCON1_V2::setChanTable(
        uint m0, uint m1, uint m2, uint m3,
        uint m4, uint m5, uint m6, uint m7,
        uint m8, uint m9, uint m10, uint m11,
        uint m12, uint m13, uint m14, uint m15)
{
    m_configuration_bits[0] = m0;
    m_configuration_bits[1] = m1;
    m_configuration_bits[2] = m2;
    m_configuration_bits[3] = m3;
    m_configuration_bits[4] = m4;
    m_configuration_bits[5] = m5;
    m_configuration_bits[6] = m6;
    m_configuration_bits[7] = m7;
    m_configuration_bits[8] = m8;
    m_configuration_bits[9] = m9;
    m_configuration_bits[10] = m10;
    m_configuration_bits[11] = m11;
    m_configuration_bits[12] = m12;
    m_configuration_bits[13] = m13;
    m_configuration_bits[14] = m14;
    m_configuration_bits[15] = m15;
}

void ADCON1_V2::setNumberOfChannels( uint nChannels ) // Number of A2D channels processor supports
{
    PinModule** save = NULL;

    if( !nChannels || nChannels <= m_nAnalogChannels) return;

    if( m_nAnalogChannels && nChannels > m_nAnalogChannels )
        save = m_AnalogPins;

    m_AnalogPins = new PinModule *[nChannels];

    for( uint i=0; i<nChannels; i++ )
    {
        if( i < m_nAnalogChannels )
        {
            if( save) m_AnalogPins[i] = save[i];
        }
        else m_AnalogPins[i] = &InvalidAnInput;
    }
    if( save) delete save;

    m_nAnalogChannels = nChannels;
}

/*
 * Configure use of adcon1 register
 *      The register is first anded with mask and then shifted
 *      right shift bits. The result being either PCFG or VCFG
 *      depending on the type of a2d being used.
 */
void ADCON1_V2::setValidCfgBits(uint mask, uint shift)
{
    mValidCfgBits = mask;
    mCfgBitShift = shift;
}

/*
 * get_adc_configmask() is called with the value of the adcon1 register
 *
 * if the configuration bit mask is less than 16, the confiiguration bit table
 * is used to determine if the channel is an analog port.
 *
 * Otherwise, each bit in the adcon1 register indicates that the port is
 * digital(1) or analog(0) aka the 18f1220.
 *
 * */
uint ADCON1_V2::get_adc_configmask(uint reg)
{

    if( mValidCfgBits <= 0xf) // use config bit table
    {
        return( m_configuration_bits[ (reg>>mCfgBitShift) & mValidCfgBits ] );
    }
    else // register directly gives Analog ports( 18f1220)
    {
        return( ~(reg >> mCfgBitShift) & mValidCfgBits);
    }
}

/*
 * Associate a processor I/O pin with an A2D channel
 */
void ADCON1_V2::setIOPin(uint channel, PinModule *newPin)
{

    if( channel < m_nAnalogChannels &&
            m_AnalogPins[channel] == &InvalidAnInput && newPin!=0) {
        m_AnalogPins[channel] = newPin;
    }
    else
    {
        printf("WARNING %s channel %u, cannot set IOpin\n",__FUNCTION__, channel);
        if( m_AnalogPins[channel] != &InvalidAnInput)
            printf("Pin Already assigned\n");
        else if( channel > m_nAnalogChannels)
            printf("channel %u >= number of channels %u\n", channel,  m_nAnalogChannels);
    }
}


//------------------------------------------------------
PinModule *ADCON1_V2::get_A2Dpin(uint channel)
{
    if( ( 1<<channel) & get_adc_configmask(value.data) )
    {
        PinModule *pm = m_AnalogPins[channel];
        if( pm != &InvalidAnInput)
            return pm;
        cout << "ADCON1_V2::getChannelVoltage channel " << channel <<
                " not analog\n";
    }
    return 0;
}

//------------------------------------------------------
double ADCON1_V2::getChannelVoltage( uint channel )
{
    double voltage=0.0;

    if( channel <= m_nAnalogChannels )
    {
        PinModule* pm = get_A2Dpin( channel );
        if( pm)
        {
            ////if( pm->getPin().snode) pm->getPin().snode->update();
            voltage = pm->getPin().get_nodeVoltage();
        }
        else
        {
            cout << "ADCON1_V2::getChannelVoltage channel " << channel <<  " not a valid pin\n";
            voltage = 0.;
        }
    }
    else cout << "ADCON1_V2::getChannelVoltage channel " << channel <<  " > m_nAnalogChannels " << m_nAnalogChannels << "\n";

    return voltage;
}

double ADCON1_V2::getVrefHi()
{
    assert(m_vrefHiChan >= 0);    // m_vrefHiChan has not been set
    if( ( m_adcon0 &&( m_adcon0->value.data & ADCON0_V2::VCFG0)) ||
        (  !m_adcon0 &&( value.data & VCFG0))) // Use Vref+
        return(getChannelVoltage(m_vrefHiChan));

    return cpu->get_Vdd();
}

double ADCON1_V2::getVrefLo()
{
    assert(m_vrefLoChan >= 0);    // m_vrefLoChan has not been set
    if( ( m_adcon0 &&( m_adcon0->value.data & ADCON0_V2::VCFG1)) ||
        (  !m_adcon0 &&( value.data & VCFG1))) // Use Vref-
        return getChannelVoltage(m_vrefLoChan);

    return 0.0;
}

//------------------------------------------------------
// ADCON2_V2
//
ADCON2_V2::ADCON2_V2(Processor *pCpu, const char *pName )
    : SfrReg(pCpu, pName )
{
}

char ADCON2_V2::get_nacq()
{
    static char acq_tab[8] = { 0, 2, 4, 6, 8, 12, 16, 20};
    return(acq_tab[((value.get() &( ACQT2 | ACQT1 | ACQT0)) >> 3) ]);
}

char ADCON2_V2::get_tad()
{
    static char adcs_tab[8] = { 2, 8, 32, 0, 4, 16, 64, 0};
    return(adcs_tab[(value.get() &( ADCS2 | ADCS1 | ADCS0)) ]);
}

bool ADCON2_V2::get_adfm()
{
    return((value.get() & ADFM) == ADFM);
}

ADCON1_2B::ADCON1_2B(Processor *pCpu, const char *pName ) :
    ADCON1_V2(pCpu, pName ), Vctmu(0.0), Vdac(0.0), Vfvr_buf2(0)
{
}

//------------------------------------------------------
PinModule *ADCON1_2B::get_A2Dpin(uint channel)
{
    if(channel <= m_nAnalogChannels)
    {
        PinModule *pm = m_AnalogPins[channel];
        if( pm != &InvalidAnInput)
            return pm;
        cout << "ADCON1_V2::getChannelVoltage channel " << channel << " not analog\n";
    }
    return 0;
}
double ADCON1_2B::getChannelVoltage(uint channel)
{
    double voltage=0.0;

    if( channel <= m_nAnalogChannels )
    {
        PinModule *pm = get_A2Dpin(channel);
        if( pm ) voltage = pm->getPin().get_nodeVoltage();
        else
        {
            cout << "ADCON1_2B::getChannelVoltage channel " << channel << " not valid for A2D\n";
        }
    }
    else if( channel == CTMU) voltage = Vctmu;
    else if( channel == DAC)  voltage = Vdac;
    else if( channel == FVR_BUF2) voltage = Vfvr_buf2;
    else cout << "ADCON1_2B::getChannelVoltage channel " << channel << " not valid for A2D\n";

    return voltage;
}
double ADCON1_2B::getVrefHi()
{
    assert(m_vrefHiChan >= 0);    // m_vrefHiChan has not been set

    switch( value.data &( PVCFG1 | PVCFG0))
    {
    case 0:			// use Vdd
    case( PVCFG1 | PVCFG0 ):	// reserved use Vdd
        return cpu->get_Vdd();
        break;

    case PVCFG0:		// use external pin Vref+
        return(getChannelVoltage(m_vrefHiChan));
        break;

    case PVCFG1:		// use FVR buf2
        return Vfvr_buf2;
        break;
    }
    return 0.0;
}
double ADCON1_2B::getVrefLo()
{
    assert(m_vrefLoChan >= 0);    // m_vrefLoChan has not been set

    // external pin Vref- ?
    if( (value.data &( NVCFG1 | NVCFG0)) == NVCFG1)
        return getChannelVoltage(m_vrefLoChan);

    return 0.0;
}

void ADCON1_2B::put(uint new_value)
{
    value.put(new_value);
}

// Special trigger from ctmu module
void ADCON1_2B::ctmu_trigger()
{
    if( value.data & TRIGSEL)	// special trigger from CTMU active?
    {
        assert(m_adcon0);
        uint value = m_adcon0->value.data;
        if( (value & ADCON0_V2::ADON))
        {
            value |= ADCON0_V2::GO;
            m_adcon0->put(value);
        }
    }
}
// Special trigger from cpp module
void ADCON1_2B::ccp_trigger()
{
    if( !(value.data & TRIGSEL))	// special trigger from ccp active?
    {
        uint value = m_adcon0->value.data;
        if( (value & ADCON0_V2::ADON))
        {
            value |= ADCON0_V2::GO;
            m_adcon0->put(value);
        }
    }
}

ANSEL_2B::ANSEL_2B(Processor *pCpu, const char *pName )
    : SfrReg(pCpu, pName ), mask(0)
{
    for(int i=0; i<8; i++)
    {
        analog_channel[i] = -1;
        m_AnalogPins[i] = &InvalidAnInput;
    }
}

void ANSEL_2B::put(uint new_value)
{
    put_value(new_value);
}

void ANSEL_2B::put_value( uint new_value )
{
    char newname[20];
    new_value &= mask;
    uint diff = value.get() ^ new_value;

    value.put(new_value);

    for(int i = 0; i < 8; i++)
    {
        if( ((1<<i) & diff) &&( m_AnalogPins[i] != &InvalidAnInput))
        {
            if( new_value &( 1<<i))
            {
                snprintf( newname, sizeof(newname), "an%d", analog_channel[i] );
                m_AnalogPins[i]->AnalogReq(this, true, newname);
            }
            else
            {
                m_AnalogPins[i]->AnalogReq( this, false, "?" /*m_AnalogPins[i]->getPin().name_str.c_str()*/);
            }
        }
    }
}
void ANSEL_2B::setIOPin( uint channel, PinModule* port, ADCON1_2B* adcon1 )
{
    char newname[20];
    uint pin = port->getPinNumber();
    m_AnalogPins[pin] = port;
    analog_channel[pin] = channel;
    adcon1->setIOPin(channel, port);
    mask |= 1<<pin;

    if( (1<<pin) & value.get() )
    {
        snprintf(newname, sizeof(newname), "an%u", channel);
        m_AnalogPins[pin]->AnalogReq(this, true, newname);
    }
}
ANSEL_2A::ANSEL_2A(Processor *pCpu, const char *pName )
    : ANSEL_2B(pCpu, pName )
{
}
void ANSEL_2A::setIOPin(uint channel, PinModule *port, ADCON1_2B *adcon1)
{
    char newname[20];
    uint bit = channel & 7;
    m_AnalogPins[bit] = port;
    analog_channel[bit] = channel;
    adcon1->setIOPin(channel, port);
    mask |= 1<<bit;

    if( (1<<bit) & value.get())
    {
        snprintf(newname, sizeof(newname), "an%u", channel);
        m_AnalogPins[bit]->AnalogReq(this, true, newname);
    }
}
//
//--------------------------------------------------
// member functions for the FVRCON_V2 class
// with one set of gains and FVRST set after delay
//--------------------------------------------------
//
FVRCON_V2::FVRCON_V2(Processor *pCpu, const char *pName, uint bitMask)
    : SfrReg(pCpu, pName ), future_cycle(0),
      adcon1(0), daccon0(0), cmModule(0), cpscon0(0)
{
    mask_writable = bitMask;
}

void  FVRCON_V2::put(uint new_value)
{
    uint masked_value =( new_value & mask_writable);
    put_value(masked_value);
}

void  FVRCON_V2::put_value(uint new_value)
{
    uint diff = value.get() ^ new_value;

    if( diff )
    {
        if( diff &  FVREN ) new_value &= ~FVRRDY;	// Clear FVRRDY regardless of ON or OFF
        if( new_value & FVREN )	// Enable ON?
        {
            future_cycle = cpu->currentCycle() + 25e-6 /cpu->seconds_per_cycle();
            cpu->setBreakAbs( future_cycle, this );
        }
        else if( future_cycle )
        {
            cpu->clear_break(this);
            future_cycle = 0;
        }
    }
    value.put( new_value );
    compute_FVR( new_value );

    update();
}

// Set FVRRDY bit after timeout
void FVRCON_V2::callback()
{
    future_cycle = 0;
    put_value(value.get() | FVRRDY);
}



double FVRCON_V2::compute_FVR(uint fvrcon)
{
    double ret = -1.;

    if( fvrcon & FVRRDY )
    {
        switch(fvrcon &( FVRS0 | FVRS1))
        {
        case 0:		// output is off
            ret = 0.0;
            break;

        case FVRS0:	// Gain = 1
            ret = 1.024;
            break;

        case FVRS1:	// Gain = 2
            ret = 2.048;
            break;

        case( FVRS0|FVRS1): // Gain = 4
            ret = 4.096;
            break;
        }
    }
    if( ret > cpu->get_Vdd() )
    {
        cerr << "warning FVRCON FVRAD("<< ret <<") > Vdd(" <<cpu->get_Vdd() << ")\n";
        ret = -1.;
    }
    for( uint i=0; i<daccon0_list.size(); i++)
    {
        daccon0_list[i]->set_FVR_CDA_volt(ret);
    }
    if( adcon1 ) adcon1->setVoltRef( ret );
    if( cmModule ) cmModule->set_FVR_volt( ret );
    if( cpscon0 ) cpscon0->set_FVR_volt( ret );
    return ret;
}

void  DACCON0_V2::compute_dac(uint value)
{
    double Vhigh = get_Vhigh(value);
    double Vlow = 0.;
    double Vout;

    if( value & DACEN )	// DAC is enabled
    {
        Dprintf(("DACCON0_V2::compute_dac Vhigh %.2f daccon1_reg %x\n", Vhigh, daccon1_reg));
        Vout =(Vhigh-Vlow)*daccon1_reg/bit_resolution - Vlow;
    }
    else if( value & DACLPS) Vout = Vhigh;
    else                     Vout = Vlow;

    set_dacoutpin(value & DACOE, 0, Vout);

    if( adcon1 ) adcon1->update_dac( Vout );
    if( cmModule ) cmModule->set_DAC_volt( Vout );
    if( cpscon0 ) cpscon0->set_DAC_volt( Vout );
}

double DACCON0_V2::get_Vhigh(uint value)
{
    uint mode =( value &( DACPSS0|DACPSS1)) >> 2;
    switch(mode)
    {
    case 0:	// Vdd
        return cpu->get_Vdd();

    case 1:	// Vref+ pin, get is from A2D setup
        if(adcon1)
            return(adcon1->getChannelVoltage(adcon1->getVrefHiChan()));
        cerr << "ERROR DACCON0 DACPSS=01b adcon1 not set\n";
        return 0.;

    case 2:	// Fixed Voltage Reference
        return FVR_CDA_volt;

    case 3:	// Reserved value
        cerr << "ERROR DACCON0 DACPSS=11b is reserved value\n";
        return 0.;
    }
    return 0.;	// cant get here
} 
