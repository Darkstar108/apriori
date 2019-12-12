#include<iostream>
#include<fstream>
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<bits/stdc++.h>
using namespace std;

int T, parts = 3, partitionSize;
double minSupport;
double minConfidence;
vector<vector<int> > transactions;
vector<vector<int> > C;
vector<vector<int> > L;
vector<vector<vector<int> > > candidateSet;
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
double getFrequency(vector<int> itemset, int part = 0, int size = 63000) {
	int freq = 0, i, j, k;
	double support;
	for(i = part*partitionSize; i < transactions.size() && (i - part*partitionSize) < size; ++i) {
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

// Check if subset is in L
bool isSubsetInL(vector<int>subset) {
	int i, j, k, flag;
	for(i = 0; i < L.size(); ++i) {
		flag = 1;
		k = 0;
		for(j = 0; j < L[i].size(); ++j) {
			if(L[i][j] != subset[k]) {
				flag = 0;
				break;
			}
			k++;
		}
		if(flag == 1)
			return true;
	}
	return false;
}

// Check if every (k-1)subset of k itemset is in L
bool prune(vector<int> itemset, int K) {
	if(K < 3)
		return true;
	vector<int> subset;
	int i, j;
	for(i = 0; i < itemset.size(); ++i) {
		subset.clear();
		for(j = 0; j < itemset.size(); ++j) {
			if(j != i)
				subset.push_back(itemset[j]);
		}
		if(!isSubsetInL(subset))
			return false;
	}
	return true;
}

// Uses L(k-1) to join (k-1)itemsets and form k itemsets
void join(int K) {
	int i, j, k = K-2;
	vector<int> tmp;
	for(i = 0; i < L.size(); ++i) {
		for(j = i+1; j < L.size(); ++j) {
			if(K > 2) {
				for(k = 0; k < K-2; ++k) {
					if(L[i][k] == L[j][k])
						tmp.push_back(L[i][k]);
					else
						break;
				}	
			}
			if(k < K-2)
				break;
			int a = L[i][K-2];
			int b = L[j][K-2];
			if(a < b) {
				tmp.push_back(a);
				tmp.push_back(b);
			} 
			else {
				tmp.push_back(b);
				tmp.push_back(a);
			}
			if(prune(tmp, K))
				C.push_back(tmp);
			tmp.clear();
		}
	}
}

// Generate's C itemset
void generateC(int K, int part, int size) {
	C.clear();
	if(K == 1) {
		set<int> S;
		for(int i = part*partitionSize; (i - part*partitionSize) < size; ++i) 
			for(int j = 0; j < transactions[i].size(); ++j)
				S.insert(transactions[i][j]);
		for(set<int>::iterator itr = S.begin(); itr != S.end(); ++itr)
			C.push_back(vector<int>(1, *itr));
	}
	else {
		join(K);
	}
	cout<<K <<" C:\n";
	print(C);
}

// Prints itemset and their support to a file
void printItemset(char* outputFile, vector<vector<vector<int> > > itemset, int printSupport = 0) {
	fstream transactionFile;
	transactionFile.open(outputFile, ios::out);
	int i, j, k;
	for(i = 0; i < itemset.size(); ++i) {
		for(j = 0; j < itemset[i].size(); ++j) {
			for(k = 0; k < itemset[i][j].size(); ++k) 
				transactionFile<<itemset[i][j][k] <<" ";
			if(printSupport == 1)
				transactionFile<<" = " <<frequentSetCount[i][j];
			transactionFile<<"\n";
		}
	}
	transactionFile.close();
}

// Generate L, which is C after removing infrequent itemsets
void generateL(int K, int part, int size) {
	int i;
	double support, freq;
	L.clear();
	for(i = 0; i < C.size(); ++i) {
		freq = getFrequency(C[i], part, size);
		support = (double) freq/size;
		support = support*100;
		if(support >= minSupport) {
			L.push_back(C[i]);
		}
	}
	candidateSet.push_back(L);
	cout<<"L:\n";
	print(L);
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

// Main process of Apriori
void aprioriProcess(int part, int size) {
	cout<<"Part: " <<part <<", Size: " <<size <<"\n\n";
	int K = 1;
	while(true) {
		generateC(K, part, size);
		if(C.empty())
			break;
		generateL(K, part, size);
		K++;
	}
}

void partitioning(char* inputFile, char* outputFile) {
	getTransactions(inputFile);
	int i, p, size;
	float f = (float) T/parts;
	partitionSize = T/parts;
	if(f > partitionSize)
		partitionSize++;
	for(p = 0; p < parts; ++p) {
		size = 0;
		for(i = p*partitionSize; i < transactions.size(); ++i) {
			if(size == partitionSize)
				break;
			size++;
		}
		aprioriProcess(p, size);
	}
	printItemset("candidateItemsets.txt", candidateSet);
	//printItemset(outputFile, frequentSet, 1);
}

int main() {
	char* inputFile = "sampleInput.txt";
	char* outputFile = "sampleOutput.txt";
	minSupport = 22.0;
	minConfidence = 50.0;
	partitioning(inputFile, outputFile);
	//generateAssociationRules();
	return 0;
}


