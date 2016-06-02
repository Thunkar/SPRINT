# include<iostream>
# include "csv.h"


const int size_y = 4000;
const int size_x = 9;

int main(int argc, char *argv[]){

    io::CSVReader<size_x> in("abalone.data");
    std::string sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings;

    std::string **dataSet = new std::string*[size_y];

    int counter = 0;

    while(in.read_row(sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings))	{
        std::string line[] = {sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings, "-1"};
        dataSet[counter] = line;
        if(++counter == size_y) break;
    }
}

int divide(std::string **array, int start, int end, int column) {
    int left;
    int right;
    float pivot;
    std::string **temp;

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

    for(int i = row; i < size_y; i++){

    }
}
