#ifndef __HSA_H__
#define __HSA_H__

#include <cstring>
#include <string>
#include <map>
#include <iostream>
#include <cstdlib>
#include <queue>
#include <vector>
#include <math.h>
#include <fstream>
#include "SelectCase.h"
#include "NodeClass.h"
#include "BFTree.h"
#include "RandomTree.h"
#include "Setting.h"

double kCalculatePValue(double *numbers, int size, double testValue);
void kOutputAndGeneratePValueOne(std::string subRootS, std::string subRootT, char* pathCost, char* pathTreeS, char* pathTreeT, FILE* fp, int testNum);

namespace HSA {
    using TN::TreeNode;
    using TN::BFTree;
    using Setting::SCORE_MAX_G;
    using Setting::SCORE_MIN_G;
    using Setting::SCORE_MAX_L;
    using Setting::SCORE_MIN_L;

    const unsigned int LINE_MAX_C = 16383U;//20180126 change from 4096U to 16383U

    typedef Setting::CostType CostType;
    typedef Setting::ScoreType ScoreType;
    typedef SelectCase::BtType BtType;
    typedef NodeClass::Value NodeClassValue;
    typedef std::pair<size_t, size_t> Point;


	class HSA {
	//public:
		CostType costMatrix[Setting::CLASS_MAX][Setting::CLASS_MAX];
		BtType btMatrix[Setting::NODE_MAX][Setting::NODE_MAX];
		ScoreType scoreMatrix[Setting::NODE_MAX][Setting::NODE_MAX];
		NodeClass nodeClass;
		BFTree treeT;
		BFTree treeS;
		BFTree subTreeS, subTreeT;

		public:
			size_t TEST_CASE = 100U;
			bool OUTPUT_ALL = false;

		inline bool BuildCost(char* path) {
			char buf[LINE_MAX_C];
			char aClass[LINE_MAX_C], bClass[LINE_MAX_C];
			ScoreType costScore;
			FILE* fp = fopen(path, "r");
			while (!feof(fp)) {
				if (fgets(buf, LINE_MAX_C, fp) &&
					sscanf(buf, "%s %s %d", aClass, bClass, &costScore) != 3) {
					fprintf(stderr, "Cost File Format Worry: %s\n", buf);
					continue;
				}
#ifdef DEBUG_Cost
				if (costScore < -128 || costScore > 127) {
					fprintf(stderr, "Cost File costScore Worry: %d\n", costScore);
					fclose(fp);
					return false;
				}
#endif // DEBUG_Cost
				NodeClassValue row = nodeClass.insert(std::string(aClass));
				NodeClassValue col = nodeClass.insert(std::string(bClass));
				costMatrix[col][row] = costMatrix[row][col] = costScore;
			}
			fclose(fp);

			/*std::cout << "\n costMatrix built is \n";
			for (int i = 0; i < 9; i++) {
				for (int j = 0; j < 9; j++) {
					std::cout << +costMatrix[i][j] << " ";
				}
				std::cout << "\n";
			}*/

			return true;
		}

		inline int BuildTree(char* path, BFTree& tree) {
#ifdef DEBUG_Tree
			//int insertNum = 0;
#endif // DEBUG_Tree
			char buf[LINE_MAX_C];
			char lineage[LINE_MAX_C], name[LINE_MAX_C], _class[LINE_MAX_C];
			FILE* fp = fopen(path, "r");
			fgets(buf, LINE_MAX_C, fp);
			int n = 0;
			while (!feof(fp)) {
				fgets(buf, LINE_MAX_C, fp);
				std::string bufstr(buf);
				//if (bufstr.length() < 3) break;
				if (sscanf(buf, "%s %s %s", lineage, name, _class) != 3) {
					fprintf(stderr, "Tree File Format Worry: %s\n", buf);
					/*fclose(fp);
					return -1;*/ //changed so it can handle the last newline symbol in file.
					continue;
				}
#ifdef DEBUG_Tree
				++insertNum;
#endif // DEBUG_Tree
				tree.insert(lineage, name, nodeClass.insert(std::string(_class)));
				//std::cout << "test" << " " << n++<< std::string(_class) <<"\n";
			}
			fclose(fp);
			tree.countAndSort();
#ifdef DEBUG_Tree
			if ((insertNum << 1) != (tree._size + 1)) {
				fprintf(stderr, "Tree File Worry: Not a BFTree.\n");
				return -1;
			}
			tree.print();
#endif // DEBUG_Tree
			return 0;
		}

		inline bool BuildTreeTS(char* pathS, char* pathT) {
			return BuildTree(pathS, treeS) == 0 && BuildTree(pathT, treeT) == 0;
		}

		inline void DP(void) {
			size_t m = (treeS._size + 1) >> 1, n = (treeT._size + 1) >> 1;
			TreeNode **pi = _row_, **pj = _col_;
			for (size_t i = 0; i < m; ++i, ++pi, pj = _col_)
				for (size_t j = 0; j < n; ++j, ++pj) {
					btMatrix[i][j] = SelectCase::NONE;
					scoreMatrix[i][j] = costMatrix[(*pi)->nodeClass][(*pj)->nodeClass];
				}
			pi = _row_; pj = _col_ + n;
			for (size_t i = 0; i < m; ++i, ++pi, pj = _col_ + n)
				for (size_t j = n; j < treeT._size; ++j, ++pj) {
					ScoreType lScore = _LSScore_;
					ScoreType rScore = _RSScore_;
					if (lScore > rScore) {
						btMatrix[i][j] = SelectCase::LS;
						scoreMatrix[i][j] = lScore;
					}
					else {
						btMatrix[i][j] = SelectCase::RS;
						scoreMatrix[i][j] = rScore;
					}
				}
			pi = _row_ + m; pj = _col_;
			for (size_t i = m; i < treeS._size; ++i, ++pi, pj = _col_)
				for (size_t j = 0; j < n; ++j, ++pj) {
					ScoreType lScore = _SLScore_;
					ScoreType rScore = _SRScore_;
					if (lScore > rScore) {
						btMatrix[i][j] = SelectCase::SL;
						scoreMatrix[i][j] = lScore;
					}
					else {
						btMatrix[i][j] = SelectCase::SR;
						scoreMatrix[i][j] = rScore;
					}
				}
			pi = _row_ + m; pj = _col_ + n;
			for (size_t i = m; i < treeS._size; ++i, ++pi, pj = _col_ + n)
				for (size_t j = n; j < treeT._size; ++j, ++pj) {
					BtType selectCase = SelectCase::NONE;
					ScoreType maxScore = SCORE_MIN_G;
					ScoreType score = _LSScore_;
					_Select_Max_(SelectCase::LS)
						score = _RSScore_;
					_Select_Max_(SelectCase::RS)
						score = _SLScore_;
					_Select_Max_(SelectCase::SL)
						score = _SRScore_;
					_Select_Max_(SelectCase::SR)
						score = _LLScore_;
					_Select_Max_(SelectCase::LL)
						score = _LRScore_;
					_Select_Max_(SelectCase::LR)
                    btMatrix[i][j] = selectCase;
					scoreMatrix[i][j] = maxScore;
				}
#ifdef DEBUG_Score
			static int db_score_time = 0;
			if (db_score_time++ > 0)
				return;
			for (size_t i = 0; i < treeS._size; ++i) {
				for (size_t j = 0; j < treeT._size; ++j) {
					printf("%d ", scoreMatrix[i][j]);
				}
				printf("\n");
			}
			printf("\n");
#endif // DEBUG_Score
		}

		inline ScoreType getMaxScore(void) {
			ScoreType maxScore = SCORE_MIN_G;
			for (size_t i = 0; i < treeS._size; ++i)
				for (size_t j = 0; j < treeT._size; ++j)
					if (scoreMatrix[i][j] > maxScore)
						maxScore = scoreMatrix[i][j];
			return maxScore;
		}

		inline void outputPValue(FILE* fp, ScoreType& maxScore) {
			fprintf(fp, "PValue:");
			double randomeTreeScores[Setting::MAX_TEST];//byK; Store the calculated scores
			ScoreType min = SCORE_MAX_G, max = SCORE_MIN_G, avg = 0;
			fprintf(fp, "DELTA score between real trees:%d\n",maxScore);
			fprintf(fp, "DELTA score between random trees:");
			for (size_t i = 0; i < TEST_CASE; ++i) {
				TN::RandomTree tree(&treeT, nodeClass.size());
				TN::RandomTree tree2(&treeS, nodeClass.size());
				tree.BuildTree();
				tree2.BuildTree();
				DP();
				ScoreType tmp = getMaxScore();
				if (tmp < min)
					min = tmp;
				if (tmp > max)
					max = tmp;
				avg += tmp;
				fprintf(fp, "%d ", tmp);
				randomeTreeScores[i] = tmp;//byK; Store the calculated scores
			}

			fprintf(fp, "\nMin:%d Max:%d AVG:%d ", min, max, avg / TEST_CASE);
			//byK; calculate p-value
			if (avg / TEST_CASE > maxScore) {
				fprintf(fp, "SomethingWrong!\n");
			}
			else {
				double pvalue;
				pvalue = kCalculatePValue(randomeTreeScores, TEST_CASE, maxScore);
				fprintf(fp, "P-value:%.5g \n", pvalue);
			}

		}


		inline void outputPValueLLL(FILE* fp, ScoreType& maxScore) {
			double randomeTreeScores[Setting::MAX_TEST];//byK; Store the calculated scores
			ScoreType min = SCORE_MAX_G, max = SCORE_MIN_G, avg = 0;
			fprintf(fp, "DELTA score between real trees:%d\n",maxScore);
			fprintf(fp, "DELTA score between random trees:");
			for (size_t i = 0; i < TEST_CASE; ++i) {
				TN::RandomTree tree(&treeT, nodeClass.size());
				TN::RandomTree tree2(&treeS, nodeClass.size());
				tree.BuildTree();
				tree2.BuildTree();
				DPL();
				ScoreType tmp = getMaxScore();
				if (tmp < min)
					min = tmp;
				if (tmp > max)
					max = tmp;
				avg += tmp;
				fprintf(fp, "%d ", tmp);
				randomeTreeScores[i] = tmp;//byK; Store the calculated scores
			}

			fprintf(fp,"\nMin:%d Max:%d AVG:%d ", min, max, avg / TEST_CASE);
			//byK; calculate p-value
			double pvalue;
			pvalue = kCalculatePValue(randomeTreeScores, TEST_CASE, maxScore);
			fprintf(fp, "P-value:%.5g \n", pvalue);


		}

		inline void getSubTree(std::string rootS, std::string rootT) {
			BFTree subTreeS, subTreeT;
			int sizeS = 0, sizeT = 0;
			TreeNode *subRankToNodeS[LINE_MAX_C], *subRankToNodeT[LINE_MAX_C];
			treeS.getRankToNodeWithID(subTreeS.rankToNode, rootS, &sizeS);
			treeT.getRankToNodeWithID(subTreeT.rankToNode, rootT, &sizeT);
			subTreeS._size = sizeS;
			subTreeT._size = sizeT;
			treeS = subTreeS;
			treeT = subTreeT;
			treeS.sort();
			treeT.sort();
			//std::ofstream myfile3;
			//myfile3.open("C:\\Users\\ATPs\\Documents\\GitHub\\VS\\HSA\\x64\\Release\\subTreeS.txt");
			//for (int i = 0; i < sizeS; i++) {
			//	std::cout << treeS.rankToNode[i]->id <<" ";
			//	//myfile3 << subRankToNodeS[i]->id <<"\n";
			//}
			//myfile3.close();
		}



        inline void outputMatch(FILE* & fp, size_t & row, size_t & col, ScoreType& maxScore) {
            fprintf(fp, "Score:%d\nRootS:%s\nRootT:%s\n", maxScore,
                ((treeS.rankToNode[row])->id).c_str(),
                ((treeT.rankToNode[col])->id).c_str()
            );
            /*static_assert((sizeof(btMatrix[0]) == sizeof(btMatrix[0][0]) * Setting::NODE_MAX),
            "btMatrix Col NUM is not NODE_MAX");
            BtType* matrix = btMatrix[0];
            row = (row << Setting::NODE_MAX_OFFSET) | col;*/
            std::string pruneS, pruneT, matchS, matchT;
            std::queue<Point> que;
            do {
                switch (btMatrix[row][col]) {
                case SelectCase::NONE:
                    break;
                case SelectCase::LL: {
                    TreeNode* pi = _row_[row];
                    TreeNode* pj = _col_[col];
                    matchS += pi->left->id + " " + pi->right->id + " ";
                    matchT += pj->left->id + " " + pj->right->id + " ";
                    que.push(Point(pi->left->rank, pj->left->rank));
                    que.push(Point(pi->right->rank, pj->right->rank));
                    break;
                }
                case SelectCase::LR: {
                    TreeNode* pi = _row_[row];
                    TreeNode* pj = _col_[col];
                    matchS += pi->left->id + " " + pi->right->id + " ";
                    matchT += pj->right->id + " " + pj->left->id + " ";
                    que.push(Point(pi->left->rank, pj->right->rank));
                    que.push(Point(pi->right->rank, pj->left->rank));
                    break;
                }
                case SelectCase::LS: {
                    pruneT += _col_[col]->right->id + " ";
                    que.push(Point(_row_[row]->rank, _col_[col]->left->rank));
                    break;
                }
                case SelectCase::RS: {
                    pruneT += _col_[col]->left->id + " ";
                    que.push(Point(_row_[row]->rank, _col_[col]->right->rank));
                    break;
                }
                case SelectCase::SL: {
                    pruneS += _row_[row]->right->id + " ";
                    que.push(Point(_row_[row]->left->rank, _col_[col]->rank));
                    break;
                }
                case SelectCase::SR: {
                    pruneS += _row_[row]->left->id + " ";
                    que.push(Point(_row_[row]->right->rank, _col_[col]->rank));
                    break;
                }
                default:
                    fprintf(stderr, "btMatrix Worry In outputResult()\n");
                }
                if (que.empty()) {
                    break;
                } else {
                    std::pair<size_t, size_t> tmp = que.front();
                    row = tmp.first;
                    col = tmp.second;
                    que.pop();
                }
            } while (1);
            fprintf(fp, "PruneS:%s\nPruneT:%s\nMatchS:%s\nMatchT:%s\n",
                pruneS.c_str(), pruneT.c_str(), matchS.c_str(), matchT.c_str()
            );
        }

        inline void outputGResult(char* path) {
            DP();
            std::string outFile(path);
            FILE* fp = fopen((outFile + Setting::GlobalC).c_str(), "w");
            if (treeS._size == 0 || treeT._size == 0) {
                fprintf(stderr, "treeS._size == 0 || treeT._size == 0 In outputLResult()\n");
                return;
            }

            size_t row = treeS._size - 1, col = treeT._size - 1;
            ScoreType maxScore = scoreMatrix[row][col];

            outputMatch(fp, row, col, maxScore);
            outputPValue(fp,maxScore);
            fclose(fp);
        }

		inline void outputTree(FILE* fp, BFTree treeUse) {
			fprintf(fp, "id\tname\tleaves\tnodeClass\trank\n");
			for (int i = 0; i < treeUse._size; i++) {
				if (treeUse.rankToNode[i]->leaves < 2) {//leaves
					fprintf(fp, "%s\t%s\t%d\t%d\t%d\n", treeUse.rankToNode[i]->id.c_str(), treeUse.rankToNode[i]->name.c_str(),
						treeUse.rankToNode[i]->leaves, treeUse.rankToNode[i]->nodeClass,
						treeUse.rankToNode[i]->rank);
				}
				else {//internal nodes, no nodeClass
					fprintf(fp, "%s\t%s\t%d\t\t%d\n", treeUse.rankToNode[i]->id.c_str(), treeUse.rankToNode[i]->name.c_str(),
						treeUse.rankToNode[i]->leaves, treeUse.rankToNode[i]->rank);
				}
			}
		}

		inline void outputNodeClass(FILE* fp) {
			fprintf(fp, "\n");
			fprintf(fp, "nodeClass:{\n");
			for (std::map<std::string, int>::iterator it = nodeClass._map.begin(); it != nodeClass._map.end(); it++) {
				fprintf(fp, "%s\t%d\n", it->first.c_str(), it->second);
			}
			fprintf(fp, "}\n");
		}

		inline void outputScoreMatrix(FILE* fp) {
			fprintf(fp, "\n");
			fprintf(fp, "scoreMatrix:(row, treeS;col, treeT){\n");
			for (int i = 0; i < treeS._size; i++) {
				for (int j = 0; j < treeT._size; j++) {
					fprintf(fp, "%d\t", scoreMatrix[i][j]);
				}
				fprintf(fp, "\n");
			}
			fprintf(fp, "}\n");
		}

		inline void outputGResult2(char* path) {
			DP();
			std::string outFile(path);
			FILE* fp = fopen(path, "w");
			if (treeS._size == 0 || treeT._size == 0) {
				fprintf(stderr, "treeS._size == 0 || treeT._size == 0 In outputLResult()\n");
				return;
			}

			if (OUTPUT_ALL) {//write the treeS, treeT, and score
				fprintf(fp, "treeS:{\n");//write treeS
				outputTree(fp, treeS);
				fprintf(fp, "}\n");
				fprintf(fp, "treeT:{\n");//write treeT
				outputTree(fp, treeT);
				fprintf(fp, "}\n");
				outputNodeClass(fp);//write nodeClass
				outputScoreMatrix(fp);//write scoreMatrix
			}

			size_t row = treeS._size - 1, col = treeT._size - 1;
			ScoreType maxScore = scoreMatrix[row][col];

			outputMatch(fp, row, col, maxScore);
			if (TEST_CASE > 1) {//do test and calculate pvalue if TEST_CASE is greater than 2
				outputPValue(fp, maxScore);
			}
			fclose(fp);
		}

        inline std::vector<std::tuple<std::string, std::string>> outputLResult(char* path, size_t num) {
            std::string outFile(path);
			std::vector<std::tuple<std::string, std::string>> subTreeRootsTS;
            FILE* fp = fopen((outFile + Setting::LocalC).c_str(), "w");
            if (treeS._size == 0 || treeT._size == 0) {
                fprintf(stderr, "treeS._size == 0 || treeT._size == 0 In outputLResult()\n");
				return subTreeRootsTS;
            }

            DPL();
            size_t inum = 1U;
			ScoreType maxScoreMax;


        outputLResultForLoopEnd:
            while (inum <= num)   {
#ifdef DEBUG_Bt
                for (size_t i = 0; i < treeS._size; ++i) {
                    for (size_t j = 0; j < treeT._size; ++j) {
                        printf("%d ", btMatrix[i][j]);
                    }
                    printf("\n");
                }
                printf("\n");
#endif // DEBUG_Bt
                size_t row, col;
                ScoreType maxScore = SCORE_MIN_L;
                for (size_t i = 0; i < treeS._size; ++i)
                    for (size_t j = 0; j < treeT._size; ++j) {
                        if (_CanUseCase_(btMatrix[i][j]) && scoreMatrix[i][j] > maxScore) {
                            maxScore = scoreMatrix[i][j];
                            row = i;
                            col = j;
                        }
                    }
                if (maxScore < SCORE_MIN_L) {
                    fprintf(fp, "All node used.\n");
                    break;
                }

                std::string *rootS = &((treeS.rankToNode[row])->id);
                std::string *rootT = &((treeT.rankToNode[col])->id);
                std::string pruneS, pruneT, matchS, matchT;
                std::vector<Point> vec;
                vec.push_back(Point(row, col));
                size_t vecSize = 1U;
                do {

                    switch (_ReCase_(btMatrix[row][col])) {
                    case SelectCase::NONE:
                        break;
                    case SelectCase::LL: {
                        TreeNode* pi = _row_[row];
                        TreeNode* pj = _col_[col];
                        matchS += pi->left->id + " " + pi->right->id + " ";
                        matchT += pj->left->id + " " + pj->right->id + " ";
                        vec.push_back(Point(pi->left->rank, pj->left->rank));
                        vec.push_back(Point(pi->right->rank, pj->right->rank));
                        break;
                    }
                    case SelectCase::LR: {
                        TreeNode* pi = _row_[row];
                        TreeNode* pj = _col_[col];
                        matchS += pi->left->id + " " + pi->right->id + " ";
                        matchT += pj->right->id + " " + pj->left->id + " ";
                        vec.push_back(Point(pi->left->rank, pj->right->rank));
                        vec.push_back(Point(pi->right->rank, pj->left->rank));
                        break;
                    }
                    case SelectCase::LS: {
                        pruneT += _col_[col]->right->id + " ";
                        vec.push_back(Point(_row_[row]->rank, _col_[col]->left->rank));
                        break;
                    }
                    case SelectCase::RS: {
                        pruneT += _col_[col]->left->id + " ";
                        vec.push_back(Point(_row_[row]->rank, _col_[col]->right->rank));
                        break;
                    }
                    case SelectCase::SL: {
                        pruneS += _row_[row]->right->id + " ";
                        vec.push_back(Point(_row_[row]->left->rank, _col_[col]->rank));
                        break;
                    }
                    case SelectCase::SR: {
                        pruneS += _row_[row]->left->id + " ";
                        vec.push_back(Point(_row_[row]->right->rank, _col_[col]->rank));
                        break;
                    }
                    case SelectCase::USED: {
                        std::pair<size_t, size_t> tmp = vec.front();
                        btMatrix[tmp.first][tmp.second] += SelectCase::USED;
                        goto outputLResultForLoopEnd;
                    }
                    default:
                        fprintf(stderr, "btMatrix Worry In outputResult()\n");
                    }
                    if (vecSize == vec.size()) {
                        break;
                    } else {
                        std::pair<size_t, size_t> tmp = vec[vecSize++];
                        row = tmp.first;
                        col = tmp.second;
                    }
                } while (1);
                while (vecSize > 0) {
                    std::pair<size_t, size_t> tmp = vec[--vecSize];
                    btMatrix[tmp.first][tmp.second] = SelectCase::USED;
                }
                fprintf(fp,
                    "%u{\nScore:%d\nRootS:%s\nRootT:%s\nPruneS:%s\nPruneT:%s\nMatchS:%s\nMatchT:%s\n}\n",
                    inum, maxScore, rootS->c_str(), rootT->c_str(),
                    pruneS.c_str(), pruneT.c_str(), matchS.c_str(), matchT.c_str()
                );


				maxScoreMax = maxScore;

				//outputPValueLT(rootS->c_str(), rootT->c_str());
				//std::cout << *rootS + " "+*rootT<<"\n";
				subTreeRootsTS.push_back(std::tuple<std::string, std::string>(*rootS, *rootT));


                ++inum;
            }


            fclose(fp);
			return subTreeRootsTS;
        }

		inline std::vector<std::tuple<std::string, std::string>> outputLResult2(char* path, size_t num) {//for verbose syntax
			std::vector<std::tuple<std::string, std::string>> subTreeRootsTS;
			FILE* fp = fopen(path, "w");//open the file
			if (treeS._size == 0 || treeT._size == 0) {
				fprintf(stderr, "treeS._size == 0 || treeT._size == 0 In outputLResult()\n");
				return subTreeRootsTS;
			}

			DPL();

			if (OUTPUT_ALL) {//write the treeS, treeT, and score
				fprintf(fp, "treeS:{\n");//write treeS
				outputTree(fp, treeS);
				fprintf(fp, "}\n");
				fprintf(fp, "treeT:{\n");//write treeT
				outputTree(fp, treeT);
				fprintf(fp, "}\n");
				outputNodeClass(fp);//write nodeClass
				outputScoreMatrix(fp);//write scoreMatrix
			}

			size_t inum = 1U;
			ScoreType maxScoreMax;


		outputLResultForLoopEnd:
			while (inum <= num) {
#ifdef DEBUG_Bt
				for (size_t i = 0; i < treeS._size; ++i) {
					for (size_t j = 0; j < treeT._size; ++j) {
						printf("%d ", btMatrix[i][j]);
					}
					printf("\n");
				}
				printf("\n");
#endif // DEBUG_Bt
				size_t row, col;
				ScoreType maxScore = SCORE_MIN_L;
				for (size_t i = 0; i < treeS._size; ++i)
					for (size_t j = 0; j < treeT._size; ++j) {
						if (_CanUseCase_(btMatrix[i][j]) && scoreMatrix[i][j] > maxScore) {
							maxScore = scoreMatrix[i][j];
							row = i;
							col = j;
						}
					}
				if (maxScore < SCORE_MIN_L) {
					fprintf(fp, "All node used.\n");
					break;
				}

				std::string *rootS = &((treeS.rankToNode[row])->id);
				std::string *rootT = &((treeT.rankToNode[col])->id);
				std::string pruneS, pruneT, matchS, matchT;
				std::vector<Point> vec;
				vec.push_back(Point(row, col));
				size_t vecSize = 1U;
				do {
					switch (_ReCase_(btMatrix[row][col])) {
					case SelectCase::NONE:
						break;
					case SelectCase::LL: {
						TreeNode* pi = _row_[row];
						TreeNode* pj = _col_[col];
						matchS += pi->left->id + " " + pi->right->id + " ";
						matchT += pj->left->id + " " + pj->right->id + " ";
						vec.push_back(Point(pi->left->rank, pj->left->rank));
						vec.push_back(Point(pi->right->rank, pj->right->rank));
						break;
					}
					case SelectCase::LR: {
						TreeNode* pi = _row_[row];
						TreeNode* pj = _col_[col];
						matchS += pi->left->id + " " + pi->right->id + " ";
						matchT += pj->right->id + " " + pj->left->id + " ";
						vec.push_back(Point(pi->left->rank, pj->right->rank));
						vec.push_back(Point(pi->right->rank, pj->left->rank));
						break;
					}
					case SelectCase::LS: {
						pruneT += _col_[col]->right->id + " ";
						vec.push_back(Point(_row_[row]->rank, _col_[col]->left->rank));
						break;
					}
					case SelectCase::RS: {
						pruneT += _col_[col]->left->id + " ";
						vec.push_back(Point(_row_[row]->rank, _col_[col]->right->rank));
						break;
					}
					case SelectCase::SL: {
						pruneS += _row_[row]->right->id + " ";
						vec.push_back(Point(_row_[row]->left->rank, _col_[col]->rank));
						break;
					}
					case SelectCase::SR: {
						pruneS += _row_[row]->left->id + " ";
						vec.push_back(Point(_row_[row]->right->rank, _col_[col]->rank));
						break;
					}
					case SelectCase::USED: {
						std::pair<size_t, size_t> tmp = vec.front();
						btMatrix[tmp.first][tmp.second] += SelectCase::USED;
						goto outputLResultForLoopEnd;
					}
					default:
						fprintf(stderr, "btMatrix Worry In outputResult()\n");
					}
					if (vecSize == vec.size()) {
						break;
					}
					else {
						std::pair<size_t, size_t> tmp = vec[vecSize++];
						row = tmp.first;
						col = tmp.second;
					}
				} while (1);
				while (vecSize > 0) {
					std::pair<size_t, size_t> tmp = vec[--vecSize];
					btMatrix[tmp.first][tmp.second] = SelectCase::USED;
				}
				fprintf(fp,
					"%u{\nScore:%d\nRootS:%s\nRootT:%s\nPruneS:%s\nPruneT:%s\nMatchS:%s\nMatchT:%s\n}\n",
					inum, maxScore, rootS->c_str(), rootT->c_str(),
					pruneS.c_str(), pruneT.c_str(), matchS.c_str(), matchT.c_str()
				);


				maxScoreMax = maxScore;

				//outputPValueLT(rootS->c_str(), rootT->c_str());
				//std::cout << *rootS + " "+*rootT<<"\n";
				subTreeRootsTS.push_back(std::tuple<std::string, std::string>(*rootS, *rootT));


				++inum;
			}


			fclose(fp);
			return subTreeRootsTS;
		}


        inline void DPL(void) {
                size_t m = (treeS._size + 1) >> 1, n = (treeT._size + 1) >> 1;
                TreeNode **pi = _row_, **pj = _col_;
                for (size_t i = 0; i < m; ++i, ++pi, pj = _col_)
                    for (size_t j = 0; j < n; ++j, ++pj) {
                        btMatrix[i][j] = SelectCase::NONE;
                        scoreMatrix[i][j] = costMatrix[(*pi)->nodeClass][(*pj)->nodeClass];
                    }
                pi = _row_; pj = _col_ + n;
                for (size_t i = 0; i < m; ++i, ++pi, pj = _col_ + n)
                    for (size_t j = n; j < treeT._size; ++j, ++pj) {
                        ScoreType lScore = _LSScore_;
                        ScoreType rScore = _RSScore_;
                        if (lScore > rScore) {
                            //if (lScore < 0) {
                            //    btMatrix[i][j] = SelectCase::NONE;
                            //    scoreMatrix[i][j] = 0;
                            //} else {
                                btMatrix[i][j] = SelectCase::LS;
                                scoreMatrix[i][j] = lScore;
                            //}
                        } else {
                            //if (rScore < 0) {
                            //    btMatrix[i][j] = SelectCase::NONE;
                            //    scoreMatrix[i][j] = 0;
                            //} else {
                                btMatrix[i][j] = SelectCase::RS;
                                scoreMatrix[i][j] = rScore;
                            //}
                        }
                    }
                pi = _row_ + m; pj = _col_;
                for (size_t i = m; i < treeS._size; ++i, ++pi, pj = _col_)
                    for (size_t j = 0; j < n; ++j, ++pj) {
                        ScoreType lScore = _SLScore_;
                        ScoreType rScore = _SRScore_;

                        if (lScore > rScore) {
                            //if (lScore < 0) {
                            //    btMatrix[i][j] = SelectCase::NONE;
                            //    scoreMatrix[i][j] = 0;
                            //} else {
                                btMatrix[i][j] = SelectCase::SL;
                                scoreMatrix[i][j] = lScore;
                            //}

                        } else {
                            //if (rScore < 0) {
                            //    btMatrix[i][j] = SelectCase::NONE;
                            //    scoreMatrix[i][j] = 0;
                            //} else {
                                btMatrix[i][j] = SelectCase::SR;
                                scoreMatrix[i][j] = rScore;
                            //}
                        }
                    }
                pi = _row_ + m; pj = _col_ + n;
                for (size_t i = m; i < treeS._size; ++i, ++pi, pj = _col_ + n)
                    for (size_t j = n; j < treeT._size; ++j, ++pj) {
                        BtType selectCase = SelectCase::NONE;
                        ScoreType maxScore = SCORE_MIN_L;
                        ScoreType score = _LSScore_;
                        _Select_Max_(SelectCase::LS)
                        score = _RSScore_;
                        _Select_Max_(SelectCase::RS)
                        score = _SLScore_;
                        _Select_Max_(SelectCase::SL)
                        score = _SRScore_;
                        _Select_Max_(SelectCase::SR)
                        score = _LLScore_;
                        _Select_Max_(SelectCase::LL)
                        score = _LRScore_;
                        _Select_Max_(SelectCase::LR)
                        btMatrix[i][j] = selectCase;
                        scoreMatrix[i][j] = maxScore;
                    }
#ifdef DEBUG_Score
                static int db_score_time = 0;
                if (db_score_time++ > 0)
                    return;
                for (size_t i = 0; i < treeS._size; ++i) {
                    for (size_t j = 0; j < treeT._size; ++j) {
                        printf("%d ", scoreMatrix[i][j]);
                    }
                    printf("\n");
                }
                printf("\n");
#endif // DEBUG_Score
				/*for (size_t i = 0; i < treeS._size; ++i) {
					for (size_t j = 0; j < treeT._size; ++j) {
						printf("%d ", scoreMatrix[i][j]);
					}
					printf("\n");
				}
				printf("\n\n\n");*/

        }

        HSA () {
            memset(costMatrix, 0, sizeof(costMatrix));//clean costMatrix
            for (size_t i = 0; i < Setting::CLASS_MAX; ++i)
                costMatrix[i][i] = Setting::MATCH_COST;
        }
        ~HSA () {
        }
    };


}

#endif // !__HSA_H__

double kCalculatePValue(double *numbers, int size, double testValue) {
	//with a list of numbers, size of numbers, and a testValue, return pvalue
	double pvalue, mean, sum = 0, sd = 0;
	for (int i = 0; i < size; i++) sum += numbers[i];
	mean = sum / size;
	for (int i = 0; i < size; i++) sd += pow(numbers[i] - mean, 2);
	sd = sqrt(sd / size);
	pvalue = erfc((testValue-mean) / sd / std::sqrt(2.0)) / 2.0;
	return pvalue;
}

void kOutputAndGeneratePValue(std::vector<std::tuple<std::string, std::string>> subTreeRootsST, char* pathCost, char* pathTreeS, char* pathTreeT, int testNum) {
	std::vector <std::tuple<std::string, std::string>> ::iterator it;
	int i = 1;
	std::string outFile(pathTreeS);
	FILE* fp = fopen((outFile + Setting::LocalC).c_str(), "a");
	for (it = subTreeRootsST.begin(); it<subTreeRootsST.end(); it++) {
		std::string subRootS, subRootT;
		subRootS = std::get<0>(*it);
		subRootT = std::get<1>(*it);
		fprintf(fp,"%d{\n",i);
		i++;
		kOutputAndGeneratePValueOne(subRootS, subRootT, pathCost, pathTreeS, pathTreeT, fp, testNum);
		fprintf(fp, "}\n");
	}
	fclose(fp);
}

void kOutputAndGeneratePValue2(std::vector<std::tuple<std::string, std::string>> subTreeRootsST,
	char* pathCost, char* pathTreeS, char* pathTreeT, int testNum, char* outfile) {
	std::vector <std::tuple<std::string, std::string>> ::iterator it;
	int i = 1;
	FILE* fp = fopen(outfile, "a");
	for (it = subTreeRootsST.begin(); it<subTreeRootsST.end(); it++) {
		std::string subRootS, subRootT;
		subRootS = std::get<0>(*it);
		subRootT = std::get<1>(*it);
		fprintf(fp, "%d{\n", i);
		i++;
		kOutputAndGeneratePValueOne(subRootS, subRootT, pathCost, pathTreeS, pathTreeT, fp, testNum);
		fprintf(fp, "}\n");
	}
	fclose(fp);
}

void kOutputAndGeneratePValueOne(std::string subRootS, std::string subRootT, char* pathCost, char* pathTreeS, char* pathTreeT, FILE* fp, int testNum) {
	HSA::HSA *subHSA = new HSA::HSA();
	subHSA->TEST_CASE = testNum;
	subHSA->BuildCost(pathCost);
	subHSA->BuildTreeTS(pathTreeS, pathTreeT);
	subHSA->getSubTree(subRootS, subRootT);
	//std::string sa, sb;
	//std::cout << "subS: " << subRootS << " subT: " << subRootT << "\n";
	subHSA->DPL();
	int maxScore;
	maxScore = subHSA->getMaxScore();
	//std::cout << "\nmaxScore: " << maxScore << "\n";

	subHSA->outputPValueLLL(fp, maxScore);

	delete subHSA;
}
