/**
 * Código base (incompleto) para implementação de relógios vetoriais.
 * Meta: implementar a interação entre três processos ilustrada na figura
 * da URL: 
 * 
 * https://people.cs.rutgers.edu/~pxk/417/notes/images/clocks-vector.png
 * 
 * Compilação: mpicc -o rvet rvet.c
 * Execução:   mpiexec -n 3 ./rvet
 */
 
#include <stdio.h>
#include <string.h>  
#include <mpi.h>     


typedef struct Clock { 
   int p[3];
} Clock;




int max (int a, int b) {
   int max_number = a;
   if (b > a) {
      max_number = b;
   }
   return max_number;
}

MPI_Datatype clock_type;


void PrintEventState(int pid, Clock *clock) {
   printf("Process: %d, Clock: (%d, %d, %d)\n", pid, clock->p[0], clock->p[1], clock->p[2]);
}

void Event(int pid, Clock *clock){
   clock->p[pid]++;   
   PrintEventState(pid,clock);
}


void Send(int s_pid, int r_pid, Clock *clock){
   Event(s_pid,clock);
   MPI_Send(clock, 1, clock_type, r_pid, 0, MPI_COMM_WORLD);
   PrintEventState(s_pid,clock);
}

void Receive(int s_pid, int r_pid, Clock *clock){
   Event(r_pid,clock);
   Clock rcv_clock;
   MPI_Recv(&rcv_clock,1, clock_type, s_pid, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   clock->p[0] = max(clock->p[0], rcv_clock.p[0]);
   clock->p[1] = max(clock->p[1], rcv_clock.p[1]);
   clock->p[2] = max(clock->p[2], rcv_clock.p[2]);
   PrintEventState(r_pid,clock);
}



// Representa o processo de rank 0
void process0(){
    Clock clock = {{0,0,0}} ;

   // evento interno ocorre
   Event(0,&clock);
   
   // envia mensagem para o processo 1
   Send(0,1,&clock);
   
   // recebe mensagem do processo 1
   Receive(1,0,&clock);
   
   // envia mensagem para o processo 2
   Send(0,2,&clock);
   
   // recebe mensagem do processo 2
   Receive(2,0,&clock);
   
   // envia mensagem para o processo 1
   Send(0,1,&clock);
   
   // evento interno ocorre
   Event(0,&clock);

}

// Representa o processo de rank 1
void process1(){
   
   // inicia com relogio vazio
   Clock clock = {{0,0,0}};
   
   // envia mensagem para o processo 0
   Send(1,0,&clock);
   
   // recebe mensagem do processo 0
   Receive(0,1,&clock);
   
   // recebe mensagem do processo 0
   Receive(0,1,&clock);
   
   
}

// Representa o processo de rank 2
void process2(){
   
   // inicia com relogio vazio
   Clock clock = {{0,0,0}} ;
   
   
   // evento interno ocorre
   Event(2,&clock);
   
   // envia mensagem para o processo 0
   Send(2,0,&clock);
   
   // recebe mensagem do processo 0
   Receive(0,2,&clock);
   
   
   
}

int main(void) {
   int my_rank;               

   MPI_Init(NULL, NULL); 
   
int lengths[1] = {3};

MPI_Aint displacements[1];
Clock dummy_clock;
MPI_Aint base_adress;
MPI_Get_address(&dummy_clock,&base_adress);
MPI_Get_address(&dummy_clock.p[0], &displacements[0]);
displacements[0] = MPI_Aint_diff(displacements[0],base_adress);

MPI_Datatype types[1] = {MPI_INT};
MPI_Type_create_struct(1,lengths,displacements,types,&clock_type);
MPI_Type_commit(&clock_type);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

   if (my_rank == 0) { 
      process0();
   } else if (my_rank == 1) {  
      process1();
   } else if (my_rank == 2) {  
      process2();
   }


   /* Finaliza MPI */
   MPI_Finalize(); 
   
   


   return 0;
}  /* main */
