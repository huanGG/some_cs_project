/*
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "hashtable.h"

#define ExecMode 1
#define DetectMode 2

void detect(char* file_path, int mode);

int main(int argc, char **argv)
{		
    int mode = DetectMode;
	int oc = 0;                     
    char* file_path;             
    while((oc = getopt(argc, argv, "f:e")) != -1)
    {
        switch(oc)
        {
            case 'f':
                file_path = optarg;
                break;
            case 'e':
                mode = ExecMode;
                break;
        }
    }
    detect(file_path, mode);
	return 0;
}

void detect(char* file_path, int mode){
    int proc_cnt = 0; // proc total count
    int file_cnt = 0; // file total count
    int t_cnt = 0;    // terminate total count
    int proc[100000];
    int lock_files[100000];
    int req_files[100000];
    int proc_exec_time[100000];
    int terminates[100000];

    HashTable lock_proc_ht; // lock_proc hashmap 
    ht_setup(&lock_proc_ht, sizeof(int), sizeof(int), 100000);
    HashTable req_proc_ht;  // req_proc hashmap
    ht_setup(&req_proc_ht, sizeof(int), sizeof(int), 100000);
    HashTable proc_depend_ht; // proc dependce hashmap
    ht_setup(&proc_depend_ht, sizeof(int), sizeof(int), 100000);
    HashTable file_ht;
    ht_setup(&file_ht, sizeof(int), sizeof(int), 100000);

    // 1.read file
    FILE *fptr = fopen(file_path, "r");
	if (fptr == NULL)
	{
		printf("Could not open file");
		return ;
	}
    char buf[200];
    while(fgets(buf, 200, fptr) != NULL)
	{
        sscanf(buf, "%d %d %d ", &proc[proc_cnt], &lock_files[proc_cnt], &req_files[proc_cnt]);
        ht_insert(&lock_proc_ht, &lock_files[proc_cnt], &proc[proc_cnt]);
        if (ht_contains(&file_ht, &lock_files[proc_cnt]) == HT_NOT_FOUND) {
            file_cnt++;
            ht_insert(&file_ht, &lock_files[proc_cnt], &lock_files[proc_cnt]);
        }
        if (ht_contains(&file_ht, &req_files[proc_cnt]) == HT_NOT_FOUND) {
            file_cnt++;
            ht_insert(&file_ht, &req_files[proc_cnt], &req_files[proc_cnt]);
        }
        proc_cnt++;
	}
    fclose(fptr);
    // 2.find depend relation
    bool find_deadlock =false;
    int dummy = -1;
    for(int i=0; i< proc_cnt; i++){
        if (ht_contains(&req_proc_ht, &req_files[i]) == HT_NOT_FOUND) {
            proc_exec_time[proc[i]] =1;
        }else {
            int* last_proc = (int*)ht_lookup(&req_proc_ht, &req_files[i]);
            proc_exec_time[proc[i]] = proc_exec_time[*last_proc] +1;
        }
        ht_insert(&req_proc_ht, &req_files[i], &proc[i]);
        
        if (mode != DetectMode){
            continue;
        }
        // find depend proc
        int* depend_proc = &dummy;
        if (ht_contains(&lock_proc_ht, &req_files[i]) == HT_FOUND){
            depend_proc = (int*)ht_lookup(&lock_proc_ht, &req_files[i]);
        }
        ht_insert(&proc_depend_ht, &proc[i], depend_proc);
        // find deadlock
        int min_proc = proc[i];
        while(*depend_proc != -1 && ht_contains(&proc_depend_ht, depend_proc) == HT_FOUND){
            min_proc = min_proc < *depend_proc ? min_proc: *depend_proc;
            if (*depend_proc == proc[i]) {
                find_deadlock = true;
                terminates[t_cnt] = min_proc;
                t_cnt++;
                ht_erase(&proc_depend_ht, &min_proc);
                break;
            }
            depend_proc = (int*)ht_lookup(&proc_depend_ht, depend_proc);
        }
    }
    // 3. calculate time
    int max_time =1;
    for(int i=0; i< 100000; i++){
        if (proc_exec_time[i] > max_time) {
            max_time = proc_exec_time[i];
        }
    }
    int total_time = max_time +1;

    // 4.output
    if (mode == ExecMode){
        printf("Processes %d\n", proc_cnt);
        printf("Files %d\n", file_cnt);
        printf("Execution time %d\n", total_time);
    }
    if (mode == DetectMode) {
        printf("Processes %d\n", proc_cnt);
        printf("Files %d\n", file_cnt);
        if (find_deadlock){
            printf("Deadlock detected\nTerminate");
            for (int i=0; i< t_cnt; i++){
                printf(" %d", terminates[i]);
            }
            printf("\n");
        }else{
            printf("No deadlocks\n");
        }
    }
}
