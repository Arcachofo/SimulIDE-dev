#ifndef Z80REGS_H
#define Z80REGS_H

// Class Z80RegPair allows access to two 8 bit registers as one 16 bits register pair.
// The register pair is composed by calling constuctor with references to high byte and low byte.
class Z80RegPair
{
    public:
        Z80RegPair( uint8_t &h, uint8_t &l ) : high(h), low(l) {;}
        void setLowByte( const uint8_t &byte ) { low = byte; }
        void setHighByte( const uint8_t &byte ) { high = byte; }
        uint8_t& getLowByte() { return low; }
        uint8_t& getHighByte() { return high; }
        Z80RegPair operator++( int ) { Z80RegPair temp = *this; uint16_t r16 = *this; r16++; *this = r16; return temp; }
        Z80RegPair operator--( int ) { Z80RegPair temp = *this; uint16_t r16 = *this; r16--; *this = r16; return temp; }
        operator uint16_t() const { return (high << 8) | low; }
        Z80RegPair& operator=( const uint16_t& word ) { high = word >> 8; low = word & 0xff; return *this; }
    
    private:
        uint8_t &high;
        uint8_t &low;
};

// Class Z80Reg16 is 16 bis register. It contains same methods as class Z80RegPair to be able to use this class by same template as class Z80RegPair.
class Z80Reg16
{
    public:
        void setLowByte( const uint8_t &byte ) { reg &= 0xff00; reg |= byte; }
        void setHighByte( const uint8_t &byte ) { reg &= 0x00ff; reg |= (byte << 8); }
        uint8_t getLowByte() { return reg & 0x00ff; }
        uint8_t getHighByte() { return reg >> 8; }
        Z80Reg16 operator++( int ) { Z80Reg16 temp = *this; reg++; return temp; }
        Z80Reg16 operator--( int ) { Z80Reg16 temp = *this; reg--; return temp; }
        operator uint16_t() const { return reg; }
        Z80Reg16& operator=( const uint16_t& word ) { reg = word; return *this; }
    
    private:
        uint16_t reg;
};
        
        
// Class Z80Flags is 8 bit register with flags. The methods allow various flags modification. It also remember if flags were modified or not.
class Z80Flags {
    enum { fC = 0x01, fN = 0x02, fP = 0x04, fX = 0x08, fH = 0x10, fY = 0x20, fZ = 0x40, fS = 0x80 };  // don't change flags numbers
    
    public:
        void clearChanged() { lastChanged = changed; changed = false; }
        bool isChanged() { return lastChanged; }
            
        bool getS() { return flags & fS; }
        bool getZ() { return flags & fZ; }
        bool getH() { return flags & fH; }
        bool getP() { return flags & fP; }
        bool getN() { return flags & fN; }
        bool getC() { return flags & fC; }
        Z80Flags& setH() { flags |= fH; changed = true; return *this; }
        Z80Flags& setN() { flags |= fN; changed = true; return *this; }
        Z80Flags& setC() { flags |= fC; changed = true; return *this; }
        Z80Flags& resetY() { flags &= ~fY; changed = true; return *this; }
        Z80Flags& resetH() { flags &= ~fH; changed = true; return *this; }
        Z80Flags& resetX() { flags &= ~fX; changed = true; return *this; }
        Z80Flags& resetN() { flags &= ~fN; changed = true; return *this; }
        Z80Flags& resetC() { flags &= ~fC; changed = true; return *this; }
        Z80Flags& resetHN() { flags &= ~( fH | fN ); changed = true; return *this; }
        Z80Flags& negC() { flags ^= fC; changed = true; return *this; }
        Z80Flags& copyYX(const uint8_t &r) { flags &= ~( fY | fX ); flags |= r & ( fY | fX ); changed = true; return *this; }
        Z80Flags& copyNegCtoH() { flags &= ~fH; flags |= (~flags << 4) & fH; changed = true; return *this; }
        Z80Flags& andZ(const bool &z) { if (!z) flags &= ~fZ; changed = true; return *this; }
        Z80Flags& orYX(const uint8_t &r) { flags |= r & ( fY | fX ); changed = true; return *this; }
        Z80Flags& storeP(const bool &p) { flags &= ~fP; flags |= p << 2 ; changed = true; return *this; }
        Z80Flags& storeC(const bool &c) { flags &= ~fC; flags |= c ; changed = true; return *this; }
        Z80Flags& storeHC(const bool &hc) { flags &= ~( fH | fC); flags |= hc | (hc << 4) ; changed = true; return *this; }
        Z80Flags& copySzeroZ(const uint8_t &r) { flags &= ~( fS | fZ ); flags |= r & fS;
                                                 if ( r == 0x00 ) flags |= fZ;
                                                 changed = true; return *this; }
        Z80Flags& copySzeroZP(const uint8_t &r) { flags &= ~( fS | fZ | fP ); flags |= r & fS;
                                                  if ( r == 0x00 ) flags |= fZ | fP;
                                                  changed = true; return *this; }
        Z80Flags& copySYXzeroZ(const uint8_t &r) { flags &= ~( fS | fZ | fY | fX ); flags |= r & ( fS | fY | fX );
                                                   if ( r == 0x00 ) flags |= fZ;
                                                   changed = true; return *this; }
        Z80Flags& copySYXzeroZparityP(const uint8_t &r) { flags &= ~( fS | fZ | fY | fX | fP ); flags |= r & ( fS | fY | fX );
                                                          if ( r == 0x00 ) flags |= fZ;
                                                          if ( parity(r) == false ) flags |= fP;
                                                          changed = true; return *this; }
        Z80Flags& overflowHP(const uint8_t &r) { flags &= ~( fH | fP );
                                                 if ( (r & 0x08) != 0x00 ) flags |= fH;
                                                 if ( (r & 0xC0) == 0x80 || (r & 0xC0) == 0x40 ) flags |= fP;
                                                 changed = true; return *this; }
        Z80Flags& overflowHC(const uint8_t &r) { flags &= ~( fH | fC );
                                                 if ( (r & 0x08) != 0x00 ) flags |= fH;
                                                 if ( (r & 0x80) != 0x00 ) flags |= fC;
                                                 changed = true; return *this; }
        Z80Flags& overflowHPC(const uint8_t &r) { flags &= ~( fH | fP | fC );
                                                  if ( (r & 0x08) != 0x00 ) flags |= fH;
                                                  if ( (r & 0xC0) == 0x80 || (r & 0xC0) == 0x40 ) flags |= fP;
                                                  if ( (r & 0x80) != 0x00 ) flags |= fC;
                                                  changed = true; return *this; }
        Z80Flags& parityP(const uint8_t &r) { flags &= ~fP; if ( parity(r) == false ) flags |= fP; changed = true; return *this; }
        Z80Flags& copyBit1YX(const uint8_t &r) { flags &= ~( fY | fX ); flags |= r & fX; if ( (r & 0x02) != 0x00 ) flags |= fY; changed = true; return *this; }
        Z80Flags& copyBit7N(const uint8_t &r) { flags &= ~fN; if ( (r & 0x80) != 0x00 ) flags |= fN; changed = true; return *this; }
        Z80Flags& inotxxPH(const uint8_t &r1, const uint8_t &r2) {
            if ((flags & fC) == 0) flags ^= ( (parity(r2 & 0x07) == true) << 2 ) & fP;
            else {
                flags &= ~fH;
                if ((r1 & 0x80) != 0) {
                    if ((r2 & 0x0f) == 0x00) flags |= fH;
                    flags ^= ( (parity((r2 - 1) & 0x07) == true) << 2) & fP;
                } else {
                    if ((r2 & 0x0f) == 0x0f) flags |= fH;
                    flags ^= ( (parity((r2 + 1) & 0x07) == true) << 2 ) & fP;
                }
            }
            changed = true;
            return *this;
        }
        operator uint8_t&() { return flags; }
        uint8_t* operator&() { return &flags; }
        uint8_t& operator=( const uint16_t& f ) { flags = f; return flags; }
    
    private:
        uint8_t flags;
        bool changed = false;
        bool lastChanged = false;
            
        // Calculation parity of bits in reg. Result is true for odd number of bits and false for even number of bits.
        bool parity( uint8_t reg ) { reg ^= (reg >> 1); reg ^= (reg >> 2); reg ^= (reg >> 4); return (reg & 0x01); }
};

#endif
