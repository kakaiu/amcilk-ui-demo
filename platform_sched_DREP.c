//DREP implementation with AMCilk interface
void job_scheduler(enum PLATFORM_SCHEDULER_TYPE e) {
    container_id_t new_p, stop_p, tmp_p;
    if (e==NEW_PROGRAM) { //when a new job begins
        new_p = get_new_program_container();
        for (int i=2; i<get_total_num_core(); i++) { //randomly get cores for the new job
            int rand = util_random_selector(get_num_running_job()); 
            //rand==0 with probablity of 1/get_num_running_job()
            //rand===0 when get_num_running_job()==1 (the new job is the first job).
            if (rand==0) { //selected
                give_core_to_container(new_p, i); //give core i to container new_p
            } //by default, a new container occupies no core unless we do give_core_to_container
        }

    } else if (e==EXIT_PROGRAM) { //when a job completes
        stop_p = get_stop_program_container();
        for (int i=2; i<get_total_num_core(); i++) {
            if (container_use_core(stop_p, i)) { //core i was used in the stop program
                tmp_p = random_pick_unfinished_container(); //give the core i to a random job
                if (tmp_p!=NULL) {
                    give_core_to_container(tmp_p, i);
                }
            }
        }
    }
}
