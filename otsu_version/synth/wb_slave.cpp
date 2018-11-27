#include "wb_slave.h"

// local functions
//fixedpoint log_ex_min(fixedpoint input);
//fixedpoint log_ex_max(fixedpoint input);
//fixedpoint sqrt_ex_min(fixedpoint input);
//fixedpoint sqrt_ex_max(fixedpoint input);
//fixedpoint sqrt_ex_huge(fixedpoint input);

WBSlave::WBSlave(sc_core::sc_module_name name) {

    SC_CTHREAD(processBus, clk.pos());
    reset_signal_is(reset, true);

    SC_CTHREAD(processSlave, clk.pos());
    reset_signal_is(reset, true);
}

void WBSlave::processBus() {
    dat_o.write(0);
    ack_o.write(false);
    int_o.write(false);
    regs[S1_DELAY] = 0;

    while (true) {
        ack_o.write(false);
        wait();
        while (stb_i.read() == false || cyc_i.read() == false) {
            wait();
        }
        if (we_i.read()) {
            receive();
        } else {
            send();
        }
    }
}

void WBSlave::processSlave() {
	// variables
	sc_uint<32> i;				// counter
	sc_uint<32> threshold;		// pixel threshold

    while (true) {
		// idle as long as no command is given
		while (regs[S1_CTRL_REG] == S1_CLEAR_CMD ) wait();

		// check command
		switch(regs[S1_CTRL_REG]){
			// reset the co-processor
			case S1_RESET_CMD:
				// reset data registers
				std::cout << "  co-processor clearing registers space" << std::endl;
				for(i=3; i<MAX_REGISTERS; i++)
					regs[i] = 0;
				break;

//			// do the fixed threshold image processing
//			case S1_PROCESS_PLAIN_CMD:
//				// binarize pixels
//				//std::cout << "  co-processor starting plain fixed binarization" << std::endl;
//				binarization(128);
//				break;

//			// do the moments preservation image processing
//			case S1_PROCESS_MOMENTS_CMD:
//				// binarize pixels
//				//std::cout << "  co-processor starting moments pres. binarization" << std::endl;
//				binarization(getMomentsThreshold());
//				break;

			// do the otsu hai arigato image processing
			case S1_PROCESS_OTSU_CMD:
				// binarize pixels
				std::cout << "  co-processor starting otsu binarization" << std::endl;
				binarization(getOtsuThreshold());
				break;

			// do the entropy image processing
//			case S1_PROCESS_ENTROPY_CMD:
//				// binarize pixels
//				//std::cout << "  co-processor starting entropy binarization" << std::endl;
//				binarization(getEntropyThreshold());
//				break;

//			// do the kittler image processing
//			case S1_PROCESS_KITTLER_CMD:
//				// binarize pixels
//				std::cout << "  co-processor starting kittler binarization" << std::endl;
//				binarization(getKittlerThreshold());
//				break;

			// should not happen
			default:
				//std::cout << "  ERROR, co-processor did not recognise command!" << std::endl;
				break;
		}

		// Set the ready bit of status register
		regs[S1_STAT_REG] |= S1_READY_STATE;
		// wait for the ctrl register to go back to idle
		while (regs[S1_CTRL_REG] != S1_CLEAR_CMD )  wait();
		// Clear the ready bit of status register
		regs[S1_STAT_REG] &= S1_IDLE_STATE;

		wait();
    }
}

/*sc_uint<32> WBSlave::getMomentsThreshold(){
	// variables
	sc_uint<32> i, j;								// loopcounter
	sc_uint<32> histogram[MAX_GREYRANGE];			// histogram for image color
	fixedpoint probabilities[MAX_GREYRANGE];		// probability of a pixel color
	fixedpoint m1, m2, m3, cd, c0, c1, z0, z1, pd, p0, p1;
	fixedpoint t;
	fixedpoint pDistr;
	sc_uint<32> threshold;							// binarize threshold

	// create histogram
	for(i=0; i<MAX_GREYRANGE; i++)
		histogram[i] = 0;
	for(i=0; i<regs[S1_LENGTH]; i++)
		histogram[regs[S1_DATA+i] & PIXEL_GREYMASK]++;

	// compute probabilities
	for(i=0; i<MAX_GREYRANGE; i++)
		probabilities[i] = (fixedpoint)histogram[i] / (fixedpoint)regs[S1_LENGTH];


	// calculate first 3 moments
	m1 = m2 = m3 = (fixedpoint)0.0;
		for (i = 0; i < MAX_GREYRANGE; i++) {
		m1 += (fixedpoint)i * probabilities[i];
		m2 += (fixedpoint)i * (fixedpoint)i * probabilities[i];
		m3 += (fixedpoint)i * (fixedpoint)i * (fixedpoint)i * probabilities[i];
	}
	//  printf ("moments: m1 = %5.2f, m2 = %5.2f, m3 = %5.2f\n", m1, m2, m3);

	cd = m2 - m1 * m1;
	c0 = (-m2 * m2 + m1 * m3) / cd;
	c1 = (-m3 + m2 * m1) / cd;
	t = sqrt_ex_huge(c1 * c1 - (fixedpoint)4.0 * c0);
	z0 = (fixedpoint)0.5 * (-c1 - t);
	z1 = (fixedpoint)0.5 * (-c1 + t);

	pd = z1 - z0;
	p0 = (z1 - m1) / pd;
	p1 = (fixedpoint)1.0 - p0;

	// find threshold
	pDistr = (fixedpoint)0.0;
	for (threshold = 0; threshold < MAX_GREYRANGE; threshold++) {
	    pDistr += probabilities[threshold];
	    if (pDistr > p0)
	    	break;
	}


	// return threshold
	return(threshold);
}*/

sc_uint<32> WBSlave::getOtsuThreshold(){
	// variables
	sc_uint<32> i, j;								// loopcounter
	sc_uint<32> histogram[MAX_GREYRANGE];			// histogram for image color
	fixedpoint probabilities[MAX_GREYRANGE];	// probability of a pixel color
	fixedpoint m0Low, m0High, m1Low, m1High, varLow, varHigh;
	fixedpoint varWithin, varWMin;
	sc_uint<32> threshold;							// binarize threshold

	// create histogram
	for(i=0; i<MAX_GREYRANGE; i++)
		histogram[i] = 0;
	for(i=0; i<regs[S1_LENGTH]; i++)
		histogram[regs[S1_DATA+i] & PIXEL_GREYMASK]++;

	// compute probabilities
	for(i=0; i<MAX_GREYRANGE; i++)
		probabilities[i] = (fixedpoint)histogram[i] / (fixedpoint)regs[S1_LENGTH];

	// find threshold
	threshold = 0;
	varWMin = (fixedpoint)100000000.0;
	for (i=1; i<(MAX_GREYRANGE-1); i++) {
		m0Low = m0High = m1Low = m1High = varLow = varHigh = (fixedpoint)0.0;
		for (j = 0; j <= i; j++) {
			m0Low += probabilities[j];
			m1Low += (fixedpoint)j * probabilities[j];
		}
		if(m0Low != (fixedpoint)0.0)
			m1Low = m1Low / m0Low;
		else
			m1Low = (fixedpoint)i;
		for (j=i+1; j<MAX_GREYRANGE; j++) {
			m0High += probabilities[j];
			m1High += (fixedpoint)j * probabilities[j];
		}
		if(m0High != (fixedpoint)0.0)
			m1High = m1High / m0High;
		else
			m1High = (fixedpoint)i;
		for (j=0; j<=i; j++)
			varLow += ((fixedpoint)j - m1Low) * ((fixedpoint)j - m1Low) * probabilities[j];
		for (j=i+1; j<MAX_GREYRANGE; j++)
			varHigh += ((fixedpoint)j - m1High) * ((fixedpoint)j - m1High) * probabilities[j];
		varWithin = m0Low * varLow + m0High * varHigh;
		if (varWithin < varWMin) {
			varWMin = varWithin;
			threshold = (sc_uint<32>)i;
		}
	}

	// return threshold
	return(threshold);
}

/*sc_uint<32> WBSlave::getEntropyThreshold(){
	// variables
	sc_uint<32> i, j;								// loopcounters
	sc_uint<32> histogram[MAX_GREYRANGE];			// histogram for image color
	fixedpoint probabilities[MAX_GREYRANGE];		// probability of a pixel color
	fixedpoint Hn, Ps, Hs;
	fixedpoint psi, psiMax;							// entropy algorithm parameters
	sc_uint<32> threshold;							// binarize threshold

	// create histogram
	for(i=0; i<MAX_GREYRANGE; i++)
		histogram[i] = 0;
	for(i=0; i<regs[S1_LENGTH]; i++)
		histogram[regs[S1_DATA+i] & PIXEL_GREYMASK]++;

	// compute probabilities
	for(i=0; i<MAX_GREYRANGE; i++)
		probabilities[i] = (fixedpoint)histogram[i] / (fixedpoint)regs[S1_LENGTH];

	// find threshold
	Hn = (fixedpoint)0.0;
	for (i=0; i<MAX_GREYRANGE; i++)
		if (probabilities[i] != (fixedpoint)0.0)
			Hn -= probabilities[i] * log_ex_min(probabilities[i]);
	psiMax = (fixedpoint)0.0;
	for (i=1; i<MAX_GREYRANGE; i++) {
		Ps = Hs = (fixedpoint)0.0;
		for (j=0; j<i; j++) {
			Ps += probabilities[j];
			if (probabilities[j] > (fixedpoint)0.0)
				Hs -= probabilities[j] * log_ex_min(probabilities[j]);
		}
		if (Ps > (fixedpoint)0.0 && Ps < (fixedpoint)1.0){
			psi = log_ex_min(Ps - Ps * Ps) + Hs / Ps + (Hn - Hs) / ((fixedpoint)1.0 - Ps);
		}
		if (psi > psiMax) {
			psiMax = psi;
			threshold = (sc_uint<32>)i;
		}
	}

	// return threshold
	return(threshold);
}*/

/*sc_uint<32> WBSlave::getKittlerThreshold(){
	// variables
	sc_uint<32> i, j;								// loopcounters
	sc_uint<32> histogram[MAX_GREYRANGE];			// histogram for image color
	fixedpoint probabilities[MAX_GREYRANGE];		// probability of a pixel color
	fixedpoint m0Low, m0High, m1Low, m1High, varLow, varHigh;
	fixedpoint term1, term2;						// Kittler algorithm parameters
	fixedpoint stdDevLow, stdDevHigh;
	fixedpoint discr, discrMin, discrMax, discrM1;
	sc_uint<32> threshold;							// binarize threshold

	// create histogram
	for(i=0; i<MAX_GREYRANGE; i++)
		histogram[i] = 0;
	for(i=0; i<regs[S1_LENGTH]; i++)
		histogram[regs[S1_DATA+i] & PIXEL_GREYMASK]++;

	// compute probabilities
	for(i=0; i<MAX_GREYRANGE; i++)
		probabilities[i] = (fixedpoint)histogram[i] / (fixedpoint)regs[S1_LENGTH];

	// find threshold
	discr = discrM1 = discrMax = discrMin = (fixedpoint)0.0;
	threshold = 0;
	for (i=1; i<MAX_GREYRANGE-1; i++) {
		m0Low = m0High = m1Low = m1High = varLow = varHigh = 0.0;
		for (j=0; j<=i; j++) {
			m0Low += probabilities[j];
			m1Low += (fixedpoint)j * probabilities[j];
		}
		if(m0Low != (fixedpoint)0.0)
			m1Low = m1Low / m0Low;
		else
			m1Low = (fixedpoint)i;
		for (j=i+1; j<MAX_GREYRANGE; j++) {
			m0High += probabilities[j];
			m1High += (fixedpoint)j * probabilities[j];
		}
		if(m0High != (fixedpoint)0.0)
			m1High = m1High / m0High;
		else
			m1High = (fixedpoint)i;
		for (j=0; j<=i; j++)
			varLow += ((fixedpoint)j - m1Low) * ((fixedpoint)j - m1Low) * probabilities[j];
		stdDevLow = sqrt_ex_min(varLow);
		for (j=i+1; j<MAX_GREYRANGE; j++)
			varHigh += ((fixedpoint)j - m1High) * ((fixedpoint)j - m1High) * probabilities[j];
		stdDevHigh = sqrt_ex_max(varHigh);
		if (stdDevLow == (fixedpoint)0.0)
			stdDevLow = m0Low;
		if (stdDevHigh == (fixedpoint)0.0)
			stdDevHigh = m0High;
		if(m0Low != (fixedpoint)0.0){
			term1 = m0Low * log_ex_max(stdDevLow / m0Low);
		}else{
			term1 = (fixedpoint)0.0;
		}
		if(m0High != (fixedpoint)0.0){
			term2 = m0High * log_ex_max(stdDevHigh / m0High);
		}else{
			term2 = (fixedpoint)0.0;
		}
		discr = term1 + term2;
		if (discr < discrM1)
			discrMin = discr;
		if (discrMin != (fixedpoint)0.0 && discr >= discrM1)
			break;
		discrM1 = discr;
	}
	threshold = (sc_uint<32>)i;

	// return threshold
	return(threshold);
}*/


void WBSlave::binarization(sc_uint<32> threshold){
	// variables
	sc_uint<32> i;

	//std::cout << "  Threshold is:" << threshold << std::endl;

	// binarization
	for(i=0; i<regs[S1_LENGTH]; i++){
		if ((regs[S1_DATA + i] & PIXEL_GREYMASK) > threshold)
			regs[S1_DATA + i] = (HI | (regs[S1_DATA + i] & PIXEL_ALPHAMASK));
		else
			regs[S1_DATA + i] = (LO | (regs[S1_DATA + i] & PIXEL_ALPHAMASK));
	}
}

void WBSlave::receive() {
    unsigned int addr = adr_i.read() >> 2;
    WBInt data = dat_i.read();
	int mask;

	if(addr < MAX_REGISTERS) {
		mask = gen_select_mask();

		regs[addr] = (regs[addr] & (~mask)) | (data & mask);
		if (regs[S1_DELAY] > 0) wait(regs[S1_DELAY]);
		ack_o.write(true);
		int_o.write(regs[S1_STAT_REG]);
		wait();
    } else {
    	//std::cout << "  ERROR, exceeded addressing range of co-processor while receiving" << std::endl;
    }
}

void WBSlave::send() {
    unsigned int addr = adr_i.read() >> 2;
    int mask;

	if(addr < MAX_REGISTERS) {
		mask = gen_select_mask();

		dat_o.write(regs[addr] & mask);

		if (regs[S1_DELAY] > 0) wait(regs[S1_DELAY]);
		ack_o.write(true);
		int_o.write(regs[S1_STAT_REG]);
		wait();
    } else {
    	//std::cout << "  ERROR, exceeded addressing range of co-processor while sending" << std::endl;
    }
}

int WBSlave::gen_select_mask() {
	int mask = 0;
	int select = sel_i.read();
	for (int i = 0; i < 4; ++i) {
		mask = mask << 8;
		if (select & 8) {
			mask |= 0xFF;
		}
		select = select << 1;
	}
	return(mask);
}

// sqrt approximation function using interpolation
/*fixedpoint sqrt_ex_min(fixedpoint input){
	// sqrt table with natural base
	fixedpoint sqrt_table[SQRT_MIN_TABLESIZE][2] = {
			{0.25,0.5},
			{0.5,0.707106781},
			{0.75,0.866025404},
			{1,1},
			{1.25,1.118033989},
			{1.5,1.224744871},
			{1.75,1.322875656},
			{2,1.414213562},
			{2.25,1.5},
			{2.5,1.58113883},
			{2.75,1.658312395},
			{3,1.732050808},
			{3.25,1.802775638},
			{3.5,1.870828693},
			{3.75,1.936491673},
			{4,2},
			{4.25,2.061552813},
			{4.5,2.121320344},
			{4.75,2.179449472},
			{5,2.236067977}
	};

	// variables
	sc_uint<5> i;			// loop counter
	fixedpoint output;		// the output of the logarithm
	fixedpoint t;			// temp variable

	// interpolate
	output = (fixedpoint)0.0;

	if (input <= sqrt_table[0][0]) {
		t = (input - sqrt_table[0][0]) / (sqrt_table[1][0] - sqrt_table[0][0]);
		output = ((fixedpoint)1.0 - t) * sqrt_table[0][1] + t * sqrt_table[1][1];
	}
	else if (sqrt_table[SQRT_MIN_TABLESIZE-1][0] <= input) {
		t = (input - sqrt_table[SQRT_MIN_TABLESIZE-2][0]) / (sqrt_table[SQRT_MIN_TABLESIZE-1][0] - sqrt_table[SQRT_MIN_TABLESIZE-2][0]);
		output = ((fixedpoint)1.0 - t) * sqrt_table[SQRT_MIN_TABLESIZE-2][1] + t * sqrt_table[SQRT_MIN_TABLESIZE-1][1];
	}
	else {
		for (i=1; i<SQRT_MIN_TABLESIZE; i++)	{
			if (sqrt_table[i-1][0] <= input && input <= sqrt_table[i][0])	{
				t = (input - sqrt_table[i-1][0]) / (sqrt_table[i][0] - sqrt_table[i-1][0]);
				output = ((fixedpoint)1.0 - t) * sqrt_table[i-1][1] + t * sqrt_table[i][1];
				break;
			}
		}
	}

	return(output);
}*/

// sqrt approximation function using interpolation
/*fixedpoint sqrt_ex_max(fixedpoint input){
	// sqrt table with natural base
	fixedpoint sqrt_table[SQRT_BIG_TABLESIZE][2] = {
			{2100,45.82575695},
			{2500,50},
			{2900,53.85164807},
			{3300,57.44562647}
	};

	// variables
	sc_uint<5> i;			// loop counter
	fixedpoint output;		// the output of the logarithm
	fixedpoint t;			// temp variable

	// interpolate
	output = (fixedpoint)0.0;

	if (input <= sqrt_table[0][0]) {
		t = (input - sqrt_table[0][0]) / (sqrt_table[1][0] - sqrt_table[0][0]);
		output = ((fixedpoint)1.0 - t) * sqrt_table[0][1] + t * sqrt_table[1][1];
	}
	else if (sqrt_table[SQRT_BIG_TABLESIZE-1][0] <= input) {
		t = (input - sqrt_table[SQRT_BIG_TABLESIZE-2][0]) / (sqrt_table[SQRT_BIG_TABLESIZE-1][0] - sqrt_table[SQRT_BIG_TABLESIZE-2][0]);
		output = ((fixedpoint)1.0 - t) * sqrt_table[SQRT_BIG_TABLESIZE-2][1] + t * sqrt_table[SQRT_BIG_TABLESIZE-1][1];
	}
	else {
		for (i=1; i<SQRT_BIG_TABLESIZE; i++)	{
			if (sqrt_table[i-1][0] <= input && input <= sqrt_table[i][0])	{
				t = (input - sqrt_table[i-1][0]) / (sqrt_table[i][0] - sqrt_table[i-1][0]);
				output = ((fixedpoint)1.0 - t) * sqrt_table[i-1][1] + t * sqrt_table[i][1];
				break;
			}
		}
	}

	return(output);
}*/

// sqrt approximation function using interpolation
/*fixedpoint sqrt_ex_huge(fixedpoint input){
	// sqrt table with natural base
	fixedpoint sqrt_table[SQRT_HUGE_TABLESIZE][2] = {
			{30333.33333,174.164673},
			{33000,181.6590212},
			{35000,187.0828693}
	};

	// variables
	sc_uint<5> i;			// loop counter
	fixedpoint output;		// the output of the logarithm
	fixedpoint t;			// temp variable

	// interpolate
	output = (fixedpoint)0.0;

	if (input <= sqrt_table[0][0]) {
		t = (input - sqrt_table[0][0]) / (sqrt_table[1][0] - sqrt_table[0][0]);
		output = ((fixedpoint)1.0 - t) * sqrt_table[0][1] + t * sqrt_table[1][1];
	}
	else if (sqrt_table[SQRT_HUGE_TABLESIZE-1][0] <= input) {
		t = (input - sqrt_table[SQRT_HUGE_TABLESIZE-2][0]) / (sqrt_table[SQRT_HUGE_TABLESIZE-1][0] - sqrt_table[SQRT_HUGE_TABLESIZE-2][0]);
		output = ((fixedpoint)1.0 - t) * sqrt_table[SQRT_HUGE_TABLESIZE-2][1] + t * sqrt_table[SQRT_HUGE_TABLESIZE-1][1];
	}
	else {
		for (i=1; i<SQRT_HUGE_TABLESIZE; i++)	{
			if (sqrt_table[i-1][0] <= input && input <= sqrt_table[i][0])	{
				t = (input - sqrt_table[i-1][0]) / (sqrt_table[i][0] - sqrt_table[i-1][0]);
				output = ((fixedpoint)1.0 - t) * sqrt_table[i-1][1] + t * sqrt_table[i][1];
				break;
			}
		}
	}

	return(output);
}*/

// log approximation function using interpolation
/*fixedpoint log_ex_min(fixedpoint input){
	// log table with natural base
	fixedpoint log_table[LOG_SMALL_TABLESIZE][2] = {
			{0.1,-2.302585093},
			{0.18,-1.714798428},
			{0.26,-1.347073648},
			{0.34,-1.078809661},
			{0.42,-0.867500568},
			{0.5,-0.693147181},
			{0.58,-0.544727175},
			{0.66,-0.415515444},
			{0.74,-0.301105093},
			{0.82,-0.198450939}
	};

	// variables
	sc_uint<5> i;			// loop counter
	fixedpoint output;		// the output of the logarithm
	fixedpoint t;			// temp variable

	// interpolate
	output = (fixedpoint)0.0;

	if (input <= log_table[0][0]) {
		t = (input - log_table[0][0]) / (log_table[1][0] - log_table[0][0]);
		output = ((fixedpoint)1.0 - t) * log_table[0][1] + t * log_table[1][1];
	}
	else if (log_table[LOG_SMALL_TABLESIZE-1][0] <= input) {
		t = (input - log_table[LOG_SMALL_TABLESIZE-2][0]) / (log_table[LOG_SMALL_TABLESIZE-1][0] - log_table[LOG_SMALL_TABLESIZE-2][0]);
		output = ((fixedpoint)1.0 - t) * log_table[LOG_SMALL_TABLESIZE-2][1] + t * log_table[LOG_SMALL_TABLESIZE-1][1];
	}
	else {
		for (i=1; i<LOG_SMALL_TABLESIZE; i++)	{
			if (log_table[i-1][0] <= input && input <= log_table[i][0])	{
				t = (input - log_table[i-1][0]) / (log_table[i][0] - log_table[i-1][0]);
				output = ((fixedpoint)1.0 - t) * log_table[i-1][1] + t * log_table[i][1];
				break;
			}
		}
	}

	return(output);
}*/

// log approximation function using interpolation
/*fixedpoint log_ex_max(fixedpoint input){
	// log table with natural base
	fixedpoint log_table[LOG_BIG_TABLESIZE][2] = {
			{0.333333333,-1.098612289},
			{0.666666667,-0.405465108},
			{1,0},
			{1.333333333,0.287682072},
			{1.666666667,0.510825624},
			{2,0.693147181},
			{3,1.098612289},
			{4,1.386294361},
			{5,1.609437912},
			{6,1.791759469},
			{7,1.945910149},
			{8,2.079441542},
			{9,2.197224577},
			{10,2.302585093}
	};

	// variables
	sc_uint<5> i;			// loop counter
	fixedpoint output;		// the output of the logarithm
	fixedpoint t;			// temp variable

	// interpolate
	output = (fixedpoint)0.0;

	if (input <= log_table[0][0]) {
		t = (input - log_table[0][0]) / (log_table[1][0] - log_table[0][0]);
		output = ((fixedpoint)1.0 - t) * log_table[0][1] + t * log_table[1][1];
	}
	else if (log_table[LOG_BIG_TABLESIZE-1][0] <= input) {
		t = (input - log_table[LOG_BIG_TABLESIZE-2][0]) / (log_table[LOG_BIG_TABLESIZE-1][0] - log_table[LOG_BIG_TABLESIZE-2][0]);
		output = ((fixedpoint)1.0 - t) * log_table[LOG_BIG_TABLESIZE-2][1] + t * log_table[LOG_BIG_TABLESIZE-1][1];
	}
	else {
		for (i=1; i<LOG_BIG_TABLESIZE; i++)	{
			if (log_table[i-1][0] <= input && input <= log_table[i][0])	{
				t = (input - log_table[i-1][0]) / (log_table[i][0] - log_table[i-1][0]);
				output = ((fixedpoint)1.0 - t) * log_table[i-1][1] + t * log_table[i][1];
				break;
			}
		}
	}

	return(output);
}*/
