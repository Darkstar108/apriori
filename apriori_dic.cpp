#include<iostream>
#include<fstream>
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<bits/stdc++.h>
using namespace std;

int T, Mparts, M;
double minSupport;
double minConfidence;
vector<vector<int> > transactions;
vector<vector<int> > C;
vector<int> Cfreq;
vector<vector<int> > L;
vector<int> Lfreq;
vector<int> Mcount;
vector<vector<int> > posFreqSet;
vector<vector<vector<int> > > frequentSet;
vector<vector<int> > frequentSetCount;

void printVector(vector<int> T) {
	for(int i = 0; i < T.size(); ++i)
		cout<<T[i] <<" ";
}

void print(vector<vector<int> > T) {
	for(int i = 0; i < T.size(); ++i) {
		for(int j = 0; j < T[i].size(); ++j)
			cout<<T[i][j] <<" ";
		cout<<"\n";
	}
	cout<<"\n";
}

// Return complement of subset S
vector<int> getSubsetComplement(vector<int> itemset, vector<int> subset) {
	vector<int> comp;
	int p = 0, m;
	for(m = 0; m < itemset.size(); ++m) {
		if(p == subset.size()) {
			comp.push_back(itemset[m]);
			continue;
		}
		if(subset[p] == itemset[m]) {
			p++;
			continue;
		}
		else {
			comp.push_back(itemset[m]);
			continue;
		}
	}
	return comp;
}

// Generate subsets of length k from given itemset
vector<vector<int> > generateKlengthSubset(vector<int> itemset, int k) {
	vector<vector<int> > res;
	vector<int> s;
	int i, j, l;
	for(i = 0; i < itemset.size()-k+1; ++i) {
		l = 0;
		while(l != k-1) {
			s.push_back(itemset[i+l]);
			l++;
		}
		for(j = i+l; j < itemset.size(); ++j) {
			s.push_back(itemset[j]);
			l++;
			if(l == k) {
				res.push_back(s);
				s.pop_back();
				l--;
			}
		}
		s.clear();
		if(k == 1)
			break;
	}
	return res;
}

// Get support of itemset
double getFrequency(vector<int> itemset) {
	int freq = 0, i, j, k;
	for(i = 0; i < transactions.size(); ++i) {
		k = 0;
		for(j = 0; j < transactions[i].size(); ++j) {
			if(itemset.size() > transactions[i].size())
				break;
			if(k == itemset.size()) 
				break;
			if(itemset[k] == transactions[i][j])
				k++;
		}
		if(k == itemset.size()) 
			freq++;
	}
	return freq;
}

// Generate Association Rules
void generateAssociationRules() {
	fstream outputFile;
	outputFile.open("association.txt", ios::out);
	vector<vector<int> > s;
	vector<int> sComplement;
	double conf, supS, supL, freq;
	int i, j, k, l, m, p, ktmp, ltmp;
	
	for(i = 1; i < frequentSet.size(); ++i) {
		for(j = 0; j < frequentSet[i].size(); ++j) {
			printVector(frequentSet[i][j]);
			for(k = 1; k < frequentSet[i][j].size(); ++k) {
				cout<<"\n" <<k <<":\n";
				s = generateKlengthSubset(frequentSet[i][j], k);
				cout<<"A\n";
				print(s);
				for(l = 0; l < s.size(); ++l) {
					sComplement = getSubsetComplement(frequentSet[i][j], s[l]);
					// Getting Confidence values
					freq = getFrequency(s[l]);
					supS = (double) freq/T;
					supS = supS*100;
					freq = getFrequency(frequentSet[i][j]);
					supL = (double) freq/T;
					supL = supL*100;
					if(supS == 0 || supL == 0)
						break;
					conf = (supL/supS)*100;
					// Printing to File
					if(conf >= minConfidence) {
						for(p = 0; p < s[l].size(); ++p)
							outputFile<<s[l][p] <<" ";
						outputFile<<" ---> ";
						for(p = 0; p < sComplement.size(); ++p)
							outputFile<<sComplement[p] <<" ";
						outputFile<<"\t" <<conf <<"\n";
					}
					printVector(s[l]);
					cout<<"\t";
					printVector(sComplement);
					cout<<" " <<conf <<"\n";
					sComplement.clear();
				}
				s.clear();
			}
			cout<<"\n";
		}
	}
	outputFile.close();
}

// Prints frequent itemsets and their support to a file
void printFrequentItemset(char* outputFile) {
	fstream transactionFile;
	transactionFile.open(outputFile, ios::out);
	vector<vector<int> > tmp;
	vector<int> tmpFreq;
	int i, j, k, maxSize = 1;
	// Copy confirmed frequent sets from L to frequentSet in ascending order of size of itemset
	for(i = 1; i <= maxSize; ++i) {
		for(j = 0; j < L.size(); ++j) {
			if(L[j].size() > maxSize)
				maxSize = L[j].size();
			if(L[j].size() == i) {
				tmp.push_back(L[j]);
				tmpFreq.push_back(Lfreq[j]);
			}	
		}
		frequentSet.push_back(tmp);
		frequentSetCount.push_back(tmpFreq);
		tmp.clear();
		tmpFreq.clear();
	}
	// Print frequentSet to file
	for(i = 0; i < frequentSet.size(); ++i) {
		for(j = 0; j < frequentSet[i].size(); ++j) {
			for(k = 0; k < frequentSet[i][j].size(); ++k) 
				transactionFile<<frequentSet[i][j][k] <<" ";
			transactionFile<<" = " <<frequentSetCount[i][j] <<"\n";
		}
	}
	transactionFile.close();
}

// Get Transactions from file and store in vector<vector<int> > transactions
void getTransactions(char* inputFile) {
	fstream transactionFile;
	transactionFile.open(inputFile, ios::in);
	int i, Iid;
	vector<int> tmp;
	while(!transactionFile.eof()) {
		transactionFile>>Iid >>Iid;
		tmp.clear();
		while(Iid != -1) {
			tmp.push_back(Iid);
			transactionFile>>Iid;
		}
		transactions.push_back(tmp);
	}
	T = transactions.size();
	transactionFile.close();
}

// Check if given itemset is in given transaction
int checkItemsetInTransaction(vector<int> transaction, vector<int> itemset) {
	int i = 0, j = 0;
	if(itemset.size() > transaction.size())
		return 0;
	sort(transaction.begin(), transaction.end());
	while(i != transaction.size() && j != itemset.size()) {
		if(transaction[i] == itemset[j])
			++j;
		i++;
	}
	if(j != itemset.size())
		return 0;
	else
		return 1;
}

// Update Frequency of all itemsets if they are in given transaction
void updateFrequency(vector<int> transaction) {
	int i, j;
	for(i = 0; i < C.size(); ++i) {
		if(checkItemsetInTransaction(transaction, C[i]) == 1)
			Cfreq[i]++;
	}
}

// Go through M transactions at a time
void traverseMtransactions(int part) {
	int start = part*M, end, i;
	if(T < start+2*M)
		end = T;
	else
		end = start + M;
	for(i = start; i < end; ++i) {
		updateFrequency(transactions[i]);
	}
	for(i = 0; i < Mcount.size(); ++i)
		Mcount[i]++;
}

// Check if subset is in vector of itemsets(C, L, posFreqSet)
int isSubsetInItemsetVector(vector<int>subset, vector<vector<int> > itemsets) {
	int i, j, k, flag;
	for(i = 0; i < itemsets.size(); ++i) {
		if(subset.size() > itemsets[i].size())
			continue;
		flag = 1;
		k = 0;
		for(j = 0; j < itemsets[i].size(); ++j) {
			if(itemsets[i][j] != subset[k]) {
				flag = 0;
				break;
			}
			k++;
		}
		if(flag == 1)
			return i;
	}
	return -1;
}

// Make sure all k-1 subsets of itemset is frequent
int pruneItemset(vector<int> itemset) {
	if(itemset.size() < 3)
		return 1;
	int i, cIndex;
	double support;
	vector<vector<int> > subsets;
	subsets = generateKlengthSubset(itemset, itemset.size()-1);
	for(int i = 0; i < subsets.size(); ++i) {
		cIndex = isSubsetInItemsetVector(subsets[i], C);
		if(cIndex > -1) {
			support = (double) Cfreq[cIndex]/T;
			support = support*100;
			if(support < minSupport)
				return 0;
		}
		else {
			if(isSubsetInItemsetVector(subsets[i], L) == -1)
				return 0;
		}
	}
	return 1;
}

// Joins possibly frequent k itemsets to get k+1 itemsets
void joinFrequentItems(vector<vector<int> > posFreqSet) {
	int i, j, K, k;
	vector<int> tmp;
	for(i = 0; i < posFreqSet.size(); ++i) {
		for(j = 0; j < posFreqSet.size(); ++j) {
			if(posFreqSet[i].size() != posFreqSet[j].size())
				break;
			K = posFreqSet[i].size()+1;
			k = K-2;
			if(K > 2) {
				for(k = 0; k < K-2; ++k) {
					if(posFreqSet[i][k] == posFreqSet[j][k])
						tmp.push_back(posFreqSet[i][k]);
					else
						break;
				}	
			}
			if(k < K-2)
				break;
			int a = posFreqSet[i][K-2];
			int b = posFreqSet[j][K-2];
			if(a < b) {
				tmp.push_back(a);
				tmp.push_back(b);
			} 
			else {
				tmp.push_back(b);
				tmp.push_back(a);
			}
			if(isSubsetInItemsetVector(tmp, C) == -1) {
				if(pruneItemset(tmp) == 1) {
					printVector(tmp);
					cout<<"\n";
					C.push_back(tmp);
					Cfreq.push_back(0);
					Mcount.push_back(0);
				}
			}
			tmp.clear();
		}
	}
}

// Check if itemsets have finished reading all transactions or if itemsets are possibly frequent
void dynamicItemsetChecking() {
	int i, cSize = C[0].size();
	double support;
	vector<vector<int> >::iterator CItr = C.begin();
	vector<int>::iterator CfreqItr = Cfreq.begin();
	vector<int>::iterator McountItr = Mcount.begin();
	
	for(i = 0; i < C.size(); ++i) {	
		support = (double) Cfreq[i]/T;
		support = support*100;
		if(support >= minSupport) 
			posFreqSet.push_back(C[i]);
			
		// If itemset has gone through all transactions
		if(Mcount[i] == 4) {
			if(support >= minSupport) {
				L.push_back(C[i]);
				Lfreq.push_back(Cfreq[i]);
			}
			C.erase(CItr+i);
			Cfreq.erase(CfreqItr+i);
			Mcount.erase(McountItr+i);
			i--;
		}
	}
	// Add confirmed frequent sets along with possibly frequent sets
	for(i = 0; i < L.size(); ++i) {
		if(isSubsetInItemsetVector(L[i], posFreqSet) == -1 && L[i].size() >= cSize) {
			posFreqSet.push_back(L[i]);
		}
	}
	cout<<"\nPossibly FreqSet:\n";
	print(posFreqSet);
	joinFrequentItems(posFreqSet);
	posFreqSet.clear();
}

// Main process of Apriori
void aprioriProcess(char* inputFile, char* outputFile) {
	getTransactions(inputFile);
	int part = 0, debug = 0;
	Mparts = 4;
	M = T/Mparts;
	set<int> S;
	for(int i = 0; i < transactions.size(); ++i) 
		for(int j = 0; j < transactions[i].size(); ++j)
			S.insert(transactions[i][j]);
	for(set<int>::iterator itr = S.begin(); itr != S.end(); ++itr) {
		C.push_back(vector<int>(1, *itr));
		Cfreq.push_back(0);
		Mcount.push_back(0);
	}
	while(true) {
		traverseMtransactions(part);
		cout<<"Part: " <<part <<"\nC: \n";
		print(C);
		cout<<"Frequencies: ";
		printVector(Cfreq);
		cout<<"\nMcounts: ";
		printVector(Mcount);
		dynamicItemsetChecking();
		cout<<"\nL: \n";
		print(L);
		cout<<"Frequencies: ";
		printVector(Lfreq);
		cout<<"\n";
		part = (part+1)%Mparts;
		debug++;
		if(C.empty())
			break;
	}
	printFrequentItemset(outputFile);
}

int main() {
	char* inputFile = "sampleInput.txt";
	char* outputFile = "sampleOutput.txt";
	minSupport = 22.0;
	minConfidence = 50.0;
	aprioriProcess(inputFile, outputFile);
	generateAssociationRules();
	return 0;
}

