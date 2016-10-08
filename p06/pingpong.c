#include "pingpong.h"
task_t Dispatcher;
task_t* task_running;
int lock;

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
	lock = 0;
	tick = 0;
	tasks_id = 0; //inicia interador para id das tasks
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
	tasks_q.user = 1;
	tasks_q.quantum = 20;
	tasks_q.activations = 0;
	tasks_q.exec_time = systime();
	tasks_q.proc_time = 0;
	queue_append(&queue_t,&tasks_q);
	task_running = &tasks_q;
	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	//setvbuf (stdout, 0, _IONBF, 0) ;
	#ifdef DEBUG
	printf("pinpong_init(): criou a task main id %d\n",tasks_q.id);
	#endif
	task_create(&Dispatcher,dispatcher_body,"");
	task_setprio(&Dispatcher,20);
	action.sa_handler = tratador ;
	sigemptyset (&action.sa_mask) ;
	action.sa_flags = 0 ;
	if (sigaction (SIGALRM, &action, 0) < 0)
	{
	perror ("Erro em sigaction: ") ;
	exit (1) ;
	}
	timer.it_value.tv_usec = 1 ;      // primeiro disparo, em micro-segundos
	timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
	timer.it_interval.tv_usec = 1000;   // disparos subsequentes, em micro-segundos
	timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos
	if (setitimer (ITIMER_REAL, &timer, 0) < 0)
	{
	perror ("Erro em setitimer: ") ;
	exit (1) ;
	}
}


int task_create (task_t *task,void (*start_func)(void *),void *arg){
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
	task->user = 1;
	task->quantum = 20;
	task->activations = 0;
	task->exec_time = systime();
	task->proc_time = 0;
	if(task->id == Dispatcher.id){task->user = 0;}
	//aloca a tarefa na fila da task main
	queue_append(&queue_t,task);
	#ifdef DEBUG
	printf("task_create(): criou a task %d\n",task->id);
	#endif
	return task->id;
}

int task_switch(task_t* task){
	task_t* aux = task_running;
	unsigned int time;
	#ifdef DEBUG
	printf("task_switch(): trocou contexto task %d -> %d\n",task_running->id,task->id);
	#endif
	queue_remove(&queue_t,task_running);
	if(task_running->morreu==0){
		queue_append(&queue_t,task_running);
		task_running->quantum = 20;
	}
	task_running = task;
	lock = 0;
	swapcontext(&aux->context,&task->context);
	task->activations++;
	return 0;
}

void task_exit(int exit_code){
	#ifdef DEBUG
	printf("task_exit(): encerrou a tarefa %d\n",task_id());
	#endif
	unsigned int time;
	if(task_id() == 1){
		task_running->morreu = 1;
		task_running->exec_time = systime() - task_running->exec_time;
		printf("Task %d exit: running time %u ms, cpu time %u ms, %d activations.\n", task_running->id, task_running->exec_time, task_running->proc_time, task_running->activations);
		task_switch(&tasks_q);
	}
	else{
	/*	do {
		if(aux->id == 1){//procura na fila de tasks a tarefa atual
			do{	
			if(ended->id == task_id()){
				ended->morreu = 1;
			}		
			ended = ended->next;}while(ended->id != queue_t->id);
			task_switch(aux);
		}
		aux = aux->next;}while(aux->id != queue_t->id);*/
		task_running->morreu = 1;;
		task_running->exec_time = systime() - task_running->exec_time;
		printf("Task %d exit: running time %u ms, cpu time %u ms, %d activations.\n", task_running->id, task_running->exec_time, task_running->proc_time, task_running->activations);
		task_switch(&Dispatcher);
	}
	//task_switch(&tasks_q);//retorna para task main
}

int task_id(){return task_running->id;}//retorna id da task atual

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
	/*task_t *aux = queue_t;
	task_t *next = task_running;
	do{
		if(aux->id != next->id){
			aux->prio_din = aux->prio_din - 1;
		}
	aux = aux->next;}while(aux->id != queue_t->id);
	next->prio_din = task_getprio(next);
	return next;*/
	return queue_t;
}

void task_yield(){
	task_switch(&Dispatcher);
}

void task_setprio (task_t *task, int prio){
	if(task == NULL){
		task_running->prio_static = prio;
		task_running->prio_din = task->prio_static;
	}
	else{
		task->prio_static = prio;
		task->prio_din = task->prio_static;
	}
}

int task_getprio (task_t *task){
	if(task == NULL){
		return task_running->prio_static;
	}
	else{
		return task->prio_static;
	}

}

void tratador(int signum){
	tick++;
	if(lock == 0){
		task_running->proc_time++;
		lock = 1;
		if(task_running->user == 1){
			if(task_running->quantum == 0){
				task_running->quantum--;
				task_switch(&Dispatcher);
			}
			else if(task_running->quantum > 0){
				task_running->quantum--;
			}
		}
		lock = 0;
	}
}

unsigned int systime(){return tick;}
