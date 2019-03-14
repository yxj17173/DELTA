#ifndef __IOLIB_H__
#define __IOLIB_H__

#include "NodeClass.h"
#include "TreeNode.h"

#define ASSERT_READ_ERROR(__condition__, __fp__) if (__condition__) {\
    fprintf(stderr, "File Format Worry.\n");\
    fclose(__fp__);\
    return nullptr;\
}

namespace IOLib {
    using TN::TreeNode;
    const unsigned int LINE_MAX_C = 16383U;//20180126 change from 4096U to 16383U

    TreeNode* BuildTree(char* path) {
        char buf[LINE_MAX_C];
        char lineage[LINE_MAX_C], name[LINE_MAX_C], nodeClass[LINE_MAX_C];
        FILE* fp = fopen(path, "r");
        fgets(buf, LINE_MAX_C, fp);
        while (!feof(fp)) {
            fgets(buf, LINE_MAX_C, fp);
            ASSERT_READ_ERROR((sscanf(buf, "%s %s %s", lineage, name, nodeClass) != 3), fp)
            printf("%s %s %s\n", lineage, name, nodeClass);
        }
        fclose(fp);
        TreeNode* root = nullptr;
        return root;
    }

    using NodeClass::NodeClassMap;
    int* BuildCost(char* path, NodeClassMap& _map) {
        int* x = new int[1000 * 1000];
    }

    #define ExitReadCostError(condition, fp) if (condition) {\
        fprintf(stderr, "Cost File Format Worry.\n");\
        fclose(fp);\
        exit(-1);\
    }

    inline size_t BuildCost(char* path) {
        unsigned int num;
        FILE* fp = fopen(path, "r");
        ExitReadCostError((fscanf(fp, "%d", &num) != 1), fp);
        char _class[LINE_MAX_C];
        for (size_t i = 0U; i < num; ++i) {
            ExitReadCostError((fscanf(fp, "%s", _class) != 1), fp);
            nodeClass.insert(std::string(_class));
        }
        for (size_t i = 0U; i < num; ++i)
            for (size_t j = i; j < num; ++j) {
                ExitReadCostError((fscanf(fp, "%d", &(cost[i][j])) != 1), fp);
                cost[j][i] = cost[i][j];
            }
        return num;
    }


    for (size_t end = nodeClass.size(); classNum < end; ++classNum) {
                costMatrix[classNum][classNum] = Setting::MATCH_COST;
                for (size_t j = classNum + 1; j < end; ++j)
                    costMatrix[j][classNum] = costMatrix[classNum][j] = Setting::NOT_MATCH_COST;
            }
            return classNum;
}

#endif // !__IOLIB_H__
