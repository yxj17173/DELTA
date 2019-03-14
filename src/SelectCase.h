#ifndef __SELECTCASE_H__
#define __SELECTCASE_H__

#include "Setting.h"

//enum SelectCase {
//    LL, LR, SL, SR, RL, RS
//};

#define _row_ treeS.rankToNode
#define _col_ treeT.rankToNode
#define _LSScore_ (scoreMatrix[(*pi)->rank][(*pj)->left->rank] - Setting::PRUNE_CONSTANT * (*pj)->right->leaves)
#define _RSScore_ (scoreMatrix[(*pi)->rank][(*pj)->right->rank] - Setting::PRUNE_CONSTANT * (*pj)->left->leaves)
#define _SLScore_ (scoreMatrix[(*pi)->left->rank][(*pj)->rank] - Setting::PRUNE_CONSTANT * (*pi)->right->leaves)
#define _SRScore_ (scoreMatrix[(*pi)->right->rank][(*pj)->rank] - Setting::PRUNE_CONSTANT * (*pi)->left->leaves)
#define _LLScore_ (scoreMatrix[(*pi)->left->rank][(*pj)->left->rank] + scoreMatrix[(*pi)->right->rank][(*pj)->right->rank])
#define _LRScore_ (scoreMatrix[(*pi)->left->rank][(*pj)->right->rank] + scoreMatrix[(*pi)->right->rank][(*pj)->left->rank])

#define _subrow_ subTreeS.rankToNode
#define _subcol_ subTreeT.rankToNode
#define _subLSScore_ (subScoreMatrix[(*pi)->rank][(*pj)->left->rank] - (*pj)->right->leaves)
#define _subRSScore_ (subScoreMatrix[(*pi)->rank][(*pj)->right->rank] - (*pj)->left->leaves)
#define _subSLScore_ (subScoreMatrix[(*pi)->left->rank][(*pj)->rank] - (*pi)->right->leaves)
#define _subSRScore_ (subScoreMatrix[(*pi)->right->rank][(*pj)->rank] - (*pi)->left->leaves)
#define _subLLScore_ (subScoreMatrix[(*pi)->left->rank][(*pj)->left->rank] + scoreMatrix[(*pi)->right->rank][(*pj)->right->rank])
#define _subLRScore_ (subScoreMatrix[(*pi)->left->rank][(*pj)->right->rank] + scoreMatrix[(*pi)->right->rank][(*pj)->left->rank])


#define _Select_Max_(_selectCase) if (score > maxScore) {\
    selectCase = _selectCase;\
    maxScore = score;\
}

#define _CanUseCase_(x) (x < SelectCase::NOBT)
#define _ReCase_(x) ((x) > SelectCase::USED? (x) - SelectCase::USED : (x))

namespace SelectCase {
    typedef Setting::BacktrackingType BtType;
    const BtType NONE = 1;
    const BtType LS = 2;
    const BtType RS = 3;
    const BtType SL = 4;
    const BtType SR = 5;
    const BtType LL = 6;
    const BtType LR = 7;
    const BtType USED = 1 << 4;

    const BtType NOBT = USED;
    // const BtType REPEAT = 14;
}


#endif // !__SELECTCASE_H__
