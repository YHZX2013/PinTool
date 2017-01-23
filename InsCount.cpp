#include "pin.H"
#include <iostream>
#include <string>
#include <map>

using namespace std;

namespace WINDOWS
{
#include <windows.h>
}

map<string, unsigned int> g_map_inscode;
unsigned int g_u_inscount = 0;
FILE *g_log_file_ptr = NULL;

INT32 Usage()
{
    cerr << "This tool prints out the number of dynamically executed " << endl <<
            "instructions, basic blocks and threads in the application." << endl << endl;

    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}


VOID Fini(INT32 code, VOID *v)
{
	if (g_log_file_ptr == NULL)
	{
		puts("File write error");
		return;
	}

	fprintf(g_log_file_ptr, "%u\n", g_u_inscount);
	map<string, unsigned int>::iterator it;

	for (it = g_map_inscode.begin(); it != g_map_inscode.end(); it++)
	{
		fprintf(g_log_file_ptr, "%s:%u\n", it->first.c_str(), it->second);
	}

	fclose(g_log_file_ptr);
	g_log_file_ptr = NULL;

	puts("Fini!");
}

VOID InsCount(char *ins)
{
	g_map_inscode[ins] ++;
	g_u_inscount ++;
}

VOID Instruction(INS ins, VOID *v)
{
	IMG img = IMG_FindByAddress(INS_Address(ins));
	if (IMG_Valid(img) && IMG_IsMainExecutable(img))
	{
		string ins_dis = INS_Disassemble(ins);
		char *ins_tmp = (char*)malloc((ins_dis.length() + 1) * sizeof(char));

		sscanf(ins_dis.c_str(), "%s", ins_tmp);

		INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsCount, IARG_PTR, ins_tmp, IARG_END); 
	}
}

int main(int argc, char *argv[])
{
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }

    string work_dir = argv[argc-1];
    int index = work_dir.rfind("\\");

    if (index == string::npos)
    {
	printf("work_dir error! Use %s\n", work_dir);
	return -1;
    }

    work_dir.assign(work_dir, 0, index);

    WINDOWS::SetCurrentDirectory(work_dir.c_str());
 
    g_log_file_ptr = fopen("inslog.txt", "w");
	
    if (g_log_file_ptr == NULL)
    {
	puts("Error open file!");
	return -1;
    }
	
    INS_AddInstrumentFunction(Instruction, NULL);
 
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();
    
    return 0;
}


