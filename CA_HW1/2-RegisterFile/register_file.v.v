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

