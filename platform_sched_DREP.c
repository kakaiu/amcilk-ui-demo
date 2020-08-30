#include "platform_sched_DREP.h"
#include "platform-interface-to-job-scheduler.h"
#include <stdio.h>

platform_program * rand_pick_unfinished_job() {
    int i = 0;
    int num_running_job = get_num_running_job();
    int rand_num = util_random_generator()%(num_running_job);
    platform_program ** container_pool = get_container_pool();
    //printf("[platform_scheduler_DREP]: finish, the number is %d\n", rand_num);
    platform_program * p;
    int c = 0;
    for (c=0; c<CONTAINER_COUNT; c++) {
        p = container_pool[c];
        if (p->state==C_ACTIVE || p->state==C_SUSPENDED) {
            if (i==rand_num) {
                return p;
            }
            i++;
        }
    }
    printf("[PLATFORM ERROR4]: BUG: in rand_pick_unfinished_job\n");
    abort();
    return NULL;
}

void DREP(enum PLATFORM_SCHEDULER_TYPE run_type) {
    int i = 0;
    int c = 0;
    int rand = 0;
    int num_core = get_total_num_core();
    int num_running_job = get_num_running_job();
    platform_program ** container_pool = get_container_pool();
    platform_program * new_p, * stop_p, * tmp_p;

    if (run_type==NEW_PROGRAM) { //if a container begins a new job
        //init new program cpu_mask
        new_p = get_new_program();
        new_p->cpu_mask[0] = 0; //core 0 is used for request receiver which is not available
        new_p->cpu_mask[1] = 0; //core 1 is used for AMCilk scheduling which is not available
        //assign cores
        if (num_running_job>0) { //if running programs exist
            for (i=2; i<num_core; i++) { //randomly get cores for the new job
                rand = util_random_generator()%num_running_job; //When the job is the first run job, nprogram_running==1, and so rand===0 which means that give all core to this job.
                if (rand==0) {
                    new_p->cpu_mask[i] = 1; //make core i active for the new program
                    for (c=0; c<CONTAINER_COUNT; c++) { //put all other program to sleep on this cpu
                        tmp_p = container_pool[c];
                        if (tmp_p->state==C_ACTIVE || tmp_p->state==C_SUSPENDED) {
                            if (tmp_p!=new_p) {
                                tmp_p->cpu_mask[i] = 0; //make core i sleep for all previous programs
                            }
                        }
                    }
                } else {
                    new_p->cpu_mask[i] = 0; //make core i sleep for the new program
                }
            }
        } else {
            printf("ERROR: in NEW_PROGRAM case, num of running program is 0\n");
            abort();
        }

    } else if (run_type==EXIT_PROGRAM) { //if a container exits a new job
        stop_p = get_stop_program();
        for (i=2; i<num_core; i++) {
            if (stop_p->cpu_mask[i]==1) { //core i was used in the finished program
                stop_p->cpu_mask[i] = 0;
                tmp_p = rand_pick_unfinished_job(); //give the core i to a random job
                if (tmp_p!=NULL) {
                    tmp_p->cpu_mask[i] = 1;
                } else {
                    printf("[ERROR]: FAILED1 TO GIVE CORES\n");
                    abort();
                }
            }
        }

    } else {
        printf("[PLATFORM ERROR3]: wrong input of run_type in platform_scheduler_DREP\n");
        abort();
    }
}