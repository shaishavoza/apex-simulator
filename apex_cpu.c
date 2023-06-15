/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"
/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
int decodeHasINS;
int integerFUHasINS;
int multiplierFUHasINS;
int loadstoreFUHasINS;
# define SIZE 100
int stall = 0;
int mulFUCount = 0;
int loadstoreCount = 0;

void enqueue(APEX_Instruction currins);
void dequeue();
APEX_Instruction inp_arr[SIZE];
int Rear = - 1;
int Front = - 1;
 
void enqueue(APEX_Instruction currins)
{
    if (Rear == SIZE - 1)
       printf("Overflow \n");
    else
    {
        if (Front == - 1)
        Front = 0;
        Rear = Rear + 1;
        inp_arr[Rear] = currins;
    }
}  
void dequeue()
{
    if (Front == - 1 || Front > Rear)
    {
        printf("Underflow \n");
        return ;
    }
    else
    {
        Front = Front + 1;
    }
}

static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        {
            printf("%s,R%d,R%d,R%d ",stage->opcode_str,stage->rd,stage->rs1,stage->rs2);
            break;
        }
        case OPCODE_ADDL:
        {
            printf("%s,R%d,R%d,#%d ",stage->opcode_str,stage->rd,stage->rs1,stage->imm);
            break;
        }
        case OPCODE_SUB:
        {
            printf("%s,R%d,R%d,R%d ",stage->opcode_str,stage->rd,stage->rs1,stage->rs2);
            break;
        }
        case OPCODE_SUBL:
        {
            printf("%s,R%d,R%d,#%d ",stage->opcode_str,stage->rd,stage->rs1,stage->imm);
            break;
        }
        case OPCODE_MUL:
        {
            printf("%s,R%d,R%d,R%d ",stage->opcode_str,stage->rd,stage->rs1,stage->rs2);
            break;
        }
        case OPCODE_DIV:
        {
            break;
        }
        case OPCODE_AND:
        {
            printf("%s,R%d,R%d,R%d ",stage->opcode_str,stage->rd,stage->rs1,stage->rs2);
            break;
        }
        case OPCODE_OR:
        {
            printf("%s,R%d,R%d,R%d ",stage->opcode_str,stage->rd,stage->rs1,stage->rs2);
            break;
        }
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_LDR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,stage->rs2);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_STR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,stage->rs2);
            break;
        }

        case OPCODE_BZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }
        case OPCODE_BNZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_NOP:
        {
            printf("%s ", stage->opcode_str);
            break;
        }

        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;
    printf("======STATE OF ARCHITECTURAL REGISTER FILE======\n");

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i][0]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i][0]);
    }

    printf("\n");
}

static void
print_data_memory(const APEX_CPU *cpu)
{
    int i;

    printf("======STATE OF DATA MEMORY======\n");

    for( int i = 0 ; i < DATA_MEMORY_SIZE/2; ++i)
    {
    printf("MEM[%-3d] value[%-3d]\n", i, cpu->data_memory[i]);
    }
    printf("\n");
    for( i = (DATA_MEMORY_SIZE/2) ; i < DATA_MEMORY_SIZE; ++i)
    {
    printf("MEM[%-3d] value[%-3d]\n", i, cpu->data_memory[i]);
    }
    printf("\n");


}
/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.imm = current_ins->imm;

        /* Update PC for next instruction */
        if(stall == 0){
        cpu->pc += 4;
        

        /* Copy data from fetch latch to decode latch*/
        cpu->decode = cpu->fetch;
        }

        if (cpu->displayFlag)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        if(cpu->single_step)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT && stall == 0)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1
                || integerFUHasINS == 1)
                {   
                    stall = 1;
                }
                else {
                    stall = 0;
                }
                break;
            }

            case OPCODE_LOAD:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || loadstoreFUHasINS == 1)
                {
                    stall = 1;
                }
                else {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->regs[cpu->decode.rd][1] = 1;
                    stall = 0;
                }
                break;
            }

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                if(integerFUHasINS == 1)
                {
                    stall = 1;
                }
                else
                {
                    stall = 0;
                    cpu->regs[cpu->decode.rd][1] = 1;
                }
                break;
            }

            case OPCODE_ADDL:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1 || integerFUHasINS == 1)
                {   
                    stall = 1;
                }
                else
                {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->regs[cpu->decode.rd][1] = 1;
                stall = 0;
                }
                break;
            }

            case OPCODE_AND:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1
                || integerFUHasINS == 1)
                {   
                    stall = 1;
                }
                else{
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2][0];
                cpu->regs[cpu->decode.rd][1] = 1;
                stall = 0;
                }
                break;
            }

            case OPCODE_BNZ:
            {
                if(integerFUHasINS == 1)
                {
                    stall = 1;
                }
                else
                {
                    stall = 0;
                }
                break;
            }

            case OPCODE_BZ:
            {
                if(integerFUHasINS == 1)
                {
                    stall = 1;
                }
                else
                {
                    stall = 0;
                }
                break;
            }

            case OPCODE_CMP:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || integerFUHasINS == 1)
                {   
                    stall = 1;
                }
                else{
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2][0];
                stall = 0;
                }
                break;
            }

            case OPCODE_HALT:
            {
                if(integerFUHasINS == 1)
                {
                    stall = 1;
                }
                else
                {
                    stall = 0;
                }
                break;
            }

            case OPCODE_LDR:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1
                || loadstoreFUHasINS == 1)
                {   
                    stall = 1;
                }
                else{
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2][0];
                cpu->regs[cpu->decode.rd][1] = 1;
                stall = 0;
                }
                break;
            }

            case OPCODE_MUL:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1
                || multiplierFUHasINS == 1)
                {   
                    stall = 1;
                }
                else{
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2][0];
                cpu->regs[cpu->decode.rd][1] = 1;
                stall = 0;
                }
                break;
            }

            case OPCODE_NOP:
            {
                if(integerFUHasINS == 1)
                {
                    stall = 1;
                }
                else
                {
                    stall = 0;
                }
                break;
            }

            case OPCODE_OR:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1
                || integerFUHasINS == 1)
                {   
                    stall = 1;
                }
                else{
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2][0];
                cpu->regs[cpu->decode.rd][1] = 1;
                stall = 0;
                }
                break;
            }

            case OPCODE_STORE:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || loadstoreFUHasINS == 1)
                {   
                    stall = 1;
                }
                else{
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2][0];
                // cpu->regs[cpu->decode.rs1][1] = 1;
                stall = 0;
                }
                break;
            }
            
            case OPCODE_STR:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1
                || loadstoreFUHasINS == 1)
                {   
                    stall = 1;
                }
                else
                {
                cpu->decode.rd_value = cpu->regs[cpu->decode.rd][0];
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2][0];
                // cpu->regs[cpu->decode.rd][1] = 1;
                stall = 0;
                }
            break;
            }

            case OPCODE_SUB:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1
                || integerFUHasINS == 1)
                {   
                    stall = 1;
                }
                else{
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2][0];
                cpu->regs[cpu->decode.rd][1] = 1;
                stall = 0;
            }
            break;
            }

            case OPCODE_SUBL:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1
                || integerFUHasINS == 1)
                {   
                    stall = 1;
                }
                else{
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->regs[cpu->decode.rd][1] = 1;
                stall = 0;
                }
                break;
            }
            case OPCODE_XOR:
            {
                if(cpu->regs[cpu->decode.rs1][1] == 1 || cpu->regs[cpu->decode.rs2][1] == 1 || cpu->regs[cpu->decode.rd][1] == 1
                || integerFUHasINS == 1)
                {   
                    stall = 1;
                }
                else{
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1][0];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2][0];
                cpu->regs[cpu->decode.rd][1] = 1;
                stall = 0;
                }
                break;
            }
        }

        /* Copy data from decode latch to execute latch*/
        if(stall == 0)
        {
            if(cpu->decode.opcode != OPCODE_LDR && cpu->decode.opcode != OPCODE_LOAD && 
            cpu->decode.opcode != OPCODE_STORE && cpu->decode.opcode != OPCODE_STR &&
            cpu->decode.opcode != OPCODE_MUL)
            {
                cpu->intergerFU = cpu->decode;
                APEX_Instruction currins = cpu->code_memory[get_code_memory_index_from_pc(cpu->decode.pc)];
                enqueue(currins);
            }
            if(cpu->decode.opcode == OPCODE_MUL)
            {
                cpu->multiplierFU = cpu->decode;
                APEX_Instruction currins = cpu->code_memory[get_code_memory_index_from_pc(cpu->decode.pc)];
                enqueue(currins);
            }
            if(cpu->decode.opcode == OPCODE_LDR || cpu->decode.opcode == OPCODE_LOAD || 
            cpu->decode.opcode == OPCODE_STORE || cpu->decode.opcode == OPCODE_STR)
            {
                // printf("%s", cpu->decode.opcode_str);
                cpu->loadstoreFU = cpu->decode;
                APEX_Instruction currins = cpu->code_memory[get_code_memory_index_from_pc(cpu->decode.pc)];
                enqueue(currins);
            }
        cpu->decode.has_insn = FALSE;
        }
        
        if (cpu->displayFlag)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
        if(cpu->single_step)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
    }
}

static void
APEX_intergerFU(APEX_CPU *cpu)
{
    if(cpu->intergerFU.has_insn)
    {
        integerFUHasINS = 1;
        switch(cpu->intergerFU.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->intergerFU.result_buffer
                    = cpu->intergerFU.rs1_value + cpu->intergerFU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->intergerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_ADDL:
            {
                cpu->intergerFU.result_buffer
                    = cpu->intergerFU.rs1_value + cpu->intergerFU.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->intergerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_AND:
            {
                cpu->intergerFU.result_buffer = cpu->intergerFU.rs1_value & cpu->intergerFU.rs2_value;
                break;
            }
            case OPCODE_BNZ:
            {
                if (cpu->zero_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->intergerFU.pc + cpu->intergerFU.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_BZ:
            {
                if (cpu->zero_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->intergerFU.pc + cpu->intergerFU.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
 
                }
                break;
            }
            case OPCODE_CMP:
            {
                if(cpu->intergerFU.rs1_value == cpu->intergerFU.rs2_value){
                    cpu->zero_flag = TRUE;
                }
                else cpu->zero_flag = FALSE;
                break;
            }
            case OPCODE_MOVC:
            {
                cpu->intergerFU.result_buffer = cpu->intergerFU.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->intergerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_HALT:
            {
                break;
            }
            case OPCODE_OR:
            {
                cpu->intergerFU.result_buffer = cpu->intergerFU.rs1_value | cpu->intergerFU.rs2_value;
                break;
            }
            case OPCODE_SUB:
            {
                cpu->intergerFU.result_buffer
                    = cpu->intergerFU.rs1_value - cpu->intergerFU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->intergerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->intergerFU.result_buffer
                    = cpu->intergerFU.rs1_value - cpu->intergerFU.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->intergerFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
            case OPCODE_XOR:
            {
                cpu->intergerFU.result_buffer = cpu->intergerFU.rs1_value ^ cpu->intergerFU.rs2_value;
                break;
            }
            case OPCODE_NOP:
            {
                break;
            }
        }
        APEX_Instruction intcurrins = cpu->code_memory[get_code_memory_index_from_pc(cpu->intergerFU.pc)];
        if(inp_arr[Front].opcode == intcurrins.opcode && inp_arr[Front].rs1 == intcurrins.rs1)
        {
            cpu->writeback = cpu->intergerFU;
            cpu->intergerFU.has_insn = FALSE;
            integerFUHasINS = 0;
        }

        if (cpu->displayFlag)
        {
            print_stage_content("INTEGERFU", &cpu->intergerFU);
        }
        if(cpu->single_step)
        {
            print_stage_content("INTEGERFU", &cpu->intergerFU);
        }
    }

}
static void
APEX_multiplierFU(APEX_CPU *cpu)
{
    if(cpu->multiplierFU.has_insn)
    {
        multiplierFUHasINS = 1;
        mulFUCount++;
        if(mulFUCount == 3)
        {
        switch(cpu->multiplierFU.opcode)
        {
            case OPCODE_MUL:
            {
                cpu->multiplierFU.result_buffer
                    = cpu->multiplierFU.rs1_value * cpu->multiplierFU.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->multiplierFU.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }
        }
        APEX_Instruction mulcurrins = cpu->code_memory[get_code_memory_index_from_pc(cpu->multiplierFU.pc)];
        if(inp_arr[Front].opcode == mulcurrins.opcode && inp_arr[Front].rs1 == mulcurrins.rs1)
        {
        cpu->writeback = cpu->multiplierFU;
        mulFUCount = 0;
        cpu->multiplierFU.has_insn = FALSE;
        multiplierFUHasINS = 0;
        }
        }

        if (cpu->displayFlag)
        {
            print_stage_content("MULTIPLIERFU", &cpu->multiplierFU);
        }
        if(cpu->single_step)
        {
            print_stage_content("MULTIPLIERFU", &cpu->multiplierFU);
        }
        
    }

}
static void
APEX_loadstoreFU(APEX_CPU *cpu)
{
    if(cpu->loadstoreFU.has_insn)
    {
        loadstoreFUHasINS = 1;
        loadstoreCount++;
        if(loadstoreCount == 4)
        {
        switch(cpu->loadstoreFU.opcode)
        {
            case OPCODE_LOAD:
            {
                cpu->loadstoreFU.memory_address
                    = cpu->loadstoreFU.rs1_value + cpu->loadstoreFU.imm;
                //memory part                
                cpu->loadstoreFU.result_buffer
                    = cpu->data_memory[cpu->loadstoreFU.memory_address];
                break;
            }
            case OPCODE_LDR:
            {
                cpu->loadstoreFU.memory_address
                    = cpu->loadstoreFU.rs1_value + cpu->loadstoreFU.rs2_value;
                //memory part
                cpu->loadstoreFU.result_buffer
                    = cpu->data_memory[cpu->loadstoreFU.memory_address];
                break;
            }
            case OPCODE_STORE:
            {
                cpu->loadstoreFU.memory_address
                    = cpu->loadstoreFU.rs2_value + cpu->loadstoreFU.imm;
                //memory part
                cpu->data_memory[cpu->loadstoreFU.memory_address]
                    = cpu->loadstoreFU.rs1_value;
                break;
            }
            case OPCODE_STR:
            {
                cpu->loadstoreFU.memory_address
                    = cpu->loadstoreFU.rs1_value + cpu->loadstoreFU.rs2_value;
                //memory part
                cpu->data_memory[cpu->loadstoreFU.memory_address]
                    = cpu->loadstoreFU.rd_value;
                break;
            }
        }
        APEX_Instruction lscurrins = cpu->code_memory[get_code_memory_index_from_pc(cpu->loadstoreFU.pc)];
        if(inp_arr[Front].opcode == lscurrins.opcode && inp_arr[Front].rs1 == lscurrins.rs1)
        {
        cpu->writeback = cpu->loadstoreFU;
        loadstoreCount = 0;
        cpu->loadstoreFU.has_insn = FALSE;
        loadstoreFUHasINS = 0;
        }
        }

        if (cpu->displayFlag)
        {
            print_stage_content("LOADSTOREFU", &cpu->loadstoreFU);
        }
        if(cpu->single_step)
        {
            print_stage_content("LOADSTOREFU", &cpu->loadstoreFU);
        }
    }

}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }

            case OPCODE_ADDL:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }
            case OPCODE_AND:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }
            case OPCODE_BNZ:
            {
                stall = 0; 
                break;
            }
            case OPCODE_BZ:
            {
                stall = 0; 
                break;
            }
            case OPCODE_CMP:
            {
                break;
            }
            case OPCODE_HALT:
            {
                break;
            }
            case OPCODE_LDR:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }
            case OPCODE_MUL:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }
            case OPCODE_NOP:
            {
                break;
            }
            case OPCODE_OR:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }
            case OPCODE_STORE:
            {
                break;
            }
            case OPCODE_STR:
            {
                break;
            }
            case OPCODE_SUB:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }
            case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd][0] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rd][1] = 0;
                break;
            }
        
        }
        dequeue();

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (cpu->displayFlag)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }
        if(cpu->single_step)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }

    /* Default */
    return 0;
}
void
simulate(APEX_CPU *cpu)
{
    print_reg_file(cpu);
    print_data_memory(cpu);
    cpu->simulateFlag = 0;
}

void
display(APEX_CPU *cpu)
{
    simulate(cpu);
    cpu->displayFlag = 0;
}
void
single_step(APEX_CPU *cpu)
{
    cpu->single_step = TRUE;
}


/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *argv[])
{
    int i;
    int argInput;
    const char *filename = argv[1];
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    if(strcmp("Display",argv[2])==0)
    {
        cpu->displayFlag = TRUE;
        argInput = strtol(argv[3],NULL,0);
        //printf("%d\n",argInput);
        cpu->display = argInput;
       
    }

    if(strcmp("Simulate",argv[2])==0)
    {
        cpu->simulateFlag = TRUE;
        argInput = strtol(argv[3],NULL,0);
        cpu->simulate = argInput;
    }

    if(strcmp("single_step",argv[2])==0)
    {
        single_step(cpu);
    }

    if(strcmp("show_mem",argv[2])==0)
    {
        cpu->showMemFlag = TRUE;
        argInput = strtol(argv[3],NULL,0);
        cpu->showMem = argInput;
    }


    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(REG_FILE_SIZE * 2) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    //cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if (cpu->single_step)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }
        if(cpu->displayFlag)
        {
            printf("------CLOCK CYCLE %d------\n",cpu->clock);
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            if(cpu->single_step)
            {
                simulate(cpu);
                cpu->single_step = FALSE;
            }
            if(cpu->showMemFlag)
            {
                printf("Data stored at %d is [%-3d]\n",cpu->showMem,cpu->data_memory[cpu->showMem]);
                cpu->showMemFlag = FALSE;
            }
            break;
        }

        //APEX_memory(cpu);
        //APEX_execute(cpu);
        APEX_intergerFU(cpu);
        APEX_multiplierFU(cpu);
        APEX_loadstoreFU(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        //print_reg_file(cpu);

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }
        if(cpu->displayFlag && (cpu->clock == cpu->display))
        {
            break;
        }
        if(cpu->simulateFlag && (cpu->simulate == cpu->clock))
        {
            break;
        }
        cpu->clock++;
    }
    if(cpu->simulateFlag)
    {
        printf("State of registers at the end of %d elapsed cycles is:\n",cpu->clock);
        simulate(cpu);       
    }
    if(cpu->displayFlag)
    {
        printf("Displaying state after %d elapsed cycles\n",cpu->clock);
        display(cpu);            
    }

    //for( int i = 0 ; i < DATA_MEMORY_SIZE; i++){
    //    printf("MEM[%-3d] value[%-3d]\n", i, cpu->data_memory[i]);
    //}
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}