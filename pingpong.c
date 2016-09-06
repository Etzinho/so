#include "pingpong.h"

void queue_append (queue_t **queue, queue_t *elem) {
    if ((queue != NULL) //&& (*queue != NULL)
        && (elem != NULL)
        && (elem->next == NULL) && (elem->prev == NULL)) {
        
        if (*queue == NULL) {
            // primeira insercao
            *queue = elem;
            elem->next = elem;
            elem->prev = elem;
        }
        else {
            // demais insercoes = insere no fim
            queue_t *aux = (*queue)->prev; // ultimo elemento
            aux->next = elem; // insere elemento no final
            elem->next = *queue; // proximo do ultimo elmento eh o primeiro
            elem->prev = aux; // anterior do elemento inserido eh o "antigo" ultimo elemento
            (*queue)->prev = elem; // anterior do primeiro elemento eh o ultimo
        }
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
	queue_t main_q;
	tasks_id = -1;
	tasks_q = NULL;
	getcontext(&main_t);
	main_q->task = &main_t;
	queue_append((queue_t**)&tasks_q,&main_q);
	//task_switch(&main_t);
}


int task_create (task_t *task,void (*start_func)(void *),void *arg){
	queue_t task_q;
	int task_id;
	char* stack;
	
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

	makecontext (&(task->context), (void*)(*start_func), 1, (void*) arg);
	tasks_id++;
	task->id = tasks_id;
	task_q.task = task;
	queue_append((queue_t**)&tasks_q,&task_q);
	return task_id;	
}

int task_switch(task_t* task){
	queue_t *aux = tasks_q->next;
	while(aux != tasks_q){
		if(aux->task->id == task_id()){
			swapcontext(&(aux->task->context),&task->context);
			task_now = task->id;
			return 0;
		}
		aux = aux->next;
	}
	return -1;
}

void task_exit(int exit_code){
	if(task_id() == 0){
	exit(0);
	}
	else{
	task_switch(&main_t);
	}
	//queue_t *aux = tasks_q->next;
	//while(aux != tasks_q){
	//	if(aux->task->id == task_id()){
	//		task_switch(&main_t);
	//		//free(aux->task);
	//		//free(aux);
	//	}
	//	aux = aux->next;
	//}
}

int task_id(){return task_now;}

