    .global main, mymain, myprintf, get_esp, get_ebp
get_esp:
         movl   %esp, %eax #copy (ESP)stack pointer into the EAX(general purpose pointer)
         ret
get_ebp:
    movl    %ebp, %eax #copy EBP(top stack pointer) to EAX
    ret
main:    
    pushl  %ebp       #Push EBP onto stack
    movl   %esp, %ebp #Copy stack pointer to ebp

/*
# (1). Write ASSEMBLY code to call myprintf(FMT)
#      HELP: How does mysum() call printf() in the class notes.
*/
    pushl $FMT
    call myprintf
    addl $4, %esp #add 4 to stack pointer



/*
# (2). Write ASSEMBLY code to call mymain(argc, argv, env)
#      HELP: When crt0.o calls main(int argc, char *argv[], char *env[]), 
#            it passes argc, argv, env to main(). 
#            Draw a diagram to see where are argc, argv, env?
*/
    pushl 16(%ebp)  
    pushl 12(%ebp)
    pushl 8(%ebp)
    call mymain
    addl $12, %esp #adds 12 to stack pointer



/*
# (3). Write code to call myprintf(fmt,a,b) 
#      HELP: same as in (1) above
*/

    pushl b
    pushl a
    pushl $fmt
    call myprintf
    addl $12, %esp


// (4). Return to caller
    movl  %ebp, %esp
    popl %ebp
    ret


    .data
FMT:    .asciz "main() in assembly call mymain() in C\n"
a:  .long 1234
b:  .long 5678
fmt:    .asciz "a=%d b=%d\n"
