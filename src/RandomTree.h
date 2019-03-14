#ifndef __RONDOMTREE_H__
#define __RONDOMTREE_H__

#include <cstdio>
#include <time.h>
#include <random>
#include "BFTree.h"
#include "Setting.h"

namespace TN {
    static size_t RandomTreeTime = 0U;
    class RandomTree {
        typedef std::uniform_int_distribution<int> Distribution;

        BFTree* _tree;
        std::default_random_engine generator;
        Distribution classDis;
    public:
        RandomTree(BFTree* tree, int classNum) : _tree(tree),
            generator(time(NULL) + RandomTreeTime++), classDis(0, classNum - 1) {}
        ~RandomTree() {}
        TreeNode* generateTree(size_t leaves) {
            TreeNode* p = _tree->rankToNode[--(_tree->_size)];
            p->leaves = leaves;
            if (leaves == 1) {
                p->nodeClass = classDis(generator);
                p->left = p->right = nullptr;
            } else {
                Distribution distribution(1, leaves - 1);
                distribution(generator);
                size_t left = distribution(generator);
                p->left = generateTree(left);
                p->right = generateTree(leaves - left);
            }
            return p;
        }

        void BuildTree(void) {
            size_t tmp = _tree->_size;
#ifdef DEBUG_generateTree
            printTree(generateTree((tmp + 1) >> 1), 0);
            if (_tree->_size != 0) {
                fprintf(stderr, "generateTree Worry: leaves and _size not match.\n");
                return;
            }
#else
            generateTree((tmp + 1) >> 1);
#endif // DEBUG_generateTree
            _tree->_size = tmp;
            _tree->sort();
        }

    };
}

#endif // !__RONDOMTREE_H__
