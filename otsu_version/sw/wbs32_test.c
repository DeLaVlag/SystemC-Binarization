/*******************************************************************************
*
*   File:        wbs32_test.c
*   Description: mblite version
*                Code for the architecture described in m32_ram_s32, i.e.
*                an MBL1C processor connected to an external memory (XRAM) and
*                to a 32-bit Wishbone slave on the same SoC as the processor.
*   Author:      Huib Lincklaen Arriens
*   Date:        April 2010
*
********************************************************************************/

#include <inttypes.h>
#include "mbl_wbs32.h"
#include "dbg_console.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// pointer to external RAM
uint32_t *XRAM = (uint32_t *)RAM_BASEADDR;

int
main ()
{
    // variables
    int i;
    int length;
//    int keuze=0;
//    char choice[1];
    
    print_str("Please choose the threshold determination method for binarization:\n");
    print_str("(1) Fixed [= 128]\n");
    print_str("(2) Moments Preservation\n");
    print_str("(3) Otsu\n");
    print_str("(4) Entropy\n");
    print_str("(5) Kittler\n");
 
    /*do{
    	print_str("Enter choice: ");
    	read_str(choice,8);
    	keuze = choice[0]-48;
    	if(keuze < 0 || keuze > 5)print_str("Not an option!\n");
    }while(keuze < 0 || keuze > 5);*/
    print_str("pre-chosen algorithm otsu\n");
    
    // reset co-processor registers
    print_str("resetting co-processor...\n");
    S1_DELAY = 0;
    S1_CTRL_REG = S1_RESET_CMD;
    // synchronise with co-processor
    handshake();
    
    // get image size
    length = getFileLength();
    print_str("image size is: ");
	print_uint(length);
	print_str("\n");

    // Writing image to co-processor
    print_str("writing to co-processor...\n");
    // write image length to co-processor register
    S1_LENGTH = length;
    // write pixels to registers of co-processor through wishbone
    for (i=0;i<length;i++){
    	*(&S1_DATA+i) = readFromFile(i);
    }
    
    // process image on co-processor
    print_str("processing image on co-processor...\n");
	// start processing the picture on the co-processor
	S1_CTRL_REG = S1_PROCESS_OTSU_CMD;
	//S1_CTRL_REG = keuze+2;
	// synchronise with co-processor
    handshake();
    
    // Reading image from co-processor
    print_str("reading from co-processor...\n");
	// read pixels from co-processor data registers through wishbone
    for (i=0;i<length;i++){
    	writeToFile(i,*(&S1_DATA+i));
    }

    // write file to out.bmp
    startFileWrite();

    // exit
    print_str("done, program exit normally\n");
    return(1);
}
