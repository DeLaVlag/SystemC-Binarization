/******************************************************************************
 *
 *  File:        mbl_wbs32.h
 *  Description: mblite version
 *               Code for the architecture described in m32_ram_s32, i.e. an
 *               MBL1C processor connected to an external 32-bit memory (XRAM) 
 *               starting at address 0x00010000 and to a 32-bit Wishbone slave 
 *               on the same SoC as the processor (starting at 0xFFFFFFC0).
 *               From wb_slave_ex.h:
 *               This Wishbone slave consists of five registers (REG0..REG4) 
 *               that can be written to and read from (resp. at addresses 
 *               2, 3, 4, 5 and 6 over a Wishbone bus.  
 *               Addresses 0 and 1 are reserved for resp. a CTRL and a
 *               STAT(us) register. 
 *               The width of all registers is controlled with the template
 *               variable DW_g (e.g. 8, 16, 32) at instantiation.
 *               Reserved bits in CTRL register (active high, write only): 
 *                   b0 : Start command
 *                   b1 : software reset (clear REG0 to REG4 etc)
 *                        This bit is cleared automatically after one clock period.
 *               Reserved bits in Status register (read only):
 *                   b0 : reflects irq signal (active high)
 *
 *  Author:      Huib Lincklaen Arriens
 *  Date:        April 2010
 *
 ******************************************************************************/

#include <inttypes.h>

/********************** EXTERNAL RAM ************************/
// external RAM address on TLM bus
#define RAM_BASEADDR  0xC1000000

/******************** WISHBONE BRIDGE ***********************/
// wishbone bridge address on TLM bus, 32-bit slave, so WORD-aligned addresses
#define WB_BASEADDR   0xC0000000

// wishbone registers
#define S1_CTRL_REG  	WBR_MEM32((WB_BASEADDR + 0x00))
#define S1_STAT_REG  	WBR_MEM32((WB_BASEADDR + 0x04))
#define S1_DELAY      	WBR_MEM32((WB_BASEADDR + 0x08))
#define S1_LENGTH      	WBR_MEM32((WB_BASEADDR + 0x0c))
#define S1_DATA      	WBR_MEM32((WB_BASEADDR + 0x10))

// CTRL register values
#define S1_CLEAR_CMD      		0		// no command, idle
#define S1_RESET_CMD      		1		// reset internal registers
#define S1_PROCESS_PLAIN_CMD	3		// process plain image on co-processor
#define S1_PROCESS_MOMENTS_CMD	4		// process moments pres image on co-processor
#define S1_PROCESS_OTSU_CMD		5		// process otsu hai image on co-processor
#define S1_PROCESS_ENTROPY_CMD	6		// process entropy on co-processor
#define S1_PROCESS_KITTLER_CMD	7		// process kittler on co-processor

// STAT register values
#define S1_IDLE_MASK 	0x00000001	// mask for the busy bit
#define S1_IDLE_STATE     	0		// co-processor not ready
#define S1_READY_STATE    	1		// co-processor is ready

// macro's
#define handshake()		{									\
	/* wait for 'done' signal from co-processor */			\
    while ((S1_STAT_REG & S1_IDLE_MASK) != S1_READY_STATE);	\
    /* reset ctrl register */								\
    S1_CTRL_REG = S1_CLEAR_CMD;								\
    }

/********************** FILE HANDLER *************************/
// filehandler address on TLM bus
#define FILE_BASEADDR   0xC1000000

// read macro's
#define getFileWidth()				WBR_MEM32((FILE_BASEADDR + 0x00))
#define getFileHeight() 			WBR_MEM32((FILE_BASEADDR + 0x04))
#define getFileLength()				WBR_MEM32((FILE_BASEADDR + 0x08))
#define readFromFile(address)		WBR_MEM32((FILE_BASEADDR + 0x0C + (address<<2)))

// write macro's
#define startFileWrite()			WBR_MEM32((FILE_BASEADDR + 0x00)) = 1
#define writeToFile(address,word)	WBR_MEM32((FILE_BASEADDR + 0x04 + (address<<2))) = word

/********************* GENERAL DEFINES ***********************/
// macro for direct memory access
#define WBR_MEM32(mem_addr)  (*((volatile uint32_t *)mem_addr)) 

