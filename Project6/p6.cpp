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

typedef char T;
typedef unordered_map<vector<T>, int> gramCounts;
typedef vector<T> gram;

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

vector<vector<gram>> getAllTestGrams(vector<gram>testTokens, string testNames[], int n) {
	vector<vector<gram>> testGramsCollection;
	
	for (int i = 0; i < testTokens.size(); i++) {
		vector<gram> testNgrams = getNgramsVector(testTokens[i], n);
		testGramsCollection.push_back(testNgrams);
	}
	
	return testGramsCollection;
}

vector<vector<T>>createSentences(vector<T> tokens, double senLength) {
	auto it = tokens.begin();
	int numTimes = floor(double(tokens.size()) / senLength);
	vector<vector<T>> allSentences;
	for (int i = 0; i < numTimes; i++) {
		auto itNext = it + senLength;
		vector<T> sentence(it, itNext);
		it = itNext;
		allSentences.push_back(sentence);
	}

	return allSentences;
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
	double V = 256;
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
		} else {
			totalProb = -DBL_MAX;
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
		} else {
			totalProb = -DBL_MAX;
			cout << totalProb << endl;
			return totalProb;
		}
	}

	return totalProb;
}

void printConfusionMatrix(vector<vector<int>> confusionMatrix) {
	cout << endl;
	for (int i = 0; i < confusionMatrix.size(); i++) {
		vector<int> confRow = confusionMatrix[i];

		for (int j = 0; j < confRow.size(); j++) {
			cout << confRow[j] <<" ";
		}
		cout << endl;
	}
}

void p6(int nSize, double delta, int senLength) {
	try {
		unordered_map <string, vector<T>> fileTokens;
		unordered_map <string, vector<vector<T>>> allTestSentences;
		unordered_map<string, unordered_map<int, gramCounts>> allGramsCollection;
		unordered_map < string, unordered_map<int, tuple<double, int>>> allSentenceProbs;

		vector<vector<int>> confusionMatrix;

		string prefix = "..\\..\\Texts\\Languages\\";
		const int numLangs = 6;
		string fileNames[numLangs] = { "sweedish", "italian", "latin", "english", "french", "danish" };

		for (int i = 0; i < numLangs; i++) {
			vector<T> tokens;
			unordered_map<int, gramCounts> allGrams;
			read_tokens(prefix+fileNames[i]+"1.txt", tokens, false);
			fileTokens[fileNames[i]] = tokens;

			for (int i = 0; i < nSize; i++) {
				allGrams[i + 1] = getNgrams(tokens, i + 1);
			}

			allGramsCollection[fileNames[i]] = allGrams;
		}
		 
		for (int i = 0; i < numLangs; i++) {
			vector<T> tokens;
			read_tokens(prefix + fileNames[i] + "2.txt", tokens, false);
			allTestSentences[fileNames[i]] = createSentences(tokens, senLength);
		}

		for (int langId = 0; langId < numLangs; langId++) {
			vector<vector<T>> sentences = allTestSentences[fileNames[langId]];	

			for (int i = 0; i < numLangs; i++) {
				string languageName = fileNames[i];
				unordered_map<int, gramCounts> allGrams = allGramsCollection[languageName];
				vector<T> tokens = fileTokens[languageName];
				double N = tokens.size();
				
				for (int j = 0; j < sentences.size(); j++) {
					vector<T> sentence = sentences[j];
					double newProb = getSentenceProb(allGrams, sentence, delta, nSize, N);
					
					if (i == 0) {
						allSentenceProbs[fileNames[langId]][j] = make_tuple(newProb, i);
					}
					else {
						double oldProb = get<0>(allSentenceProbs[fileNames[langId]][j]);
						if (oldProb < newProb) {
							allSentenceProbs[fileNames[langId]][j] = make_tuple(newProb, i);
						}
					}
					
				}
			}
		}

		double numIncorrect = 0;
		double numClassified = 0;

		for (int langId = 0; langId < numLangs; langId++) {
			unordered_map<int, tuple<double, int>>sentenceProbs = allSentenceProbs[fileNames[langId]];
			vector<int> confRow(numLangs, 0);

			for (int j = 0; j < sentenceProbs.size(); j++) {
				numClassified++;
				string actualLanguage = fileNames[langId];
				int predictedLangId = get<1>(sentenceProbs[j]);
				string predictedLanguage = fileNames[predictedLangId];
				confRow[predictedLangId]++;

				if (predictedLanguage != actualLanguage) {
					numIncorrect++;
				}
			}

			confusionMatrix.push_back(confRow);

		}

		double percentWrong = 100*  numIncorrect / numClassified;
		cout << percentWrong << endl;
		printConfusionMatrix(confusionMatrix);

	} catch (FileReadException e) {
		e.Report();
	}
}

int main(int argc, char* argv[]) {
	int n = atoi(argv[1]);
	double delta = atof(argv[2]);
	int senLength = atoi(argv[3]);



	cout << 1 << " " << 0.05 << " " << 50 << endl;
	p6(1, 0.05, 50);
	cout << 2 << " " << 0.05 << " " << 50 << endl;
	p6(2, 0.05, 50);
	cout << 3 << " " << 0.05 << " " << 50 << endl;
	p6(3, 0.05, 50);


	cout << 1 << " " << 0 << " " << 50 << endl;
	p6(1, 0, 50);
	cout << 2 << " " << 0 << " " << 50 << endl;
	p6(2, 0, 50);
	cout << 3 << " " << 0 << " " << 50 << endl;
	p6(3, 0, 50);

	cout << 3 << " " << 0.05 << " " << 50 << endl;
	p6(3, 0.05, 50);
	cout << 3 << " " << 0.005 << " " << 50 << endl;
	p6(3, 0.005, 50);
	cout << 3 << " " << 0.0005 << " " << 50 << endl;
	p6(3, 0.0005, 50);

	cout << 2 << " " << 0.05 << " " << 50 << endl;
	p6(2, 0.05, 50);
	cout << 2 << " " << 0.05 << " " << 50 << endl;
	p6(2, 0.05, 50);
	cout << 2 << " " << 0.05 << " " << 50 << endl;
	p6(2, 0.05, 50);

	return 1;
}