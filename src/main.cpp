# include <iostream>
# include "math.h"
# include <iomanip>
# include <sstream>
# include <fstream>
# include <random>
# include <algorithm>

using namespace std;

int size_y, size_x, n_classes;
double gini_threshold;
int* attrs;

// Struct representing an internal node of the classifier
struct TreeNode {
	string *split_condition;
	int winning_class;
	TreeNode *left;
	TreeNode *right;
};

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
string* bestGiniSplit(string ** data_set, int size_y) {
	double best_gini = 2;
	int row_split, column;
	string value;
	double majority_class = -1;
	for (int j = 1; j < size_x; j++) {
		if (attrs[j] == 1) quicksort(data_set, 0, size_y - 1, j);
		for (int i = 0; i < size_y; i++) {
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
		string *str = new string[4];
		str[0] = to_string(row_split);
		str[1] = to_string(column);
		str[2] = to_string(best_gini);
		str[3] = value;
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
void insertTerminalNode(string ** data_set, int size_y, TreeNode* parent) {
	if (size_y == 0) return;
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

// Main recursive algorithm. Really poorly optimized. Sorry.
void SPRINT(string **data_set, int size_y, TreeNode *&root, TreeNode* parent) {
	string *best_split = bestGiniSplit(data_set, size_y);
	if (!best_split) {
		insertTerminalNode(data_set, size_y, parent);
		return;
	}

	int row_split = atoi(best_split[0].c_str());
	int column = atoi(best_split[1].c_str());
	double gini = atof(best_split[2].c_str());


	int left_size_y = row_split + 1;
	int right_size_y = size_y - row_split - 1;

	string **left_leaf;
	string **right_leaf;

	if (attrs[column] == 0) { // If nominal attribute, use vectors to store the leaves temporarily (we don't know their size).
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

		for (int i = 0; i < left_size_y; i++) { // This is ugly
			left_leaf[i] = new string[size_x];
			for (int j = 0; j < size_x; j++) {
				left_leaf[i][j] = left_leaf_vector[i][j];
			}
		}

		for (int i = 0; i < right_size_y; i++) { // Yep, still ugly
			right_leaf[i] = new string[size_x];
			for (int j = 0; j < size_x; j++) {
				right_leaf[i][j] = right_leaf_vector[i][j];
			}
		}
	}
	else {
		left_leaf = new string*[left_size_y];
		right_leaf = new string*[right_size_y];
		for (int i = 0; i < left_size_y; i++) { // Pretty sure the memory can be copied in chunks in order to divide the dataset. Don't know how.
			left_leaf[i] = new string[size_x];
			for (int j = 0; j < size_x; j++) {
				left_leaf[i][j] = data_set[i][j];
			}
		}

		for (int i = 0; i < right_size_y; i++) {
			right_leaf[i] = new string[size_x];
			for (int j = 0; j < size_x; j++) {
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
		if (gini >= gini_threshold) {
			SPRINT(left_leaf, left_size_y, root, node);
			SPRINT(right_leaf, right_size_y, root, node);
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
		confusion_matrix[real_class][winning_class]++;
	}
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

double kFoldCrossValidation(string **data_set, int k, int **k_matrix, int test_set_index, int test_set_size) {
	int training_set_size = (k - 1)*test_set_size;
	string **test_set = new string*[test_set_size];
	string **training_set;
	TreeNode *root;

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
		SPRINT(training_set, training_set_size, root, nullptr);
	}
	else {
		begin = clock();
		SPRINT(test_set, test_set_size, root, nullptr);
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
	cout << "Success ratio of partition: " << test_set_index << " -> " << success_ratio << endl;
	cout << "Time to build the classifier: " << sprint_elapsed << "s. Time to classify the test data: " << classify_elapsed << "s." << endl;
	return success_ratio;
}

// There you go
int main(int argc, char *argv[]) {

	cout << "Reading configuration file" << endl;

	ifstream configfs;
	configfs.open("sprint.cnf");
	if (!configfs) {
		std::cout << "Failed to open the configuration file." << std::endl;
		return 1;
	}
	string line;
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

	for (int i = 0; i < k; i++) {
		acc_success_ratio += kFoldCrossValidation(data_set, k, k_matrix, test_set_index++, test_set_size);
	}

	cout << "Total success ratio after cross-validation: " << acc_success_ratio / (double)k << endl;
}
