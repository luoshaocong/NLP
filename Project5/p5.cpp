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

void printNgram(vector<T>nGram) {
	for (int i = 0; i < nGram.size(); i++) {
		cout << nGram[i] << " ";
	}

}
unordered_map<int, unordered_map<int, int>> getAllNgramRates(unordered_map<int, unordered_map<vector<T>, int>> allGrams, double vocabSize) {
	unordered_map<int, unordered_map<int, int>> allNgramRates;

	for (int i = 0; i < allGrams.size(); i++) {
		
		unordered_map<vector<T>, int> nGrams = allGrams[i + 1];
		unordered_map<int, int> rates;
		int sumKnowns = 0;

		for (auto j = nGrams.begin(); j != nGrams.end(); ++j) {

			int count = j->second;
			if (rates.count(count) == 0) {
				rates[count] = 1;
				sumKnowns++;
			}
			else {
				rates[count] = rates[count] + 1;
				sumKnowns++;
			}
		}

		rates[0] = pow(vocabSize, i + 1) - sumKnowns;
		allNgramRates[i + 1] = rates;
	}
	return allNgramRates;
}

double getGoodTuringProb(double rate, double Nr1, double Nr, double N) {
	double prob = (rate + 1)*Nr1 / (N*Nr);
	return prob;
}


unordered_map<int, vector<double>> getAllGoodTuringProbs(unordered_map<int, unordered_map<int, int>> allNgramRates, int tokensSize, int threshold) {

	unordered_map<int, vector<double>> allNgramRatesProb;

	for (int j = 1; j <= allNgramRates.size(); j++) {

		unordered_map<int, int> nGramRates = allNgramRates[j];
		vector<double> rateProbs;
		double N = tokensSize;
		double sumKnowns = 0;
		for (int i = 0; i < nGramRates.size(); i++) {
			double prob;
			if (nGramRates.count(i) != 0 && nGramRates.count(i + 1) != 0) {
				double Nr = nGramRates[i];
				double Nr1 = nGramRates[i + 1];

				prob = getGoodTuringProb(i, Nr1, Nr, N);
				rateProbs.push_back(prob);
				if (i != 0) {
					sumKnowns += prob;
				}

				if (rateProbs.size() == (threshold)) {
					break;
				}
			}
			else {
				break;
			}
		}

		allNgramRatesProb[j] = rateProbs;
	}
	return allNgramRatesProb;
}

unordered_map<int, unordered_map<vector<T>, double>> getAllMleProbs(unordered_map<int, unordered_map<vector<T>, int>> allGrams, unordered_map<int, vector<double>> allGoodTuringProbs, int tokensSize) {
	unordered_map<int, unordered_map<vector<T>, double>> allMleProbs;
	for (int i = 1; i <= allGrams.size(); i++) {
		unordered_map<vector<T>, int> nGrams = allGrams[i];
		vector<double> gtProbs = allGoodTuringProbs[i];

		unordered_map<vector<T>, double> mleProbs;
		double N = tokensSize;

		
		
		for (auto j = nGrams.begin(); j != nGrams.end(); j++) {
			vector<T> gram = j->first;
			if (j->second >= gtProbs.size()) {
				double mleProb = double(j->second) / N;
	
				mleProbs[gram] = mleProb;
			}
			else {
			}
		}
		allMleProbs[i] = mleProbs;
	}

	return allMleProbs;
}

vector<double> getNormFactors(unordered_map<int, vector<double>> allGoodTuringProbs, unordered_map<int, unordered_map<vector<T>, double>> allMleProbs, unordered_map<int, unordered_map<vector<T>, int>> allGrams, unordered_map<int, unordered_map<int, int>> allNgramRates) {
	vector<double> normFactors;

	for (int i = 1; i <= allGrams.size(); i++) {
		unordered_map<vector<T>, int> nGrams = allGrams[i];
		vector<double> gtProbs = allGoodTuringProbs[i];

		double numUnseen = allNgramRates[i][0];
		double totalUnseenProb = 0;
		if (gtProbs.size() > 0) {
			totalUnseenProb = gtProbs[0] * numUnseen;
		}
		

		double idealSeenProb = 1 - totalUnseenProb;

		unordered_map<vector<T>, double> mleProbs = allMleProbs[i];

		double seenProbsSum = 0;

		for (auto j = nGrams.begin(); j != nGrams.end(); j++) {
			vector<T> gram = j->first;
			if (j->second >= gtProbs.size()) {
				seenProbsSum += mleProbs[gram];
			}
			else {
				seenProbsSum += gtProbs[j->second];
			}
		}
		double y = idealSeenProb / seenProbsSum;

		normFactors.push_back(y);
	}


	return normFactors;
}

struct Quotient {
	vector<T> numerator;
	vector<T> denominator;
};

void p5(string textModel, string sentence, int nSize, double threshold) {

	vector<T> tokens;
	vector<T> sentenceTokens;
	read_tokens(textModel, tokens, false);
	read_tokens(sentence, sentenceTokens, false);

	vector<vector<T>> sentenceNgrams = getNgramsVector(sentenceTokens, nSize);

	unordered_map<int, unordered_map<vector<T>, int>> allGrams;
	unordered_map<int, unordered_map<int, int>> allNgramRates;
	unordered_map<int, vector<double>> allGoodTuringProbs;
	unordered_map<int, unordered_map<vector<T>, double>> allMleProbs;


	for (int i = 0; i < nSize; i++) {
		allGrams[i + 1] = getNgrams(tokens, i + 1);
	}

	double vocabSize = allGrams[1].size() + 1;

	allNgramRates = getAllNgramRates(allGrams, vocabSize);


	allGoodTuringProbs = getAllGoodTuringProbs(allNgramRates, tokens.size(), threshold);

	allMleProbs = getAllMleProbs(allGrams, allGoodTuringProbs, tokens.size());

	vector<double> normFactors = getNormFactors(allGoodTuringProbs, allMleProbs, allGrams, allNgramRates);

	vector<T> firstGram = sentenceNgrams[0];
	vector<Quotient> quotients;

	for (int i = firstGram.size() - 1; i > 0; i--) {
		Quotient q;
		vector<T> numerator(firstGram.begin(), firstGram.end() - i);
		vector<T> denominator(firstGram.begin(), firstGram.end() - i - 1);

		q.numerator = numerator;
		if (denominator.size() > 0)
		{
			q.denominator = denominator;
		}

		quotients.push_back(q);
	}

	for (int i = 0; i < sentenceNgrams.size(); i++) {
			Quotient q;
			vector<T> nGram = sentenceNgrams[i];
			vector<T> smGram = nGram;
			smGram.pop_back();

			q.denominator = smGram;
			q.numerator = nGram;
			quotients.push_back(q);
	}

	double totalProb = 0;

	for (int i = 0; i < quotients.size(); i++) {
		vector<T> numerator = quotients[i].numerator;
		vector<T> denominator = quotients[i].denominator;

		vector<double> numdumProbs;
		vector<vector<T>> numdum;

		numdum.push_back(numerator);
		if (denominator.size() > 0) {
			numdum.push_back(denominator);
		}


		for (int j = 0; j < numdum.size(); j++) {

			int numSize = numdum[j].size();
			double prob;
			double y = normFactors[numSize - 1];

			int rate = allGrams[numSize][numdum[j]];
			vector<double> gtProbsList = allGoodTuringProbs[numSize];
			unordered_map<vector<T>, double> mleProbs = allMleProbs[numSize];

			if (rate < gtProbsList.size()) {
				prob = gtProbsList[rate];
				if (rate != 0) {
					prob = prob* y;
				}
				numdumProbs.push_back(prob);
			} else {
				prob = mleProbs[numdum[j]];
				prob = prob* y;
				numdumProbs.push_back(prob);
			}

		}
		
		if (numdum.size() == 1) {
			totalProb += log(numdumProbs[0]);
		}
		else {
			totalProb += log(numdumProbs[0] / numdumProbs[1]);
		}
	}

	cout << totalProb << endl;
	
}

int main(int argc, char* argv[]) {
	string file1 = string(argv[1]);
	string file2 = string(argv[2]);
	int size = atoi(argv[3]);
	double threshold = atof(argv[4]);
	p5(file1, file2, size, threshold);
	return 1;
}