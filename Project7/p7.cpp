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
typedef unordered_map<vector<T>, int> gramCounts;


void printNgram(vector<T>nGram) {
	for (int i = 0; i < nGram.size(); i++) {
		cout << nGram[i] << " ";
	}
}

vector<vector<T>> createSentences(vector<T> checkTokens) {
	vector<vector<T>> checkSentences;
	vector<T> sentence;
	for (int i = 0; i < checkTokens.size(); i++) {
		
		if (checkTokens[i] != "<END>") {
			sentence.push_back(checkTokens[i]);
		}
		else {
			checkSentences.push_back(sentence);
			sentence.clear();
		}
	}

	return checkSentences;
}

unordered_map<vector<T>, vector<vector<T>>> createAllCandidateSentences(vector<T> dictTokens, vector<vector<T>> checkSentences) {
	
	unordered_map<vector<T>, vector<vector<T>>> allCandidateSentences;
	
	for (int i = 0; i < checkSentences.size(); i++) {

		vector<T> sentence = checkSentences[i];
		vector<vector<T>> candidateSentences;
		candidateSentences.push_back(sentence);

		for (int j = 0; j < sentence.size(); j++) {
			string sentenceWord = sentence[j];
			
			for (int k = 0; k < dictTokens.size(); k++) {
				
				unsigned int distance = uiLevenshteinDistance(sentenceWord, dictTokens[k]);
				if (distance <= 1) {
					vector<T> newSentence(sentence.begin(), sentence.end());
					newSentence[j] = dictTokens[k];
					candidateSentences.push_back(newSentence);
				}
			}
			allCandidateSentences[sentence] = candidateSentences;
		}
	}

	return allCandidateSentences;
}

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


double getBottom(double V, double tokenSize, int nSize, double delta) {
	double B = pow(V, nSize);
	double bottom = tokenSize + B * delta;
	return bottom;
}

double getProb(vector<T> nGram, unordered_map<vector<T>, int> nGrams, double bottom, double delta) {
	double prob;

	double count = nGrams[nGram];
	double top = count + delta;

	prob = top / bottom;

	return prob;
}

double getSentenceProb(unordered_map<int, gramCounts> allGrams, vector<T> sentenceTokens, double delta, int nSize, double N) {
	vector<vector<T>> sentenceNgrams = getNgramsVector(sentenceTokens, nSize);
	double V = allGrams[1].size() + 1;
	double bottom = getBottom(V, N, nSize, delta);

	double totalProb = 0;

	for (int i = 0; i < sentenceNgrams.size(); i++) {
		vector<T> nGram = sentenceNgrams[i];

		bottom = getBottom(V, N, nSize, delta);
		double topProb = getProb(nGram, allGrams[nSize], bottom, delta);
		double bottomProb = 1;

		if (nSize > 1) {
			nGram.pop_back();
			bottom = getBottom(V, N, nSize - 1, delta);
			bottomProb = getProb(nGram, allGrams[nSize - 1], bottom, delta);
		}


		if (topProb != 0 && bottomProb != 0) {
			totalProb += log(topProb / bottomProb);
		}
		else {
			totalProb = -DBL_MAX;
			cout << totalProb << endl;
			return totalProb;
		}
	}

	vector<T> nGram = sentenceNgrams[0];

	for (int j = 0; j < (nGram.size() - 1); j++) {
		vector<T> tempGram;

		for (int i = 0; i <= j; i++) {
			tempGram.push_back(nGram[i]);
		}

		bottom = getBottom(V, N, j + 1, delta);
		double topProb = getProb(tempGram, allGrams[j + 1], bottom, delta);
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
			cout << totalProb << endl;
			return totalProb;
		}
	}

	return totalProb;
}

unordered_map<vector<T>, vector<T>> getLikelyCandidates(vector<vector<T>> checkSentences, unordered_map<vector<T>, vector<vector<T>>> allCandidateSentences, unordered_map<int, unordered_map<vector<T>, int>> allGrams, double delta, int nSize, double N) {
	unordered_map<vector<T>, vector<T>>  likelyCandidateSentences;
	for (int i = 0; i < checkSentences.size(); i++) {
		vector<T> sentence = checkSentences[i];
		vector<vector<T>> candidateSentences = allCandidateSentences[sentence];

		double maxProb = NULL;
		cout << "size: " << candidateSentences.size() << endl;
		for (int j = 0; j < candidateSentences.size(); j++) {
			vector<T> candidateSentence = candidateSentences[j];
			double prob = getSentenceProb(allGrams, candidateSentence, delta, nSize, N);
		
			if (maxProb == NULL) {
				maxProb = prob;
				likelyCandidateSentences[sentence] = candidateSentence;
			}
			else if (maxProb < prob) {
				maxProb = prob;
				likelyCandidateSentences[sentence] = candidateSentence;
			}
		}
	}

	return likelyCandidateSentences;
}




void p7(string trainFile, string checkFile, string dictionary, int nSize, int threshold, double delta, int model) {
	vector<T> checkTokens;
	vector<T> dictTokens;
	vector<T> trainTokens;
	vector<vector<T>> checkSentences;
	unordered_map<int, unordered_map<vector<T>, int>> allGrams;

	read_tokens(checkFile, checkTokens, true);
	read_tokens(dictionary, dictTokens, false);
	read_tokens(trainFile, trainTokens, false);

	double N = trainTokens.size();

	for (int i = 0; i < nSize; i++) {
		allGrams[i + 1] = getNgrams(trainTokens, i + 1);
	}

	unordered_map<vector<T>, vector<vector<T>>> allCandidateSentences;
	unordered_map<vector<T>, vector<T>>  likelyCandidateSentences;

	checkSentences = createSentences(checkTokens);

	allCandidateSentences = createAllCandidateSentences(dictTokens, checkSentences);
	
	likelyCandidateSentences = getLikelyCandidates(checkSentences, allCandidateSentences, allGrams, delta, nSize, N);
	

	for (auto i = likelyCandidateSentences.begin(); i != likelyCandidateSentences.end(); i++) {
		vector<T> candidateSentence = i->second;
		printNgram(candidateSentence);
		cout << endl;
	}
	

}

int main(int argc, char* argv[]) {
	string trainFile = string(argv[1]);
	string checkFile = string(argv[2]);
	string dictionary = string(argv[3]);
	int nSize = atoi(argv[4]);
	int threshold = atoi(argv[5]);
	double delta = atof(argv[6]);
	int model = atoi(argv[7]);

	printf("\n\n\n 2 3 1 1\n");
	p7(trainFile, checkFile, dictionary, 2, 3, 1, 1);
	printf("\n\n\n 2 3 0.1 1\n");
	p7(trainFile, checkFile, dictionary, 2, 3, 0.1, 1);
	printf("\n\n\n 2 3 0.01 1\n");

	p7(trainFile, checkFile, dictionary, 2, 3, 0.01, 1);

	printf("\n\n\n 1 3 0.01 1\n");

	p7(trainFile, checkFile, dictionary, 1, 3, 0.01, 1);
	printf("\n\n\n 2 3 0.01 1\n");

	p7(trainFile, checkFile, dictionary, 2, 3, 0.01, 1);
	printf("\n\n\n 3 3 0.01 1\n");

	p7(trainFile, checkFile, dictionary, 3, 3, 0.01, 1);
	printf("\n\n\n 1 3 1 0\n");

	p7(trainFile, checkFile, dictionary, 1, 3, 1, 0);
	printf("\n\n\n 2 3 1 0\n");

	p7(trainFile, checkFile, dictionary, 2, 3, 1, 0);
	printf("\n\n\n 3 3 1 0\n");

	p7(trainFile, checkFile, dictionary, 3, 3, 1, 0);

	return 1;
}