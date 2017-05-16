// Computer Architecture Homework #2
// Noh Dong Ju & Park Kyung Won & Yu Jin woo

#include <stdio.h>
#include <stdlib.h>

#include "datatypes.h"
#include "fileread.h"

//#include "read.h" //read config and instruction
//#include "simul.h"//func related simulation

//typedef struct Config { int justtest;  } Config;

int main(int argc, char* argv[])
{
	//
	// Print intro message
	//
	printf("+------------------------------------------------------------------------------+\n");
	printf("                     Computer Architecture Homework #2\m");
	printf("                 Noh Dong Ju & Park Kyung Won & Yu Jin woo\n");
	printf("+------------------------------------------------------------------------------+\n");
	printf(" Usage : 1) Just excute this program, \n");
	printf("            then program take 'config.conf' and 'instruction.inst'\n");
	printf("            and simulate OoO. when end simulation, program will print out\n");
	printf("            '[dump]_[width]_[ROS_size]_[RS_size]_output.out'\n");
	printf("\n");
	printf("         2) drag and drop on exe.file or give filepath as argument,\n");
	printf("            then program take every '~.conf' and '~.inst' as input,\n");
	printf("            and simulate all combinaion of these. when end simulation,\n");
	printf("            program will print out\n");
	printf("            '[dump]_[width]_[ROS_size]_[RS_size]_[inst_name]_output.out'\n");
	printf("+------------------------------------------------------------------------------+\n");

	// 
	// Read config and Data file and make config array
	//
	printf("\n");
	printf("Read Config and Instruction files : \n");
	
	//���� �Է� �μ��� ������ �ޱ� ���� �ʿ��� ���� ����
	char** conf_name; int conf_len;//name = ���ϰ��, len= �� ����,
	char** inst_name;  int inst_len; char** inst_filename;
	conf_name = malloc(sizeof(char*)*argc);
	inst_name = malloc(sizeof(char*)*argc);//���� �������� �׷��� ũ���� ����
	inst_filename = malloc(sizeof(char*)*argc);
	conf_len = 0;
	inst_len = 0;

	if (argc == 1)
	{//if there is no input, set basic value;
		
		conf_len = 1;
		conf_name[0] = "config.conf";

		inst_len = 1;
		inst_name[0] = "instruction.inst";
		inst_filename[0] = "instruction.inst";
	}
	else
	{//if there is input from argv,  ��� �Է� �μ��� �о� .conf�� conf�� .inst�� inst��, �������� ����.

		char* file_name = 0;//Ȯ���ڳ� ���� ���� ����� 
		for (int idx = 1; idx < argc; ++idx)//every config_file 
		{
			file_name = argv[idx] + (strlen(argv[idx]) - 5); // �� �� 5�ڸ��� �����´�. �� �� .conf���� .inst���� �Ѵ� �ƴ����� Ȯ���Ѵ�. 
			//printf("%s", file_name);

			if (0 == strcmp(file_name, ".conf"))//if conf file
			{
				//printf("is conf", file_name);
				conf_name[conf_len] = argv[idx];
				++conf_len;
			}
			else if (0 == strcmp(file_name, ".inst"))
			{
				//printf("is inst", file_name);
				inst_name[inst_len] = argv[idx];
				inst_filename[inst_len] = get_filename(argv[idx]);
				//printf("%s", inst_filename[inst_len]);
				++inst_len;
			}
			else
			{
				printf("File  %s is nethier .conf and .inst! \n", argv[idx]);
			}
		}
	}
	printf("Total %3d Config and %3d Instruction finded\n", conf_len, inst_len);
	//���� ������ ������ ��� �ľ� ��

	//
	// simulate and for every config combination
	//


	//config ������ �о� ����
	struct Config* configs;
	configs = malloc(sizeof(struct Config)*(conf_len));
	
	int for_ignore = 0;
	for (int idx = 0; idx < conf_len; ++idx)
	{
		if (!config_reader(conf_name[idx], configs + idx - for_ignore))
		{
			printf("Cannot find file %s\n", conf_name[idx]); 
			++for_ignore;
		}
	}
	conf_len -= for_ignore;

	//inst ������ �ʿ��� �� �Ѱ��� �о� ����
	struct INST* inst_arr; int inst_arr_len;

	for (int inst_idx = 0; inst_idx < inst_len; ++inst_idx)
	{//��� �ν�Ʈ���ǿ� ���� 
		if (!make_inst_array(inst_name[inst_idx], &inst_arr, &inst_arr_len)) 
		{
			printf("Error dectection - ignore this file %s\n", inst_filename[inst_idx]);
			continue;
			//return 1;
		}
		//printf("inst_%d : %s\n", inst_idx + 1, inst_filename[inst_idx]);

		for (int conf_idx = 0; conf_idx < conf_len; ++conf_idx)
		{//��� config�� ����

			printf("Task %d in total %d\n", inst_idx + 1, (inst_idx+1)*(conf_idx+1));
			printf("inst_%d : %s\n", inst_idx + 1, inst_filename[inst_idx]);
			printf("conf_%d : ", conf_idx+1 );
			Config_printer(configs + conf_idx);
			//simul_ooo(configs + idx_conf);//simulate
		}

		free(inst_arr);

	}

	free(configs);// free config
	free(conf_name);
	free(inst_name);
	free(inst_filename);

	printf("Program done - Press any key to quit");
	getchar();

	return 0;
}

