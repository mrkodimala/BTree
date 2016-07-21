#include<stdio.h>
#include<string.h>

const int blocksize = 512;

struct Student{
	int rollno;
	char name[24];
	char clgname[20];
};

struct LeafPage{
	int pagetype;
	int tableid;
	struct Student data[10];
	char unused[24];
};

struct Teacher{
	int staffid;
	char name[24];
	char dept[12];
	char clgname[20];
};

struct TeacherLeafPage{
	int pagetype;
	int tableid;
	struct Teacher data[7];
	char unused[56];
};

struct NonLeafPage{
	int pagetype;
	int keys[63];
	int offsets[64];
};

struct TableSector{
	int tableIds[8];
	int rootNodeIds[8];
};

int convertStringToNum1(char *str)
{
	int sum = 0;
	for (int i = 0; str[i] != 0 && str[i] != ','; i++)
	{
		sum = sum * 10 + (str[i] - '0');
	}
	return sum;
}

void separateValues1(char *studentstring, char *name, char *clgname, int *rollno)
{
	char buffer[30];
	int buffer_offset = 0;
	int student_offset = 0;
	while (studentstring[student_offset] != ',')
	{
		buffer[buffer_offset++] = studentstring[student_offset++];
	}
	buffer[buffer_offset] = '\0';
	*rollno = convertStringToNum1(buffer);
	buffer_offset = 0;
	student_offset++;
	while (studentstring[student_offset] != ',')
	{
		name[buffer_offset++] = studentstring[student_offset++];
	}
	name[buffer_offset] = '\0';
	buffer_offset = 0;
	student_offset++;
	while (studentstring[student_offset] != '\0'&&studentstring[student_offset] != '\n')
	{
		clgname[buffer_offset++] = studentstring[student_offset++];
	}
	clgname[buffer_offset] = '\0';
}


/*void LoadFromTheFileAndSave(char *source, char *destination)
{
	FILE *norfile, *binfile;
	errno_t e = fopen_s(&norfile, source, "r+");
	errno_t e1 = fopen_s(&binfile,"s.bin", "wb+");
	char stringdata[64];
	char name[24];
	char college[24];
	int rollno;
	struct LeafPage l;
	int data_offset = 0;
	struct Student s;
	struct NonLeafPage n;
	int BlocksCount=0;
	int of_offsets=0, key_offset=0;
	if (e1 != 0 || e != 0)
	{
		printf("e1=%d\te2=%d\n", e, e1);
		printf("Error while loading pages");
		return;
	}
	while (!feof(norfile))
	{
		fgets(stringdata, 64, norfile);
		separateValues1(stringdata, name, college, &rollno);
		strcpy_s(s.clgname, college);
		strcpy_s(s.name, name);
		s.rollno = rollno;
		l.data[data_offset++] = s;
		if (data_offset ==10)
		{
			l.pagetype = 1;
			fwrite(&l, sizeof(struct LeafPage), 1, binfile);
			n.offsets[of_offsets++] = BlocksCount*blocksize;
			//printf("n.offset=%d\n\n", n.offsets[0]);
			n.keys[key_offset++] = l.data[9].rollno + 1;
			BlocksCount++;
			data_offset = 0;
		}
	}
	if (of_offsets < 64)
	{
		n.offsets[of_offsets++] = 0;
		n.keys[key_offset++] = 0;
	}
	n.pagetype = 2;
	n.offsets[0] = 0;
	fwrite(&n, sizeof(struct NonLeafPage), 1, binfile);
	_fcloseall();
}*/


void printStudentRecords(char *filename)
{
	FILE *bfile;
	errno_t e = fopen_s(&bfile, filename, "rb");
	struct TableSector t;
	fseek(bfile, 0, SEEK_SET);
	fread(&t, sizeof(struct TableSector), 1, bfile);
	int offset = t.rootNodeIds[0];
	fseek(bfile, offset, SEEK_SET);
	struct NonLeafPage n;
	struct LeafPage l;
	struct Student s;
	fread(&n, sizeof(struct NonLeafPage), 1, bfile);
	for (int i = 0; i < 64; i++)
	{
		if (n.offsets[i] == 0 && i != 0)
			return;
		int offset = n.offsets[i];
		//printf("offset=%d\n", offset);
		fseek(bfile, offset, SEEK_SET);
		fread(&l, sizeof(struct LeafPage), 1, bfile);
		for (int j = 0; j < 10; j++)
		{
			s = l.data[j];
			printf("%d\t%s\t%s\n", s.rollno, s.name, s.clgname);
		}
	}
}


void LoadFromTheFileAndSave(char *source, char *destination)
{
	FILE *norfile, *binfile;
	errno_t e = fopen_s(&norfile, source, "r+");
	errno_t e1 = fopen_s(&binfile, "s.bin", "wb+");
	char stringdata[64];
	char name[24];
	char college[24];
	int rollno;
	struct LeafPage l;
	int data_offset = 0;
	struct Student s;
	struct NonLeafPage n;
	memset(&n, 0, sizeof(struct NonLeafPage));
	struct TableSector t;
	int BlocksCount = 1;
	int tableOffset = 0;
	int of_offsets = 0, key_offset = 0;
	if (e1 != 0 || e != 0)
	{
		printf("e1=%d\te2=%d\n", e, e1);
		printf("Error while loading pages");
		return;
	}
	while (!feof(norfile))
	{
		fgets(stringdata, 64, norfile);
		separateValues1(stringdata, name, college, &rollno);
		strcpy_s(s.clgname, college);
		strcpy_s(s.name, name);
		s.rollno = rollno;
		l.data[data_offset++] = s;
		if (data_offset == 10)
		{
			l.pagetype = 1;
			fseek(binfile, (BlocksCount*blocksize), SEEK_SET);
			l.pagetype = 1;
			l.tableid = 1;
			fwrite(&l, sizeof(struct LeafPage), 1, binfile);
			n.offsets[of_offsets++] = BlocksCount*blocksize;
			BlocksCount++;
			n.keys[key_offset++] = l.data[9].rollno + 1;
			if (of_offsets > 1)
			{
				n.offsets[0] = 512;
				n.pagetype = 2;
				fwrite(&n, sizeof(struct NonLeafPage), 1, binfile);
				//printf(" Non Leaf Block Count=%d\n", BlocksCount);
				BlocksCount++;
			}
			t.tableIds[tableOffset] = 1;
			t.rootNodeIds[tableOffset] = (BlocksCount-1)*blocksize;
			data_offset = 0;
		}
	}
	printf("Blocks Count=%d\n", BlocksCount);
	printf("table id=%d \t table root=%d\n", t.tableIds[0], t.rootNodeIds[0]);
	fseek(binfile, 0, SEEK_SET);
	fwrite(&t, sizeof(struct TableSector), 1, binfile);
	_fcloseall();
}


void separateValuesTeacher(char *studentstring, char *name, char *clgname,char *dept, int *rollno)
{
	char buffer[30];
	int buffer_offset = 0;
	int student_offset = 0;
	while (studentstring[student_offset] != ',')
	{
		buffer[buffer_offset++] = studentstring[student_offset++];
	}
	buffer[buffer_offset] = '\0';
	*rollno = convertStringToNum1(buffer);
	buffer_offset = 0;
	student_offset++;
	while (studentstring[student_offset] != ',')
	{
		name[buffer_offset++] = studentstring[student_offset++];
	}
	name[buffer_offset] = '\0';
	buffer_offset = 0;
	student_offset++;
	while (studentstring[student_offset] != ',')
	{
		clgname[buffer_offset++] = studentstring[student_offset++];
	}
	clgname[buffer_offset] = '\0';
	buffer_offset = 0;
	student_offset++;
	while (studentstring[student_offset] != '\0'&&studentstring[student_offset] != '\n')
	{
		dept[buffer_offset++] = studentstring[student_offset++];
	}
	dept[buffer_offset] = '\0';
}


void LoadTeacherDataFromFile(char *source,char *destination)
{
	FILE *norfile, *binfile;
	errno_t e = fopen_s(&norfile,source, "r+");
	errno_t e1 = fopen_s(&binfile, destination, "r+b");
	char stringdata[64];
	char name[24];
	char dept[12];
	char college[24];
	int staffid;
	struct TeacherLeafPage l;
	memset(&l, 0, sizeof(struct TeacherLeafPage));
	int data_offset = 0;
	struct Teacher s;
	struct NonLeafPage n;
	memset(&n, 0, sizeof(struct NonLeafPage));
	struct TableSector t;
	int BlocksCount = 128;
	int tableOffset = 1;
	int of_offsets = 0, key_offset = 0;
	if (e1 != 0 || e != 0)
	{
		printf("e1=%d\te2=%d\n", e, e1);
		printf("Error while loading pages");
		return;
	}

	fread(&t, sizeof(struct TableSector), 1, binfile);

	fseek(binfile, 0, SEEK_END);
	while (!feof(norfile))
	{
		fgets(stringdata, 64, norfile);
		separateValuesTeacher(stringdata, name, college,dept, &staffid);
		strcpy_s(s.clgname, college);
		strcpy_s(s.name, name);
		strcpy_s(s.dept, dept);
		s.staffid = staffid;
		//printf("%d\t%s\t%s\t%s\n", s.staffid, s.name, s.dept, s.clgname);
		l.data[data_offset++] = s;
		//printf("data offset=%d\n", data_offset);
		if (data_offset == 7)
		{
			fseek(binfile, (BlocksCount*blocksize), SEEK_SET);
			l.pagetype = 1;
			l.tableid = 2;
			fwrite(&l, sizeof(struct TeacherLeafPage), 1, binfile);
			n.offsets[of_offsets++] = BlocksCount*blocksize;
			//printf("\n\noffset=%d\tBlock Count=%d\n\n",of_offsets, n.offsets[of_offsets-1]/512);
			BlocksCount++;
			n.keys[key_offset++] = l.data[6].staffid + 1;
			if (of_offsets > 1)
			{
				n.offsets[0] = 128 * 512;
				n.pagetype = 2;
				fwrite(&n, sizeof(struct NonLeafPage), 1, binfile);
				//printf(" Non Leaf Block Count=%d\n", BlocksCount);
				BlocksCount++;
			}
			t.tableIds[tableOffset] = 1;
			t.rootNodeIds[tableOffset] = (BlocksCount - 1)*blocksize;
			data_offset = 0;
		}
	}
	fseek(binfile, t.rootNodeIds[1], SEEK_SET);
	//printf("\npostion=%d\n", ftell(binfile));
	fwrite(&n, sizeof(struct NonLeafPage), 1, binfile);
	//printf("Blocks Count=%d\n", BlocksCount);
	//printf("table id=%d \t table root=%d\n", t.tableIds[1], t.rootNodeIds[1]);
	fseek(binfile, 0, SEEK_SET);
	fwrite(&t, sizeof(struct TableSector), 1, binfile);
	_fcloseall();
}



void printLeafBlocks(char *filename)
{
	FILE *bfile;
	errno_t e = fopen_s(&bfile, filename, "rb");
	struct TableSector t;
	fseek(bfile, 0, SEEK_SET);
	fread(&t, sizeof(struct TableSector), 1, bfile);
	int offset = t.rootNodeIds[1];
	printf("Table Offset=%d\n", offset);
	fseek(bfile, offset, SEEK_SET);
	struct NonLeafPage n;
	struct TeacherLeafPage tlp;
	fread(&n, sizeof(struct NonLeafPage), 1, bfile);
	for (int i = 0; i < 64; i++)
	{
		offset = n.offsets[i];
		printf("%d\t%d\n", offset, offset / 512);
		fseek(bfile, offset, SEEK_SET);
		fread(&tlp, sizeof(struct TeacherLeafPage), 1, bfile);
		for (int j = 0; j < 7; j++)
		{
			struct Teacher tea = tlp.data[j];
			printf("%d\t%s\t%s\t%s\n", tea.staffid, tea.name, tea.clgname, tea.dept);
		}
	}
	fclose(bfile);
}

void findCollegeName(char *filename,char *clgname,int rootoffset,int rollno)
{
	FILE *file;
	errno_t e = fopen_s(&file, filename, "rb");
	struct NonLeafPage n;
	struct LeafPage l;
	fseek(file, rootoffset, SEEK_SET);
	fread(&n, sizeof(struct NonLeafPage), 1, file);
	for (int i = 0; i < 63; i++)
	{
		if (rollno < n.keys[i])
		{
			int offset = n.offsets[i];
			fseek(file, offset, SEEK_SET);
			fread(&l, sizeof(struct LeafPage), 1, file);
			int flag = 0;
			for (int j = 0; j < 10; j++)
			{
				if (l.data[j].rollno == rollno)
				{
					flag = 1;
					printf("%d\t%s\t%s\n", l.data[j].rollno, l.data[j].name, l.data[j].clgname);
					int k;
					for (k = 0; l.data[j].clgname[k] != '\0'; k++)
					{
						clgname[k] = l.data[j].clgname[k];
					}
					clgname[k] = '\0';
				}
			}
			if (flag == 0)
			{
				printf("No Student found with that roll no");
				clgname[0] = '\0';
			}
			break;
		}
	}
	fclose(file);
}

int countbyCollegenameofTeachers(char *filename, char *clgname, int teachersoffset)
{
	FILE *bfile;
	errno_t e = fopen_s(&bfile, filename, "rb");
	if (e != 0)
	{
		printf("File not found\n");
		return 0;
	}
	fseek(bfile, teachersoffset, SEEK_SET);
	struct NonLeafPage n;
	struct TeacherLeafPage t;
	struct Teacher teacher;
	fread(&n, sizeof(struct NonLeafPage), 1, bfile);
	int count = 0;
	for (int i = 0; i < 64; i++)
	{
		//printf("n=%d\n", n.offsets[i]);
		int offset = n.offsets[i];
		fseek(bfile, offset, SEEK_SET);
		fread(&t, sizeof(struct TeacherLeafPage), 1, bfile);
		for (int j = 0; j < 7; j++)
		{
			teacher = t.data[j];
			//printf("%d\t%s\t%s\t%s\n", teacher.staffid, teacher.name, teacher.dept, teacher.clgname);
			int k = _strcmpi(teacher.clgname, clgname);
			if (k == 0)
			{
				count++;
			}
		}
	}
	return count;
}


void performquery1(char *filename)
{
	FILE *bfile;
	errno_t e = fopen_s(&bfile, filename, "rb");
	struct TableSector t;
	fread(&t, sizeof(struct TableSector), 1, bfile);
	fclose(bfile);
	int studentoffset = t.rootNodeIds[0];
	int teacheroffset = t.rootNodeIds[1];
	int rollno=0;
	printf("Enter Student Roll No\n");
	scanf_s("%d", &rollno);
	char clgname[30];
	findCollegeName(filename, clgname, studentoffset, rollno);
	if (clgname[0] != '\0')
	{
		int count=countbyCollegenameofTeachers(filename, clgname, teacheroffset);
		printf("\n\nCollegename=%s\nCount of teachers=%d", clgname,count);
	}
	getchar();
}



int main()
{
	char filename1[30] = "data640.txt";
	char filename2[30] = "s.bin";
	LoadFromTheFileAndSave(filename1,filename2);
	char filename3[30] = "teachers448.txt";
	LoadTeacherDataFromFile(filename3, filename2);
	performquery1(filename2);
	getchar();
	return 0;
}
