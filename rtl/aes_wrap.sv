`include "apb_bus.sv"

module aes_wrap
(
  input  logic   clk_i,
  input  logic   rst_ni,
  APB_BUS.Slave  apb_slave
);

  // The core decodes 32-bit word addresses, APB carries byte addresses.
  aes aes_i
  (
    .clk        ( clk_i                                ),
    .reset_n    ( rst_ni                               ),
    .cs         ( apb_slave.psel & apb_slave.penable   ),
    .we         ( apb_slave.pwrite                     ),
    .address    ( apb_slave.paddr[9:2]                 ),
    .write_data ( apb_slave.pwdata                     ),
    .read_data  ( apb_slave.prdata                     )
  );

  assign apb_slave.pready  = 1'b1;
  assign apb_slave.pslverr = 1'b0;

endmodule
