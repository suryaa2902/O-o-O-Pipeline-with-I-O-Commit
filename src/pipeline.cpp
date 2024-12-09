//////////////////////////////////////////////////////////////////////
// In part B, you must modify this file to implement the following: //
// - void pipe_cycle_issue(Pipeline *p)                             //
// - void pipe_cycle_schedule(Pipeline *p)                          //
// - void pipe_cycle_writeback(Pipeline *p)                         //
// - void pipe_cycle_commit(Pipeline *p)                            //
//////////////////////////////////////////////////////////////////////

//Author: Suryaa Senthilkumar Shanthi

// pipeline.cpp
// Implements the out-of-order pipeline.

#include "pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cassert>

//#include <inttypes.h>  //from code resth

//#define MAX_REST_ENTRIES 256  //from code resth

/**
 * The width of the pipeline; that is, the maximum number of instructions that
 * can be processed during any given cycle in each of the issue, schedule, and
 * commit stages of the pipeline.
 * 
 * (Note that this does not apply to the writeback stage: as many as
 * MAX_WRITEBACKS instructions can be written back to the ROB in a single
 * cycle!)
 * 
 * When the width is 1, the pipeline is scalar.
 * When the width is greater than 1, the pipeline is superscalar.
 */
extern uint32_t PIPE_WIDTH;

/**
 * The number of entries in the ROB; that is, the maximum number of
 * instructions that can be stored in the ROB at any given time.
 * 
 * You should use only this many entries of the ROB::entries array.
 */
extern uint32_t NUM_ROB_ENTRIES;

/**
 * Whether to use in-order scheduling or out-of-order scheduling.
 * 
 * The possible values are SCHED_IN_ORDER for in-order scheduling and
 * SCHED_OUT_OF_ORDER for out-of-order scheduling.
 * 
 * Your implementation of pipe_cycle_sched() should check this value and
 * implement scheduling of instructions accordingly.
 */
extern SchedulingPolicy SCHED_POLICY;

/**
 * The number of cycles an LD instruction should take to execute.
 * 
 * This is used by the code in exeq.cpp to determine how long to wait before
 * considering the execution of an LD instruction done.
 */
extern uint32_t LOAD_EXE_CYCLES;

/**
 * Read a single trace record from the trace file and use it to populate the
 * given fe_latch.
 * 
 * You should not modify this function.
 * 
 * @param p the pipeline whose trace file should be read
 * @param fe_latch the PipelineLatch struct to populate
 */

/*
//from code
//rest.h
typedef struct REST_Entry_Struct
{  bool valid;
   bool scheduled;
   InstInfo instt;
}RESTEntry;

typedef struct REST
{  RESTEntry REST_Entries[MAX_REST_ENTRIES];
}REST;

extern int32_t NUM_REST_ENTRIES;
static int rest_count = 0;

REST* rest_init(void)
{  int ii;
   REST *t = (REST *) calloc (1, sizeof (REST));
   for(ii=0; ii<MAX_REST_ENTRIES; ii++)
   {  t->REST_Entries[ii].valid = false;
   }
   assert(NUM_REST_ENTRIES <= MAX_REST_ENTRIES);
   return t;
}

bool rest_check_space(REST *t)
{  return rest_count < NUM_REST_ENTRIES;
/* 
  int i;
   for(i=0;i<MAX_REST_ENTRIES; i++)
   {  if(t->REST_Entries[i].valid)
      {  return true;
      }
   }
   return false;
*/
/*
}

void rest_insert(REST *t, InstInfo instt)
{  assert(rest_check_space(t));
   assert(instt.dr_tag != -1);
   assert(!t->REST_Entries[instt.dr_tag].scheduled);
   if(t->REST_Entries[instt.dr_tag].valid)
   {  assert(!t->REST_Entries[instt.dr_tag].valid);
   }
   t->REST_Entries[instt.dr_tag].instt = instt;
   t->REST_Entries[instt.dr_tag].valid = true;
   rest_count++;
}

void rest_remove(REST *t, InstInfo instt)
{  assert(instt.dr_tag != -1);
   assert(t->REST_Entries[instt.dr_tag].valid);
   t->REST_Entries[instt.dr_tag].valid = false;
   t->REST_Entries[instt.dr_tag].scheduled = false;
   rest_count--;
}

void rest_wakeup(REST *t, int tag)
{  int i;
   for(i=0; i<MAX_REST_ENTRIES; i++)
   {  if(t->REST_Entries[i].valid)
      {  if (t->REST_Entries[i].instt.src1_tag == tag)
         {  t->REST_Entries[i].instt.src1_ready = true;
         }
         if (t->REST_Entries[i].instt.src2_tag == tag)
         {  t->REST_Entries[i].instt.src2_ready = true;
         }
      }
   }
}

void rest_schedule(REST *t, InstInfo instt)
{  assert(instt.dr_tag != -1);
   assert(t->REST_Entries[instt.dr_tag].valid);
   assert(!t->REST_Entries[instt.dr_tag].scheduled);
   t->REST_Entries[instt.dr_tag].scheduled = true;
}
//from code
//resth
*/

void pipe_fetch_inst(Pipeline *p, PipelineLatch *fe_latch)
{
    InstInfo *inst = &fe_latch->inst;
    TraceRec trace_rec;
    uint8_t *trace_rec_buf = (uint8_t *)&trace_rec;
    size_t bytes_read_total = 0;
    ssize_t bytes_read_last = 0;
    size_t bytes_left = sizeof(TraceRec);

    // Read a total of sizeof(TraceRec) bytes from the trace file.
    while (bytes_left > 0)
    {
        bytes_read_last = read(p->trace_fd, trace_rec_buf, bytes_left);
        if (bytes_read_last <= 0)
        {
            // EOF or error
            break;
        }

        trace_rec_buf += bytes_read_last;
        bytes_read_total += bytes_read_last;
        bytes_left -= bytes_read_last;
    }

    // Check for error conditions.
    if (bytes_left > 0 || trace_rec.op_type >= NUM_OP_TYPES)
    {
        fe_latch->valid = false;
        p->halt_inst_num = p->last_inst_num;

        if (p->stat_retired_inst >= p->halt_inst_num)
        {
            p->halt = true;
        }

        if (bytes_read_last == -1)
        {
            fprintf(stderr, "\n");
            perror("Couldn't read from pipe");
            return;
        }

        if (bytes_read_total == 0)
        {
            // No more trace records to read
            return;
        }

        // Too few bytes read or invalid op_type
        fprintf(stderr, "\n");
        fprintf(stderr, "Error: Invalid trace file\n");
        return;
    }

    // Got a valid trace record!
    fe_latch->valid = true;
    fe_latch->stall = false;
    inst->inst_num = ++p->last_inst_num;
    inst->op_type = (OpType)trace_rec.op_type;

    inst->dest_reg = trace_rec.dest_needed ? trace_rec.dest_reg : -1;
    inst->src1_reg = trace_rec.src1_needed ? trace_rec.src1_reg : -1;
    inst->src2_reg = trace_rec.src2_needed ? trace_rec.src2_reg : -1;

    inst->dr_tag = -1;
    inst->src1_tag = -1;
    inst->src2_tag = -1;
    inst->src1_ready = false;
    inst->src2_ready = false;
    inst->exe_wait_cycles = 0;
}

/**
 * Allocate and initialize a new pipeline.
 * 
 * You should not need to modify this function.
 * 
 * @param trace_fd the file descriptor from which to read trace records
 * @return a pointer to a newly allocated pipeline
 */
Pipeline *pipe_init(int trace_fd)
{
    printf("\n** PIPELINE IS %d WIDE **\n\n", PIPE_WIDTH);

    // Allocate pipeline.
    Pipeline *p = (Pipeline *)calloc(1, sizeof(Pipeline));

    // Initialize pipeline.
    p->rat = rat_init();
    p->rob = rob_init();
    p->exeq = exeq_init();
  //  p->rest = rest_init();    //from code
    p->trace_fd = trace_fd;
    p->halt_inst_num = (uint64_t)(-1) - 3;

    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        p->FE_latch[i].valid = false;
        p->ID_latch[i].valid = false;
        p->SC_latch[i].valid = false;
    }
    for (unsigned int i = 0; i < MAX_WRITEBACKS; i++)
    {
        p->EX_latch[i].valid = false;
    }

    return p;
}

/**
 * Commit the given instruction.
 * 
 * This updates counters and flags on the pipeline.
 * 
 * This function is implemented for you. You should not modify it.
 * 
 * @param p the pipeline to update.
 * @param inst the instruction to commit.
 */
void pipe_commit_inst(Pipeline *p, InstInfo inst)
{
    p->stat_retired_inst++;

    if (inst.inst_num >= p->halt_inst_num)
    {
        p->halt = true;
    }
}

/**
 * Print out the state of the pipeline for debugging purposes.
 * 
 * You may use this function to help debug your pipeline implementation, but
 * please remove calls to this function before submitting the lab.
 * 
 * @param p the pipeline
 */
void pipe_print_state(Pipeline *p)
{
    printf("\n--------------------------------------------\n");
    printf("Cycle count: %lu, retired instructions: %lu\n",
           (unsigned long)p->stat_num_cycle,
           (unsigned long)p->stat_retired_inst);

    // Print table header
    for (unsigned int latch_type = 0; latch_type < 4; latch_type++)
    {
        switch (latch_type)
        {
        case 0:
            printf(" FE:    ");
            break;
        case 1:
            printf(" ID:    ");
            break;
        case 2:
            printf(" SCH:   ");
            break;
        case 3:
            printf(" EX:    ");
            break;
        default:
            printf(" ------ ");
        }
    }
    printf("\n");

    // Print row for each lane in pipeline width
    unsigned int ex_i = 0;
    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (p->FE_latch[i].valid)
        {
            printf(" %6lu ",
                   (unsigned long)p->FE_latch[i].inst.inst_num);
        }
        else
        {
            printf(" ------ ");
        }
        if (p->ID_latch[i].valid)
        {
            printf(" %6lu ",
                   (unsigned long)p->ID_latch[i].inst.inst_num);
        }
        else
        {
            printf(" ------ ");
        }
        if (p->SC_latch[i].valid)
        {
            printf(" %6lu ",
                   (unsigned long)p->SC_latch[i].inst.inst_num);
        }
        else
        {
            printf(" ------ ");
        }
        for (; ex_i < MAX_WRITEBACKS; ex_i++)
        {
            if (p->EX_latch[ex_i].valid)
            {
                printf(" %6lu ",
                       (unsigned long)p->EX_latch[ex_i].inst.inst_num);
                ex_i++;
                break;
            }
        }
        printf("\n");
    }
    printf("\n");

    rat_print_state(p->rat);
    exeq_print_state(p->exeq);
    rob_print_state(p->rob);
}

/**
 * Simulate one cycle of all stages of a pipeline.
 * 
 * You should not need to modify this function except for debugging purposes.
 * If you add code to print debug output in this function, remove it or comment
 * it out before you submit the lab.
 * 
 * @param p the pipeline to simulate
 */
void pipe_cycle(Pipeline *p)
{
    p->stat_num_cycle++;

    // In our simulator, stages are processed in reverse order.
    pipe_cycle_commit(p);
    pipe_cycle_writeback(p);
    pipe_cycle_exe(p);
    pipe_cycle_schedule(p);
    pipe_cycle_issue(p);
    pipe_cycle_decode(p);
    pipe_cycle_fetch(p);

    // You can uncomment the following line to print out the pipeline state
    // after each clock cycle for debugging purposes.
    // Make sure you comment it out or remove it before you submit the lab.
    //pipe_print_state(p);
}

/**
 * Simulate one cycle of the fetch stage of a pipeline.
 * 
 * This function is implemented for you. You should not modify it.
 * 
 * @param p the pipeline to simulate
 */
void pipe_cycle_fetch(Pipeline *p)
{
    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (!p->FE_latch[i].stall && !p->FE_latch[i].valid)
        {
            // No stall and latch empty, so fetch a new instruction.
            pipe_fetch_inst(p, &p->FE_latch[i]);
        }
    }
}

/**
 * Simulate one cycle of the instruction decode stage of a pipeline.
 * 
 * This function is implemented for you. You should not modify it.
 * 
 * @param p the pipeline to simulate
 */
void pipe_cycle_decode(Pipeline *p)
{
    static uint64_t next_inst_num = 1;
    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (!p->ID_latch[i].stall && !p->ID_latch[i].valid)
        {
            // No stall and latch empty, so decode the next instruction.
            // Loop to find the next in-order instruction.
            for (unsigned int j = 0; j < PIPE_WIDTH; j++)
            {
                if (p->FE_latch[j].valid &&
                    p->FE_latch[j].inst.inst_num == next_inst_num)
                {
                    p->ID_latch[i] = p->FE_latch[j];
                    p->FE_latch[j].valid = false;
                    next_inst_num++;
                    break;
                }
            }
        }
    }
}

/**
 * Simulate one cycle of the execute stage of a pipeline. This handles
 * instructions that take multiple cycles to execute.
 * 
 * This function is implemented for you. You should not modify it.
 * 
 * @param p the pipeline to simulate
 */
void pipe_cycle_exe(Pipeline *p)
{
    // If all operations are single-cycle, just copy SC latches to EX latches.
    if (LOAD_EXE_CYCLES == 1)
    {
        for (unsigned int i = 0; i < PIPE_WIDTH; i++)
        {
            if (p->SC_latch[i].valid)
            {
                p->EX_latch[i] = p->SC_latch[i];
                p->SC_latch[i].valid = false;
            }
        }
        return;
    }

    // Otherwise, we need to handle multi-cycle instructions with EXEQ.

    // All valid entries from the SC latches are inserted into the EXEQ.
    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (p->SC_latch[i].valid)
        {
            if (!exeq_insert(p->exeq, p->SC_latch[i].inst))
            {
                fprintf(stderr, "Error: EXEQ full\n");
                p->halt = true;
                return;
            }

            p->SC_latch[i].valid = false;
        }
    }

    // Cycle the EXEQ to reduce wait time for each instruction by 1 cycle.
    exeq_cycle(p->exeq);

    // Transfer all finished entries from the EXEQ to the EX latch.
    for (unsigned int i = 0; i < MAX_WRITEBACKS && exeq_check_done(p->exeq); i++)
    {
        p->EX_latch[i].valid = true;
        p->EX_latch[i].stall = false;
        p->EX_latch[i].inst = exeq_remove(p->exeq);
    }
}

/**
 * Simulate one cycle of the issue stage of a pipeline: insert decoded
 * instructions into the ROB and perform register renaming.
 * 
 * You must implement this function in pipeline.cpp in part B of the
 * assignment.
 * 
 * @param p the pipeline to simulate
 */
void pipe_cycle_issue(Pipeline *p)
{
    // TODO: For each valid instruction from the ID stage:

    // TODO: If there is space in the ROB, insert the instruction.
    // TODO: Set the entry invalid in the previous latch when you do so.

    // TODO: Then, check RAT for this instruction's source operands:
    // TODO: If src1 is not remapped, mark src1 as ready.
    // TODO: If src1 is remapped, set src1 tag accordingly, and set src1 ready
    //       according to whether the ROB entry with that tag is ready.
    // TODO: If src2 is not remapped, mark src2 as ready.
    // TODO: If src2 is remapped, set src2 tag accordingly, and set src2 ready
    //       according to whether the ROB entry with that tag is ready.

    // TODO: Set the tag for this instruction's destination register.
    // TODO: If this instruction writes to a register, update the RAT
    //       accordingly.
/*
    int min = -1;
    unsigned int i;
    for(i=0; i<PIPE_WIDTH; i++)
    {  if ((min == -1) && p->ID_latch[i].valid) 
       {  min = i;
       }
       else if ((p->ID_latch[i].inst.inst_num < p->ID_latch[min].inst.inst_num) && p->ID_latch[i].valid) 
       {  min = i;
       }
    }
    unsigned int j;
    for(j=0; j<PIPE_WIDTH; j++)
    {  assert(rob_check_space(p->rob) == rest_check_space(p->rest));
       if(!((min >= 0) && p->ID_latch[min].valid && rob_check_space(p-rob)))
       {  return;
       }
       InstInfo id_inst = p->ID_latch[min].inst;
       if((id_inst.src1_reg != -1) && (rat_getremap(p->rat, id_inst.src1_reg) != -1))
       {  id_inst.src1_tag = rat_get_remap(p->rat, id_inst.src1_reg);
       }
       else
       {  id_inst.src1_tag = -1;
       }
       if((id_inst.src2_reg != -1) && (rat_get_remap( p->rat, id_inst.src2_reg) != -1))
       {  id_inst.src2_tag = rat_get_remap(p->rat, id_inst.src2_reg);
       }
       else
       {  id_inst.src2_tag = -1;
       }
       if(id_inst.src1_tag == -1 || rob_check_ready(p->rob, id_inst.src1_tag)) 
       {  id_inst.src1_ready = true;
       }
       if(id_inst.src2_tag == -1 || rob_check_ready(p->rob, id_inst.src2_tag)) 
       {  id_inst.src2_ready = true;
       }
       p->ID_latch[min].valid = false;
       if(rob_check_space(p->rob)) 
       {  int tag = rob_insert(p->rob, id_inst);
          id_inst.dr_tag = tag;
       }
       if(rest_check_space(p->rest))
       {  rest_insert(p->rest, id_inst);
       }
       if(id_inst.dest_reg != -1) 
       {rat_set_remap(p->rat, id_inst.dest_reg, id_inst.dr_tag);
       }
    }
}
*/

    for(unsigned int ii=0;ii<PIPE_WIDTH;ii++)
    {
     if(rob_check_space(p->rob))
     {
      if(p->ID_latch[ii].valid)
      {
         int temp_tail; 
         temp_tail=rob_insert(p->rob, p->ID_latch[ii].inst);
         p->rob->entries[temp_tail].valid = 1;
         p->rob->entries[temp_tail].exec = 0;
         p->rob->entries[temp_tail].ready = 0;
         p->rob->entries[temp_tail].inst.src1_tag = rat_get_remap(p->rat, p->rob->entries[temp_tail].inst.src1_reg);
         p->rob->entries[temp_tail].inst.src2_tag = rat_get_remap(p->rat, p->rob->entries[temp_tail].inst.src2_reg);
         if(p->rob->entries[temp_tail].inst.src1_tag == -1)
         {
           p->rob->entries[temp_tail].inst.src1_ready = true;
         }
         else 
         {
           if(rob_check_ready(p->rob, p->rob->entries[temp_tail].inst.src1_tag))
           {
              p->rob->entries[temp_tail].inst.src1_ready = true;
           }
           else
           {
              p->rob->entries[temp_tail].inst.src1_ready = false;
           }
         }
         if(p->rob->entries[temp_tail].inst.src2_tag == -1)
         {
            p->rob->entries[temp_tail].inst.src2_ready = true;
         }
         else 
         {
            if(rob_check_ready(p->rob, p->rob->entries[temp_tail].inst.src2_tag))
            {
               p->rob->entries[temp_tail].inst.src2_ready = true;
            }
            else
            {
               p->rob->entries[temp_tail].inst.src2_ready = false;
            }
         }
         if(p->rob->entries[temp_tail].inst.dest_reg > -1)
         {
            rat_set_remap(p->rat, p->rob->entries[temp_tail].inst.dest_reg, temp_tail);
         }
         p->rob->entries[temp_tail].inst.dr_tag =temp_tail;
         p->ID_latch[ii].valid=false;
         if(p->ID_latch[ii].stall)
         {
               p->ID_latch[ii].stall=false;
         }
       
      }
      else
      {
         if(p->ID_latch[ii].valid)
         {
            p->ID_latch[ii].stall = true;
         }
      }
   }
 }
}

/*
typedef struct oldest
{  InstInfo inst;
   bool valid;
}oldest_t;

oldest_t oldest(REST* t)
{  oldest_t o;
   o.valid = false;
   int k;
   for(k=0; k<NUM_REST_ENTRIES; k++)
   {  if(o.valid == false && t->REST_Entries[k].valid && !t->REST_Entries[k].scheduled) 
      {  o.valid = true;
         o.inst = t->REST_Entries[k].inst;
      }
      else if (t->REST_Entries[k].valid && (t->REST_Entries[j].inst.inst_num < o.inst.inst_num) && !t->REST_Entries[ji].scheduled) 
      {  o.inst = t->REST_Entries[k].inst;
      }
   }
   return o; 
}

oldest_t oldest_and_ready(REST* t)
{  oldest_t o;
   o.valid = false;
   int j;
   for(j=0; j<NUM_REST_ENTRIES; i++)
   {  if(o.valid == false && t->REST_Entries[j].valid && !t->REST_Entries[j].scheduled && t->REST_Entries[j].inst.src1_ready && t->REST_Entries[j].inst.src2_ready)
      {  o.valid = true;
         o.inst = t->REST_Entries[j].inst;
      }
      else if (t->REST_Entries[i].valid && (o.inst.inst_num > t->REST_Entries[i].inst.inst_num) && !t->REST_Entries[i].scheduled && t->REST_Entries[i].inst.src1_ready && t->REST_Entries[i].inst.src2_ready)
           {  o.inst = t->REST_Entries[j].inst;
           }
   }
   return o;
}
*/

/**
 * Simulate one cycle of the scheduling stage of a pipeline: schedule
 * instructions to execute if they are ready.
 * 
 * You must implement this function in pipeline.cpp in part B of the
 * assignment.
 * 
 * @param p the pipeline to simulate
 */
void pipe_cycle_schedule(Pipeline *p)
{
    // TODO: Implement two scheduling policies:

    if (SCHED_POLICY == SCHED_IN_ORDER)
    {
        // In-order scheduling:
        // TODO: Find the oldest valid entry in the ROB that is not already
        //       executing.
        // TODO: Check if it is stalled, i.e., if at least one source operand
        //       is not ready.
        // TODO: If so, stop scheduling instructions.
        // TODO: Otherwise, mark it as executing in the ROB and send it to the
        //       next latch.
        // TODO: Repeat for each lane of the pipeline.
/*
        unsigned int i;
        for(i=0; i<PIPE_WIDTH; i++)
        {  oldest_t o = oldest(p->rest);
           if(o.valid) 
           {  int tag = o.inst.dr_tag;
               if(p->rest->REST_Entries[tag].inst.src1_ready && p->rest->REST_Entries[tag].inst.src2_ready) 
               {  p->rest->REST_Entries[tag].scheduled = true;
                  p->SC_latch[i].inst = p->rest->REST_Entries[tag].inst;
                  p->SC_latch[i].valid = true;
                  p->SC_latch[i].stall = false;
               }
           }
        }
    }
    
*/

        for(unsigned int ii=0;ii<PIPE_WIDTH;ii++)
        {  uint64_t temp_inst_num=(1<<64)-1;
           InstInfo temp_inst;
           unsigned int temp_index = -1;
           for(unsigned int kk=0;kk<NUM_ROB_ENTRIES; kk++)
           {  if(p->rob->entries[kk].valid&&(p->rob->entries[kk].ready == false) && (p->rob->entries[kk].exec == false))
              {  if(p->rob->entries[kk].inst.inst_num<temp_inst_num)
                 {  temp_inst_num = p->rob->entries[kk].inst.inst_num;
                    temp_inst = p->rob->entries[kk].inst; 
                    temp_index = kk;
                 }
              }
           }
           if(temp_inst_num == ((2<<64)-1))
           {  p->SC_latch[ii].valid = false; 
              p->SC_latch[ii].stall = true;
              goto skip;
           }
           rob_mark_exec(p->rob, temp_inst);
           if(p->rob->entries[temp_index].exec)
           {  p->SC_latch[ii].inst = p->rob->entries[temp_index].inst;
              p->SC_latch[ii].valid = true; 
              p->SC_latch[ii].stall = false;
           }
           else
           {  p->SC_latch[ii].stall = true; 
              p->SC_latch[ii].valid = false;
              goto skip;
           }
           skip:
           return;
        }
    }


    if (SCHED_POLICY == SCHED_OUT_OF_ORDER)
    {
        // Out-of-order scheduling:
        // TODO: Find the oldest valid entry in the ROB that has both source
        //       operands ready but is not already executing.
        // TODO: Mark it as executing in the ROB and send it to the next latch.
        // TODO: Repeat for each lane of the pipeline.

/*
        unsigned int i;
        for(i=0; i<PIPE_WIDTH; i++)
        {  oldest_t o = oldest_and_ready(p->rest);
           if(o.valid) 
           {  int tag = o.inst.dr_tag;
               if(p->rest->REST_Entries[tag].inst.src1_ready && p->rest->REST_Entries[tag].inst.src2_ready)
               {  p->rest->REST_Entries[tag].scheduled = true;
                  p->SC_latch[i].inst = p->pipe_REST->REST_Entries[tag].inst;
                  p->SC_latch[i].valid = true;
                  p->SC_latch[i].stall = false;
               }
           }
        }
    }
} 
*/


        for(unsigned int ii=0;ii<PIPE_WIDTH;ii++)
        {  ROB* temp_pipe_rob = NULL; 
           temp_pipe_rob = new ROB[1];
           unsigned int index = 0;
           temp_pipe_rob->entries[0].inst.inst_num=(1<<64)-1;
           for(unsigned int kk=0;kk<NUM_ROB_ENTRIES; kk++)
           {  if(p->rob->entries[kk].valid&&(p->rob->entries[kk].ready == false) && (p->rob->entries[kk].exec == false))
              {  if((p->rob->entries[kk].inst.src1_ready == true) && (p->rob->entries[kk].inst.src2_ready == true))
                 {  temp_pipe_rob->entries[index].inst = p->rob->entries[kk].inst;
                    temp_pipe_rob->entries[index].valid = true;
                    temp_pipe_rob->entries[index].exec = false;
                    temp_pipe_rob->entries[index].ready= false;
                    index++;
                 }
              }
           }
           uint64_t temp_inst_num = (1<<64)-1;
           InstInfo temp_inst; 
           unsigned int temp_index;
           for(unsigned int i=0;i<index;i++)
           {  if(temp_pipe_rob->entries[i].inst.inst_num<temp_inst_num)
              {  temp_inst_num = temp_pipe_rob->entries[i].inst.inst_num;
                 temp_inst = temp_pipe_rob->entries[i].inst; 
                 temp_index = i;
              }
           }
           if(temp_inst_num == ((2<<64) - 1))
           {  p->SC_latch[ii].valid = false; 
              p->SC_latch[ii].stall = true;
              goto skip1;
           }
           for(unsigned int k=0;k<NUM_ROB_ENTRIES;k++)
           {  if(p->rob->entries[k].inst.inst_num == temp_inst_num)
              {  p->rob->entries[k].exec = true;
                 p->SC_latch[ii].inst = p->rob->entries[k].inst;
                 p->SC_latch[ii].valid = true; 
                 p->SC_latch[ii].stall = false;
                 break;
              }
           }
           skip1:
           delete[] temp_pipe_rob;
           return;
        } 
    }
}


/**
 * Simulate one cycle of the writeback stage of a pipeline: update the ROB
 * with information from instructions that have finished executing.
 * 
 * You must implement this function in pipeline.cpp in part B of the
 * assignment.
 * 
 * @param p the pipeline to simulate
 */
void pipe_cycle_writeback(Pipeline *p)
{
    // TODO: For each valid instruction from the EX stage:
    // TODO: Broadcast the result to all ROB entries.
    // TODO: Update the ROB: mark the instruction ready to commit.
    // TODO: Invalidate the instruction in the previous latch.

    // Remember: how many instructions can the EX stage send to the WB stage
    // in one cycle?
    //unsigned int i;

/*
    for(unsigned int ii=0;ii<MAX_WRITEBACKS;ii++)
    {  if(p->EX_latch[ii].valid)
       {  InstInfo ex_inst = p->EX_latch[i].inst;
          rob_wakeup(p->rob,ex_inst.dr_tag);
          rob_mark_ready(p->rob,ex_inst);
          p->EX_latch[i].valid = false;
          p->EX_latch[i].stall = false;
       }
    }
}
*/

    for(unsigned int j=0;j<PIPE_WIDTH;j++)
    {  
    for(unsigned int i=0;i<MAX_WRITEBACKS;i++)
    {  if(p->EX_latch[i].valid)
       {  InstInfo removed_inst = p->EX_latch[i].inst;
          p->EX_latch[i].valid = false;
          rob_wakeup(p->rob, removed_inst.dr_tag);
//          rob_remove(p->rob, removed_inst);
          rob_mark_ready(p->rob, removed_inst);
       }
    }
    }

/*
  //for(unsigned int jj=0;jj<PIPE_WIDTH;jj++)
  { for(unsigned int ii=0;ii<MAX_WRITEBACKS;ii++)
    { if(p->EX_latch[ii].valid && (p->EX_latch[ii].inst.exe_wait_cycles==0))
      {  p->rob->entries[p->EX_latch[ii].inst.dr_tag].ready=true;
         rob_wakeup(p->rob,p->EX_latch[ii].inst.dr_tag);
         p->EX_latch[ii].valid=false;
      }
    }
  }
*/
/*
    for(unsigned int kk=0;kk<NUM_ROB_ENTRIES;kk++)
    {  if(p->rob->entries[kk].valid && p->rob->entries[kk].ready)
       {  p->rob->entries[kk].exec=0;
       }
    }
*/
}


/**
 * Simulate one cycle of the commit stage of a pipeline: commit instructions
 * in the ROB that are ready to commit.
 * 
 * You must implement this function in pipeline.cpp in part B of the
 * assignment.
 * 
 * @param p the pipeline to simulate
 */
void pipe_cycle_commit(Pipeline *p)
{
    // TODO: Check if the instruction at the head of the ROB is ready to
    //       commit.
    // TODO: If so, remove it from the ROB.
    // TODO: Commit that instruction.
    // TODO: If a RAT mapping exists and is still relevant, update the RAT.
    // TODO: Repeat for each lane of the pipeline.

    // The following code is DUMMY CODE to ensure that the base code compiles
    // and that the simulation terminates. Replace it with a correct
    // implementation!
/*
    static uint32_t last=1;
    unsigned int ii = 0;
    for(ii=0;ii<PIPE_WIDTH;ii++)
    {  if(rob_check_head(p->rob))
       {  p->stat_retired_inst++;
          InstInfo commit_inst = rob_remove_head(p->rob);
          //REST_remove(p->pipe_REST,commit_inst);
          if(p->rat->entries[commit_inst.dest_reg].prf_id == commit_inst.dr_tag)
          {  rat_reset_entry(p->rat,commit_inst.dest_reg);
          }
          if(commit_inst.inst_num >= p->halt_inst_num)
          {  p->halt = true;
          }
       }
       if(SCHED_POLICY == SCHED_IN_ORDER)
       {  last++;
       }
    }
}
*/
for(unsigned int i=0;i<PIPE_WIDTH;i++)
{ if(rob_check_head(p->rob))
  { InstInfo inst = rob_remove_head(p->rob);
    p->stat_retired_inst++;
    if(inst.inst_num >= p->halt_inst_num)
     { p->halt = true;}
    if(rat_get_remap(p->rat, inst.dest_reg)!=-1)
    {rat_reset_entry(p->rat, inst.dest_reg);}
   }
}
}

/*
    for(unsigned int ii=0;ii<PIPE_WIDTH;ii++)
 {  InstInfo temp;
    temp.inst_num = -1;
    unsigned int prf_id_rat;
    unsigned int index_rob = p->rob->head_ptr;
    if(rob_check_head(p->rob))
    {  temp = rob_remove_head(p->rob);
       p->rob->entries[index_rob].ready = false;
       prf_id_rat = rat_get_remap(p->rat, temp.dest_reg);
       assert(index_rob == p->rob->entries[index_rob].inst.dr_tag);
       if(index_rob == prf_id_rat)
       {  rat_reset_entry(p->rat, temp.dest_reg);
       }
       p->stat_retired_inst++;
       if(temp.inst_num>=p->halt_inst_num)//p->EX_latch[ii].inst.inst_num>=p->halt_inst_num)
       {  //p->stat_retired_inst++;
          p->halt=true;
       }
       
    }
    else
    {
    }
 }
}
*/
/*    for (unsigned int i = 0; i < PIPE_WIDTH; i++)
    {
        if (p->FE_latch[i].valid)
        {
            pipe_commit_inst(p, p->FE_latch[i].inst);
            p->FE_latch[i].valid = false;
        }
    }
}
*/
