#include <stdio.h>
#include <stdlib.h>
#include "components.h"
#include "cycleindex.c"

#define REGISTER_SIZE 16

struct Config
{
	int debug;
	int width;
	int rob_size;
	int rs_size;
};


void fetch(int fetch_width, FILE* inst_mem, struct FQ* fetch_queue, struct Cycle_index* idx )
/*{
	for (int num_fetch = 0;
		num_fetch < fetch_width && num_fetch < ((*idx).length - (*idx).n_elem);//������� �� ä��ų� N��ŭ ��ġ�ߴٸ� ����
		++num_fetch)
	{
		//fetch_queue[(*idx).end]�� instruction ä���
		if ( ~ (read_instruction(inst_mem, (fetch_queue + (*idx).end) ) ) )
			break;//if is eof, fetch stop;

		(*idx).end = ((*idx).end + 1) % (*idx).length;//����Ŭ�� ���
	}
}*/
//������� �ִٸ�,
//���ְ� ���� read_inst �Լ��� �̿��ؼ� ���� instruction��
//FQ���·� ��ȯ�ؼ� ����.



// fetch queue�� RS, ROB, RT�� �����ͷ� �޾Ƽ�
// n_fq��n_rs, n_rob�� ����Ͽ�
// ������ ���� ��ŭ �ν�Ʈ������ fetch_queue���� ������
// RS�� ROB�� �־��ش�
// (RS�� ���� ��, operand �� �� �ϳ��� Q�� time�� -1�� ������ ���´�)
// delta_n_fq -= �̵��� �ν�Ʈ���� ����; �� ���ش�
// delta_n_rs += �̵��� �ν�Ʈ���� ����; �� ���ش�
// delta_n_rob += �̵��� �ν�Ʈ���� ����; �� ���ش�
// �׸��� output�� �ش��ϴ� RT�� RF_VALID�� �������ش�
void decode();

// RS�� �����ͷ� �޾Ƽ�
// time�� -1�� �͵鿡 ����
// operand ��ΰ� V �����̶��
// time�� -1���� '���࿡ �ɸ��� �ð�' ������ �ٲ��ش�. ( ���� ���� )
void issue();

// RS�� �����ͷ� �޾Ƽ�
// execution�� �Ѵ� (operand�� ���� V �����̸� time �ϳ� ��)
void execution();


// RS�� ROB�� �����ͷ� �޾Ƽ�
// time = 0�� �ν�Ʈ������ RS���� �����ϰ�,
// �� �ν�Ʈ������ ROB���� C ���·� �ٲ۴�
// �׸��� �� �ν�Ʈ������ ���� ROB�� �ڸ� (ex ROB3)
// �� operand�� Q�� ������ �ִ� RS�� �ν�Ʈ��������
// ���� �������ְ� �� operand�� Q���� V�� �ٲ۴�
// delta_n_rs -= ������ �ν�Ʈ���� ����;
void rs_retire();

// ROB�� RAT�� �����ͷ� �޾Ƽ�
// ROB���� C�̰�, �ڱ� ���� �ν�Ʈ������ ��� C�� �ν�Ʈ���ǵ���
// ���ְ� RAT���� RF_VALID�� T�� �ٲپ� �ش�
// delta_n_rob -= ������ �ν�Ʈ���� ����;
void commit(struct RAT* rat , struct ROB* rob, struct Cycle_index&index_rob));

void simul_ooo(struct Config* config)
{
	//
	// Init simulator
	//

	//Init report data
	struct Status {
		int cycle;//num of cycle
		struct
		{
			int total;//totalnum
			int intalu;//alu num
			int memread;//read mem
			int memwrite;//write mem
		} inst;//num of instructions
	};

	// Init OoO
	enum Debug { None = 0, ROB = 1, RSROB = 2 } debug = (*config).debug; //set Debug_mode from config
	
	int N = (*config).width;//set fetch = decode = issue width

	struct FQ* fetch_queue = malloc(sizeof(struct FI) * 2 * N);//set fetch_queue
	struct Cycle_index index_fq = {0,0,0,2*N};//fetch_queue�� �ε���

	struct ROB* rob = malloc(sizeof(struct ROB) * ((*config).rob_size));
	struct Cycle_index index_rob = { 0,0,0,(*config).rob_size};//rob�� �ε���

	struct RS*	rs = malloc( sizeof(struct RS) * ((*config).rs_size) );
	int n_rs; // Reservation Station�� �ν�Ʈ������ �� ���� ����ִ��� �˷��ش�
	int delta_n_rs; // n_rs�� ��ȭ�� �ǹ��Ѵ�.  ���������� ������ ����.

	struct RAT	rat[REGISTER_SIZE];


	//
	// Starting Simulation
	//

	do
	{//do one cycle

		//# of elements update
		updatelen(&index_fq);//update len_of_fq
		updatelen(&index_rob);//update len_of_rob

		//in ROS
		commit (rat, rob, &index_rob)

		//in every RS
		for each elem in RS
		{
			if elem is blank and decode_(in_this_cycle < N) ;
				decode;

			else if elem time is - 1;
				issue
				//if have q value, access Ros[q] and if Ros[q].state == c, set timer

			else //timer seted mean it alredy issued
				--(elem.time)//one step forward
				if elem.time<0//ex completed in this step
					Ros(elem.ROS_index).state = C

		}
		//in fetch queue
		for fetch_num_in_this_cycle < min ( N , fetch_que_size - fetch_queue_elem_num)
		{
			fetch(end_of_fetch_queue);
		}


	}
	while( ~is_fetch_eof && (rob_elem_num > 0) )

	free(fetch_queue)
	free(rob.data)
		free(rs
}

