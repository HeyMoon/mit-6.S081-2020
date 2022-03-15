.section .text
.global sum_to
# sum_to(int n)
# expects argument in a0
# returns results in a0
sum_to:
      mv t0, a0  # t0 <- a0
      li a0, 0   # a0 <- 0
   loop:
      add a0, a0, t0  # a0 <- a0 + t0
      addi t0, t0, -1 # t0 <- t0 - 1
      bnez t0, loop  # if t0 > 0, goto loop
      ret

main:
       li a0,10 # a0 <- 10
       call sum_to # a0 <- sum_to(a0)sp