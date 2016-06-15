#SPRINT Algorithm C++ implementation
Really basic version + random forest generation. Not threaded. Feel free to PR.

#Usage
Configuration file (named config.cnf and on the same folder as the executable and the dataset) required using the following format (remove comments, they're not supported):

```
sprint #Either sprint (1 tree, k fold cross validation) or random forest (k trees)
compact #Either compact (console output) or verbose (results are printed to result.data file, with predicted class labels first)
adult-proc.data #Dataset file name. CSV, one record per row. Supports strings and numbers. Class label (integer) always first.
32561 #Number of rows (size of the dataset) to read. Can be less than the actual dataset size.
15  #Number of attributes (including class label) in the dataset
2 #Number of classes
0.4 #Gini dissimilarity measure threshold to stop the algorithm
0,1,0,1,0,1,0,0,0,0,0,1,1,1,0 #Number array of size = number_of_attributes (including class label). First one is ignored, for the rest 1 means it is a numerical attribute, 0 categorical.
10  #K of the K fold cross validation procedure for the sprint mode, number of trees to generate for the random forest one.
```
