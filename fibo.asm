main: add a0, zero, zero
addi a0, 10
jal fibo
nop
report: lui t0, 192
ori t0, 0
sh t0, v0
stop: jr stop
nop
nop

fibo: add t0, zero, zero
addi t0, 2 #loop variable n
add s0, zero, zero #fn-2
add s1, zero, zero #fn-1
addi s1, 1
next: sub t1, t0, a0
bez t1, rtn #check loop variable
nop
nop
add s2, s0, s1
add s0, s1, zero
add s1, s2, zero
add v0, s2, zero
addi t0, 1 #update loop variable
jr next
nop
nop
rtn: jr ra
nop
nop
 
