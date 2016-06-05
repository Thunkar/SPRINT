# include <iostream>
# include "csv.h"
# include "math.h"


using namespace std;
using namespace io;

const int size_y = 1000;
const int size_x = 9;
const int n_classes = 2;
const int max_tree_depth = 5;
const double gini_threshold = 0.1;

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

    for(int i = 0; i < size_y; i++) {
        if(i <= split_row){
            gini_matrix[0][atoi(data_set[i][0].c_str())]++;
            total_yes++;
        }
        else {
            gini_matrix[1][atoi(data_set[i][0].c_str())]++;
            total_no++;
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
    double best_gini = 1;
    int row_split, column;
    string value;
    for(int j = 1; j < size_x; j++) {
        quicksort(data_set, 0, size_y - 1, j);
        for(int i = 0; i < size_y; i++){
            double current_gini = gini(data_set, size_y, i, j);
            if(current_gini <= best_gini){
                best_gini = current_gini;
                column = j; 
                row_split = i;
                value = data_set[i][j];
            }
        } 
    } 
    return new string[4]{ to_string(row_split), to_string(column), to_string(best_gini), value }; 
}

void SPRINT(string **data_set, int size_y) {
    string *best_split = bestGiniSplit(data_set, size_y);
    int row_split = atoi(best_split[0].c_str());
    int column = atoi(best_split[1].c_str());
    double gini = atof(best_split[2].c_str());
    string value = best_split[3];

    cout << "Column: " << column  << " Gini: " << gini << " Value: " << value << endl;

    if(gini < gini_threshold) return;

    int left_size_y = row_split + 1;
    int right_size_y = size_y - row_split - 1;

    string **left_leaf = new string*[left_size_y];
    string **right_leaf = new string*[right_size_y];

    for(int i = 0; i < left_size_y; i++) {
        left_leaf[i] = new string[size_x];
        for(int j = 0; j < size_x; j++){
            left_leaf[i][j] = data_set[i][j];
        }
    }

    for(int i = 0; i < right_size_y; i++) {
        right_leaf[i] = new string[size_x];
        for(int j = 0; j < size_x; j++){
            right_leaf[i][j] = data_set[i+row_split][j];
        }
    }
    SPRINT(left_leaf, left_size_y);
    SPRINT(right_leaf, right_size_y);
}

int main(int argc, char *argv[]){

    CSVReader<size_x> in("abalone.data");
    string sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings;

    string **data_set = new string*[size_y];

    for(int i = 0; i < size_y; i++){
        data_set[i] = new string[size_x];
        in.read_row(sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings);
        sex = sex.compare("M") ? "1" : "0";
        string line[] = {sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings};
        for(int j = 0; j < size_x; j++) {
            data_set[i][j] = line[j];
        }
    }

    SPRINT(data_set, size_y);

}

