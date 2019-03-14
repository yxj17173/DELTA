#include <cstdio>
#include <iostream>
#include "HSA.h"
#include "argProcessor.h"
//#define DEBUG
//#define GLOBAL
//HSA::HSA hsa;

int Setting::PRUNE_CONSTANT;


int main(int argc, char** argv) {
	if (argc < 4) {//not enough input argments. print long help info
		printHelpInfo(true);
		return (-1);
	}
	else {
		if (argv[1][0] != '-') { //simple syntax. the same as previous
			HSA::HSA *hsa = new HSA::HSA();

#ifdef DEBUG
			if (hsa->BuildCost("C:\\Users\\ATPs\\Documents\\GitHub\\VS\\HSA\\x64\\Release\\cost2.tsv") && hsa->BuildTreeTS("C:\\Users\\ATPs\\Documents\\GitHub\\VS\\HSA\\x64\\Release\\fun.alm", "C:\\Users\\ATPs\\Documents\\GitHub\\VS\\HSA\\x64\\Release\\fun.alm")) {
#ifdef GLOBAL
				hsa->outputGResult("C:\\Users\\ATPs\\Documents\\GitHub\\VS\\HSA\\x64\\Release\\fun.almg");
#else
				std::vector<std::tuple<std::string, std::string>> subTreeRootsTS;
				subTreeRootsTS = hsa->outputLResult("C:\\Users\\ATPs\\Documents\\GitHub\\VS\\HSA\\x64\\Release\\fun.alm", 4);
				delete hsa;
				kOutputAndGeneratePValue(subTreeRootsTS, "C:\\Users\\ATPs\\Documents\\GitHub\\VS\\HSA\\x64\\Release\\cost2.tsv", "C:\\Users\\ATPs\\Documents\\GitHub\\VS\\HSA\\x64\\Release\\fun.alm", "C:\\Users\\ATPs\\Documents\\GitHub\\VS\\HSA\\x64\\Release\\fun.alm");

#endif // GLOBAL

			}
#else
			if (argc > 4) {
				int testNum = 100;
				if (hsa->BuildCost(argv[3]) && hsa->BuildTreeTS(argv[1], argv[2])) {
					if (*(argv[4]) == Setting::GlobalC) {
						if (argc > 5) {
							sscanf(argv[5], "%ut", &testNum);
							hsa->TEST_CASE = testNum;
						}
						hsa->outputGResult(argv[1]);
					}
					else {
						if (argc > 6) {
							sscanf(argv[6], "%ut", &testNum);
							hsa->TEST_CASE = testNum;
						}
						int num = 1;
						sscanf(argv[5], "%d", &num);
						std::vector<std::tuple<std::string, std::string>> subTreeRootsTS;
						subTreeRootsTS = hsa->outputLResult(argv[1], num);//vector of subTreeRoots ids
						kOutputAndGeneratePValue(subTreeRootsTS, argv[3], argv[1], argv[2], testNum);
					}

				}
			}
			else {//print help information
				printHelpInfo(true);
			}
#endif // !DEBUG
			}
		else {//verbose syntax
			HSA::HSA *hsa2 = new HSA::HSA();
			ArgProcessor args(argc, argv);
			hsa2->TEST_CASE = args.argS.test;
			hsa2->OUTPUT_ALL = args.argS.all;
			int testNum = args.argS.test;

			Setting::PRUNE_CONSTANT = args.argS.prune;//20180313
			

			if (hsa2->BuildCost(args.argS.cost) && hsa2->BuildTreeTS(args.argS.treeS, args.argS.treeT)) {//build cost, build TreeTS
				if (args.argS.method == Setting::GlobalC) {//global alignment
					hsa2->outputGResult2(args.argS.outfile);
					return 0;
				}
				if (args.argS.method == Setting::LocalC) {//local alignment
					int num = args.argS.max_target;//number of local alignments to output
					std::vector<std::tuple<std::string, std::string>> subTreeRootsTS;
					subTreeRootsTS = hsa2->outputLResult2(args.argS.outfile, num);//vector of subTreeRoots ids
					if (testNum > 1) {
						kOutputAndGeneratePValue2(subTreeRootsTS, args.argS.cost, args.argS.treeS, args.argS.treeT, testNum,args.argS.outfile);
					}

					return 0;
				}
				std::cerr << "check the method, not g or l";
				return -1;
			}
		}
	}

    return 0;
}
