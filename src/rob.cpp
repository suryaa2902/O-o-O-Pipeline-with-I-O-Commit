///////////////////////////////////////////////////////////
// You must modify this file to implement the following: //
// - bool rob_check_space(ROB *rob)                      //
// - int rob_insert(ROB *rob, InstInfo inst)             //
// - void rob_mark_exec(ROB *rob, InstInfo inst)         //
// - void rob_mark_ready(ROB *rob, InstInfo inst)        //
// - bool rob_check_ready(ROB *rob, int tag)             //
// - bool rob_check_head(ROB *rob)                       //
// - void rob_wakeup(ROB *rob, int tag)                  //
// - InstInfo rob_remove_head(ROB *rob)                  //
///////////////////////////////////////////////////////////

// Author: Suryaa Senthilkumar Shanthi

// rob.cpp
// Implements the re-order buffer.

#include "rob.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

using namespace std;

/**
 * The number of entries in the ROB; that is, the maximum number of
 * instructions that can be stored in the ROB at any given time.
 * 
 * You should use only this many entries of the ROB::entries array.
 */
extern uint32_t NUM_ROB_ENTRIES;
static int rob_count;

/**
 * Allocate and initialize a new ROB.
 * 
 * This function has been implemented for you.
 * 
 * @return a pointer to a newly allocated ROB
 */
ROB *rob_init()
{
    ROB *rob = (ROB *)calloc(1, sizeof(ROB));

    rob->head_ptr = 0;
    rob->tail_ptr = 0;

    for (int i = 0; i < MAX_ROB_ENTRIES; i++)
    {
        rob->entries[i].valid = false;
        rob->entries[i].ready = false;
        rob->entries[i].exec = false;
    }

    return rob;
}

/**
 * Print out the state of the ROB for debugging purposes.
 * 
 * This function is called automatically in pipe_print_state(), but you may
 * also use it to help debug your ROB implementation. If you choose to do so,
 * please remove calls to this function before submitting the lab.
 * 
 * @param rob the ROB
 */
void rob_print_state(ROB *rob)
{
    printf("Current ROB state:\n");
    printf("Entry  Inst   Valid   ready\n");
    for (int i = 0; i < 7; i++)
    {
        printf("%5d ::  %d\t", i, (int)rob->entries[i].inst.inst_num);
        printf(" %5d\t", rob->entries[i].valid);
        printf(" %5d\n", rob->entries[i].ready);
        printf(" %5d\n", rob->entries[i].exec);
    }
    printf("\n");
}

/**
 * Check if there is space available to insert another instruction into the
 * ROB.
 * 
 * You must implement this function in part A of the assignment.
 * 
 * @param rob the ROB
 * @return true if the ROB has space for another instruction, false otherwise
 */
bool rob_check_space(ROB *rob)
{
    // TODO: Return true if there is space to insert another instruction into
    //       the ROB, false otherwise.

/*
    if((rob->entries[rob->tail_ptr].valid == false))
    { 
     return true;
    }
    else
    {
     return false;
    }
*/

    return rob_count < MAX_ROB_ENTRIES;
}

/**
 * Insert an instruction into the ROB at the tail pointer.
 * 
 * You must implement this function in part A of the assignment.
 * 
 * @param rob the ROB
 * @param inst the instruction to insert
 * @return the ID (index) of the newly inserted instruction in the ROB, or -1
 *         if there is no more space in the ROB
 */
int rob_insert(ROB *rob, InstInfo inst)
{
    // TODO: Check if there is space available in the ROB. If so:
    // TODO: Create an entry containing this instruction at the tail of the
    //       ROB. Set all the fields on the entry to their correct values.
    // TODO: Advance the tail pointer, wrapping around if needed.


    if(rob_check_space(rob))
    {
     int temp_tail;
     rob->entries[rob->tail_ptr].valid=true; rob->entries[rob->tail_ptr].inst=inst;
     temp_tail=rob->tail_ptr;
     if(++rob->tail_ptr==NUM_ROB_ENTRIES)
     {
      rob->tail_ptr=0;
//      return(temp_tail);
     }
     return(temp_tail);
   }
   else
   {
    return -1;
   }
}   

/*
    assert(inst.dr_tag==-1);
    assert(rob_check_space(rob));
    assert(!rob->entries[rob->tail_ptr].valid);
    assert(!rob->entries[rob->tail_ptr].ready);
    assert(!rob->entries[rob->tail_ptr].exec);

    rob_count++;

    rob->entries[rob->tail_ptr].inst = inst;

    rob->entries[rob->tail_ptr].inst.dr_tag = rob->tail_ptr;
    rob->entries[rob->tail_ptr].valid = true;

    int old_tail = rob->tail_ptr;
    rob->tail_ptr = rob->tail_ptr+1 == NUM_ROB_ENTRIES ? 0 : rob->tail_ptr+1;
    return old_tail;
 
}
*/

/**
 * Find the given instruction in the ROB and mark it as executing.
 * 
 * In part B, you will call this function when an instruction is scheduled for
 * execution.
 * 
 * You must implement this function in part A of the assignment.
 * 
 * @param rob the ROB
 * @param inst the instruction that is now executing
 */
void rob_mark_exec(ROB *rob, InstInfo inst)
{
    // TODO: Find the entry in the ROB containing the given instruction.
    //       (Hint: Is there an easy way to tell at what index the given
    //       instruction is located in the ROB?)
    // TODO: Update that entry.

/*
    for(int i=0;i<NUM_ROB_ENTRIES;i++)
    {
     if(rob->entries[i].valid==true)
     {
      if(rob->entries[i].inst.inst_num==inst.inst_num)
      {
       if((rob->entries[i].inst.src1_ready==true)&&(rob->entries[i].inst.src2_ready==true))
       {
        rob->entries[i].exec=true;
        break;
       }
       else
       {
        rob->entries[i].exec=false;
        break;
       }
      }
     }
    }
}
*/

    assert(inst.dr_tag!=-1);
    assert(!rob->entries[inst.dr_tag].exec);
    rob->entries[inst.dr_tag].exec = true;
}


/**
 * Find the given instruction in the ROB and mark it as having its output
 * ready (i.e., being ready to commit).
 * 
 * In part B, you will call this function when an instruction is finished
 * executing and is being written back.
 * 
 * You must implement this function in part A of the assignment.
 * 
 * @param rob the ROB
 * @param inst the instruction whose output is ready
 */
void rob_mark_ready(ROB *rob, InstInfo inst)
{
    // TODO: Find the entry in the ROB containing the given instruction.
    //       (Hint: Is there an easy way to tell at what index the given
    //       instruction is located in the ROB?)
    // TODO: Update that entry.
/*
    for(unsigned int index=0;index<NUM_ROB_ENTRIES;index++)
    {  if(rob->entries[index].valid == true)
       {  if(rob->entries[index].inst.inst_num == inst.inst_num)
          {  if(rob->entries[index].exec == true)
             {  rob->entries[index].ready = true;
                break;
             }
             else
             {  rob->entries[index].ready = true;
                break;
             }
          }
       }
    }
*/
    assert(inst.dr_tag!=-1);
    assert(!rob->entries[inst.dr_tag].ready);
    rob->entries[inst.dr_tag].ready = true;
}

/**
 * Check if the instruction with the given tag (ID/index) has its output ready.
 * 
 * This is the same as asking if that instruction is ready to commit.
 * 
 * You must implement this function in part A of the assignment.
 * 
 * @param rob the ROB
 * @param tag the tag (ID/index) of the instruction to check
 * @return true if the instruction has its output ready, false if it is not or
 *         if there is no valid instruction at this tag in the ROB
 */
bool rob_check_ready(ROB *rob, int tag)
{
    // TODO: Return true if the instruction at this tag (ID/index) is valid and
    //       has its output ready (i.e., is ready to commit), false otherwise.
/*
    if((rob->entries[tag].ready == true)&&(rob->entries[tag].valid == true))
    {  return true;
    }
    else
    {  return false;
    }
*/
    assert(rob->entries[tag].valid);
    return rob->entries[tag].ready;
}

/**
 * Check if the instruction at the head of the ROB is ready to commit.
 * 
 * You must implement this function in part A of the assignment.
 * 
 * @param rob the ROB
 * @return true if the instruction at the head of the ROB is ready to commit,
 *         false if it is not or if there is no valid instruction at the head
 *         of the ROB
 */
bool rob_check_head(ROB *rob)
{
    // TODO: Return true if the instruction at the head of the ROB is valid and
    //       ready to commit, false otherwise.
    if((rob->entries[rob->head_ptr].ready == true) && (rob->entries[rob->head_ptr].valid == true))
    {  return true;
    }
    else 
    {  return false;
    }
    //assert(rob->entries[rob->head_ptr].valid);
    //return rob_check_ready(rob, rob->head_ptr);
}

/**
 * Wake up instructions that are dependent on the instruction with the given
 * tag.
 * 
 * In part B, you will call this function during the writeback stage of the
 * pipeline: for each instruction that has finished executing, you will call
 * this function with its destination tag to indicate that the data with that
 * tag is now ready.
 * 
 * Then, for each source operand for each valid instruction in the ROB, if the
 * tag of the source operand matches the given tag, this function should mark
 * that source operand as ready.
 * 
 * You must implement this function in part A of the assignment.
 * 
 * @param rob the ROB
 * @param tag the tag of the instruction that has finished executing
 */
void rob_wakeup(ROB *rob, int tag)
{
    // TODO: Update the relevant src1 ready bits throughout the ROB.
    // TODO: Update the relevant src2 ready bits throughout the ROB.
    for(unsigned int i = 0; i < NUM_ROB_ENTRIES; i++)
    {
      if(rob->entries[i].valid == true)
      {
        if(rob->entries[i].inst.src1_tag == tag)
        {
          rob->entries[i].inst.src1_ready = true; rob->entries[i].inst.src1_tag = -1;
        }
        if(rob->entries[i].inst.src2_tag==tag)
        {
          rob->entries[i].inst.src2_ready=true; rob->entries[i].inst.src2_tag=-1;
        }
      }
    }
}

/**
 * If the head entry of the ROB is ready to commit, remove that entry and
 * return the instruction contained there.
 * 
 * You must implement this function in part A of the assignment.
 * 
 * @param rob the ROB
 * @return the instruction that was previously at the head of the ROB
 */
InstInfo rob_remove_head(ROB *rob)
{
    // TODO: Check if the head entry is ready to commit. If so:
    // TODO: Remove that entry.
    // TODO: Advance the head pointer, wrapping around if needed.
    // TODO: Return the instruction in the removed entry.
/*
    InstInfo temp;
    temp.inst_num = -1;
    if(rob_check_head(rob))
    {  rob->entries[rob->head_ptr].inst;
       if(++rob->head_ptr == NUM_ROB_ENTRIES)
       {  rob->head_ptr = 0;
       }
       return temp;
    }
    else
    {  return temp;
    }
*/
    assert(rob->entries[rob->head_ptr].valid);
    InstInfo head = rob->entries[rob->head_ptr].inst;
    rob->head_ptr = rob->head_ptr+1 == NUM_ROB_ENTRIES ? 0 :rob->head_ptr+1;
    return head;
}
