# code to display "hello world"
main: lui t2, -128 #check if LCD is ready to display
ori t2, 7
lui t1, -64
ori t1, 0
add s0, zero, zero
addi s0, 5
load: lh t0, t2, zero
nop
sh t1, t0
sub s1, t0, s0
bez s1, print
nop
nop
jr load
nop
nop


# print on LCD
# LCD controller's registers are mapped to the normal address space 0x8000-0x800F
# Font ram's units are mapped to the normal address space 0x8800-0x8FFF (128 ASCII characters, each stores with 16 16-bit words)

print: add v1, zero, zero #use v1 to write 1 and zero to write 0
addi v1, 1
# set LCD write mode
lui t2, -128
ori t2, 5
sh t2, zero # lcdReadEnable = 0
# lower CS to select the LCD first
lui t2, -128
ori t2, 0 
sh t2, zero # cs = 0
# print all the visible ASCII symbols 
# Start at row 0 (20 rows, 320 pixels/row) and col 0 (20 columns, 240 pixels/column)
# due to the lack of a multiplier, the starting position of any character (upper left corner) must be specified in pixels
# future solution to multiplication can be based on the built-in multiplier of the FPGA with a control structure similar to LCD controller
add t0, zero, zero
add t1, zero, zero
addi t1, 96 #96 visible ASCII symbols
lui s0, 0
ori s0, 0 # starting row
lui s1, 0
ori s1, 0 # starting column
dispLoop: slt t2, t0, t1
bez t2, printDone
nop
nop
add a0, s0, zero
add a1, s1, zero
add v0, t0, zero
addi v0, 32 # askii of space is 32 
jal disp
nop
addi s1, 12
lui t2, 0
ori t2, 240
slt s2, s1, t2
bez s2, delColBias
nop
nop
addi t0, 1
jr dispLoop
nop
nop
delColBias: sub s1, s1, t2
addi s0, 16
lui t2, 1
ori t2, 64
slt s2, s0, t2
bez s2, delRowBias
nop
nop
addi t0, 1
jr dispLoop
nop
nop
delRowBias: sub s0, s0, t2
addi t0, 1
jr dispLoop
nop
nop


# everything done
printDone: lui t2, -128
ori t2, 0
sh t2, v1 # raise CS to de-select the LCD
finish: jr finish
nop
nop

################# procedure to display an ascii character #################
#### a0 stores the starting row, a1 stores the starting column
#### v0 stores the ascii value of the character to be displayed
disp: addi sp, -1
sh sp, s0
addi sp, -1
sh sp, s1
addi sp, -1
sh sp, s2
addi sp, -1
sh sp, t0
addi sp, -1
sh sp, t1
addi sp, -1
sh sp, t2
## set row range, which does not change for each column. Writing will be performed in a column-major order
# lower cd to send command
lui t2, -128
ori t2, 1
sh t2, zero # cd = 0 for command
# write command word 0x2b to configure the row (page) (0-319)
lui t2, -128
ori t2, 6
lui t1, 0 
ori t1, 43 # prepare command word: ili9341_pageaddrset 0x2b
sh t2, t1 # write command word to lcd port
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# raise CD to send data next
lui t2, -128
ori t2, 1
sh t2, v1 # cd = 1 for data 
# start row address - high byte 
lui t2, -128
ori t2, 6
add t1, a0, zero
add s0, zero, zero
addi s0, 8 
srl t1, t1, s0
sh t2, t1 # write high byte of start row
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# start row address - low byte
lui t2, -128
ori t2, 6
sh t2, a0 # write low byte of start row, a0[7:0] will be effective
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# end row address - high byte 
lui t2, -128
ori t2, 6
add t1, a0, zero 
addi t1, 15
srl t1, t1, s0 #s0 still has value 8
sh t2, t1 # write high byte of end row
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# end row address - low byte
lui t2, -128
ori t2, 6
add t1, a0, zero 
addi t1, 15
sh t2, t1 # write low byte of end row, t1[7:0] will be effective
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
## 12 columns to write
add t0, zero, zero
addi t0, 12 # set column loop variable (outer)
columnLoop: bez t0, exit
nop
nop
jr nextColumn
nop
nop
exit: lh t2, sp, zero
addi sp, 1
lh t1, sp, zero
addi sp, 1
lh t0, sp, zero
addi sp, 1
lh s2, sp, zero
addi sp, 1
lh s1, sp, zero
addi sp, 1
lh s0, sp, zero
addi sp, 1
jr ra
nop
nop
nextColumn: addi t0, -1
## set column address
# lower CD to send command first
lui t2, -128
ori t2, 1
sh t2, zero # cd = 0 for command
# write command word 0x2A to configure the column address (0-239)
lui t2, -128
ori t2, 6
lui t1, 0 
ori t1, 42 # prepare command word: ILI9341_COLADDRSET 0x2A
sh t2, t1 # write command word to LCD port
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# raise CD to send data next
lui t2, -128
ori t2, 1
sh t2, v1 # cd = 1 for data 
# start col address - high byte 
lui t2, -128
ori t2, 6
add t1, a1, t0
add s0, zero, zero
addi s0, 8
srl t1, t1, s0
sh t2, t1 # write high byte of start column
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# start col address - low byte
lui t2, -128
ori t2, 6
add t1, a1, t0 
sh t2, t1 # write low byte of start column, t1[7:0] are effective
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# end col address - high byte 
lui t2, -128
ori t2, 6
add t1, a1, t0 
srl t1, t1, s0 #s0 still has value 8
sh t2, t1 # write high byte of end column
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# end col address - low byte
lui t2, -128
ori t2, 6
add t1, a1, t0 
sh t2, t1 # write low byte of end column, t1[7:0] are effective
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
## read the font ram to get the column word into s1
add s2, zero, zero
addi s2, 17 # the first 5 bits must be 10001
add s0, zero, zero
addi s0, 7
sll s2, s2, s0
add s2, s2, v0 # add the ascii offset - 7 bits
add s0, zero, zero
addi s0, 4
sll s2, s2, s0
add s2, s2, t0 # add the column offset - 4 bits
lh s1, s2, zero # a column of pixels is loaded into s1
nop # load use nop
## 16 pixels to write  
add t1, zero, zero
addi t1, 16
lui s0, -128
lui t2, -128
ori t2, 1
sh t2, zero # cd = 0 for command
lui t2, -128
ori t2, 6
lui s2, 0
ori s2, 44 # prepare command word: 0x2c (fill in data)
sh t2, s2 # write command word to lcd port
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
lui t2, -128
ori t2, 1
sh t2, v1 # cd = 1 for data
pixelLoop: bez t1, columnDone
nop
nop
jr nextPixel
nop
nop
columnDone: jr columnLoop
nop
nop
nextPixel: and s2, s0, s1
srl s0, s0, v1 # prepare for next bit
addi t1, -1 # prepare for next loop
bez s2, write0
nop
nop 
# write 1 with 16'd65535: white pixel
# color high byte
write1: lui t2, -128
ori t2, 6
lui s2, 0 
ori s2, -1
sh t2, s2 # write color high byte, the color is white here 0xFFFF, high byte 0xFF (-1) and low byte 0xFF (-1)
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# color low byte
lui t2, -128
ori t2, 6
lui s2, 0 
ori s2, -1
sh t2, s2 # write color low byte
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
jr pixelLoop
nop
nop
# write 0 with 16'd31: blue pixel
# color high byte
write0: lui t2, -128
ori t2, 6
lui s2, 0 
ori s2, 0
sh t2, s2 # write color high byte, the color is white here 0xFFFF, high byte 0xFF (-1) and low byte 0xFF (-1)
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
# color low byte
lui t2, -128
ori t2, 6
lui s2, 0 
ori s2, 31
sh t2, s2 # write color low byte
lui t2, -128
ori t2, 2
sh t2, zero # write a wr low
lui t2, -128
ori t2, 2
sh t2, v1 # write a wr high
jr pixelLoop
nop
nop
##### disp function body ends #####

##### a delay procedure ###########
delay: addi sp, -1
sh sp, t0
addi sp, -1
sh sp, t1 #push t0 and t1 into the stack
add t0, a0, zero
add t1, a1, zero 
inner: add a1, t1, zero 
more: addi a1, -1
bez a1, updates0
nop
nop
jr more
nop
nop
updates0: addi a0, -1
bez a0, rtn
nop
nop
jr inner
nop
nop
rtn: add a0, t0, zero
add a1, t1, zero
lh t1, sp, zero
addi sp, 1
lh t0, sp, zero
addi sp, 1
jr ra
nop
nop

