
struct Cycle_index
{
	int head;//������ (�� ó�� ����)
	int tail;// ���� (�� �� ���� + 1)
	int len;// ���� ����
	int size;// ��� ũ��
	int is_reversed;
};

inline void updatelen(struct Cycle_index* cidx)
{
	if ((*cidx).is_reversed)
		(*cidx).len = (*cidx).size - ((*cidx).head - (*cidx).tail);
	else
		(*cidx).len = (*cidx).tail - (*cidx).head;
}

inline void foward_head