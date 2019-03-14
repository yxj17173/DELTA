# DELTA (Developmental Cell Lineage Tree Alignment)
A novel computational framework for comparative analysis of developmental lineage trees reveals developmental motifs and their genetic basis

There are two ways to use this program.

## Method 1: the simple syntax
One of the following scripts:
```
HSA <TreeS file path> <TreeT file path> <Cost file path> <g>
HSA <TreeS file path> <TreeT file path> <Cost file path> <l> <n>
HSA <TreeS file path> <TreeT file path> <Cost file path> <g> <testNum>
HSA <TreeS file path> <TreeT file path> <Cost file path> <l> <n> <testNum>
```
g: global alignment

l: local alignment

n: number of local alignments to output

testNum: number of test for calculate p-value (default:100, max:10000,min:2)


## Method 2: the verbose way
```
HSA [-h][--help]
```
-h: print short help and description

--help: print detailed help and description

### Usage:
```
HSA [-treeS <TreeS file path>][-treeT ] [-cost <Cost file path>][-method ] [-max_target <target num for l>][-test testNum] [-outfile <output file path>][-all ] [-prune pruneScore]
```
### required options: -treeS, -treeT, -cost, -method

### description:

#### required options:

-treeS: TreeS file path

-treeT: TreeT file path

-cost: Cost file path. the cost file contains the score for different types of leaves

#### optional parameters:

-method: l or g. g for global alignment; l for local alignment. default: g.

-max_target: target num for l, local alignment. default: 1

-test: testNum to calculate p-value. If testNum <=2, do not output p-value. default 0

-outfile: output file path. default: TreeS file path + l or g, based on -method

-all: T or F. output as much information as possible. default F;

-prune: pruneScore is the punish for pruning one leaf. default 1
