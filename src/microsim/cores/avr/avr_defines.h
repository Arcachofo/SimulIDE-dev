
enum {
    S_C = 0,S_Z,S_N,S_V,S_S,S_H,S_T,S_I,  // SREG bit indexes
    R_XL = 0x1a, R_XH,R_YL,R_YH,R_ZL,R_ZH,// 16 bits register pairs
//    R_SPL = 32+0x3d, R_SPH,               // stack pointer
//    R_SREG = 32+0x3f,                     // real SREG
//    MAX_IOs    = 280,                     // maximum number of IO registers, on normal AVRs. Bigger AVRs need more than 256-32 (mega1280)
};

//Core states.
/*enum {
    cpu_Limbo = 0,  // before initialization is finished
    cpu_Stopped,    // all is stopped, timers included
    cpu_Running,    // we're free running
    cpu_Sleeping,   // we're now sleeping until an interrupt
    cpu_Step,       // run ONE instruction, then...
    cpu_StepDone,   // tell gdb it's all OK, and give it registers
    cpu_Done,       // avr software stopped gracefully
    cpu_Crashed,    // avr software crashed (watchdog fired)
};*/

#define get_d5(o) \
        const uint8_t d = (o >> 4) & 0x1f;

#define get_vd5(o) \
        get_d5(o) \
        const uint8_t vd = m_dataMem[d];

#define get_r5(o) \
        const uint8_t r = ((o >> 5) & 0x10) | (o & 0xf);

#define get_d5_a6(o) \
        get_d5(o); \
        const uint8_t A = ((((o >> 9) & 3) << 4) | ((o) & 0xf)) + 32;

#define get_vd5_s3(o) \
        get_vd5(o); \
        const uint8_t s = o & 7;

#define get_vd5_s3_mask(o) \
        get_vd5_s3(o); \
        const uint8_t mask = 1 << s;

#define get_vd5_vr5(o) \
        get_r5(o); \
        get_d5(o); \
        const uint8_t vd = m_dataMem[d], vr = m_dataMem[r];

#define get_d5_vr5(o) \
        get_d5(o); \
        get_r5(o); \
        const uint8_t vr = m_dataMem[r];

#define get_h4_k8(o) \
        const uint8_t h = 16 + ((o >> 4) & 0xf); \
        const uint8_t k = ((o & 0x0f00) >> 4) | (o & 0xf);

#define get_vh4_k8(o) \
        get_h4_k8(o) \
        const uint8_t vh = m_dataMem[h];

#define get_d5_q6(o) \
        get_d5(o) \
        const uint8_t q = ((o & 0x2000) >> 8) | ((o & 0x0c00) >> 7) | (o & 0x7);

#define get_io5(o) \
        const uint8_t io = ((o >> 3) & 0x1f) + 32;

#define get_io5_b3(o) \
        get_io5(o); \
        const uint8_t b = o & 0x7;

#define get_io5_b3mask(o) \
        get_io5(o); \
        const uint8_t mask = 1 << (o & 0x7);

#define get_vp2_k6(o) \
        const uint8_t p = 24 + ((o >> 3) & 0x6); \
        const uint8_t k = ((o & 0x00c0) >> 2) | (o & 0xf); \
        const uint16_t vp = m_dataMem[p] | (m_dataMem[p + 1] << 8);

//#define AVR_DATA_TO_IO(v) ((v) - 32)
//#define AVR_IO_TO_DATA(v) ((v) + 32)

//#define AVR_FUSE_LOW  0
//#define AVR_FUSE_HIGH 1
//#define AVR_FUSE_EXT  2
