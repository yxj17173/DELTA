#include <map>
#include <string>
#include <cstring>
#include <iostream>
using namespace std;

void printHelpInfo(bool fullOption);
char * str2charx(string str);

struct HSAargs {
	char *treeS;
	char *treeT;
	char *cost;
	char method;
	int max_target;
	int test;
	char *outfile;
	bool all;
	int prune;//20180313
};

void printHSAargs(HSAargs argS) {
	cout << "parameters in HSAargs:\n";
	cout << "treeS " << argS.treeS << endl;
	cout << "treeT " << argS.treeT << endl;
	cout << "cost " << argS.cost << endl;
	cout << "method " << argS.method << endl;
	cout << "max_target " << argS.max_target << endl;
	cout << "test " << argS.test << endl;
	cout << "outfile " << argS.outfile << endl;
	cout << "all " << argS.all << endl;
	cout << "prune " << argS.prune << endl;//20180313
}


void printMap(map<string, string> amap) {
	for (map<string, string>::iterator it = amap.begin(); it != amap.end(); it++) {
		cout << it->first << " " << it->second << "\n";
	}
}

class ArgProcessor {
public:
	map<string, string> argVal;
	HSAargs argS;
	ArgProcessor(int argc, char* argv[]);


};

ArgProcessor::ArgProcessor(int argc, char* argv[]) {
	//default setting for optional parameters
	//cout << "build parameters\n";
	argVal["max_target"] = "1";
	argVal["test"] = "0";
	argVal["outfile"] = "";
	argVal["all"] = "F";
	argVal["prune"] = "1";

	//required options
	argVal["treeS"] = "";
	argVal["treeT"] = "";
	argVal["cost"] = "";
	argVal["method"] = "g";

	char *arg;

	if (argc % 2 != 1) {//the number of input is not right
		cerr << "wrong syntax. \n\n";
		printHelpInfo(true);
		exit(-2);
	}
	for (int i = 1; i < argc; i = i + 2) {
		arg = argv[i];
		if (arg[0] != '-') {//not begin with '-'
			cerr << "wrong syntax. \n\n";
			printHelpInfo(true);
			exit(-4);
		}
		string argStr(&arg[1]);
		if (argVal.find(argStr) == argVal.end()) {//include "-" undefined parameters
			cerr << "wrong syntax. \n\n";
			printHelpInfo(true);
			exit(-6);
		}
		else {
			string argStrV(argv[i + 1]);
			argVal[argStr] = argStrV;
		}
	}

	if (argVal["outfile"] == "") argVal["outfile"] = argVal["treeS"] + argVal["method"];//set un-defined outfile 

	//fill in argS
	argS.treeS = str2charx(argVal["treeS"]);
	argS.treeT = str2charx(argVal["treeT"]);
	argS.cost = str2charx(argVal["cost"]);
	argS.method = *str2charx(argVal["method"]);
	argS.max_target = stoi(argVal["max_target"]);
	argS.test = stoi(argVal["test"]);
	argS.outfile = str2charx(argVal["outfile"]);
	argS.all = argVal["all"] == "T" ? true : false;
	argS.prune = stoi(argVal["prune"]);

	//printMap(argVal);
	//printHSAargs(argS);
}

void printHelpInfo(bool fullOption) {
	using namespace std;
	cout << "There are two ways to use this program.\n";
	cout << "Method 1: the simple syntax. One of the following scripts.\n";
	cout << "  HSA <TreeS file path> <TreeT file path> <Cost file path> <g>\n";
	cout << "  HSA <TreeS file path> <TreeT file path> <Cost file path> <l> <n>\n";
	cout << "  HSA <TreeS file path> <TreeT file path> <Cost file path> <g> <testNum>\n";
	cout << "  HSA <TreeS file path> <TreeT file path> <Cost file path> <l> <n> <testNum>\n";
	cout << "  g: global alignment\n";
	cout << "  l: local alignment\n";
	cout << "  n: number of local alignments to output\n";
	cout << "  testNum: number of test for calculate p-value (default:100, max:10000,min:2)\n" << endl;

	cout << "Method 2: the verbose way.\n";
	cout << "  HSA [-h] [--help]" << endl;
	cout << "  -h: print short help and description" << endl;
	cout << "  --help: print detailed help and description" << endl;
	cout << "  Usage: " << endl;
	cout << "  HSA [-treeS <TreeS file path>] [-treeT <TreeT file path>] " << endl;
	cout << "      [-cost <Cost file path>] [-method <l or g>] [-max_target <target num for l>] " << endl;
	cout << "      [-test testNum] [-outfile <output file path>] [-all <T or F>] [-prune pruneScore]" << endl;
	cout << "  required options: -treeS, -treeT, -cost, -method" << endl;

	if (fullOption) {
		cout << "\n  description:" << endl;
		cout << "  required options: " << endl;
		cout << "    -treeS: TreeS file path" << endl;
		cout << "    -treeT: TreeT file path" << endl;
		cout << "    -cost: Cost file path. the cost file contains the score for different types of leaves" << endl;

		cout << "  optional parameters:" << endl;
		cout << "    -method: l or g. g for global alignment; l for local alignment. default: g." << endl;
		cout << "    -max_target: target num for l, local alignment. default: 1" << endl;
		cout << "    -test: testNum to calculate p-value. If testNum <=2, do not output p-value. default 0" << endl;
		cout << "    -outfile: output file path. default: TreeS file path + l or g, based on -method" << endl;
		cout << "    -all: T or F. output as much information as possible. default F;" << endl;
		cout << "    -prune: pruneScore is the punish for pruning one leaf. default 1" << endl;//20180313
	}

}

char * str2charx(string str) {
	char * cstr = new char[str.length() + 1];
	strcpy(cstr,  str.c_str());//strcpy_s(cstr, str.length() + 1, str.c_str());
	return cstr;
}
