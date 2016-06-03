# include<iostream>
# include "csv.h"

const int size_y = 10;
const int csv_width = 9; 
const int size_x = 10;


int divide(std::string **array, int start, int end, int column) {
    int left;
    int right;
    float pivot;
    std::string *temp = new std::string[size_x];

    pivot = std::atof(array[start][column].c_str());
    left = start;
    right = end;

    while (left < right) {
        while (std::atof(array[right][column].c_str()) > pivot) {
            right--;
        }

        while ((left < right) && (std::atof(array[left][column].c_str()) <= pivot)) {
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

void quicksort(std::string **array, int start, int end, int column)
{
    int pivot;

    if (start < end) {
        pivot = divide(array, start, end, column);

        quicksort(array, start, pivot - 1, column);

        quicksort(array, pivot + 1, end, column);
    }
}


std::string SPRINT(std::string **dataSet, int row, int column) {
    quicksort(dataSet, row, size_y - 1, column);
    for(int i = row; i < size_y; i++){
        std::cout << dataSet[i][1] << std::endl;
    }
}

int main(int argc, char *argv[]){

    io::CSVReader<csv_width> in("abalone.data");
    std::string sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings;

    std::string **dataSet = new std::string*[size_y];

    for(int i = 0; i < size_y; i++){
        dataSet[i] = new std::string[size_x];
        in.read_row(sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings);
        std::string line[] = {sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings, "-1"};
        for(int j = 0; j < size_x; j++) {
           *&dataSet[i][j] = line[j];
        }
    }
    SPRINT(dataSet, 0, 1);
}

