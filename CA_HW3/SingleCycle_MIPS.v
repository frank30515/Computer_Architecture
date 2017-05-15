// Single Cycle MIPS
//=========================================================
// Input/Output Signals:
// positive-edge triggered         clk
// active low asynchronous reset   rst_n
// instruction memory interface    IR_addr, IR
// output for testing purposes     RF_writedata  
//=========================================================
// Wire/Reg Specifications:
// control signals             MemToReg, MemRead, MemWrite, 
//                             RegDst, RegWrite, Branch, 
//                             Jump, ALUSrc, ALUOp
// ALU control signals         ALUctrl_i
// ALU input signals           ALUin1_i, ALUin2_i
// ALU output signals          ALUresult_o, ALUzero
// instruction specifications  r, j, jal, jr, lw, sw, beq
// sign-extended signal        SignExtend
// MUX output signals          MUX_RegDST, MUX_MemToReg, 
//                             MUX_Src, MUX_Branch, MUX_Jump
// registers input signals     Reg_R1, Reg_R2, Reg_W, WriteData 
// registers                   Register
// registers output signals    ReadData1, ReadData2
// data memory contral signals CEN, OEN, WEN
// data memory output signals  ReadDataMem
// program counter/address     PCin, PCnext, JumpAddr, BranchAddr
//=========================================================

module SingleCycle_MIPS( 
    clk,
    rst_n,
    IR_addr,
    IR,
    RF_writedata,
    ReadDataMem,
    CEN,
    WEN,
    A,
    ReadData2,
    OEN
);

//==== in/out declaration =================================
    //-------- processor ----------------------------------
    input         clk, rst_n;
    input  [31:0] IR;
    output [31:0] IR_addr, RF_writedata;
    //-------- data memory --------------------------------
    input  [31:0] ReadDataMem;  // read_data from memory
    output        CEN;  // chip_enable, 0 when you read/write data from/to memory
    output        WEN;  // write_enable, 0 when you write data into SRAM & 1 when you read data from SRAM
    output  [6:0] A;  // address
    output [31:0] ReadData2;  // write_data to memory
    output        OEN;  // output_enable, 0

//==== reg/wire declaration ===============================

	//the wire for PC
	wire [31:0] PC_i;
	wire [31:0] PC_nxt;
	wire [31:0] PC_o;
	wire [31:0] PC_add4;
	wire [31:0] jump_addr;
	//the wire for control
	wire		RegDst;
	wire		Jump;
	wire		Branch;
	wire		MemRead;
	wire		MemToReg;
	wire [ 1:0]	ALUOp;
	wire		MemWrite;
	wire		ALUSrc;
	wire		RegWrite;
	wire		jal;
	wire		jr;
	//the wire for register
	wire [ 4:0] pre_Reg_W;
	wire [ 4:0] Reg_R1;
	wire [ 4:0] Reg_R2;
	wire [ 4:0] Reg_W;	
	wire [31:0] WriteData;
	wire [31:0] ReadData1;
	// wire [31:0] ReadData2; already is an output
	wire [31:0] IR_extend;
	//the wire for ALU
	wire [ 2:0] ALUctrl;
	wire [31:0] ALUin1;
	wire [31:0] ALUin2;
	wire [31:0] ALUresult;
	wire        ALUzero;
	
	wire [31:0] BranchAddr;
	wire [31:0] MUX_Branch_o;
	wire [31:0] MUX_Jump_o;
	wire [31:0] MUX_MemToReg_o;
	
//==== module connection ==================================

	assign PC_i = PC_nxt ;

	PC PC (
	.PC_i(PC_i),
	.PC_o(IR_addr),
	.clk(clk),
	.rst_n(rst_n)
	);
	
	Control Control (
	.opcode_i(IR[31:26]),
	.func_i(IR[5:0]),
	.RegDst_o(RegDst),
	.Jump_o(Jump),
	.Branch_o(Branch),
	.MemRead_o(MemRead),
	.MemToReg_o(MemToReg),
	.ALUOp_o(ALUOp),
	.MemWrite_o(MemWrite),
	.ALUSrc_o(ALUSrc),
	.RegWrite_o(RegWrite),
	.jal_o(jal),
	.jr_o(jr)
	);
	
	MUX2_5b MUX_RegDST (
	.data0_i(IR[20:16]),
	.data1_i(IR[15:11]),
	.sel_i(RegDst),
	.data_o(pre_Reg_W)
	);
	
	MUX2_5b MUX_jal0 (
	.data0_i(pre_Reg_W),
	.data1_i(5'd31),
	.sel_i(jal),
	.data_o(Reg_W)
	);
	
	assign Reg_R1 = IR[25:21] ;
	assign Reg_R2 = IR[20:16] ;
	Register Register (
	.Reg_R1_i(Reg_R1),
	.Reg_R2_i(Reg_R2),
	.Reg_W_i(Reg_W),
	.RegWrite_i(RegWrite),
	.WriteData_i(WriteData),
	.ReadData1_o(ReadData1),
	.ReadData2_o(ReadData2),
	.clk(clk),
	.rst_n(rst_n)
	);
	
	assign RF_writedata = WriteData ;
	
	Sign_Extend Sign_Extend (
	.data_i(IR[15:0]),
	.data_o(IR_extend)
	);
	
	MUX2_32b MUX_Src (
	.data0_i(ReadData2),
	.data1_i(IR_extend),
	.sel_i(ALUSrc),
	.data_o(ALUin2)
	);
	
	ALU_Control ALU_Control (
	.func_i(IR[3:0]),
	.ALUOp_i(ALUOp),
	.ALUctrl_o(ALUctrl)
	);
	
	assign ALUin1 = ReadData1 ;
	
	ALU ALU (
	.ALUin1_i(ALUin1),
	.ALUin2_i(ALUin2),
	.ALUctrl_i(ALUctrl),
	.ALUresult_o(ALUresult),
	.ALUzero_o(ALUzero)
	);
	
	assign A = ALUresult[8:2] ;
	
	//adder for PC+4
	Adder Adder0 (
	.data0_i(IR_addr),
	.data1_i(32'b100),
	.data_o(PC_add4)
	);
	
	assign jump_addr = {PC_add4[31:28],IR[25:0],2'b00} ;
	
	Adder Adder1 (
	.data0_i(PC_add4),
	.data1_i({IR_extend[29:0],2'b00}),
	.data_o(BranchAddr)
	);	

	MUX2_32b MUX_Branch (
	.data0_i(PC_add4),
	.data1_i(BranchAddr),
	.sel_i((Branch & ALUzero)),
	.data_o(MUX_Branch_o)
	);	

	MUX2_32b MUX_Jump (
	.data0_i(MUX_Branch_o),
	.data1_i(jump_addr),
	.sel_i(Jump),
	.data_o(MUX_Jump_o)
	);		
	
	MUX2_32b MUX_jr (
	.data0_i(MUX_Jump_o),
	.data1_i(ReadData1),
	.sel_i(jr),
	.data_o(PC_nxt)
	);	
	 
	MUX2_32b MUX_MemToReg (
	.data0_i(ALUresult),
	.data1_i(ReadDataMem),
	.sel_i(MemToReg),
	.data_o(MUX_MemToReg_o)
	);	
	
	MUX2_32b MUX_jal1 (
	.data0_i(MUX_MemToReg_o),
	.data1_i(PC_add4),
	.sel_i(jal),
	.data_o(WriteData)
	);	
	
	assign CEN = ~(MemWrite | MemRead) ;
	assign WEN = ~MemWrite | MemRead ;
	assign OEN = 0 ;

//==== combinational part =================================


//==== sequential part ====================================


//=========================================================
endmodule

// Control
module Control (
	opcode_i,
	func_i,	
	RegDst_o,
	Jump_o,
	Branch_o,
	MemRead_o,
	MemToReg_o,
	ALUOp_o,
	MemWrite_o,
	ALUSrc_o,
	RegWrite_o,
	jal_o,
	jr_o
);

	input  [5:0] opcode_i;
	input  [5:0] func_i;
	output 	     RegDst_o;
	output 		 Jump_o;
	output		 Branch_o;
	output		 MemRead_o;
	output		 MemToReg_o;
	output [1:0] ALUOp_o;
	output		 MemWrite_o;
	output 		 ALUSrc_o;
	output 		 RegWrite_o;
	output		 jal_o;
	output		 jr_o;
	
	wire r,lw,sw,beq,jump,jal,jr;
	
	assign r    = ( opcode_i == 6'b000000 ) ? 1'b1 : 1'b0 ;
	assign lw   = ( opcode_i == 6'b100011 ) ? 1'b1 : 1'b0 ;
	assign sw   = ( opcode_i == 6'b101011 ) ? 1'b1 : 1'b0 ;
	assign beq  = ( opcode_i == 6'b000100 ) ? 1'b1 : 1'b0 ;
	assign jump = ( opcode_i == 6'b000010 ) ? 1'b1 : 1'b0 ;
	assign jal  = ( opcode_i == 6'b000011 ) ? 1'b1 : 1'b0 ;
	assign jr   = ( (opcode_i == 6'b000000) && (func_i == 6'b001000) ) ? 1'b1 : 1'b0 ;
	
	assign RegDst_o   = r ;
	assign Jump_o 	  = jump | jal ;
	assign Branch_o   = beq ;
    assign MemRead_o  = lw ;
	assign MemToReg_o = lw ;
	assign ALUOp_o	  = { r , beq } ;
	assign MemWrite_o = sw ;
	assign ALUSrc_o   = lw | sw ;
	assign RegWrite_o = r | lw | jal ;
	assign jal_o 	  = jal ;
	assign jr_o	      = jr ;	

endmodule

// Register
module Register (
	Reg_R1_i,
	Reg_R2_i,
	Reg_W_i,
	RegWrite_i,
	WriteData_i,
	ReadData1_o,
	ReadData2_o,
	clk,
	rst_n
);

	input		  clk; // this clk is for writing data into reg
	input 	      rst_n;
	input		  RegWrite_i;
	input  [ 4:0] Reg_R1_i;
	input  [ 4:0] Reg_R2_i;
	input  [ 4:0] Reg_W_i;
	input  [31:0] WriteData_i;
	output [31:0] ReadData1_o;
	output [31:0] ReadData2_o;
	
	reg [31:0] register [0:31] ; //32 32bits registers
	integer i ; // use for 'for' loop
	
	assign ReadData1_o = register[Reg_R1_i] ;
	assign ReadData2_o = register[Reg_R2_i] ;
	
	//Writing data 
	always @(posedge clk or negedge rst_n) begin
	
		if (~rst_n) begin
			for ( i = 0 ; i < 32 ; i = i + 1) begin
				register[i] <= 32'b0 ;
			end	
		end 
		
		else 
		if (RegWrite_i) begin // avoid latch
			for ( i = 0 ; i < 32 ; i = i + 1) begin
				if ( i ==  Reg_W_i)
					register[Reg_W_i] <= WriteData_i ;
				else
					register[i] <= register[i] ;
			end			
			
		end
			
		else begin
			for ( i = 0 ; i < 32 ; i = i + 1)
				register[i] <= register[i] ;
		end	
		
	end	

endmodule

//Program Counter
module PC (
	PC_i,
	PC_o,
	clk,
	rst_n
);

	input		  	  clk;
	input			  rst_n;	
	input  	   [31:0] PC_i;
	output reg [31:0] PC_o;
	
	always @ (posedge clk or negedge rst_n) begin
		if(~rst_n)
			PC_o <= 32'b0 ;
		else
			PC_o <= PC_i;	
	end
	
endmodule

// ALU
module ALU (
	ALUin1_i,
	ALUin2_i,
	ALUctrl_i,
	ALUresult_o,
	ALUzero_o
);

	input signed [31:0] ALUin1_i;
	input signed [31:0] ALUin2_i;
	input 		 [ 2:0] ALUctrl_i;
	
	output reg signed [31:0] ALUresult_o;
	output 		  			 ALUzero_o;
	
	assign ALUzero_o = (ALUin1_i == ALUin2_i) ? 1'b1 : 1'b0 ;
	
	always @(*) begin
		case(ALUctrl_i)
			3'b000 : ALUresult_o = ALUin1_i & ALUin2_i ; //AND
			3'b001 : ALUresult_o = ALUin1_i | ALUin2_i ; //OR
			3'b010 : ALUresult_o = ALUin1_i + ALUin2_i ; //add
			3'b110 : ALUresult_o = ALUin1_i - ALUin2_i ; //subtract
			3'b111 : ALUresult_o = (ALUin1_i < ALUin2_i) ? 32'b1 : 32'b0 ; //set on less than
			default : ALUresult_o = 32'b0;
		endcase
	end 

endmodule

//ALU Control for ALU
module ALU_Control (
	func_i,
	ALUOp_i,
	ALUctrl_o
);

	input  [3:0] func_i;
	input  [1:0] ALUOp_i;	
	output [2:0] ALUctrl_o;
	
	assign ALUctrl_o =
	( ALUOp_i == 2'b10 ) ? //op-code for r-type
		(	( func_i == 4'b0000) ? 3'b010 : //add
			( func_i == 4'b0010) ? 3'b110 : //sub
			( func_i == 4'b0100) ? 3'b000 : //and
			( func_i == 4'b0101) ? 3'b001 : //or
			( func_i == 4'b1010) ? 3'b111 : 3'b000 // slt and deault=0
		) :
	( ALUOp_i == 2'b01 ) ? 3'b110 : //op-code for branch
	( ALUOp_i == 2'b00 ) ? 3'b010 : 3'b000; //op-code for lw sw

endmodule

// Adder
module Adder (
	data0_i,
	data1_i,
	data_o
);

	input  [31:0] data0_i;
	input  [31:0] data1_i;
	output [31:0] data_o;
	
	assign data_o = data0_i + data1_i ;
	
endmodule

// extend 16-bits to 32-bits
module Sign_Extend (
	data_i,
	data_o
);

	input  [15:0] data_i;
	output [31:0] data_o;
	
	assign data_o = { {16{data_i[15]}} , data_i } ;
	
endmodule

// 2-to-1 MUX for  whose input is 5-bits
module MUX2_5b (
	data0_i,
	data1_i,
	sel_i,
	data_o
);

	input  [4:0] data0_i;
	input  [4:0] data1_i;
	input		 sel_i;	
	output [4:0] data_o;
	
	assign data_o = sel_i ? data1_i : data0_i;
	
endmodule

// 2-to-1 MUX for Branch, Jump, Control, MemToReg whose input is 32-bits
module MUX2_32b (
	data0_i,
	data1_i,
	sel_i,
	data_o
);

	input  [31:0] data0_i;
	input  [31:0] data1_i;
	input	      sel_i;	
	output [31:0] data_o;
	
	assign data_o = sel_i ? data1_i : data0_i;
	
endmodule
