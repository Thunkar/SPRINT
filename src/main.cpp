# include<iostream>
# include "csv.h"
 
int main(int argc, char *argv[]){
  io::CSVReader<9> in("abalone.data");
  std::string sex; double length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings;
  while(in.read_row(sex, length, diameter, height, wholeWeight, shuckedWeight, visceraWeight, shellWeight, rings))	{
  	std::cout << sex << std::endl;
  }
}