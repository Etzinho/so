// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__
#ifndef NULL
#define NULL ((void *) 0)
#endif
#define STACKSIZE 32768
#include <ucontext.h>

// Estrutura que define uma tarefa
typedef struct task_t
{
	ucontext_t context;
	int id;
} task_t ;

typedef struct queue_t
{
	struct queue_t *prev ;  // aponta para o elemento anterior na fila
	struct queue_t *next ;  // aponta para o elemento seguinte na fila
	task_t *task;
} queue_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

queue_t *tasks_q;
int tasks_id;
int task_now;
task_t main_t;
#endif
