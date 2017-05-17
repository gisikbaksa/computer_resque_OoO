// Computer Architecture Homework #2
// Noh Dong Ju & Park Kyung Won & Yu Jin woo

#include <stdio.h>
#include <stdlib.h>

#include "data_structures.h"
#include "file_read.h"
#include "simulator.h"

//#include "read.h" //read config and instruction
//#include "simul.h"//func related simulation

//typedef struct Config { int justtest;  } Config;

void disp_title(void);
void disp_taskdata(struct CONFIG* configs, int inst_idx, int conf_idx, char** inst_filename);
void disp_end(void);
void disp_error(void);
int get_filepath_from_arg(int argc, char** argv, char*** conf_path, int* conf_len, char*** inst_path, int* inst_len, char*** inst_filename);

int main(int argc, char* argv[])
{
	//
	// Print intro message
	//
	disp_title();

	// 
	// Read config and Data file and make config array
	//
	printf("\n");
	printf("Read Config and Instruction files : \n");
	

	char** conf_name=0;  int conf_len=0;//name = ���ϰ��, len= �� ����,
	char** inst_name=0;  int inst_len=0; char** inst_filename=0;

	if (get_filepath_from_arg(argc, argv, &conf_name, &conf_len, &inst_name, &inst_len, &inst_filename) != 0)
	{//���� �޸� �����̳� ��Ÿ ������ ���� ������ �ȵǸ�,
		disp_error();
		return 1;
	}

	printf("Total %3d Config and %3d Instruction founded\n", conf_len, inst_len);
	//���� ������ ������ ��� �ľ� ��

	if (conf_len == 0 || inst_len == 0)
	{//���� ������ ���� ����� �־����� �ʾҴٸ� ���ĸ� ������ �� �����Ƿ� ���� �����Ѵ�.
		printf("Too few files. Need .conf and .inst both.\n");
		disp_end();
		return 0;
	}


	//
	// simulate and for every config combination
	//


	//config ������ �о� ����
	struct CONFIG* configs;
	configs = malloc(sizeof(struct CONFIG)*(conf_len));

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

	//inst ������ �ʿ��� �� �Ѱ��� �о� ����(�뷮�� ũ�Ƿ�)
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

			//���� ���
			disp_taskdata(configs, inst_idx, conf_idx, inst_filename);

			//�ùķ��̼�
			struct REPORT* report;
			report = core_simulator(configs + conf_idx, inst_arr, inst_arr_len);//simulate

			//���� ���
			char out_filename[256];
			sprintf(out_filename, "%d_%d_%d_%d_%s_report.out",
					configs[conf_idx].Dump, configs[conf_idx].Width, configs[conf_idx].ROB_size, configs[conf_idx].RS_size, inst_filename[inst_idx]);
			FILE* f_report = fopen(out_filename, "w");
			
			if (f_report == NULL)
			{
				printf("Error : Lack of memory\n");
				disp_error();
				return 1;
			}
			REPORT_fprinter(report, f_report);

			fclose(f_report);			
			free(report);

			printf("out : %s", out_filename);
			printf("\n\n");
		}

		free(inst_arr);

	}

	free(configs);// free config
	free(conf_name);
	free(inst_name);
	free(inst_filename);

	disp_end();

	return 0;
}

void disp_title(void)
{
	printf("+------------------------------------------------------------------------------+\n");
	printf("                     Computer Architecture Homework #2\n");
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
}

void disp_taskdata(struct CONFIG* configs, int inst_idx, int conf_idx, char** inst_filename)
{
	printf("Task %d in total %d\n", inst_idx + 1, (inst_idx + 1)*(conf_idx + 1));
	printf("inst_%d : %s\n", inst_idx + 1, inst_filename[inst_idx]);
	printf("conf_%d : ", conf_idx + 1);
	Config_printer(configs + conf_idx);
}

void disp_end(void)
{
	printf("Program done - Press any key to quit");
	getchar();
}

void disp_error(void)
{
	printf("Program error - Press any key to quit");
	getchar();
}

int get_filepath_from_arg(int argc, char** argv,char*** conf_path, int* conf_len, char*** inst_path, int* inst_len, char*** inst_filename)
{

	(*conf_path) = malloc(sizeof(char*)*argc);
	(*inst_path) = malloc(sizeof(char*)*argc);//���� �������� �׷��� ũ���� ����
	(*inst_filename) = malloc(sizeof(char*)*argc);
	if ((*conf_path) == NULL ||
		(*inst_path) == NULL ||
		(*inst_filename) == NULL)
	{
		printf("Lack of memory!\n");
		return 1;
	}
	(*conf_len) = 0;
	(*inst_len) = 0;

	if (argc == 1)
	{//if there is no input, set basic value;

		(*conf_len) = 1;
		(*conf_path)[0] = "config.conf";

		(*inst_len) = 1;
		(*inst_path)[0] = "instruction.inst";
		(*inst_filename)[0] = "instruction.inst";

	}
	else
	{//if there is input from argv,  ��� �Է� �μ��� �о� .conf�� conf�� .inst�� inst��, �������� ����.

		char* file_name = 0;//Ȯ���ڳ� ���� ���� ����� �ӽ� ������

		for (int idx = 1; idx < argc; ++idx)//every config_file 
		{
			//printf("%s\n", argv[idx]);

			file_name = argv[idx] + (strlen(argv[idx]) - 5); // �� �� 5�ڸ��� �����´�. �� �� .conf���� .inst���� �Ѵ� �ƴ����� Ȯ���Ѵ�. 
			
			if (0 == strcmp(file_name, ".conf"))//if conf file
			{
				(*conf_path)[*conf_len] = argv[idx];
				++(*conf_len);
			}
			else if (0 == strcmp(file_name, ".inst"))
			{
				
				(*inst_path)[*inst_len] = argv[idx];
				(*inst_filename)[*inst_len] = get_filename(argv[idx]);
				//printf("%s", inst_filename[inst_len]);
				++(*inst_len);
			}
			else
			{
				printf("File %s is nethier .conf and .inst! \n", argv[idx]);
			}
		}

	}
	return 0;
}