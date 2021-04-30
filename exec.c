#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdlib.h>

int do_something(void){        //added this function
    printf("[!] Cargando\n\n");
    return 0;
}

void change_endianness(char *big_endian_ptr, char *little_endian_ptr){
	little_endian_ptr[0] = big_endian_ptr[3];
	little_endian_ptr[1] = big_endian_ptr[2];
	little_endian_ptr[2] = big_endian_ptr[1];
	little_endian_ptr[3] = big_endian_ptr[0];    
}

int main(int argc, char **argv){

    if(argc < 2){

        printf("ejecute con:  %s [program] [break point]\n", argv[0]);
        exit(1);

    } else{
        printf("debbugeando el programa: [ %s ], punto de rruptura: [ 0x%x == %d ], numero de argumentos: %d\n", argv[1], argv[2], argv[2], argc);

    }

    pid_t child_pid;
    char *programname = argv[1];

    child_pid = fork();
    if (child_pid == 0){
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        do_something();      //added this function call
        execl(programname, programname, NULL);
    }

    else if (child_pid > 0) {

        int status;
        wait(&status);

        while (WIFSTOPPED(status)){

            struct user_regs_struct regs;
            ptrace(PTRACE_GETREGS, child_pid, 0, &regs);

            unsigned instr = ptrace(PTRACE_PEEKTEXT, child_pid, regs.rip, 0);

            int little_endian;
            change_endianness((char *)&instr, (char *)&little_endian);

            printf("----- RIP = [ 0x%08x == %d ], instr = [ 0x%x ] -----\n", regs.rip, regs.rip, little_endian);
            printf("- EAX = %15.llx,\t EAX = %lld\n", regs.rax, regs.rax);
            printf("- EBX = %15.llx,\t EBX = %lld\n", regs.rbx, regs.rbx);
            if(regs.rcx == 0){
                printf("- ECX = 15.%llx,\t ECX = %lld\n", regs.rcx, regs.rcx);
            } else{
                printf("- ECX = %15.llx,\t ECX = %lld\n", regs.rcx, regs.rcx);
            }
            printf("- EDX = %15.llx,\t EDX = %lld\n", regs.rdx, regs.rdx);
            printf("- RDI = %15.llx,\t RDI = %lld\n", regs.rdi, regs.rdi);
            printf("-------------------------------------------------------------------\n");

            ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0);
            wait(&status);

            if(argc != 2 && argc > 2){

                if ((int)regs.rip == atoi(argv[2])){

                    exit(0);

                } else{

                    printf("RIP = %d,  breakpoint = %d \n\n", regs.rip, atoi(argv[2]));

                }


            } else{
                argv[2] = "000";
                puts(" ");
            }

        }

    }

} 
