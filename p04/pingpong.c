#include "pingpong.h"
task_t Dispatcher;

void queue_append (task_t **queue, task_t *elem) {
        if (*queue == NULL) {
            // primeira insercao
            *queue = elem;
            elem->next = elem;
            elem->prev = elem;
        }
        else {
            // demais insercoes = insere no fim
            task_t *aux = (*queue)->prev; // ultimo elemento
            aux->next = elem; // insere elemento no final
            elem->next = (*queue); // proximo do ultimo elmento eh o primeiro
            elem->prev = aux; // anterior do elemento inserido eh o "antigo" ultimo elemento
            (*queue)->prev = elem; // anterior do primeiro elemento eh o ultimo
        }
    }

task_t* queue_remove (task_t **queue, task_t *elem) {
	task_t * aux = *queue;
	do{		
	aux = aux->next;}while(aux->id != elem->id && aux->id != (*queue)->id);
	if(aux->id == elem->id){
		task_t *prev, *next;
		next = aux->next;
		prev = aux->prev;
		next->prev = prev;
		prev->next = next;
		if((*queue)->id == elem->id){
			(*queue) = next;
			if((*queue)->id == elem->id){
				*queue = NULL;
			}
		}
		elem->next = NULL;
		elem->prev = NULL;
		return elem;
	}
    return NULL;
}

void pingpong_init(){
	char* stack;
	queue_t = NULL;
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
	tasks_q.morreu = 1;
	tasks_q.prio_static = 20;
	tasks_q.prio_din = 20;
	tasks_q.next = NULL;
	tasks_q.prev = NULL;
	queue_append(&queue_t,&tasks_q);
	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	//setvbuf (stdout, 0, _IONBF, 0) ;
	#ifdef DEBUG
	printf("pinpong_init(): criou a task main id %d\n",tasks_q.id);
	#endif
	task_create(&Dispatcher,dispatcher_body,"");
	task_setprio(&Dispatcher,20);
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
	task->morreu = 0;
	task->prio_static = 0;
	task->prio_din = 0;
	//aloca a tarefa na fila da task main
	queue_append(&queue_t,task);
	#ifdef DEBUG
	printf("task_create(): criou a task %d\n",task->id);
	#endif
	return task_id;
}

int task_switch(task_t* task){
	task_t * aux;
	//ponteiro para o começo da fila de tasks
	aux = queue_t;
	do {
	if(aux->id == task_id()){//procura na fila de tasks a tarefa atual
		task_now = task->id;//seta a task atual a task a ser trocada
		#ifdef DEBUG
		printf("task_switch(): trocou contexto task %d -> %d\n",aux->id,task->id);
		#endif
		queue_remove(&queue_t,aux);
		if(aux->morreu==0){queue_append(&queue_t,aux);}
		swapcontext(&aux->context,&task->context);
		return 0;
	}
	aux = aux->next;}while(aux->id != queue_t->id);
	return -1;
}

void task_exit(int exit_code){
	#ifdef DEBUG
	printf("task_exit(): encerrou a tarefa %d\n",task_id());
	#endif
	task_t *aux = queue_t;
	task_t *ended = queue_t;
	if(task_id() == 1){
		task_switch(&tasks_q);
	}
	else{
		do {
		if(aux->id == 1){//procura na fila de tasks a tarefa atual
			do{	
			if(ended->id == task_id()){
				ended->morreu = 1;
			}		
			ended = ended->next;}while(ended->id != queue_t->id);
			task_switch(aux);
		}
		aux = aux->next;}while(aux->id != queue_t->id);
	}
	//task_switch(&tasks_q);//retorna para task main
}

int task_id(){return task_now;}//retorna id da task atual

void dispatcher_body(){
	task_t *schedu = NULL;
	task_t *aux = queue_t;
	while(1){
		if(aux->id == 1 && aux->next->id == 1){task_exit(0);}
		schedu = scheduler();
		if(schedu != NULL){
			task_switch(schedu);
		}
	}
}

task_t* scheduler(){
	task_t *aux = queue_t;
	task_t *next = queue_t;
	
	do{
		if(aux->prio_din <= next->prio_din){
			next = aux;
		}
	aux = aux->next;}while(aux->id != queue_t->id);
	
	do{
		if(aux->id != next->id){
			aux->prio_din = aux->prio_din - 1;
		}
	aux = aux->next;}while(aux->id != queue_t->id);
	next->prio_din = task_getprio(next);
	return next;
}

void task_yield(){
	task_switch(&Dispatcher);
}

void task_setprio (task_t *task, int prio){
	if(task == NULL){
		task = queue_t;
		do{
			if(task->id == task_id()){
				task->prio_static = prio;
				task->prio_din = task->prio_static;
			}
		task = task->next;}while(task->id != queue_t->id);
	}
	else{
		task->prio_static = prio;
		task->prio_din = task->prio_static;
	}
}

int task_getprio (task_t *task){
	if(task == NULL){
		task = queue_t;
		do{
			if(task->id == task_id()){
				return task->prio_static;
			}
		task = task->next;}while(task->id != queue_t->id);
	}
	else{
		return task->prio_static;
	}

}

