//behavior tb
`timescale 1ns/10ps
`define CYCLE	10
`define HCYCLE	5

module alu_tb;
    reg  [3:0] ctrl;
    reg  [7:0] x;
    reg  [7:0] y;
    wire       carry;
    wire [7:0] out;
    
    alu alu1(ctrl,x,y,carry,out);

   initial begin
       $dumpfile("alu.vcd");
       $dumpvars;
//        $fsdbDumpfile("alu.fsdb");
//        $fsdbDumpvars;
   end

    initial begin
	
	    ctrl = 4'b1101;
        x    = 8'b0110_0001;
        y    = 8'b1110_0011;
        
        #(`CYCLE);        
        ctrl = 4'b0000;        
        #(`HCYCLE);
        if( (out == 8'b0100_0100) && (carry == 0) ) $display( "PASS --- 0000 add" );
        else $display( "FAIL --- 0000 add" );
		
		#(`CYCLE);        
        ctrl = 4'b0001;        
        #(`HCYCLE);
        if( (out == 8'b0111_1110) && (carry == 0) ) $display( "PASS --- 0001 sub" );
        else $display( "FAIL --- 0001 sub" );
		
		#(`CYCLE);        
        ctrl = 4'b0010;        
        #(`HCYCLE);
        if( out == 8'b0110_0001 ) $display( "PASS --- 0010 and" );
        else $display( "FAIL --- 0010 and" );
		
		#(`CYCLE);        
        ctrl = 4'b0011;        
        #(`HCYCLE);
        if( out == 8'b1110_0011 ) $display( "PASS --- 0011 or" );
        else $display( "FAIL --- 0011 or" );
		
		#(`CYCLE);         
        ctrl = 4'b0100;        
        #(`HCYCLE);
        if( out == 8'b1001_1110 ) $display( "PASS --- 0100 not" );
        else $display( "FAIL --- 0100 not" );
		
		#(`CYCLE);         
        ctrl = 4'b0101;        
        #(`HCYCLE);
        if( out == 8'b1000_0010 ) $display( "PASS --- 0101 xnor" );
        else $display( "FAIL --- 0101 xnor" );
		
		#(`CYCLE);         
        ctrl = 4'b0110;        
        #(`HCYCLE);
        if( out == 8'b0001_1100 ) $display( "PASS --- 0110 nor" );
        else $display( "FAIL --- 0110 nor" );
		
		#(`CYCLE);         
        ctrl = 4'b0111;        
        #(`HCYCLE);
        if( out == 8'b1100_0110 ) $display( "PASS --- 0111 sll" );
        else $display( "FAIL --- 0111 sll" );
		
		#(`CYCLE);         
        ctrl = 4'b1000;        
        #(`HCYCLE);
        if( out == 8'b0111_0001 ) $display( "PASS --- 1000 srl" );
        else $display( "FAIL --- 1000 srl" );
		
		#(`CYCLE);         
        ctrl = 4'b1001;        
        #(`HCYCLE);
        if( out == 8'b0011_0000 ) $display( "PASS --- 1001 sra" );
        else $display( "FAIL --- 1001 sra" );
		 
		#(`CYCLE);         
        ctrl = 4'b1010;        
        #(`HCYCLE);
        if( out == 8'b1100_0010 ) $display( "PASS --- 1010 rl" );
        else $display( "FAIL --- 1010 rl" );
		
		#(`CYCLE);         
        ctrl = 4'b1011;        
        #(`HCYCLE);
        if( out == 8'b1011_0000 ) $display( "PASS --- 1011 rr" );
        else $display( "FAIL --- 1011 rr" );
		
		#(`CYCLE);         
        ctrl = 4'b1100;        
        #(`HCYCLE);
        if( out == 0 ) $display( "PASS --- 1100 equal" );
        else $display( "FAIL --- 1100 equal" );
		
		#(`CYCLE);         
        ctrl = 4'b1101;        
        #(`HCYCLE);
        if( out == 0 ) $display( "PASS --- 1101 NOP" );
        else $display( "FAIL --- 1101 NOP" );
		
		#(`CYCLE);         
        ctrl = 4'b1110;        
        #(`HCYCLE);
        if( out == 0 ) $display( "PASS --- 1110 NOP" );
        else $display( "FAIL --- 1110 NOP" );
		
		#(`CYCLE);
        ctrl = 4'b1111;
        #(`HCYCLE);
        if( out == 0 ) $display( "PASS --- 1111 NOP" );
        else $display( "FAIL --- 1111 NOP" );
        
        // finish tb
        #(`CYCLE) $finish;
    end

endmodule
