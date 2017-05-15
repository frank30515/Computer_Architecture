module simple_calculator(
    Clk,
    WEN,
    RW,
    RX,
    RY,
    DataIn,
    Sel,
    Ctrl,
    busY,
    Carry
);

    input        Clk;
    input        WEN;
    input  [2:0] RW, RX, RY;
    input  [7:0] DataIn;
    input        Sel;
    input  [3:0] Ctrl;
    output [7:0] busY;
    output       Carry;
	
	wire [7:0] reg_xout;
	wire [7:0] alu_out;
	wire [7:0] mux_xout;
	
	register_file reg0 (.Clk(Clk),.WEN(WEN),.RW(RW),.busW(alu_out),.RX(RX),.RY(RY),.busX(reg_xout),.busY(busY));
	mux mux0 (.ctrl(Sel),.out(mux_xout),.in0(DataIn),.in1(reg_xout));	
	alu alu0 (.ctrl(Ctrl),.x(mux_xout),.y(busY),.carry(Carry),.out(alu_out));


endmodule

module alu(ctrl,x,y,carry,out);
    
    input  [3:0] ctrl;
    input  [7:0] x;
    input  [7:0] y;
    output       carry;
    output [7:0] out;
	
	reg carry; 
	reg [7:0] out;
	
	always @(ctrl or x or y) begin
		case(ctrl)
		4'b0000: {carry, out [7:0]} = (x[7:0] + y[7:0]);
		4'b0001: {carry, out [7:0]} = (x[7:0] - y[7:0]);
		4'b0010: out = (x&y);
		4'b0011: out = (x|y);
		4'b0100: out = (~x) ;
		4'b0101: out = (x^y);
		4'b0110: out = (~(x|y));
		4'b0111: out = (y << x[2:0]);
		4'b1000: out = (y >> x[2:0]);
		4'b1001: out = ({x[7],x[7:1]});
		4'b1010: out = ({x[6:0],x[7]});
		4'b1011: out = ({x[0],x[7:1]});
		4'b1100: out = ((x==y)?1:0);
		default: out = 0;
		
		endcase 
		
	end
    

endmodule


module register_file(Clk,WEN,RW,busW,RX,RY,busX,busY);

    input        Clk, WEN;
    input  [2:0] RW, RX, RY;
    input  [7:0] busW;
    output [7:0] busX, busY;
	
	reg [7:0] r [7:0];//register
	
	initial begin
		r[0] <= 8'd0;
		r[1] <= 8'd0;
		r[2] <= 8'd0;
		r[3] <= 8'd0;
		r[4] <= 8'd0;
		r[5] <= 8'd0;
		r[6] <= 8'd0;
		r[7] <= 8'd0;	
	end
	    
    // write your design here
	always @(posedge Clk) begin
		if(WEN)begin
			case(RW)
			3'b000 : r[0] <= 0;
			3'b001 : r[1] <= busW;
			3'b010 : r[2] <= busW;
			3'b011 : r[3] <= busW;
			3'b100 : r[4] <= busW;
			3'b101 : r[5] <= busW;
			3'b110 : r[6] <= busW;
			3'b111 : r[7] <= busW;
			endcase
		
		end
		
	end
	
	reg [7:0] busX;
	reg [7:0] busY;

	always @(RX,r[0],r[1],r[2],r[3],r[4],r[5],r[6],r[7])  begin
		case (RX)
		3'b000 : busX = r[0];
		3'b001 : busX = r[1];
		3'b010 : busX = r[2];
		3'b011 : busX = r[3];
		3'b100 : busX = r[4];
		3'b101 : busX = r[5];
		3'b110 : busX = r[6];
		3'b111 : busX = r[7];
		default: busX = 0;
		endcase
	end
	
	always @(RY,r[0],r[1],r[2],r[3],r[4],r[5],r[6],r[7])begin
		case (RY) 
		3'b000 : busY = r[0];
		3'b001 : busY = r[1];
		3'b010 : busY = r[2];
		3'b011 : busY = r[3];
		3'b100 : busY = r[4];
		3'b101 : busY = r[5];
		3'b110 : busY = r[6];
		3'b111 : busY = r[7];
		default: busY = 0;
		endcase	
	
	end
    
endmodule



module mux (ctrl,out,in0,in1);

	input ctrl;
	input [7:0] in0, in1;
	output [7:0] out;
	
	
	reg [7:0] out;
	always @(*) begin
		if(ctrl)
		out = in1;
		else
		out = in0;	
	end

endmodule



