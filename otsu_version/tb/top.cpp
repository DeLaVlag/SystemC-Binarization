//
// SimSoC Initial software, Copyright © INRIA 2007, 2008, 2009, 2010
// LGPL license version 3
//

#include <libsimsoc/context.hpp>
#include <libsimsoc/processors/mblite/mblite_processor.hpp>
#include <libsimsoc/components/memory.hpp>
#include <libsimsoc/components/debug_console.hpp>
#include <libsimsoc/components/irqc.hpp>
#include <libsimsoc/components/bus.hpp>
#include <libsimsoc/components/dce_null_modem.hpp>
#include <libsimsoc/components/stop_box.hpp>

#include "tb_init.h"
#include "wb.h"
#include "file.h"
#include "WBBridgeStructs.h"

#ifdef __RTL_SIMULATION__
#include "WBSlave_rtl_wrapper.h"
#define WBSlave WBSlave_rtl_wrapper
#else
#include "../synth/wb_slave.h"
#endif

using namespace std;
using namespace sc_core;
using namespace simsoc;

static const std::string infile = "../../../../in.bmp";
static const std::string outfile = "../../../../out.bmp";

class SubSystem: public Module {
public:
    static const uint32_t MEM_BASE =  0x00000000;
    static const uint32_t MEM_SIZE =   0x4000000;
    static const uint32_t CONS_BASE = 0x40000000;
    static const uint32_t IRQC_BASE = 0x17000000;
    static const uint32_t WB_BASE =   0xC0000000;
    static const uint32_t FILE_BASE = 0xC1000000;

    Bus bus;
    Memory mem;
    DebugConsole cons;
    Irqc irqc;
    Processor *proc;
    WBBridge wbBridge;
    WBSlave slave;
    sc_in<bool> clk;
    sc_in<bool> reset;
    WB_Signals wbSignals;
    sc_signal<bool> stb;
    FileHandler file;

    SubSystem(sc_module_name name, StopBox *sb) :
            Module(name),
            // module instantiation
            bus("BUS"), mem("MEMORY", MEM_SIZE), cons("CONSOLE", sb), irqc("IRQC"),
            proc(NULL), wbBridge("WBBridge", main_context().is_big_endian()),
            slave("Slave"), clk("CLK"), reset("RESET"), stb("STB"),
            file("FileHandler", infile, outfile) {
    	// create microblaze processor
        proc = new MBLITE_Processor("MBLITE");
        // connect MB to tlm bus and irq signal lines
        proc->get_rw_port()(bus.target_sockets);
        irqc.out_signal(proc->get_it_port());
        // connect other soc's to tlm bus
        bus.bind_target(mem.rw_socket, MEM_BASE, MEM_SIZE);
        bus.bind_target(cons.target_socket, CONS_BASE, DebugConsole::SIZE);
        bus.bind_target(irqc.rw_port, IRQC_BASE, Irqc::SIZE);
        bus.bind_target(wbBridge.rw_socket, WB_BASE, WBBridge::SIZE);
        bus.bind_target(file.rw_socket, FILE_BASE, FileHandler::SIZE);
        // configure debug port
        cons.set_big_endian(main_context().is_big_endian());
        cons.initiator_socket(proc->debug_port);
        // connect WB Master lines
        wbBridge.clk(clk);
        wbBridge.reset(reset);
        wbBridge.wb_out.adr_o(wbSignals.adr_o);
        wbBridge.wb_out.cyc_o(wbSignals.cyc_o);
        wbBridge.wb_out.dat_o(wbSignals.dat_o);
        wbBridge.wb_out.sel_o(wbSignals.sel_o);
        wbBridge.wb_out.we_o(wbSignals.we_o);
        wbBridge.wb_out.stb_o[0](stb);
        wbBridge.wb_in.ack_i(wbSignals.ack_i);
        wbBridge.wb_in.dat_i(wbSignals.dat_i);
        wbBridge.wb_in.int_i(wbSignals.int_i);
        // connect WB Slave lines
        slave.ack_o(wbSignals.ack_i);
        slave.dat_o(wbSignals.dat_i);
        slave.int_o(wbSignals.int_i);
        slave.clk(clk);
        slave.adr_i(wbSignals.adr_o);
        slave.cyc_i(wbSignals.cyc_o);
        slave.dat_i(wbSignals.dat_o);
        slave.reset(reset);
        slave.sel_i(wbSignals.sel_o);
        slave.stb_i(stb);
        slave.we_i(wbSignals.we_o);
    }
};

class Top: public Module {
public:
    sc_time clock_cycle;
    sc_clock clk;
    StopBox sb;
    SubSystem soc0;
    GenReset resetGenerator;
    sc_signal<bool> reset;
    Top(sc_module_name name) :
            Module(name), clock_cycle(10, SC_NS), clk("CLK", clock_cycle), sb(
            "STOPBOX", 2), soc0("SOC0", &sb), resetGenerator("ResetGenerator") {
        resetGenerator.clk.bind(clk);
        resetGenerator.reset.bind(reset);
        soc0.clk.bind(clk);
        soc0.reset.bind(reset);
    }
};

int sc_main(int argc, char *argv[]) {
    sc_trace_file* fp( sc_create_vcd_trace_file( "tr" ) );
    simsoc_init(argc, argv);
    Top top("TOP");
        sc_trace( fp, top.soc0.slave.ack_o, "ack_o" );
        sc_trace( fp, top.soc0.slave.dat_o, "dat_o" );
        sc_trace( fp, top.soc0.slave.int_o, "int_o" );
        sc_trace( fp, top.soc0.slave.clk, "clk" );
        sc_trace( fp, top.soc0.slave.adr_i, "adr_i" );
        sc_trace( fp, top.soc0.slave.cyc_i, "cyc_i" );
        sc_trace( fp, top.soc0.slave.dat_i, "dat_i" );
        sc_trace( fp, top.soc0.slave.reset, "reset" );
        sc_trace( fp, top.soc0.slave.sel_i, "sel_i" );
        sc_trace( fp, top.soc0.slave.stb_i, "stb_i" );
        sc_trace( fp, top.soc0.slave.we_i, "we_i" );
    sc_start();
    sc_close_vcd_trace_file( fp );
    return 0;
}
