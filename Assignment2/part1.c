#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define MAX_NAME_SIZE 50
#define MAX 10

enum algorithm{FirstComeFirstServe,ShortestJobFirst,Priority,RoundRobin,PriorityWithRoundRobin};

typedef struct task{
    char name[MAX_NAME_SIZE];
    int priority;
    int arrival_time;
    int burst_time;
}task;

typedef struct Queue
{
        int capacity;
        int size;
        int front;
        int rear;
        task *elements;
}Queue;

void build(char* line, task *current_task);
int table(char* file_address, task tasks[MAX]);
void display(task tasks[MAX], int num_of_tasks);
void schedule(enum algorithm algorithm, task tasks[MAX], int num_of_tasks);
void copy(task * destination, task * origin);
int isEmpty(Queue *Q);
Queue * createQueue(int maxElements);
task * Dequeue(Queue *Q);
void Enqueue(Queue *Q, task *element);

int main(){

    enum algorithm currentAlgorithm;
    task tasks[MAX];
    char file_address1[50];
    printf("Please Enter File Address:\n");
    scanf("%s",file_address1);
    int num_of_tasks_file1 = table(file_address1,tasks);
    display(tasks,num_of_tasks_file1);

    printf("Please choose algorithm(FirstComeFirstServe = 0,ShortestJobFirst = 1,Priority = 2,RoundRobin = 3,PriorityWithRoundRobin = 4:\n");
    scanf("%d",&currentAlgorithm);

    printf("Algorithm Output:\n");

    schedule(currentAlgorithm,tasks,num_of_tasks_file1);

    return 0;
 }

//given a line from file and a pointer to a task space in array, parse the line to task
void build(char* line, task *current_task){

    //remove \n from end
    if(line[strlen(line)-1]=='\n')
            line[strlen(line)-1]=0;

    //first we seperate the string with coma delimeter
    char inputs[4][MAX_NAME_SIZE] = { 0 };
    char delim[] = ",";
    char *ptr = strtok(line, delim);
    int i = 0;

	while(ptr != NULL)
	{   
		strcpy(inputs[i++],ptr);
		ptr = strtok(NULL, delim);
	}

    //and then we copy the data into task structure
    strcpy(current_task->name,inputs[0]);
    current_task->priority = atoi(inputs[1]);
    current_task->arrival_time = atoi(inputs[2]);
    current_task->burst_time = atoi(inputs[3]);
 }

//given the tasks array and the file address, open file and put the tasks in the array
//return number of tasks in file
int table(char* file_address, task tasks[MAX]){

    int i = 0;
    FILE *fp; // declaration of file pointer
    char con[1000]; // variable to read the content
    fp = fopen(file_address,"r");// opening of file
    if (!fp)// checking for error
    {
        printf("FILE READING ERROR WHILE READING %s",file_address);
        return 0;
    }
    while (fgets(con,1000, fp)!=NULL)// reading file content and build database    
        build(con,&tasks[i++]);
    fclose(fp); // closing file
    return i;
}

void display(task tasks[MAX], int num_of_tasks){
    printf("Task Arrival Time Burst Time Priority\n");
    printf("---- ------------ ---------- --------\n");
    for(int i = 0;i < num_of_tasks;i ++)
        printf("%4s %12d %10d %8d\n",tasks[i].name,tasks[i].arrival_time,tasks[i].burst_time,tasks[i].priority);
}

void schedule(enum algorithm algorithm, task tasks[MAX], int num_of_tasks){
    char outputTaskID[100][MAX_NAME_SIZE];
    int outputCPUTime[100];
    int outputNumOfChunks = 0;
    int ShortestJobFirst_time = 0;
    int RR_time = 0;
    int taskIndex = 0;
    int outputIndex = 0;
    int round_robin_q;
    int RR_task_count = 0;
    int RR_currently_occupied_flag = 0;

    int priorityWithRR_hasArrived[MAX] = { 0 };
    int priorityWithRR_priorities[MAX] = { INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX };
    int priorityWithRR_is_task_done[MAX] = { 0 };
    int priorityWithRR_is_in_queue[MAX] = { 0 };
    int priorityWithRR_time = 0;
    int priorityWithRR_counter;
    int occupied_CPU_time_left = 0;
    int priorityWithRR_tasks_left_for_RR;
    int priority_or_RR = 0;


    task algorithm_tasks[MAX];
    task temp;
    task * RR_temp;

    Queue * Q = createQueue(MAX);
    switch(algorithm){

        case FirstComeFirstServe:

            //number of elements in output is the same as number of tasks because FCFS is non-preemptive
            outputNumOfChunks = num_of_tasks;

            //copy tasks into algorithm_tasks
            for(int i = 0;i < num_of_tasks;i ++)
                copy(&algorithm_tasks[i],&tasks[i]);
            
            //sort algorithm_tasks by arrival time using bubble sort
            for(int i = 0;i < num_of_tasks;i ++)
                for(int j = i;j < num_of_tasks;j ++)
                    if(algorithm_tasks[j].arrival_time < algorithm_tasks[i].arrival_time){
                        copy(&temp,&algorithm_tasks[j]);
                        copy(&algorithm_tasks[j],&algorithm_tasks[i]);
                        copy(&algorithm_tasks[i],&temp);
                    }

            //copy algorithm_tasks into output
            for(int i = 0;i < num_of_tasks;i ++){
                strcpy(outputTaskID[i],algorithm_tasks[i].name);
                outputCPUTime[i] = algorithm_tasks[i].burst_time;
            }
        break;
        
        case ShortestJobFirst:

            //number of elements in output is the same as number of tasks because ShortestJobFirst is non-preemptive
            outputNumOfChunks = num_of_tasks;

            //copy tasks into algorithm_tasks
            for(int i = 0;i < num_of_tasks;i ++)
                copy(&algorithm_tasks[i],&tasks[i]);

            //sort algorithm_tasks by burst time using bubble sort
            for(int i = 0;i < num_of_tasks;i ++)
                for(int j = i;j < num_of_tasks;j ++)
                    if(algorithm_tasks[j].burst_time < algorithm_tasks[i].burst_time){
                        copy(&temp,&algorithm_tasks[j]);
                        copy(&algorithm_tasks[j],&algorithm_tasks[i]);
                        copy(&algorithm_tasks[i],&temp);
                    }

            //sort algorithm_tasks by arrival time using bubble sort
            for(int i = 0;i < num_of_tasks;i ++)
                for(int j = i;j < num_of_tasks;j ++)
                    if(algorithm_tasks[j].arrival_time < algorithm_tasks[i].arrival_time){
                        copy(&temp,&algorithm_tasks[j]);
                        copy(&algorithm_tasks[j],&algorithm_tasks[i]);
                        copy(&algorithm_tasks[i],&temp);
                    }

            
            while(1){
                
                //copy task in taskIndex to output
                strcpy(outputTaskID[outputIndex],algorithm_tasks[taskIndex].name);
                outputCPUTime[outputIndex] = algorithm_tasks[taskIndex].burst_time;
                outputIndex++;

                //advance the time by the burst time amount
                ShortestJobFirst_time += algorithm_tasks[taskIndex].burst_time;

                //mark the task as done
                algorithm_tasks[taskIndex].burst_time = -1;

                //decrement number of tasks left. if no more tasks left, stop.
                num_of_tasks --;
                if(!num_of_tasks)
                    break;

                //reset taskIndex
                taskIndex = -1;

                //find next task to run on CPU
                while(taskIndex == -1){
                    
                    int minBurstTime = INT_MAX;
                    for(int j = 0;j < outputNumOfChunks;j ++){

                        //if task already done continue
                        if(algorithm_tasks[j].burst_time == -1)
                            continue;

                        //if task not arrived yet continue
                        if(ShortestJobFirst_time < algorithm_tasks[j].arrival_time)
                            continue;

                        //if task has arrived and not done yet, check if it has minimum burst time, if so make it ready for running
                        if(algorithm_tasks[j].burst_time < minBurstTime){
                            minBurstTime = algorithm_tasks[j].burst_time;
                            taskIndex = j;
                        }
                    }

                    //if no task is available, increment time and try again
                    if(taskIndex == -1)
                        ShortestJobFirst_time ++;
                } //end of while

            } //end of while

        break;

        case Priority:

            //number of elements in output is the same as number of tasks because Priority is non-preemptive
            outputNumOfChunks = num_of_tasks;

            //copy tasks into algorithm_tasks
            for(int i = 0;i < num_of_tasks;i ++)
                copy(&algorithm_tasks[i],&tasks[i]);

            //sort algorithm_tasks by priority using bubble sort
            for(int i = 0;i < num_of_tasks;i ++)
                for(int j = i;j < num_of_tasks;j ++)
                    if(algorithm_tasks[j].priority < algorithm_tasks[i].priority){
                        copy(&temp,&algorithm_tasks[j]);
                        copy(&algorithm_tasks[j],&algorithm_tasks[i]);
                        copy(&algorithm_tasks[i],&temp);
                    }

            //sort algorithm_tasks by arrival time using bubble sort
            for(int i = 0;i < num_of_tasks;i ++)
                for(int j = i;j < num_of_tasks;j ++)
                    if(algorithm_tasks[j].arrival_time < algorithm_tasks[i].arrival_time){
                        copy(&temp,&algorithm_tasks[j]);
                        copy(&algorithm_tasks[j],&algorithm_tasks[i]);
                        copy(&algorithm_tasks[i],&temp);
                    }

            
            while(1){
                
                //copy task in taskIndex to output
                strcpy(outputTaskID[outputIndex],algorithm_tasks[taskIndex].name);
                outputCPUTime[outputIndex] = algorithm_tasks[taskIndex].burst_time;
                outputIndex++;

                //advance the time by the burst time amount
                ShortestJobFirst_time += algorithm_tasks[taskIndex].burst_time;

                //mark the task as done
                algorithm_tasks[taskIndex].burst_time = -1;

                //decrement number of tasks left. if no more tasks left, stop.
                num_of_tasks --;
                if(!num_of_tasks)
                    break;

                //reset taskIndex
                taskIndex = -1;

                //find next task to run on CPU
                while(taskIndex == -1){
                    
                    int minPriority = INT_MAX;
                    for(int j = 0;j < outputNumOfChunks;j ++){

                        //if task already done continue
                        if(algorithm_tasks[j].burst_time == -1)
                            continue;

                        //if task not arrived yet continue
                        if(ShortestJobFirst_time < algorithm_tasks[j].arrival_time)
                            continue;

                        //if task has arrived and not done yet, check if it has minimum priority, if so make it ready for running
                        if(algorithm_tasks[j].priority < minPriority){
                            minPriority = algorithm_tasks[j].priority;
                            taskIndex = j;
                        }
                    }

                    //if no task is available, increment time and try again
                    if(taskIndex == -1)
                        ShortestJobFirst_time ++;
                } //end of while

            } //end of while

        break;

        case RoundRobin:
            
            printf("Please enter Q for round robin:");
            scanf("%d", &round_robin_q);

            //copy tasks into algorithm_tasks
            for(int i = 0;i < num_of_tasks;i ++)
                copy(&algorithm_tasks[i],&tasks[i]);

            
            while(1){

                //push tasks that just arrived into RR_queue
                for(int i = 0;i < num_of_tasks;i ++)
                    if(algorithm_tasks[i].arrival_time == RR_time){
                        Enqueue(Q,&algorithm_tasks[i]);
                        RR_task_count ++;
                    }

                //if queue is empty and all tasks have arrived, stop
                if(isEmpty(Q) && RR_task_count == num_of_tasks)
                    break;

                
                //if CPU is currently occupied
                if(RR_currently_occupied_flag){

                    //decrease the currently_occupied_flag
                    RR_currently_occupied_flag--;

                    //if it reaches zero, push RR_temp back into queue
                    if(!RR_currently_occupied_flag){

                        RR_temp->burst_time -= round_robin_q;
                        if(RR_temp->burst_time>0){
                            Enqueue(Q,RR_temp);
                        }
                        free(RR_temp);
                    }
                }

                //if CPU is not currently occupied(task has finished its CPU time)
                else{

                    //if queue is empty but not all tasks have arrived, only increase time
                    if(isEmpty(Q)){
                        RR_time++;
                        continue;
                    }

                    //get first task in RR_queue, put it in RR_temp
                    RR_temp = Dequeue(Q);

                    //if burst time of RR_temp is larger than the round_robin_q, put it in output for round_robin_q timestamps and update occupied_flag
                    if(RR_temp->burst_time>=round_robin_q){

                        strcpy(outputTaskID[outputIndex],RR_temp->name);
                        outputCPUTime[outputIndex] = round_robin_q;
                        outputNumOfChunks++;

                        RR_currently_occupied_flag = round_robin_q;

                    
                    }

                    //if burst time of RR_temp is smaller than the round_robin_q, put it in output for as much as it has left timestamps
                    else{
                        strcpy(outputTaskID[outputIndex],RR_temp->name);
                        outputCPUTime[outputIndex] = RR_temp->burst_time;
                        outputNumOfChunks++;
                        free(RR_temp);
                    }
                    outputIndex++;

                }
                RR_time++;
                
            }
        break;

        case PriorityWithRoundRobin:

        printf("Please enter Q for round robin:");
            scanf("%d", &round_robin_q);

        //copy tasks into algorithm_tasks
        for(int i = 0;i < num_of_tasks;i ++)
            copy(&algorithm_tasks[i],&tasks[i]);

        while(1){

            //update priorities of tasks that just arrived and remember that they arrived
            for(int i = 0;i < num_of_tasks;i ++)
                if(algorithm_tasks[i].arrival_time == priorityWithRR_time){
                    priorityWithRR_priorities[i] = algorithm_tasks[i].priority;
                    priorityWithRR_hasArrived[i] = 1;
                }

            //if amount of done tasks equals num of tasks, stop
            int doneCounter = 0;
            for(int i = 0;i < num_of_tasks;i ++)
                if(priorityWithRR_is_task_done[i])
                    doneCounter++;
            if(doneCounter==num_of_tasks)
                break;

            //if all tasks that arrived are done, increase time and continue
            int areAllArrivedDone = 1;
            for(int i = 0;i < num_of_tasks;i ++)
                if(priorityWithRR_hasArrived[i] && !priorityWithRR_is_task_done[i])
                    areAllArrivedDone = 0;
            if(areAllArrivedDone){
                priorityWithRR_time++;
                continue;
            }

            //if CPU is currently occupied, increase time, decrease CPU time left, and continue
            if(occupied_CPU_time_left>0){
                occupied_CPU_time_left--;
                priorityWithRR_time++;
                continue;
            }

           
            
            //find the maximum priority
            int maximumPriority = 250;
            int maximumPriorityIndex;
            for(int i = 0;i < num_of_tasks;i ++){
                if(priorityWithRR_is_task_done[i])
                    continue;
                if(priorityWithRR_priorities[i]<maximumPriority){
                    maximumPriority = priorityWithRR_priorities[i];
                    maximumPriorityIndex = i;
                }
            }

            //count how many tasks have the maximum priority
            priorityWithRR_counter = 0;
            for(int i = 0;i < num_of_tasks;i ++){
                if(priorityWithRR_is_task_done[i])
                    continue;
                if(priorityWithRR_priorities[i] == maximumPriority)
                    priorityWithRR_counter++;
            }

            //if only 1 task with maximum priority, put  it in output as non-preemptive
            if(priorityWithRR_counter == 1){

                strcpy(outputTaskID[outputIndex],algorithm_tasks[maximumPriorityIndex].name);
                outputCPUTime[outputIndex] = algorithm_tasks[maximumPriorityIndex].burst_time;
                outputIndex++;
                priorityWithRR_is_task_done[maximumPriorityIndex] = 1;
                outputNumOfChunks++;

                occupied_CPU_time_left = algorithm_tasks[maximumPriorityIndex].burst_time - 1;
            }
            
            //if 2 tasks or more are with maximum priority, do RR
            else{

                //enqueue all tasks with highest priority
                for(int i = 0;i < num_of_tasks;i ++){
                    if(priorityWithRR_is_task_done[i] || priorityWithRR_is_in_queue[i])
                        continue;
                    if(priorityWithRR_priorities[i] == maximumPriority){
                        Enqueue(Q,&algorithm_tasks[i]);
                        priorityWithRR_is_in_queue[i] = 1;
                    }
                }

                //dequeue task from top
                task * current_RR_task = Dequeue(Q);

                //if burst time left is longer than round_robin_q, take CPU for round_robin_q seconds and enqueue back if it hsa burst time left
                if(current_RR_task->burst_time >= round_robin_q){
                    strcpy(outputTaskID[outputIndex],current_RR_task->name);
                    outputCPUTime[outputIndex] = round_robin_q;
                    outputIndex++;
                    outputNumOfChunks++;
                    current_RR_task->burst_time -= round_robin_q;
                    occupied_CPU_time_left = round_robin_q - 1;
                    
                    //find current task index
                    int TaskIndex;
                    for(int i = 0;i < num_of_tasks;i ++)
                        if(!strcmp(algorithm_tasks[i].name,current_RR_task->name))
                            TaskIndex = i;
                    
                    //if it has burst time left, update it in main task array and enqueue it
                    if(current_RR_task->burst_time > 0){
                        algorithm_tasks[TaskIndex].burst_time = current_RR_task->burst_time;
                        Enqueue(Q,current_RR_task);
                    }

                    //otherwise mark it as done
                    else{
                        priorityWithRR_is_task_done[TaskIndex] = 1;
                    }
                    
                }

                //otherwise take CPU for remaining burst time
                else{
                    strcpy(outputTaskID[outputIndex],current_RR_task->name);
                    outputCPUTime[outputIndex] = current_RR_task->burst_time;
                    outputIndex++;
                    outputNumOfChunks++;
                    occupied_CPU_time_left = current_RR_task->burst_time - 1;

                    //find current task index and mark it as done
                    int doneTaskIndex;
                    for(int i = 0;i < num_of_tasks;i ++)
                        if(!strcmp(algorithm_tasks[i].name,current_RR_task->name))
                            doneTaskIndex = i;
                    priorityWithRR_is_task_done[doneTaskIndex] = 1;
                }

                //free temporary memory
                free(current_RR_task);
            }

            //increase time
            priorityWithRR_time++;
        }

        break;

        default:
        break;
    }
    for(int i = 0;i < outputNumOfChunks;i ++)
        printf("<P%s,%d>", outputTaskID[i], outputCPUTime[i]);
    printf("\n");
}

void copy(task * destination, task * origin){
    destination->arrival_time = origin->arrival_time;
    strcpy(destination->name,origin->name);
    destination->burst_time = origin->burst_time;
    destination->priority = origin->priority;
}

Queue * createQueue(int maxElements)
{
        /* Create a Queue */
        Queue *Q;
        Q = (Queue *)malloc(sizeof(Queue));
        /* Initialise its properties */
        Q->elements = (task *)malloc(sizeof(task)*maxElements);
        Q->size = 0;
        Q->capacity = maxElements;
        Q->front = 0;
        Q->rear = -1;
        /* Return the pointer */
        return Q;
}
task * Dequeue(Queue *Q)
{
        int output_index;
        task * ouput_task = (task *)malloc(sizeof(task));
        /* If Queue size is zero then it is empty. So we cannot pop */
        if(Q->size==0)
        {
                printf("Queue is Empty\n");
                return NULL;
        }
        /* Removing an element is equivalent to incrementing index of front by one */
        else
        {
                output_index = Q->front;
                Q->size--;
                Q->front++;
                /* As we fill elements in circular fashion */
                if(Q->front==Q->capacity)
                {
                        Q->front=0;
                }
        }
        copy(ouput_task,&Q->elements[output_index]);
        return ouput_task;
}
int isEmpty(Queue *Q)
{
        if(Q->size==0)
        {
                return 1;
        }
        return 0;
}
void Enqueue(Queue *Q, task *element)
{
        /* If the Queue is full, we cannot push an element into it as there is no space for it.*/
        if(Q->size == Q->capacity)
        {
                printf("Queue is Full\n");
        }
        else
        {
                Q->size++;
                Q->rear = Q->rear + 1;
                /* As we fill the queue in circular fashion */
                if(Q->rear == Q->capacity)
                {
                        Q->rear = 0;
                }
                /* Insert the element in its rear side */ 
                copy(&Q->elements[Q->rear],element);
        }
        return;
}