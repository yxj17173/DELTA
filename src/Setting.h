#ifndef __SETTING_H__
#define __SETTING_H__

//#define DEBUG
//#define DEBUG_Tree
//#define DEBUG_Cost
//#define DEBUG_Score
//#define DEBUG_Bt
//#define DEBUG_generateTree

//#define GLOBAL


#ifndef GLOBAL
#define LOCAL
#endif // !GLOBAL

#include <limits.h>

namespace Setting {
    typedef char BacktrackingType;

    const unsigned int CLASS_MAX = 16383U;
    const unsigned int NODE_MAX_OFFSET = 11U;
    //const unsigned int NODE_MAX = 1U << NODE_MAX_OFFSET;//NODE_MAX 2048. change
	const unsigned int NODE_MAX = 16383U;
    const unsigned int NODE_MAX_MASK = NODE_MAX - 1U;

    typedef char CostType;
    const CostType NOT_MATCH_COST = -2;
    const CostType MATCH_COST = 2;

	extern int PRUNE_CONSTANT;//20180313

    typedef int ScoreType;
    const int SCORE_MAX = INT_MAX;
    const int SCORE_MIN = INT_MIN;
    const int SCORE_MAX_G = INT_MAX;
    const int SCORE_MIN_G = INT_MIN;
    const int SCORE_MAX_L = INT_MAX;
    const int SCORE_MIN_L = INT_MIN;
	const int MAX_TEST = 10000;

	const size_t TEST_CASE = 100U;
	//extern int TEST_CASE;

    const char* TreeRootName = "Root";
    const char* ResultHeader = "score root1 root2 prune1 prune2 match1 match2";

    const char GlobalC = 'g';
    const char LocalC = 'l';
}


#endif // !__SETTING_H__
