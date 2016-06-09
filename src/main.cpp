# include <iostream>
# include "csv.h"
# include "math.h"
# include <iomanip>


using namespace std;
using namespace io;

const int size_y = 4000;
const int size_x = 9;
const int n_classes = 2;
const double gini_threshold = 0.0001;

CSVReader<size_x> in("abalone.data");

int *attrs = new int[size_x]{0,1,1,1,1,1,1,1,1};

struct TreeNode {
    string *split_condition;
    int winning_class;
    TreeNode *left; 
    TreeNode *right; 
};


int divide(string **array, int start, int end, int column) {
    int left;
    int right;
    double pivot;
    string *temp = new string[size_x];

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

void quicksort(string **array, int start, int end, int column)
{
    int pivot;

    if (start < end) {
        pivot = divide(array, start, end, column);

        quicksort(array, start, pivot - 1, column);

        quicksort(array, pivot + 1, end, column);
    }
}


double gini(string **data_set, int size_y, int split_row, int column) {
    double **gini_matrix = new double*[2];
    for(int i = 0; i < 2; i++) {
        gini_matrix[i] = new double[n_classes];
        for(int j = 0; j < n_classes; j++){
            gini_matrix[i][j] = 0;
        }
    }
    double total_yes = 0;
    double total_no = 0;

    double *class_rank = new double[n_classes];

    for(int i = 0; i < size_y; i++) {
        int class_id = atoi(data_set[i][0].c_str());
        if(attrs[column] == 1) {
            if(i <= split_row){
                gini_matrix[0][class_id]++;
                total_yes++;
            }
            else {
                gini_matrix[1][class_id]++;
                total_no++;
            }
        } else {
            if(data_set[split_row][column].compare(data_set[i][column]) == 0){
                gini_matrix[0][class_id]++;
                total_yes++;
            }else {
                gini_matrix[1][class_id]++;
                total_no++;
            }
        }
    }

    double gini_yes = 0;
    double gini_no = 0;

    for(int j = 0; j < n_classes; j++){
        gini_yes += (gini_matrix[0][j]/total_yes)*(gini_matrix[0][j]/total_yes);
        gini_no += (gini_matrix[1][j]/total_no)*(gini_matrix[1][j]/total_no);
    }
    gini_yes = (1 - gini_yes)*(total_yes/(double)size_y);
    gini_no = (1 - gini_no)*(total_no/(double)size_y);

    if(total_no == 0) 
        gini_no = 0;
    if(total_yes == 0)
        gini_yes = 0;
    return gini_yes + gini_no;

}

string* bestGiniSplit(string ** data_set, int size_y) {
    double best_gini = 2;
    int row_split, column;
    string value;
    double majority_class = -1;
    for(int j = 1; j < size_x; j++) {
        if(attrs[j] == 1) quicksort(data_set, 0, size_y - 1, j);
        for(int i = 0; i < size_y; i++){
            double current_gini = gini(data_set, size_y, i, j);
            if(current_gini < best_gini){
                best_gini = current_gini;
                column = j; 
                row_split = i;
                if(attrs[j] == 1 && i < size_y - 1)
                    value = to_string((atof(data_set[i][j].c_str())+atof(data_set[i+1][j].c_str()))/2);
                else 
                    value = data_set[i][j];
            }
        } 
    } 
    if(best_gini < 2)
        return new string[4]{ to_string(row_split), to_string(column), to_string(best_gini), value}; 
    else 
        return nullptr;
}

void printTree(TreeNode* p, int indent){
    if(p) {
        if(p->right) {
            printTree(p->right, indent+4);
        }
        if (indent) {
            cout << setw(indent) << ' ';
        }
        if (p->right) cout<<"n /\n" << setw(indent) << ' ';
        if(p->split_condition)
            cout<< "Col: " << p->split_condition[1] << " <= " << p->split_condition[3] << endl;
        else 
            cout << "Class: " << p->winning_class << endl;
        if(p->left) {
            cout << setw(indent) << ' ' <<"y \\\n";
            printTree(p->left, indent+4);
        }
    }
}

void insertTerminalNode(string ** data_set, int size_y, TreeNode* parent) {
    if(size_y == 0) return;
    int *class_rank = new int[n_classes];
    for(int i = 0; i < n_classes; i++) {
        class_rank[i] = 0;
    }
    int max_class_rank = -1;
    int winning_class = -1;
    for(int i = 0; i < size_y; i++) {
        int current_class = atoi(data_set[i][0].c_str());
        if(++class_rank[current_class] > max_class_rank){
            max_class_rank = class_rank[current_class];
            winning_class = current_class;
        }
    } 

    TreeNode* node = new TreeNode();
    *node = { nullptr, winning_class, nullptr, nullptr, };
    if(parent){
        if(parent->left)
            parent->right = node;
        else
            parent->left = node;
    }
}

void SPRINT(string **data_set, int size_y, TreeNode *&root, TreeNode* parent) {
    string *best_split = bestGiniSplit(data_set, size_y);
    if(!best_split){
        insertTerminalNode(data_set, size_y, parent);
        return;
    }

    int row_split = atoi(best_split[0].c_str());
    int column = atoi(best_split[1].c_str());
    double gini = atof(best_split[2].c_str());

    TreeNode* node = new TreeNode();
    *node = { best_split, -1, nullptr, nullptr, };
    if(parent){
        if(parent->left)
            parent->right = node;
        else
            parent->left = node;
    } else {
        root = node;
    }

    int left_size_y = row_split + 1;
    int right_size_y = size_y - row_split - 1;

    string **left_leaf;
    string **right_leaf;

    if(attrs[column] == 0) {
        vector<string*> left_leaf_vector;
        vector<string*> right_leaf_vector;
        for(int i = 0; i < size_y; i++) {
            if(data_set[row_split][column].compare(data_set[i][column]) == 0){
                left_leaf_vector.push_back(data_set[i]);    
            } else {
                right_leaf_vector.push_back(data_set[i]);
            }
        }

        left_size_y = left_leaf_vector.size();
        right_size_y = right_leaf_vector.size();

        for(int i = 0; i < left_size_y; i++) {
            left_leaf[i] = new string[size_x];
            for(int j = 0; j < size_x; j++){
                left_leaf[i][j] = left_leaf_vector[i][j];
            }
        }

        for(int i = 0; i < right_size_y; i++) {
            right_leaf[i] = new string[size_x];
            for(int j = 0; j < size_x; j++){
                right_leaf[i][j] = right_leaf_vector[i][j];
            }
        }
    } else {
        left_leaf = new string*[left_size_y];
        right_leaf = new string*[right_size_y];
        for(int i = 0; i < left_size_y; i++) {
            left_leaf[i] = new string[size_x];
            for(int j = 0; j < size_x; j++){
                left_leaf[i][j] = data_set[i][j];
            }
        }

        for(int i = 0; i < right_size_y; i++) {
            right_leaf[i] = new string[size_x];
            for(int j = 0; j < size_x; j++){
                right_leaf[i][j] = data_set[i+left_size_y][j];
            }
        }
    }

    if(gini >= gini_threshold){
        SPRINT(left_leaf, left_size_y, root, node);
        SPRINT(right_leaf, right_size_y, root, node);
    } else {
        insertTerminalNode(left_leaf, left_size_y, node);
        if(right_size_y != 0)
            insertTerminalNode(right_leaf, right_size_y, node);
        else
            insertTerminalNode(left_leaf, left_size_y, node);
    }
}

int** classify(string **data_set, int size_y, TreeNode* classifier) {
    int** confusion_matrix = new int*[n_classes];
    for(int i = 0; i < n_classes; i++) {
        confusion_matrix[i] = new int[n_classes];
        for(int j = 0; j < n_classes; j++) {
            confusion_matrix[i][j] = 0;
        }
    }
    for(int i = 0; i < size_y; i++) {
        TreeNode* current_node = classifier;
        while(current_node->split_condition){
            int current_column = atoi(current_node->split_condition[1].c_str());
            string current_value = data_set[i][current_column];
            string split_value = current_node->split_condition[3];
            if(attrs[current_column] == 1){
                if(atof(current_value.c_str()) <= atof(split_value.c_str()))
                    current_node = current_node->left;
                else
                    current_node = current_node->right;

            } else {
                if(split_value.compare(current_value) == 0)
                    current_node = current_node->left;
                else 
                    current_node = current_node->right;
            }
        }
        int real_class = atoi(data_set[i][0].c_str());
        int winning_class = current_node->winning_class;
        confusion_matrix[real_class][winning_class]++;     
    }
    return confusion_matrix;
} 


void read_cars(string** data_set) {
    string risk, age, type;

    for(int i = 0; i < size_y; i++){
        data_set[i] = new string[size_x];
        //        in.read_row(risk, age, type);
        string line[] = {risk, age, type};
        for(int j = 0; j < size_x; j++) {
            data_set[i][j] = line[j];
        }
    } 
}

void read_abalone(string** data_set) {
    string sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings;


    for(int i = 0; i < size_y; i++){
        data_set[i] = new string[size_x];
        in.read_row(sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings);
        sex = sex.compare("M") ? "1" : "0";
        string line[] = {sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings};
        for(int j = 0; j < size_x; j++) {
            data_set[i][j] = line[j];
        }
    }
}


int main(int argc, char *argv[]){

    string **data_set = new string*[size_y];
    read_abalone(data_set);

    TreeNode *root; 
    SPRINT(data_set, size_y, root, nullptr);
    cout << "=========== Decision Tree ==========" << endl;
    //   printTree(root, 0);
    cout << endl << "========= Confusion Matrix ========" << endl;
    int** confusion_matrix = classify(data_set, size_y, root);
    int success = 0;
    int error = 0;
    for(int i = 0; i < n_classes; i++) {
        for(int j = 0; j < n_classes; j++) {
            if(i == j) success+=confusion_matrix[i][j];
            else error+=confusion_matrix[i][j];
            cout << confusion_matrix[i][j] << " ";
        }
        cout << endl;
    }
    cout << "Success: " << (double)success/(double)size_y << " Error: " << (double)error/(double)size_y << endl;
}

