#ifndef WB_SLAVE_H
#define WB_SLAVE_H

#define SC_INCLUDE_FX
#include <systemc.h>
using namespace sc_core;
using namespace sc_dt;
#include "WBStructs.h"

// image parameters
#define MAX_IMAGESIZE	65536			// 65k pixels maximum
#define MAX_REGISTERS	MAX_IMAGESIZE+4	// data registers plus 4 control registers
#define MAX_GREYRANGE	256				// maximum greyscale color range

// fixed point define
#define fixedpoint		sc_fixed<64,32>	// approximates a double

// interpolation table defines
//#define LOG_SMALL_TABLESIZE	10
//#define LOG_BIG_TABLESIZE	14
//#define SQRT_MIN_TABLESIZE	20
//#define SQRT_BIG_TABLESIZE	4
//#define SQRT_HUGE_TABLESIZE	3

// binarizing parameters
#define PIXEL_GREYMASK	0x000000FF		// only grey scale value
#define PIXEL_RGBMASK	0x00FFFFFF		// only rgb components
#define PIXEL_ALPHAMASK	0xFF000000		// only alpha component
#define HI 				0x00FFFFFF  	// max RGB value
#define LO 				0x00000000		// min RGB value

// CTRL register values
#define S1_CLEAR_CMD      		0	// no command, idle
#define S1_RESET_CMD      		1	// reset internal registers
//#define S1_PROCESS_PLAIN_CMD	3	// process plain image on co-processor
//#define S1_PROCESS_MOMENTS_CMD	4	// process moments pres image on co-processor
#define S1_PROCESS_OTSU_CMD		5	// process otsu hai image on co-processor
//#define S1_PROCESS_ENTROPY_CMD	6	// process entropy on co-processor
//#define S1_PROCESS_KITTLER_CMD	7	// process entropy on co-processor

// STAT register values
#define S1_IDLE_MASK 	0x00000001		// mask for the busy bit
#define S1_IDLE_STATE     	0		// co-processor not ready
#define S1_READY_STATE    	1		// co-processor is ready

// wishbone registers used in regs[]
#define S1_CTRL_REG  	0
#define S1_STAT_REG  	1
#define S1_DELAY      	2
#define S1_LENGTH      	3
#define S1_DATA      	4

SC_MODULE(WBSlave) {
public:
    typedef WBSlave SC_CURRENT_USER_MODULE;
    WBSlave(sc_core::sc_module_name name);

    // wishbone connections
    BoolIn clk; // master clock input
    BoolIn reset; // synchronous active high reset
    WBIntIn adr_i; // address bits
    WBIntIn dat_i; // databus input
    BoolIn we_i; // write enable input
    BoolIn stb_i; // strobe signals / core select signal
    BoolIn cyc_i; // valid BUS cycle input
    sc_in<sc_uint<WBParameters::BYTES> > sel_i; // byte selector
    WBIntOut dat_o; // databus output
    BoolOut ack_o; // buscycle acknowledge output
    BoolOut int_o; // interrupt request output

private:
    void processBus();
    void processSlave();
    void receive();
    void send();
    int gen_select_mask();
    void binarization(sc_uint<32> threshold);
    //sc_uint<32> getMomentsThreshold();
    sc_uint<32> getOtsuThreshold();
    //sc_uint<32> getEntropyThreshold();
    //sc_uint<32> getKittlerThreshold();

    sc_uint<32> regs[MAX_REGISTERS];		// control and data registers
};

#endif
