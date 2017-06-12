#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <iostream>
using namespace std;

//Metrics
#define RETRY 4
#define TMT 100
#define AMOUNT 10
#define N 100

//Define the algorithms used
#define ISSUER true
#define RECEIVER true

//Other defines
#define MAXLOOPS 1000
#define MAX_INT 1000000009

class processor{
private:
    struct node{
        int exectime;
        int deltime;
        node* next,*prev;
    };
    int cputime;
    int tasks;
    static const int MAX_CPU_TIME  = TMT*10;
    static const int MAX_NUM_TASKS = 10;
    static const int MIN_CPU_TIME  = TMT*3;
    static const int MIN_NUM_TASKS = 3;
    node* firsttask,*lasttask;
public:
    processor(){cputime = tasks = recMsgs = sentMsgs = 0, firsttask = lasttask = NULL;}

    void freeCpu(int time){
        node* aux;
        while (firsttask!=NULL&&firsttask->deltime<=time){
            cputime-=firsttask->exectime;
            tasks--;
            aux = firsttask->next;
            free(firsttask);
            firsttask = aux;
        }
        if (firsttask==NULL)lasttask = NULL;
    }

    void assignTask(int time,int totaltime){
        int lastendtime = 0;
        if (lasttask==NULL){
            lasttask = (node*)malloc(sizeof(node));
            lasttask->prev = NULL;
        }
        else {
            lasttask->next = (node*)malloc(sizeof(node));
            lastendtime = lasttask->deltime;
            lasttask->next->prev = lasttask;
            lasttask = lasttask->next;
        }
        lasttask->deltime = max(totaltime,lastendtime)+time;
        lasttask->exectime = time;
        lasttask->next = NULL;
        if (firsttask==NULL)firsttask = lasttask;
        tasks++;
        cputime+=time;
    }

    int deleteTask(){
        if (lasttask==NULL)return -1;
        int exectime = lasttask->exectime;
        node* aux = lasttask->prev;
        free(lasttask);
        lasttask = aux;
        if (lasttask==NULL)firsttask = NULL;
        return exectime;
    }

    bool canAdd(){
        if (tasks>MAX_NUM_TASKS)return false;
        //if (cputime>MAX_CPU_TIME)return false;
        return true;
    }

    bool canReceive(){
        if (tasks>MIN_NUM_TASKS)return false;
        //if (cputime>MIN_CPU_TIME)return false;
        return true;
    }

    bool workOverload(){
        return !canAdd();
    }

    ~processor(){
        node* aux;
        while (firsttask!=NULL){
            aux = firsttask->next;
            free(firsttask);
            firsttask = aux;
        }
    }
    void pclear(){
        freeCpu(MAX_INT);
        firsttask = lasttask = NULL;
        sentMsgs = recMsgs = 0;
    }

    int sentMsgs;
    int recMsgs;
};

//Global variables
int arr[N];
processor processors[N];

int issueProcess(int procnode){
    random_shuffle(arr,arr+N);//Try to add the task to 'RETRY' random processors
    for (int curtask = 0,j=0;j<RETRY && curtask< N;j++,curtask++){
        if (arr[curtask]==procnode){
            j--;
            continue;
        }
        int index = arr[curtask];

        processors[procnode].sentMsgs++;
        processors[index].recMsgs++;
        if (processors[index].canAdd()){
            return index;
        }
    }
    return -1;
}

int receiveProcess(int procnode,int totaltime){
    random_shuffle(arr,arr+N);//Try to receive a task from 'RETRY' random processors
    for (int curtask = 0,j=0;j<RETRY && curtask< N;j++,curtask++){
        if (arr[curtask]==procnode){
            j--;
            continue;
        }
        int index = arr[curtask];

        processors[procnode].sentMsgs++;
        processors[index].recMsgs++;
        if (processors[index].workOverload()){
            int exectime = processors[index].deleteTask();
            processors[procnode].assignTask(exectime,totaltime);
            return index;
        }
    }
    return -1;
}

void simulateProcessAlloc(int amount){
    int loop = 0;
    int totaltime = 0;
    int procnode, index, proctime;

    while (loop++<MAXLOOPS){
        for (int i=0;i<N;i++){
            processors[i].freeCpu(totaltime);
        }if (RECEIVER){
            for (int i=0;i<N;i++){
                if (processors[i].canReceive())receiveProcess(i,totaltime);
            }
        }
        for (int i=0;i<amount;i++){
            proctime = rand()%(2*TMT-1)+1;//Generates random number with medium TMT
            procnode = rand()%N;//Creates task in random processor

            if (processors[procnode].canAdd()){
                processors[procnode].assignTask(proctime,totaltime);
                continue;
            }//If possible, add task to processor

            index = -1;
            if (ISSUER){
                index = issueProcess(procnode);
                if (index>=0)processors[index].assignTask(proctime,totaltime);
            }

            if (index<0)processors[procnode].assignTask(proctime,totaltime);
        }
        totaltime+=TMT;
    }
}

int main(){
    freopen("log.txt", "w", stdout);

    for (int i=0;i<N;i++)arr[i] = i;
    pair<int,int> res[N];

    srand(time(NULL));//Initialize random seed

    simulateProcessAlloc(AMOUNT);
    for (int i=0;i<N;i++){
        res[i] = make_pair(processors[i].sentMsgs,processors[i].recMsgs);
        processors[i].pclear();
    }
    simulateProcessAlloc(AMOUNT/3);

    //Save results in file
    printf("No |Carga leve                          |Carga intensa\n");
    printf("   |N Msgs transmitidas|N Msgs recebidas|N Msgs transmitidas|N Msgs recebidas\n");
    for (int i=0;i<N;i++){
        printf("%-3d %-19d %-16d %-19d %-16d\n",i,processors[i].sentMsgs,processors[i].recMsgs,
               res[i].first,res[i].second);
    }
    fclose(stdout);
}
