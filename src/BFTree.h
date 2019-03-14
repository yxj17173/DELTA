#ifndef __BFTREE_H__
#define __BFTREE_H__

#include <string>
#include <algorithm>
#include "NodeClass.h"
#include "Setting.h"

namespace TN {
    typedef size_t RankType;

    struct TreeNode {
        std::string id, name;
        size_t leaves;
        NodeClass::Value nodeClass;
        RankType rank;
        TreeNode *left, *right;
        TreeNode() : leaves(0U), left(nullptr), right(nullptr) {}//way of initiate TreeNode.
        TreeNode(const char* _id) : id(_id), leaves(0U), left(nullptr), right(nullptr) {}
        TreeNode(const char* _id, const char* _name, const NodeClass::Value _class,
            TreeNode* _left = nullptr, TreeNode* _right = nullptr)
            : id(_id), name(_name), leaves(1U),
            nodeClass(_class), left(_left), right(_right) {}
    };

    inline bool CmpTreeNode(const TreeNode* a, const TreeNode* b) {//compare number of leaves for two TreeNode
        return a->leaves < b->leaves;
    }
    
    const size_t NODE_MAX_NUM = 16383U;//20180126 change from 4096U to 16383U
    class BFTree {

        void freeBFTree(TreeNode* node) {//delete node and all its children
            if (node) {
                freeBFTree(node->left);
                freeBFTree(node->right);
                delete node;
            }
        }
    public:
        size_t countLeaves(TreeNode* node) {//count the number of leaves for a TreeNode
            if (node) {
                node->leaves += countLeaves(node->left) + countLeaves(node->right);
                return node->leaves;
            } else {
                return 0U;
            }
        }

        size_t _size;//total number of nodes
        TreeNode* rankToNode[NODE_MAX_NUM];

        BFTree() : _size(1U) {//initiate BFTree with only the Root node. _size(1U) is used to set _size = 1.
            rankToNode[0] = new TreeNode(Setting::TreeRootName);
        }
        ~BFTree() {//destruct BFTree.
            /*for (size_t i = 0; i < _size; ++i)
                delete rankToNode[i];*/
        }
		
		inline void getRankToNodewithNode(TreeNode *rootNode, TreeNode** subRankToNode, int* positon) {
			if (rootNode!=nullptr) {
				subRankToNode[*positon] = rootNode;
				*positon = *positon + 1;
				//std::cout << *positon << " ";
				//std::cout << rootNode->id << " ";
				//std::cout << subRankToNode[*positon-1]->id << " ";
				if (rootNode->left) {
					getRankToNodewithNode(rootNode->left, subRankToNode, positon);
				}
				if (rootNode->right) {
					getRankToNodewithNode(rootNode->right, subRankToNode, positon);
				}
			}
			
		}

	

		inline void getRankToNodeWithID(TreeNode **subRankToNode, std::string idIn, int* position) {//get a BFTree with the id			
			TreeNode *rootNode;
			int tempi;
			for (int i = 0; i < _size; i++) {
				if (rankToNode[i]->id == idIn) {
					rootNode = rankToNode[i];
					tempi = i;
					break;
				}
			}
			//std::cout << "the id is for " << idIn << " " << tempi << "\n";
			//std::vector<TreeNode> vectorNodes;
			/*int *position;
			*position = 0;*/
			getRankToNodewithNode(rootNode, subRankToNode, position);
			/*for (int i = 0; i < *position; i++) {
				std::cout << subRankToNode[i]->id << " ";
			}*/
		}

        inline void sort(void) {
            std::sort(rankToNode, rankToNode + _size, CmpTreeNode);
			//sort TreeNodes in rankToNode according to the number of leaves in accending order. lower level comes first
            for (size_t i = 0; i < _size; ++i)
                rankToNode[i]->rank = i;//get the rank for each TreeNode in rankToNode
        }

        inline void countAndSort(void) {
            countLeaves(rankToNode[0]);
            sort();
        }

        inline void insert(char* id, char* name, NodeClass::Value _class) {
            char* path = id;
            TreeNode *tmpT, *p = rankToNode[0];
            TreeNode** f = &tmpT;
            while (*path) {
                if (p == nullptr) {
                    char tmp = *path;
                    *path = '\0';
                    rankToNode[_size++] = *f = p = new TreeNode(id);
                    *path = tmp;
                }
                if (*(path++) == '0') {
                    f = &(p->left);
                    p = p->left;
                } else {
                    f = &(p->right);
                    p = p->right;
                }
            }
            rankToNode[_size++] = *f = new TreeNode(id, name, _class);
        }

        inline RankType getRank(register TreeNode* node) {
            return node->rank;
        }

        inline TreeNode* getNode(register RankType rank) {
            return rankToNode[rank];
        }

        TreeNode* generateTree(size_t leaves) {
            TreeNode* p;
            if (leaves == 1) {
                p = new TreeNode();
            } else {
                size_t left = 1;
                p->left = generateTree(left);
                p->right = generateTree(leaves - left);
            }
            return p;
        }

        inline void print(void) {
            for (size_t i = 0; i < _size; ++i) {
                TreeNode* p = rankToNode[i];
                printf("id:%s name:%s class:%d\n", (p->id).c_str(), (p->name).c_str(), p->nodeClass);
                printf("leaves:%u rank:%u\n", p->leaves, p->rank);
                printf("left:%s ", p->left ? (p->left->id).c_str() : "NULL");
                printf("right:%s\n\n", p->right ? (p->right->id).c_str() : "NULL");
            }
        }
    };
    
    void printTree(TreeNode* p, int level) {
        if (p == nullptr)
            return;
        for (int i = 0; i < level; ++i)
            printf("        ");
        printf("|_%3u %3d\n", p->leaves,
            (p->left && p->right? -1 : p->nodeClass));
        printTree(p->left, level + 1);
        printTree(p->right, level + 1);
    }
}

#endif // !__BFTREE_H__
