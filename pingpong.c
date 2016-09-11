#include "pingpong.h"

void queue_append (task_t *queue, task_t *elem) {
        if (queue == NULL) {
            // primeira insercao
            queue = elem;
            elem->next = elem;
            elem->prev = elem;
        }
        else {
            // demais insercoes = insere no fim
            task_t *aux = queue->prev; // ultimo elemento
            aux->next = elem; // insere elemento no final
            elem->next = queue; // proximo do ultimo elmento eh o primeiro
            elem->prev = aux; // anterior do elemento inserido eh o "antigo" ultimo elemento
            queue->prev = elem; // anterior do primeiro elemento eh o ultimo
        }
    }

queue_t *queue_remove (queue_t **queue, queue_t *elem) {
    if ((queue != NULL) && (*queue != NULL) // fila existe?
        && ((*queue)->next != NULL) // esta vazia?
        && (elem != NULL) // elemento existe?
        ) {
        queue_t *aux = *queue;
        // busca pelo elemento na lista
        while ((aux->next != *queue) && (aux != elem)) {
            aux = aux->next;
        }
        if (aux == elem) {
            // encontrou, remove da lista
            // (apenas acerta os apontamentos
            elem->next->prev = elem->prev;
            elem->prev->next = elem->next;

            if (*queue == elem) {
                // o elemento removido eh o primeiro
                // precisa acertar a cabeca da lista
                *queue = elem->next;
                if (*queue == elem)
                    // o elemento removido eh o unico elemento da lista
                    // a lista deve ficar vazia
                    *queue = NULL;
            }

            // desconecta o elemento da lista
            elem->prev = elem->next = NULL;
            return elem;
        }
    }
    return NULL;
}

void pingpong_init(){
	char* stack;
	tasks_id = 0; //inicia interador para id das tasks
	task_now = tasks_id; //seta tarefa atual a tarefa main
	//inicia as variaveis da tarefa main
	getcontext(&(tasks_q.context));
	stack = malloc (STACKSIZE) ;
	if (stack)
	{
	  tasks_q.context.uc_stack.ss_sp = stack ;
	  tasks_q.context.uc_stack.ss_size = STACKSIZE;
	  tasks_q.context.uc_stack.ss_flags = 0;
	  tasks_q.context.uc_link = 0;
	}
	tasks_q.id = 0;
	tasks_q.next = &tasks_q;
	tasks_q.prev = &tasks_q;
}


int task_create (task_t *task,void (*start_func)(void *),void *arg){
	int task_id;
	char* stack;
	//seta contexto da task
	getcontext(&(task->context));
	stack = malloc (STACKSIZE) ;
	if (stack)
	{
	  task->context.uc_stack.ss_sp = stack ;
	  task->context.uc_stack.ss_size = STACKSIZE;
	  task->context.uc_stack.ss_flags = 0;
	  task->context.uc_link = 0;
	}
	else
	{
	  perror ("Erro na criação da pilha: ");
	  exit (-1);
	}
	makecontext (&(task->context), (void*)start_func, 1, (void*) arg);
	//cria id da nova task
	tasks_id++;
	task->id = tasks_id;
	task->next = NULL;
	task->prev = NULL;
	//aloca a tarefa na fila da task main
	queue_append(&tasks_q,task);
	return task_id;
}

int task_switch(task_t* task){
	task_t * aux;
	//ponteiro para o começo da fila de tasks
	aux = &tasks_q;
	do {
	if(aux->id == task_id()){//procura na fila de tasks a tarefa atual
		task_now = task->id;//seta a task atual a task a ser trocada
		swapcontext(&aux->context,&task->context);
		return 0;
	}
	aux = aux->next;}while(aux->id != tasks_q.id);
	return -1;
}

void task_exit(int exit_code){
	task_switch(&tasks_q);//retorna para task main
}

int task_id(){return task_now;}//retorna id da task atual

