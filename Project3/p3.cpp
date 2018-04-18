#include <string>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;


#include "../Ngrams/fileRead.cpp"
#include "../Ngrams/VectorHash.h"
#include "../Ngrams/utilsToStudents.h"

typedef string T;
vector<T> set;

unordered_map<vector<T>, int> getNgrams(vector<T>tokens, int n) {

	unordered_map<vector<T>, int> database;

	for (int i = 0; i <= tokens.size() - n; i++)
	{
		vector<T> nGram(n);

		for (unsigned int j = 0; j < n; j++) {
			nGram[j] = tokens[i + j];
		}

		if (database.count(nGram) == 0) {
			database[nGram] = 1;
		}
		else {
			database[nGram] = database[nGram] + 1;
		}
	}

	return database;

}


vector<double> getProbs(vector<T> prevGram, double denominator, unordered_map<vector<T>, int> yGrams) {
	vector<double> probs;

	for (auto i = set.begin(); i != set.end(); ++i) {
		vector<T>vGram;
		vGram.push_back(*i);

		vector<T> tempGram;

		tempGram.reserve(prevGram.size() + vGram.size());
		tempGram.insert(tempGram.end(), prevGram.begin(), prevGram.end());
		tempGram.insert(tempGram.end(), vGram.begin(), vGram.end());

		double probability = double(yGrams[tempGram]) / denominator;
		probs.push_back(probability);

	}

	return probs;
}


void p3(string fileName, int Nsize) {
	try {
		vector<T> tokens;
		read_tokens(fileName, tokens, true);
		set = tokens;
		sort(set.begin(), set.end());
		set.erase(unique(set.begin(), set.end()), set.end());


		vector<double> probs;
		vector<T> prevGram;

		int xSize = 1;
		int ySize = 2;
		unordered_map<vector<T>, int> yGrams;
		unordered_map<vector<T>, int> xGrams;
		vector<T> sentence;
		int denominator = tokens.size();

		if (Nsize > 1) {
			sentence.push_back("<END>");
		}
		else if (Nsize == 1) {
			yGrams = getNgrams(tokens, 1);
			xSize = 0;
		}

		bool isEnd = false;
		while (!isEnd) {
			vector<T> prevGram(sentence.end() - xSize, sentence.end());

			if (Nsize > 1) {

				if (ySize <= Nsize) {
					yGrams = getNgrams(tokens, ySize);
					xGrams = getNgrams(tokens, xSize);
				}

				if (ySize < Nsize) {
					++ySize;
					++xSize;
				}



				denominator = xGrams[prevGram];
			}


			probs = getProbs(prevGram, denominator, yGrams);
			int i = drawIndex(probs);
			T gram = set[i];
			sentence.push_back(gram);
			cout << gram << " ";

			if (gram == "<END>") {
				isEnd = true;
			}
		}
	}
	catch (FileReadException e) {
		e.Report();
	}
}

int main(int argc, char* argv[]) {
	string file1 = string(argv[1]);
	int size = atoi(argv[2]);
	p3(file1, size + 1);

	return 1;
}
