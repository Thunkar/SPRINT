# include<iostream>
# include "csv.h"

using namespace std;
using namespace io;

const int size_y = 10;
const int csv_width = 9; 
const int size_x = 10;
const int n_classes = 2;

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

double gini(string **data_set, int current_y_size, int split_row, int column) {
   quicksort(data_set, 0, current_y_size - 1, column);
   double **gini_matrix = new double*[2];
   gini_matrix[0] = new double[n_classes];
   gini_matrix[1] = new double[n_classes];

   double total_yes, total_no;

   for(int i = 0; i < current_y_size; i++) {
      if(i <= split_row){
          gini_matrix[0][atoi(data_set[i][0].c_str())]++;
          total_yes++;
      }
      else {
          gini_matrix[1][atoi(data_set[i][0].c_str())]++;
          total_no++;
      }
   }

   double gini_yes, gini_no;

   for(int j = 0; j < n_classes; j++){
        gini_yes += (gini_matrix[0][j]/total_yes)*(gini_matrix[0][j]/total_yes);
        gini_no += (gini_matrix[1][j]/total_no)*(gini_matrix[1][j]/total_no);
   }
   gini_yes = (1 - gini_yes)*(total_yes/(double)current_y_size);
   gini_no = (1 - gini_no)*(total_no/(double)current_y_size);
   
   return gini_yes + gini_no;

}

string SPRINT(string **data_set) {
    quicksort(data_set, 0, size_y -1, 1);
    for(int i = 0; i < size_y; i++){
        cout << data_set[i][0];
        cout << " ";
        cout << data_set[i][1] << endl;
    }
    cout << "Gini 4: ";
    cout << gini(data_set, size_y, 4, 1) << endl; 
}

int main(int argc, char *argv[]){

    CSVReader<csv_width> in("abalone.data");
    string sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings;

    string **data_set = new string*[size_y];

    for(int i = 0; i < size_y; i++){
        data_set[i] = new string[size_x];
        in.read_row(sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings);
        sex = sex.compare("M") ? "1" : "0";
        string line[] = {sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings, "-1"};
        for(int j = 0; j < size_x; j++) {
           data_set[i][j] = line[j];
        }
    }
    SPRINT(data_set);
}

