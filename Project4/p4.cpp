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

vector<vector<T>> getNgramsVector(vector<T>tokens, int n) {

	vector<vector<T>> nGrams;

	for (int i = 0; i <= tokens.size() - n; i++) {
		vector<T> nGram(n);

		for (unsigned int j = 0; j < n; j++) {
			nGram[j] = tokens[i + j];
		}

		nGrams.push_back(nGram);
	}

	return nGrams;
}

vector<double> getProbs(unordered_map<vector<T>, int> sentenceGrams, unordered_map<vector<T>, int> nGrams, double bottom, double delta) {
	vector<double> probs;
	double prob;
	double sum = 0;
	for (auto i = sentenceGrams.begin(); i !=  sentenceGrams.end(); ++i) {
		vector<T> nGram = i->first;
		int count = nGrams[nGram];
		int top = count + delta;

		prob = top / bottom;
		probs.push_back(prob);

		sum += log(prob);

		for (unsigned int j = 0; j < nGram.size(); j++) {
			cout << nGram[j] << " ";
		}
		cout << " : " << prob << endl;
	}

	cout << sum << endl;

	return probs;
}

double getProb(vector<T> nGram, unordered_map<vector<T>, int> nGrams, double bottom, double delta) {
	double prob;

	double count = nGrams[nGram];
	double top = count + delta;

	prob = top / bottom;

	return prob;
}

double getBottom(double V, double tokenSize, int nSize, double delta) {
	double B = pow(V, nSize);
	double bottom = tokenSize + B * delta;
	return bottom;
}

void p4(string fileName, string fileName2, int nSize, double delta) {
	vector<T> tokens;
	vector<T> sentenceTokens;
	read_tokens(fileName, tokens, false);
	read_tokens(fileName2, sentenceTokens, false);

	double N = tokens.size();

	unordered_map<int, unordered_map<vector<T>, int>> allGrams;

	for (int i = 0; i < nSize; i++) {
		allGrams[i + 1] = getNgrams(tokens, i + 1);
	}
	
	vector<vector<T>> sentenceNgrams = getNgramsVector(sentenceTokens, nSize);

	double V = allGrams[1].size() + 1;
	double bottom = getBottom(V, N, nSize, delta); 

	double totalProb = 0;
	
	for (int i = 0; i < sentenceNgrams.size() ; i++) {
		vector<T> nGram = sentenceNgrams[i];

		bottom = getBottom(V, N, nSize, delta);
		
		double topProb = getProb(nGram, allGrams[nSize], bottom, delta);
		
		nGram.pop_back();

		bottom = getBottom(V, N, nSize-1, delta);
		double bottomProb = getProb(nGram, allGrams[nSize - 1], bottom, delta);

		if (nSize == 1) {
			bottomProb = 1;
		}

		if (topProb != 0 && bottomProb != 0) {
			totalProb += log(topProb / bottomProb);
		}
		else {
			totalProb = -DBL_MAX;
			break;
		}
	}

	vector<T> nGram = sentenceNgrams[0];

	for (int j = 0; j < (nGram.size() - 1); j++) {
		vector<T> tempGram;

		for (int i = 0; i <= j; i++) {
			tempGram.push_back(nGram[i]);
			cout << nGram[i] << " ";
		}
		bottom = getBottom(V, N, j+1, delta);
		double topProb = getProb(tempGram, allGrams[j+1], bottom, delta);
		double bottomProb = 1.0;

		if (j > 0) {
			tempGram.pop_back();
			bottom = getBottom(V, N, j, delta);
			bottomProb = getProb(tempGram, allGrams[j], bottom, delta);
		}

		if (topProb != 0 && bottomProb != 0) {
			totalProb += log(topProb / bottomProb);
		}
		else {
			totalProb = -DBL_MAX;
			break;
		}
	}

	cout << totalProb << endl;
}


int main(int argc, char* argv[]) {
	string file1 = string(argv[1]);
	string file2 = string(argv[2]);
	int size = atoi(argv[3]);
	double delta = atof(argv[4]);
	p4(file1, file2, size, delta);
	return 1;
}
