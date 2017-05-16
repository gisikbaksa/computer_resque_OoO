#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "data_structures.h"

//���� �� �۷ι��� ����
static int inst_length;
static int pc = 0;
static int cycle = 1;

static int decoded = 0;
static int issued = 0;
static int issueWaiting = 0;
static int cnt_IntAlu = 0;
static int cnt_MemRead = 0;
static int cnt_MemWrite = 0;

struct REPORT *core_simulator(struct CONFIG *config, struct INST *arr_inst, int arr_inst_len);

void fetch(struct CONFIG *config, struct FQ *fetch_queue, struct CA_status *fq_status, struct INST *arr_inst);
void decode(struct CONFIG *config, struct FQ *fetch_queue, struct CA_status *fq_status, struct RS *rs_ele, struct RAT *rat, struct ROB *rob, struct CA_status *rob_status);

void issue(struct CONFIG *config, struct RS *rs_ele);
void execute(struct RS *rs_ele, struct ROB* rob_ele, bool *is_completed_this_cycle);
void ex_and_issue(struct CONFIG *config, struct ROB *rob, struct CA_status *rob_status, struct RS *rs, bool *is_completed_this_cycle);

void wait(void);
void rs_retire(struct RS *rs_ele, struct ROB *rob);
void commit(struct CONFIG *config, struct ROB *rob, struct CA_status *rob_status, struct RAT *rat);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct REPORT *core_simulator(struct CONFIG *config, struct INST *arr_inst, int arr_inst_len)
{
	//���� �� �۷ι� �ʱ�ȭ
	pc = 0;
	cycle = 1;
	inst_length = arr_inst_len;

	decoded = 0;
	issued = 0;
	issueWaiting = 0;
	cnt_IntAlu = 0;
	cnt_MemRead = 0;
	cnt_MemWrite = 0;

	// Initializing ...

	int i;

	// RAT
	struct RAT rat[17]; // 0 means no. rat[1] ~ rat[16] are Arch Register entries

	// Fetch Queue
	struct FQ* fetch_queue = calloc(2 * (*config).Width, sizeof(struct FQ) );//struct FQ fetch_queue[2 * (*config).Width]
	struct CA_status fq_status;
	fq_status.size = 2 * (*config).Width;
	fq_status.head = 0;
	fq_status.occupied = 0;

	// Reservation Station
	struct RS* rs = calloc((*config).RS_size, sizeof(struct RS)); //struct RS rs[(*config).RS_size];
	for (i = 0; i < (*config).RS_size; i++) { rs[i].is_valid = false; }//initialize

	// is_completed_this_cycle? to prevent issue of insturction in RS which has V value promoted from Q in current cycle.
	bool* is_completed_this_cycle = calloc((*config).RS_size, sizeof(bool));//bool is_completed_this_cycle[(*config).ROB_size];
	
	// ReOrdering Budder
	struct ROB* rob = calloc((*config).ROB_size, sizeof(struct ROB));//struct ROB rob[(*config).ROB_size];
	struct CA_status rob_status;
	rob_status.size = (*config).ROB_size;
	rob_status.head = 0;
	rob_status.occupied = 0;
	
	while (pc < inst_length)
	{	
		//�� ������ ���� Ƚ���� �ʱ�ȭ�Ѵ�.
		decoded = 0;
		issued = 0;

		// Flush is_completed_this_cycle array
		for (i = 0; i < (*config).RS_size; i++) { is_completed_this_cycle[i] = false; }

		//////////////////////////////////////////////////////////////Loop1
		//ROB�� rob_status.occupied��ŭ ���鼭 commit/ (ex/issue) ����

		commit(config, rob, &rob_status, rat);

		ex_and_issue(config, rob, &rob_status, rs);

		
		//////////////////////////////////////////////////////////////

		// Issue older instructions first
		for (i = 0; i < issueWaiting && i < (*config).Width; i++)
		{
			// Find oldest among them
			struct RS *oldest = &issuewait_list[0];
			int oldest_reference = issuewait_reference[0];
			for (int j = 1; j < issueWaiting; j++)
			{
				if ( issuewait_list[j].time_decoded < (*oldest).time_decoded )
				{
					oldest = &issuewait_list[j];
					oldest_reference = issuewait_reference[j];
				}
			}

			// Issue oldest one
			issue(config, &rs[oldest_reference], issuewait_list, rob, is_completed_this_cycle, issuewait_reference, -1);
			is_issued_this_cycle[oldest_reference] = true;
			issued++;
		}
		issueWaiting = 0;
		
		// Commit ROB -> RAT
		commit(config, rob, &rob_status, rat);
		
		// For every entries in Reservation Station,
		for (i = 0; i < (*config).RS_size; i++)
		{
			if ( rs[i].is_valid ) // Instruction is inside this entry of RS
			{
				switch ( rs[i].time_left)
				{
					case 0:
						rs_retire(&rs[i], rob);
						break;
					case -1:
						issue(config, &rs[i], issuewait_list, rob, is_completed_this_cycle, issuewait_reference, i);
						break;
					default:
						execute(config, &rs[i], is_issued_this_cycle, i);
						break;
				}	
			}
			else                  // This entry of RS is empty now
			{
				// Try to decode instruction into empty place
				decode(config, fetch_queue, &fq_status, &rs[i], rat, rob, &rob_status);
			}	
		}

		// Fetch instructions
		fetch(config, fetch_queue, &fq_status, arr_inst);

		// Dump
		switch ((*config).Dump)
		{
			case 0:
				break;
			case 1:
				printf("= Cycle %-5d\n", cycle);
				ROB_arr_reporter(rob, rob_status);
				break;
			case 2:
				printf("= Cycle %-5d\n", cycle);
				RS_arr_reporter(rs, (*config).RS_size);
				ROB_arr_reporter(rob, rob_status);
				break;
			default:
				printf("= Cycle %-5d\n", cycle);
				//debug mode
				FQ_arr_printer(fetch_queue, fq_status);
				RAT_arr_printer(rat, 17);
				RS_arr_printer(rs, (*config).RS_size);
				ROB_arr_printer(rob, rob_status);
				getchar();
				break;
		}
		cycle++;
	}
	// Simulation finished
	
	//free array
	free(fetch_queue);
	free(rs);
	free(rob);

	free(is_completed_this_cycle);
	

	// Write a report
	struct REPORT *ptr_report = malloc(sizeof(struct REPORT));
	(*ptr_report).Cycles = cycle;
	(*ptr_report).Total_Insts = inst_length;
    (*ptr_report).IntAlu = cnt_IntAlu;
	(*ptr_report).MemRead = cnt_MemRead;
	(*ptr_report).MemWrite = cnt_MemWrite;
	(*ptr_report).IPC = ((double)inst_length / (double) cycle);


	return ptr_report;	
}

void fetch(struct CONFIG *config, struct FQ *fetch_queue, struct CA_status *fq_status, struct INST *arr_inst)
{
	int fetch_num = ((*config).Width > (*fq_status).size - (*fq_status).occupied) ? (*fq_status).size - (*fq_status).occupied : (*config).Width;
	int i;
	for (i = 0; i < fetch_num && pc < inst_length; i++)
	{
		fetch_queue[ca_next_pos(fq_status)].opcode = arr_inst[pc].opcode;
		fetch_queue[ca_next_pos(fq_status)].dest = arr_inst[pc].dest;
		fetch_queue[ca_next_pos(fq_status)].oprd_1 = arr_inst[pc].oprd_1;
		fetch_queue[ca_next_pos(fq_status)].oprd_2 = arr_inst[pc].oprd_2;
		ca_cnt_push(fq_status);
		pc++;
	}
}

void decode(struct CONFIG *config, struct FQ *fetch_queue, struct CA_status *fq_status, struct RS *rs_ele, struct RAT *rat, struct ROB *rob, struct CA_status *rob_status)
{
	if ((*fq_status).occupied > 0 && decoded < (*config).Width && (*rob_status).occupied < (*rob_status).size)
	{
		// Putting first element of Fetch Queue to Reservation Station	
		(*rs_ele).is_valid = true;
		(*rs_ele).time_decoded = cycle;
		(*rs_ele).opcode = fetch_queue[(*fq_status).head].opcode;
		// Count Instruction number
		switch ((*rs_ele).opcode)
		{
		case 0:
			cnt_IntAlu++;
			break;
		case 1:
			cnt_MemRead++;
			break;
		case 2:
			cnt_MemWrite++;
			break;
		}

		int oprd_1 = fetch_queue[(*fq_status).head].oprd_1;
		int oprd_2 = fetch_queue[(*fq_status).head].oprd_2;
		if (oprd_1 == 0 || rat[oprd_1].RF_valid)
		{
			(*rs_ele).oprd_1.state = V;
		}
		else
		{
			(*rs_ele).oprd_1.state = Q;
			(*rs_ele).oprd_1.data.q = rat[fetch_queue[(*fq_status).head].oprd_1].Q;
		}

		if (oprd_2 == 0 || rat[oprd_2].RF_valid)
		{
			(*rs_ele).oprd_2.state = V;
		}
		else
		{
			(*rs_ele).oprd_2.state = Q;
			(*rs_ele).oprd_2.data.q = rat[fetch_queue[(*fq_status).head].oprd_2].Q;
		}

		(*rs_ele).time_left = -1; // Waiting to be issued

								  // Putting first element of Fetch Queue to ROB
		rob[ca_next_pos(rob_status)].opcode = fetch_queue[(*fq_status).head].opcode;
		rob[ca_next_pos(rob_status)].dest = fetch_queue[(*fq_status).head].dest;
		rob[ca_next_pos(rob_status)].status = P;

		// Modify RAT status
		if (fetch_queue[(*fq_status).head].dest != 0)
		{
			rat[fetch_queue[(*fq_status).head].dest].RF_valid = false;		// Now data isn't inside the Arch Register file anymore
			rat[fetch_queue[(*fq_status).head].dest].Q = ca_next_pos(rob_status);   // So leave reference to ROB entry in RAT
		}

		ca_cnt_push(rob_status); // Element has been pushed to ROB
		ca_cnt_pop(fq_status);   // Element has been poped from Fetch Queue
		decoded++;
	}
}

void issue(struct CONFIG *config, struct RS *rs_ele)
{
	if (issued < (*config).Width)
	{//���� width��ŭ issue�� ���� �ʾҴٸ�
		if ((*rs_ele).oprd_1.state == V &&
			(*rs_ele).oprd_2.state == V)
		{//issue ������ �˻��ϰ� �����Ѵٸ�, issue�� �Ѵ�.

			(*rs_ele).time_left = ((*rs_ele).opcode == MemRead) ? 3 : 1;
			++issued;

		}
			
	}
}
/*
if (issued < (*config).Width)
{
// Q ->V Promotion
if ((*rs_ele).oprd_1.state == Q && rob[((*rs_ele).oprd_1.data.q)].status)
{
(*rs_ele).oprd_1.state = V;
}

if ((*rs_ele).oprd_2.state == Q && rob[((*rs_ele).oprd_2.data.q)].status)
{
(*rs_ele).oprd_2.state = V;
}

// Issue
if ((*rs_ele).oprd_1.state && (*rs_ele).oprd_2.state && !is_completed_this_cycle[(*rs_ele).rob_dest])
{
(*rs_ele).time_left = ((*rs_ele).opcode == MemRead) ? 3 : 1;
}
issued++;
}*/
void execute(struct RS *rs_ele, struct ROB* rob_ele, bool *is_completed_this_cycle)
{
	//if ( executed < (*config).Width)
	//�̹� �̽��� �ִ� N������ �����ϱ� ������, ex�� �ִ� N�������� ����ȴ�. �˻��ʿ� ����

	if (rs_ele->time_left == 0)
	{//���� ���� ������̶��, �����ϰ� RS�� ��� ���� ROB�� C ���·� �ٲ۴�.
		
		(rs_ele->is_valid) = false;
		(*is_completed_this_cycle) = true;
		(rob_ele->status) = C;
	}
	else
	{//���� issue�� �Ǿ��ִٸ�,
		--(rs_ele->time_left);//ALU�� �ִ´� ( ���� ��� �ܰ� ).
	}

}

void ex_and_issue(struct CONFIG *config, struct ROB *rob, struct CA_status *rob_status, struct RS *rs, bool *is_completed_this_cycle)
{
	int i;
	struct ROB * rob_ele;
	struct RS  * rs_ele;
	for (i = 0; i < (rob_status->occupied); ++i)
	{//��� ���ҿ� ����, �˻��Ѵ�

		rob_ele = rob + ((rob_status->head + i) % rob_status->size);
		if (rob_ele->status == P)
		{//���� ���°� P���, �̴� issue�� ���� ex�� ����� �����Ѵ�.
			
			rs_ele = (rs + (rob_ele->rs_dest));

			if (rs_ele->time_left >= 0)
			{//���� Issue �Ǿ��ٸ�
				execute(rs + (rob_ele->rs_dest), rob_ele, is_completed_this_cycle + (rob_ele->rs_dest));
				//���� �� ���� �Ϸ��Ѵ�.
			}
			else
			{//���� Issue �ȵǾ��ٸ�
				issue(config, rs_ele);
				//�̽� ������ �˻��ϰ� ������ ��� �̽��Ѵ�.
			}

		}

	}

}

void wait(void)
{
}

void rs_retire(struct RS *rs_ele, struct ROB *rob)
{
	rob[(*rs_ele).rob_dest].status = C;
	(*rs_ele).is_valid = false;
}

void commit(struct CONFIG *config, struct ROB *rob, struct CA_status *rob_status, struct RAT *rat)
{
	int i;
	int num_of_search;
	struct ROB* rob_head;
	// Only permits upto N commits
	num_of_search = (*config).Width > (*rob_status).occupied ? (*rob_status).occupied : (*config).Width;

	//���� ROB�� ���� ������ Width �� ���� ���ڱ����� Ȯ���ϸ� ��.
	//�ֳĸ� �ִ� Ŀ���� ���������� C�� ��쿡�� Width���� �����ϰ�(�ִ��� �ٿ��� Head - Width ����)
	//���� ������ Width���� �۴ٸ�, �ִ� Ŀ���� ���� ���������� �����ϱ� ����.

	for (i = 0; i < num_of_search; i++)
	{
		rob_head = rob + (rob_status->head);
		if ( (rob_head->status) == C)
		{
			rat[rob_head->dest].RF_valid = true;
			ca_cnt_pop(rob_status);
		}
		else
		{
			break;
		}
	}

}