# include <iostream>
# include "math.h"
# include <iomanip>
# include <sstream>
# include <fstream>
# include <random>
# include <algorithm>
# include <set>
# include <cstring>
# include <map>
# include <utility>

using namespace std;

int size_y, size_x, n_classes;
double gini_threshold;
int* attrs;
string mode;

// Struct representing an internal node of the classifier
struct TreeNode {
    string *split_condition;
    int winning_class;
    TreeNode *left;
    TreeNode *right;
};

struct TreeStats {
    int node_count;
    int leaf_count;
    int gini_count;
    int sprint_count;
};

TreeStats *current_tree_stats;

// Auxiliary method for quicksort ordering. Tailored to use the specified column of the dataset.
int divide(string **array, int start, int end, int column) {
    int left;
    int right;
    double pivot;
    string *temp;

    pivot = atof(array[start][column].c_str());
    left = start;
    right = end;

    while (left < right) {
        while (atof(array[right][column].c_str()) > pivot) {
            right--;
        }

        while ((left < right) && (atof(array[left][column].c_str()) <= pivot)) {
            left++;
        }

        if (left < right) {
            temp = array[left];
            array[left] = array[right];
            array[right] = temp;
        }
    }

    temp = array[right];
    array[right] = array[start];
    array[start] = temp;

    return right;
}

// Custom implementation of quicksort for two-dimensional arrays. Column is the column (duh) used for ordering.
void quicksort(string **array, int start, int end, int column)
{
    int pivot;

    if (start < end) {
        pivot = divide(array, start, end, column);

        quicksort(array, start, pivot - 1, column);

        quicksort(array, pivot + 1, end, column);
    }
}

// Calculates the gini measure of dissimilarity after splitting the dataset at the specified split_row, in relation to the attribute in column. Uses attrs to determine if column attribute is numerical or nominal.
double gini(string **data_set, int size_y, int split_row, int column) {
    current_tree_stats->gini_count++;
    double **gini_matrix = new double*[2];
    for (int i = 0; i < 2; i++) {
        gini_matrix[i] = new double[n_classes];
        for (int j = 0; j < n_classes; j++) {
            gini_matrix[i][j] = 0;
        }
    }

    double total_yes = 0;
    double total_no = 0;

    for (int i = 0; i < size_y; i++) {
        int class_id = atoi(data_set[i][0].c_str());
        if (attrs[column] == 1) {
            if (i <= split_row) {
                gini_matrix[0][class_id]++;
                total_yes++;
            }
            else {
                gini_matrix[1][class_id]++;
                total_no++;
            }
        }
        else {
            if (data_set[split_row][column].compare(data_set[i][column]) == 0) {
                gini_matrix[0][class_id]++;
                total_yes++;
            }
            else {
                gini_matrix[1][class_id]++;
                total_no++;
            }
        }
    }

    double gini_yes = 0;
    double gini_no = 0;

    for (int j = 0; j < n_classes; j++) {
        gini_yes += (gini_matrix[0][j] / total_yes)*(gini_matrix[0][j] / total_yes);
        gini_no += (gini_matrix[1][j] / total_no)*(gini_matrix[1][j] / total_no);
    }

    delete[] gini_matrix[0];
    delete[] gini_matrix[1];
    delete[] gini_matrix;

    gini_yes = (1 - gini_yes)*(total_yes / (double)size_y);
    gini_no = (1 - gini_no)*(total_no / (double)size_y);

    if (total_no == 0)
        gini_no = 0;
    if (total_yes == 0)
        gini_yes = 0;
    return gini_yes + gini_no;

}

// Returns an array with the information needed to split the dataset optimally. {split_row, attr_column, gini_value, split_value}. The latter will be stored in the internal node of the classifier.
string* bestGiniSplit(string **data_set, int size_y, int col_size, int *col_set){
    double best_gini = 2;
    int row_split, column;
    string value;
    double majority_class = -1;
    set<string> nominal_already_checked;
    for (int j = 1; j < col_size; j++) {
        if (attrs[j] == 1) quicksort(data_set, 0, size_y - 1, j);
        for (int i = 0; i < size_y; i++) {
            if(attrs[j] == 0){
                if(nominal_already_checked.find(data_set[i][j]) != nominal_already_checked.end())
                    continue;
                else 
                    nominal_already_checked.insert(data_set[i][j]);
            }
            double current_gini = gini(data_set, size_y, i, j);
            if (current_gini < best_gini) {
                best_gini = current_gini;
                column = j; 
                row_split = i;
                if (attrs[j] == 1 && i < size_y - 1)
                    value = to_string((atof(data_set[i][j].c_str()) + atof(data_set[i + 1][j].c_str())) / 2);
                else
                    value = data_set[i][j];
            }
        }
    }
    if (best_gini < 2) {
        string *str = new string[5];
        str[0] = to_string(row_split);
        str[1] = to_string(column);
        str[2] = to_string(best_gini);
        str[3] = value;
        str[4] = to_string(col_set[column]); //value which stores real column id
        return str;
    }
    else
        return nullptr;
}
// Utility method to print classifier trees. Only suitable for small ones (demonstration purpuses).
void printTree(TreeNode* p, int indent) {
    if (p) {
        if (p->right) {
            printTree(p->right, indent + 4);
        }
        if (indent) {
            cout << setw(indent) << ' ';
        }
        if (p->right) cout << "n /\n" << setw(indent) << ' ';
        if (p->split_condition)
            cout << "Col: " << p->split_condition[1] << " <= " << p->split_condition[3] << endl;
        else
            cout << "Class: " << p->winning_class << endl;
        if (p->left) {
            cout << setw(indent) << ' ' << "y \\\n";
            printTree(p->left, indent + 4);
        }
    }
}

// Inserts a terminal node with the decision class at the bottom of the tree.
void insertTerminalNode(string **data_set, int size_y, TreeNode* parent) {
    if (size_y == 0) return;
    current_tree_stats->leaf_count++;
    int *class_rank = new int[n_classes];
    for (int i = 0; i < n_classes; i++) {
        class_rank[i] = 0;
    }
    int max_class_rank = -1;
    int winning_class = -1;
    for (int i = 0; i < size_y; i++) {
        int current_class = atoi(data_set[i][0].c_str());
        if (++class_rank[current_class] > max_class_rank) {
            max_class_rank = class_rank[current_class];
            winning_class = current_class;
        }
    }

    TreeNode* node = new TreeNode();
    node->split_condition = nullptr;
    node->winning_class = winning_class;
    node->left = nullptr;
    node->right = nullptr;
    if (parent) {
        if (parent->left)
            parent->right = node;
        else
            parent->left = node;
    }
}

// Main recursive algorithm.
void SPRINT(string **data_set, int size_y, TreeNode *&root, TreeNode* parent, int col_size, int* col_set){
    current_tree_stats->sprint_count++;
    string *best_split = bestGiniSplit(data_set, size_y, col_size, col_set); 
    if (!best_split) {
        insertTerminalNode(data_set, size_y, parent); 
        return;
    }

    int row_split = atoi(best_split[0].c_str());
    int column = atoi(best_split[1].c_str());
    double gini = atof(best_split[2].c_str());
    int real_column = atoi(best_split[4].c_str());//add statement for real column id

    int left_size_y = row_split + 1;
    int right_size_y = size_y - row_split - 1;

    string **left_leaf;
    string **right_leaf;

    if (attrs[real_column] == 0) { // If nominal attribute, use vectors to store the leaves temporarily (we don't know their size).
        vector<string*> left_leaf_vector;
        vector<string*> right_leaf_vector;
        for (int i = 0; i < size_y; i++) {
            if (data_set[row_split][column].compare(data_set[i][column]) == 0) {
                left_leaf_vector.push_back(data_set[i]);
            }
            else {
                right_leaf_vector.push_back(data_set[i]);
            }
        }

        left_size_y = left_leaf_vector.size();
        right_size_y = right_leaf_vector.size();

        left_leaf = new string*[left_size_y];
        right_leaf = new string*[right_size_y];

        for (int i = 0; i < left_size_y; i++) { // use memcpy or std::copy
            left_leaf[i] = new string[size_x];
            for (int j = 0; j < col_size; j++) {
                left_leaf[i][j] = left_leaf_vector[i][j];
            }
        }

        for (int i = 0; i < right_size_y; i++) { // use memcpy or std::copy
            right_leaf[i] = new string[size_x];
            for (int j = 0; j < col_size; j++) {
                right_leaf[i][j] = right_leaf_vector[i][j];
            }
        }
    }
    else {
        left_leaf = new string*[left_size_y];
        right_leaf = new string*[right_size_y];
        for (int i = 0; i < left_size_y; i++) { // use std::copy or memcpy - optimization #1
            left_leaf[i] = new string[size_x];
            for (int j = 0; j < col_size; j++) {
                left_leaf[i][j] = data_set[i][j];
            }
        }

        for (int i = 0; i < right_size_y; i++) {
            right_leaf[i] = new string[size_x];
            for (int j = 0; j < col_size; j++) {
                right_leaf[i][j] = data_set[i + left_size_y][j];
            }
        }
    }

    if (right_size_y > 0) {
        TreeNode* node = new TreeNode();
        node->split_condition = best_split;
        node->winning_class = -1;
        node->left = nullptr;
        node->right = nullptr;
        if (parent) {
            if (parent->left)
                parent->right = node;
            else
                parent->left = node;
        }
        else {
            root = node;
        }
        current_tree_stats->node_count++;
        if (gini >= gini_threshold) {
            SPRINT(left_leaf, left_size_y, root, node, col_size, col_set);
            SPRINT(right_leaf, right_size_y, root, node, col_size, col_set);
        }
        else {
            insertTerminalNode(left_leaf, left_size_y, node); 
            insertTerminalNode(right_leaf, right_size_y, node); 
        }
    }
    else {
        insertTerminalNode(left_leaf, left_size_y, parent); 
    }
    for (int i = 0; i < right_size_y; i++) {
        delete[] right_leaf[i];
    }
    delete[] right_leaf;

    for (int i = 0; i < left_size_y; i++) {
        delete[] left_leaf[i];
    }
    delete[] left_leaf;
}

// Takes a test set and transverses the tree for each sample, in order to build the confusion matrix of the classifier.
int** classify(string **data_set, int size_y, TreeNode* classifier) {
    int** confusion_matrix = new int*[n_classes];
    for (int i = 0; i < n_classes; i++) {
        confusion_matrix[i] = new int[n_classes];
        for (int j = 0; j < n_classes; j++) {
            confusion_matrix[i][j] = 0;
        }
    }
    ofstream resultfs;
    if(mode.compare("verbose") == 0){
        resultfs.open("result.data", ios_base::app); 
    }
    for (int i = 0; i < size_y; i++) {
        TreeNode* current_node = classifier;
        while (current_node->split_condition) {
            int current_column = atoi(current_node->split_condition[1].c_str());
            string current_value = data_set[i][current_column];
            string split_value = current_node->split_condition[3];
            if (attrs[current_column] == 1) {
                if (atof(current_value.c_str()) <= atof(split_value.c_str()))
                    current_node = current_node->left;
                else
                    current_node = current_node->right;

            }
            else {
                if (split_value.compare(current_value) == 0)
                    current_node = current_node->left;
                else
                    current_node = current_node->right;
            }
        }
        int real_class = atoi(data_set[i][0].c_str());
        int winning_class = current_node->winning_class;
        resultfs << winning_class << ",";
        if(mode.compare("verbose") == 0){
            for(int j = 0; j < size_x; j++){
                resultfs << data_set[i][j];
                if(j != size_x - 1)
                    resultfs << ",";
            }
        }
        resultfs << endl;
        confusion_matrix[real_class][winning_class]++;
    }
    if(mode.compare("verbose") == 0)
        resultfs.close();
    return confusion_matrix;
}

// Delete tree to free up memory
void deleteTree(TreeNode *root) {
    if (root->left)
        deleteTree(root->left);

    if (root->right)
        deleteTree(root->right);

    if (root->split_condition) 
        delete[] root->split_condition;

    delete root;
}

void resetCurrentTreeStats() {

    if(current_tree_stats)
        delete current_tree_stats;
    current_tree_stats = new TreeStats();
    current_tree_stats->node_count = 0;
    current_tree_stats->leaf_count = 0;
    current_tree_stats->gini_count = 0;
    current_tree_stats->sprint_count = 0;

}

double kFoldCrossValidation(string **data_set, int k, int **k_matrix, int test_set_index, int test_set_size) {
    resetCurrentTreeStats();
    int training_set_size = (k - 1)*test_set_size;
    string **test_set = new string*[test_set_size];
    string **training_set;
    TreeNode *root;
    int *columns = new int[size_x];

    for(int i = 0; i < size_x; i++){
        columns[i] = i;
    }

    for (int i = 0; i < test_set_size; i++) {
        test_set[i] = new string[size_x];
        for (int j = 0; j < size_x; j++) {
            test_set[i][j] = data_set[k_matrix[test_set_index][i]][j];
        }
    }

    clock_t begin;

    if (training_set_size > 0) {
        training_set = new string*[training_set_size];
        for (int i = 0; i < training_set_size; i++) {
            training_set[i] = new string[size_x];
            int k_matrix_row = (int)((double)i / (double)test_set_size);
            int k_matrix_column = i - test_set_size*k_matrix_row;
            k_matrix_row = k_matrix_row < test_set_index ? k_matrix_row : k_matrix_row + 1;
            for (int j = 0; j < size_x; j++) {
                training_set[i][j] = data_set[k_matrix[k_matrix_row][k_matrix_column]][j];
            }
        }
        begin = clock();
        SPRINT(training_set, training_set_size, root, nullptr, size_x, columns);
    }
    else {
        begin = clock();
        SPRINT(test_set, test_set_size, root, nullptr, size_x, columns);
    }

    clock_t end = clock();
    double sprint_elapsed = double(end - begin) / CLOCKS_PER_SEC;

    begin = clock();
    int** confusion_matrix = classify(test_set, test_set_size, root);
    end = clock();

    double classify_elapsed = double(end - begin) / CLOCKS_PER_SEC;
    int success = 0;
    int error = 0;
    for (int i = 0; i < n_classes; i++) {
        for (int j = 0; j < n_classes; j++) {
            if (i == j) success += confusion_matrix[i][j];
            else error += confusion_matrix[i][j];
        }
    }
    if (training_set_size>0) {
        for (int i = 0; i < training_set_size; i++) {
            delete[] training_set[i];
        }
        delete[] training_set;
    }
    for (int i = 0; i < test_set_size; i++) {
        delete[] test_set[i];
    }
    delete[] test_set;

    for (int i = 0; i < n_classes; i++) {
        delete[] confusion_matrix[i];
    }
    delete[] confusion_matrix;
    deleteTree(root);
    double success_ratio = (double)success / (double)test_set_size;
    cout << "======" << endl;
    cout << "Success ratio of partition: " << test_set_index << " -> " << success_ratio << endl;
    cout << "Time to build the classifier: " << sprint_elapsed << "s. Time to classify the test data: " << classify_elapsed << "s." << endl;
    cout << "Node count: " << current_tree_stats->node_count << " Leaf count: " << current_tree_stats->leaf_count << " Gini measures calculated: " << current_tree_stats->gini_count << " SPRINT iterations: " << current_tree_stats->sprint_count << endl;
    return success_ratio;
}

//Method takes one data object and runs the classification process on a single tree in the Random forest. Returns winning class.
int classifyRF(string *data, TreeNode* classifier){	
    TreeNode* current_node = classifier;
    while (current_node->split_condition) {
        int current_column = atoi(current_node->split_condition[4].c_str());
        string current_value = data[current_column];
        string split_value = current_node->split_condition[3];
        if (attrs[current_column] == 1) {
            if (atof(current_value.c_str()) <= atof(split_value.c_str()))
                current_node = current_node->left;
            else
                current_node = current_node->right;
        }
        else {
            if (split_value.compare(current_value) == 0)
                current_node = current_node->left;
            else
                current_node = current_node->right;
        }
    }

    int winning_class = current_node->winning_class;
    return winning_class;
}	

//Method runs classifyRF on each tree in forest and returns the class which has majority.
int voting(std::map<int,TreeNode*> &forest, string *data, int *contains){
    std::map<int,int> result;
    int winner = -1, max = 0, temp=0;
    for(std::map<int, TreeNode*>::iterator it = forest.begin();it!=forest.end();++it){
        if(!contains[it->first])
            result[classifyRF(data,it->second)]++;
    }
    for(std::map<int,int>::iterator iter = result.begin();iter!=result.end();++iter){
        temp = iter->second;
        if(temp > max){
            winner = iter->first;
            max = temp;
        }
    }
    return winner;
}

int voting(std::map<int,TreeNode*> &forest, string *data){
    std::map<int,int> result;
    int winner = -1, max = 0, temp=0;
    for(std::map<int, TreeNode*>::iterator it = forest.begin();it!=forest.end();++it){
        result[classifyRF(data,it->second)]++;
    }
    for(std::map<int,int>::iterator iter = result.begin();iter!=result.end();++iter){
        temp = iter->second;
        if(temp > max){
            winner = iter->first;
            max = temp;
        }
    }
    return winner;
}

// To use on test set
int** votingTest(std::map<int, TreeNode*> &forest, string **dataset, int size, int attr_size){
    int** confusion_matrix = new int*[n_classes];
    for (int i = 0; i < n_classes; i++) {
        confusion_matrix[i] = new int[n_classes];
        for (int j = 0; j < n_classes; j++) {
            confusion_matrix[i][j] = 0;
        }
    }
    for (int i = 0; i < size; i++) {
        int winning_class = voting(forest, dataset[i]);
        int real_class = atoi(dataset[i][0].c_str());
        confusion_matrix[real_class][winning_class]++;
    }
    return confusion_matrix;
}

//Random Forest function
void randomForest(string **dataset, int n){
    typedef std::map<int, TreeNode*> Forest;
    typedef std::pair<int, TreeNode*> RFTree;
    map< int , vector< int>> indexMap;
    vector<int> testvector;	
    int num_samples=n;
    Forest forest;
    string** temp_sample_set = new string*[(int)(2*size_y/3)];
    int	sample_size = (int)2*size_y/3;
    int attr_size = (int)sqrt(size_x);
    int tempIndex,tempVal;
    int **col_set = new int*[n];
    int *has_element = new int[n];
    //building random forest phase
    std::random_device rd;
    std::uniform_int_distribution<int> generator_y(0,size_y-1);
    int maxVal = size_x-1;
    int *arr;
    arr = new int[maxVal];
    for(int k=1 ; k<size_x;k++)
        arr[k-1] = k;
    //make samples inside loop
    clock_t begin;

    for(int i=0 ; i<num_samples ; i++){
        begin = clock();
        //index_set[i] = new int[sample_size];
        col_set[i] = new int[attr_size];
        random_shuffle(&arr[0], &arr[maxVal- 1]);
        for(int a = 0 ; a < attr_size+1 ; a++){
            if(a == 0){
                col_set[i][a] = a;
            }else{
                col_set[i][a] = arr[a];
            }
        }
        for(int j=0 ; j<sample_size ; j++){	
            tempIndex = generator_y(rd);
            temp_sample_set[j] = new string[(attr_size+1)];
            for(int k=0 ; k < attr_size+1 ; k++){
                temp_sample_set[j][k] = dataset[tempIndex][(col_set[i][k])];
            }			
            testvector.push_back(tempIndex);
        }
        TreeNode* test = new TreeNode();
        resetCurrentTreeStats();
        SPRINT(temp_sample_set, sample_size, test, nullptr, (attr_size+1), col_set[i]);
        forest.insert(RFTree(i,test));
        std::sort (testvector.begin(), testvector.end());
        indexMap.insert(std::make_pair(i,testvector));
        if (sample_size>0) {
            for (int t = 0; t < sample_size; t++) {
                delete[] temp_sample_set[t];
            }
        }
        testvector.clear();
        clock_t end = clock();
        double rf_elapsed = double(end - begin) / CLOCKS_PER_SEC;
        cout << "============" << endl;
        cout<<"Time to build random forest tree-> "<<i<<": "<<rf_elapsed<<endl;
        cout << "Node count: " << current_tree_stats->node_count << " Leaf count: " << current_tree_stats->leaf_count << " Gini measures calculated: " << current_tree_stats->gini_count << " SPRINT iterations: " << current_tree_stats->sprint_count << endl;
    }

    //voting and efficiency calculation phase
    int** confusion_matrix = new int*[n_classes];
    for (int i = 0; i < n_classes; i++) {
        confusion_matrix[i] = new int[n_classes];
        for (int j = 0; j < n_classes; j++) {
            confusion_matrix[i][j] = 0;
        }
    }
    ofstream resultfs;
    if(mode.compare("verbose") == 0){
        resultfs.open("result.data", ios_base::app);
    }
    clock_t begin1 = clock();
    int voted_winner, actual_winner;

    for(int i=0;i<size_y;i++){
        for(std::map<int, vector<int>>::iterator it = indexMap.begin();it!=indexMap.end();++it){
            if(std::binary_search (it->second.begin(), it->second.end(), i)){
                has_element[it->first]=1;
            }else{
                has_element[it->first] = 0;
            }
        }
        voted_winner = voting(forest, dataset[i], has_element);
        actual_winner = atoi(dataset[i][0].c_str());
        resultfs << voted_winner << ",";
        if(mode.compare("verbose") == 0) {
            for(int j = 0; j < size_x; j++){
                resultfs << dataset[i][j];
                if(j != size_x -1)
                    resultfs << ",";
            }
        }
        resultfs << endl;
        confusion_matrix[actual_winner][voted_winner]++;
    }
    if(mode.compare("verbose") == 0)
        resultfs.close();

    int success = 0;
    int error = 0;
    for (int i = 0; i < n_classes; i++) {
        for (int j = 0; j < n_classes; j++) {
            if (i == j) success += confusion_matrix[i][j];
            else error += confusion_matrix[i][j];
        }
    }
    clock_t end1 = clock();
    double voting_elapsed = double(end1 - begin1) / CLOCKS_PER_SEC;
    cout << "=========" << endl;	
    cout<<"Size of database: "<<size_y<<endl;
    cout<<"Success: "<<success<<endl;
    cout<<"Error: "<<error<<endl;
    double success_ratio = (double)success / (double)size_y;
    cout<<"Ratio of success: "<<success_ratio<<endl;
    cout<<"Time taken to classify dataset: "<<voting_elapsed<<"s"<<endl;
}



// There you go
int main(int argc, char *argv[]) {

    cout << "Reading configuration file" << endl;

    ifstream configfs;
    configfs.open("config.cnf");
    if (!configfs) {
        std::cout << "Failed to open the configuration file." << std::endl;
        return 1;
    }
    string line;
    getline(configfs, line);
    string type = string(line);
    getline(configfs, line);
    mode = string(line);
    getline(configfs, line);
    string data_set_name = string(line);
    getline(configfs, line);
    size_y = atoi(line.c_str());
    getline(configfs, line);
    size_x = atoi(line.c_str());
    getline(configfs, line);
    n_classes = atoi(line.c_str());
    getline(configfs, line);
    gini_threshold = atof(line.c_str());
    getline(configfs, line);
    attrs = new int[size_x];
    istringstream attrss(line);
    string token;
    for (int j = 0; j < size_x; j++) {
        getline(attrss, token, ',');
        attrs[j] = atoi(token.c_str());
    }

    getline(configfs, line);
    int k = atoi(line.c_str());
    int num = k; 

    int test_set_size = (int)((double)size_y / (double)k);

    string **data_set = new string*[size_y];
    int *indexes = new int[size_y];
    int counter = 0;

    cout << "Loading dataset into memory" << endl;

    ifstream datafs;
    datafs.open(data_set_name);
    if (!datafs) {
        std::cout << "Failed to open the dataset file." << std::endl;
        return 1;
    }
    for (int i = 0; i < size_y; i++) {
        string line;
        getline(datafs, line);
        istringstream liness(line);
        string token;
        data_set[i] = new string[size_x];
        indexes[i] = counter++;
        for (int j = 0; j < size_x; j++) {
            getline(liness, token, ',');
            data_set[i][j] = token;
        }
    }
    remove("result.data");
    if(type.compare("sprint") == 0) {

        random_shuffle(&indexes[0], &indexes[size_y - 1]);

        int **k_matrix = new int*[k];

        for (int i = 0; i < k; i++) {
            k_matrix[i] = new int[test_set_size];
            for (int j = 0; j < test_set_size; j++) {
                k_matrix[i][j] = indexes[i*test_set_size + j];
            }
        }

        int test_set_index = 0;
        double acc_success_ratio = 0;

        cout << "Building classifiers..." << endl;
        clock_t begin = clock();
        for (int i = 0; i < k; i++) {
            acc_success_ratio += kFoldCrossValidation(data_set, k, k_matrix, test_set_index++, test_set_size);
        }
        clock_t end = clock();
        double total_elapsed = double(end - begin) / CLOCKS_PER_SEC;
        cout << "===========" << endl;
        cout << "Total success ratio after cross-validation: " << acc_success_ratio / (double)k << endl;
        cout << "Total elapsed time: " << total_elapsed << "s" << endl;
    } else {
        clock_t begin = clock();
        randomForest(data_set,num);
        clock_t end = clock();
        double total_elapsed = double(end - begin) / CLOCKS_PER_SEC;
        cout << "Total elapsed time: " << total_elapsed << "s" << endl;
    }
}
