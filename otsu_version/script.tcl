############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 2013 Xilinx Inc. All rights reserved.
############################################################
open_project VLSIProject
set_top WBSlave
add_files /home/dirkvos/Desktop/otsu_version/synth/WBStructs.h -cflags "-m64"
add_files /home/dirkvos/Desktop/otsu_version/synth/sc_otsu.cpp -cflags "-m64"
add_files /home/dirkvos/Desktop/otsu_version/synth/sc_otsu.h -cflags "-m64"
add_files /home/dirkvos/Desktop/otsu_version/synth/wb_slave.cpp -cflags "-m64"
add_files /home/dirkvos/Desktop/otsu_version/synth/wb_slave.h -cflags "-m64"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/WBBridgeStructs.h -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/WBParameters.h -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/file.cpp -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/file.h -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/tb_init.cpp -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/tb_init.h -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/top.cpp -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/wb.cpp -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/wb.h -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/bmp/EasyBMP_BMP.h -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/bmp/EasyBMP_DataStructures.h -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/bmp/EasyBMP_VariousBMPutilities.h -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/bmp/EasyBMP.h -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
add_files -tb /home/dirkvos/Desktop/otsu_version/tb/bmp/EasyBMP.cpp -cflags "-I/opt/eds/systemc/simsoc/include -I/opt/eds/systemc/simsoc/include/libsimsoc -I/opt/eds/systemc/sysc/systemc-2.3.0/include -m64 -DSC_INCLUDE_DYNAMIC_PROCESSES"
#
open_solution "solution1"

#set_part {xc7vx485tffg1761-2}
set_part {xc6slx150tfgg484-3}
#set_part {xc3s2000fg676-4}
#set_part xc6slx150tfgg676-3
#set_part xc6vlx240tlff1156-1
#set_part {xc7vx550tffg1927-2}

create_clock -period 10 -name default

csim_design -O -ldflags {-m64 -L/opt/eds/systemc/simsoc/lib-linux64-gcc-4.4.7 -lsimsoc} -argv {/home/dirkvos/Desktop/otsu_version/sw/wbs32_test}

# Source x_hls.tcl to determine which steps to execute
source x_hls.tcl

# Set any optimization directives
# set_directive_interface -register fxp_sqrt_top in_val
# set_directive_interface -mode ap_ctrl_hs -register fxp_sqrt_top return
# set_directive_inline -region fxp_sqrt_top
# set_directive_pipeline fxp_sqrt_top
#set_directive_loop_flatten my_exp
#set_directive_dataflow  my_exp
#set_directive_latency -min=44 -max=44 exp
#set_directive_latency -min=44 -max=44 LOOP_X1
#set_directive_loop_flatten Loop-1.1

#set_directive_interface -mode ap_ctrl_hs my_expA
#set_directive_interface -mode ap_vld my_expA my_expA_X
#set_directive_interface -mode ap_vld my_expA my_expA_Y

# End of directives

if {$hls_exec == 1} {
	# Run Synthesis and Exit
	csynth_design
} elseif {$hls_exec == 2} {
	# Run Synthesis, RTL Simulation and Exit
	csynth_design
	cosim_design -O -ldflags {-m64 -L/opt/eds/systemc/simsoc/lib-linux64-gcc-4.4.7 -lsimsoc} -trace_level all -argv {/home/dirkvos/Desktop/otsu_version/sw/wbs32_test} -rtl vhdl -tool modelsim
} elseif {$hls_exec == 3} {
	# Run Synthesis, RTL Simulation, RTL implementation and Exit
	csynth_design
	cosim_design -O -ldflags {-m64 -L/opt/eds/systemc/simsoc/lib-linux64-gcc-4.4.7 -lsimsoc} -trace_level all -argv {/home/dirkvos/Desktop/otsu_version/sw/wbs32_test} -rtl vhdl -tool modelsim
	#export_design -evaluate vhdl -format pcore -use_netlist top
	export_design 
} else {
	# Default is to exit after setup
}

exit

