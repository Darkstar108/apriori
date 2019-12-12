#include<iostream>
#include<fstream>
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<bits/stdc++.h>
using namespace std;

const int hashmapSize = 7;
int T, option;
double minSupport;
double minConfidence;
vector<vector<int> > transactions;
vector<vector<int> > C;
vector<vector<int> > L;
vector<vector<vector<int> > > frequentSet;
vector<vector<int> > frequentSetCount;
struct hashmap {
	int bucketcnt;
	vector<vector<int> > itemsets;
	vector<int> itemsetcnt;
} H[hashmapSize];

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

// Invert Database to show transactions for each item
void invertDatabase(int I, char* inputFile) {
	fstream transactionFile;
	transactionFile.open(inputFile, ios::in);
	fstream invertedFile;
	invertedFile.open("inverted.txt", ios::out);
	vector<int> items[I];
	int Tid, Iid, i, j;
	for(i = 0; i < T; ++i) {
		transactionFile>>Tid;
		transactionFile>>Iid;
		while(Iid != -1) {
			items[Iid].push_back(Tid);
			transactionFile>>Iid;
		}
	}
	for(i = 0; i < I; ++i) {
		invertedFile<<i <<" " <<items[i].size() <<" ";
		for(j = 0; j < items[i].size(); ++j)
			invertedFile<<items[i][j] <<" ";
		invertedFile<<-1 <<"\n";
	}
	transactionFile.close();
	invertedFile.close();
}

// Create transaction database
char* createDatabase() {
	fstream transactionFile;
	char* inputFile = "inputFile.txt";
	transactionFile.open(inputFile, ios::out);
	T = (rand()%500)+350;
	int I = (rand()%50)+10;
	set<int> items;
	int items_limit, i, j, tmp_item;
	cout<<T <<" " <<I;
	// For each transaction
	for(i = 0; i < T; ++i) {
		transactionFile<<i+1000 <<" ";
		items.clear();
		items_limit = rand()%(I/2)+1;
		for(j = 0; j < items_limit;) {
			tmp_item = rand()%I;
			if(items.find(tmp_item) == items.end()) {
				items.insert(tmp_item);
				++j;
			}
		}
		for(set<int>::iterator itr = items.begin(); itr != items.end(); itr++)
			transactionFile<<*itr <<" ";
		transactionFile<<-1 <<"\n";
	}
	transactionFile.close();
	invertDatabase(I, inputFile);
	return inputFile;
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

// Initailise hashmap bucketcnts to 0
void initialiseHashMap() {
	for(int i = 0; i < hashmapSize; ++i) {
		H[i].bucketcnt = 0;
	}
}

// Check if itemset is in hashmap
int checkItemsetInHash(vector<int> itemset, int h) {
	int i, j, k = 0;
	for(i = 0; i < H[h].itemsets.size(); ++i) {
		for(j = 0; j < H[h].itemsets[i].size(); ++j) {
			if(H[h].itemsets[i][j] == itemset[k])
				k++;
			else {
				k = 0;
				break;
			}
		}
		if(j == H[h].itemsets[i].size())
			return i;
	}
	return -1;
}

// Insert 2 itemset to Hashmap
void hashItemset(vector<int> itemset) {
	int x, y, h, i;
	
	x = itemset[0];
	y = itemset[1];
	h = (10*x + y)%hashmapSize;
	
	H[h].bucketcnt++;
	i = checkItemsetInHash(itemset, h);
	if(i != -1) 
		H[h].itemsetcnt[i]++;
	else {
		H[h].itemsetcnt.push_back(0);
		H[h].itemsets.push_back(itemset);
	}	
}

// Get support of itemset
double getFrequency(vector<int> itemset) {
	int freq = 0, i, j, k;
	double support;
	for(i = 0; i < transactions.size(); ++i) {
		k = 0;
		if(option == 1 && itemset.size() == 1) {
			vector<vector<int> > s;
			s = generateKlengthSubset(transactions[i], 2);
			for(j = 0; j < s.size(); ++j) {
				hashItemset(s[j]);
			}
		}
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
				print(s);
				for(l = 0; l < s.size(); ++l) {
					sComplement = getSubsetComplement(frequentSet[i][j], s[l]);
					// Getting Confidence values
					freq = getFrequency(s[l]);
					cout<<"Freq:" <<freq;
					supS = (double) freq/T;
					supS = supS*100;
					freq = frequentSetCount[i][j];
					supL = (double) freq/T;
					supL = supL*100;
					cout<<" SupS:" <<supS <<" SupL:" <<supL <<"\n";
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
void generateC(int K) {
	C.clear();
	if(K == 1) {
		set<int> S;
		for(int i = 0; i < transactions.size(); ++i) 
			for(int j = 0; j < transactions[i].size(); ++j)
				S.insert(transactions[i][j]);
		for(set<int>::iterator itr = S.begin(); itr != S.end(); ++itr)
			C.push_back(vector<int>(1, *itr));
	}
	if(K == 2 && option == 1) {
		int h, i;
		for(h = 0; h < hashmapSize; ++h) {
			if(((H[h].bucketcnt/T)*100) < minSupport)
				continue;
			for(i = 0; i < H[h].itemsets.size(); ++i) {
				if(((H[h].itemsetcnt[i]/T)*100) < minSupport)
					continue;
				C.push_back(H[h].itemsets[i]);
			}
		}
		sort(C.begin(), C.end());
	}
	else {
		join(K);
	}
	cout<<K <<" C:\n";
	print(C);
}

// Prints frequent itemsets and their support to a file
void printFrequentItemset(char* outputFile) {
	fstream transactionFile;
	transactionFile.open(outputFile, ios::out);
	int i, j, k;
	for(i = 0; i < frequentSet.size(); ++i) {
		for(j = 0; j < frequentSet[i].size(); ++j) {
			for(k = 0; k < frequentSet[i][j].size(); ++k) 
				transactionFile<<frequentSet[i][j][k] <<" ";
			transactionFile<<" = " <<frequentSetCount[i][j] <<"\n";
		}
	}
	transactionFile.close();
}

// Generate L, which is C after removing infrequent itemsets
void generateL(int K) {
	if(option == 1 && K == 2) {
		L = C;
		cout<<"L:\n";
		print(L);
		return;
	}
	int i;
	double support, freq;
	vector<int> freqList;
	L.clear();
	for(i = 0; i < C.size(); ++i) {
		freq = getFrequency(C[i]);
		support = (double) freq/T;
		support = support*100;
		if(support >= minSupport) {
			L.push_back(C[i]);
			freqList.push_back(freq);
		}
	}
	frequentSet.push_back(L);
	frequentSetCount.push_back(freqList);
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

// Generate FrequentItemsets using brute force
void bruteForceFrequentItemset(char* inputFile, char* outputFile) {
	getTransactions(inputFile);
	vector<vector<int> > prev, cur;
	vector<int> tmp, freqList;
	int k = 2, n, i, j, freq;
	set<int> S;
	
	// Generate 1-Itemset
	for(i = 0; i < transactions.size(); ++i) 
		for(j = 0; j < transactions[i].size(); ++j)
			S.insert(transactions[i][j]);
	for(set<int>::iterator itr = S.begin(); itr != S.end(); ++itr) {
		tmp.push_back(*itr);
		freq = getFrequency(tmp);
		freqList.push_back(freq);
		cur.push_back(tmp);
		tmp.pop_back();
	}
	frequentSetCount.push_back(freqList);
	frequentSet.push_back(cur);
	freqList.clear();
	
	// Generate other itemsets using prev itemset generated
	n = S.size();
	while(k <= n) {
		prev = cur;
		cur.clear();
		for(i = 0; i < prev.size(); ++i) {
			tmp = prev[i];
			// For each item that is not in prev itemset, add to prev to get cur
			for(set<int>::iterator itr = S.begin(); itr != S.end(); ++itr) {
				for(j = 0; j < tmp.size(); ++j)
					if(*itr <= tmp[j])
						break;
				if(j == tmp.size()) {
					tmp.push_back(*itr);
					freq = getFrequency(tmp);
					freqList.push_back(freq);
					cur.push_back(tmp);
					tmp.pop_back();
				}
			}
		}
		
		frequentSetCount.push_back(freqList);
		frequentSet.push_back(cur);
		freqList.clear();
		k++;
	}
	printFrequentItemset(outputFile);
}

// Main process of Apriori
// Option 0 = Normal, 1 = Hashing, 2 - Partitioning
void aprioriProcess(char* inputFile, char* outputFile) {
	getTransactions(inputFile);
	int K = 1;
	if(option == 1)
		initialiseHashMap();
	while(true) {
		generateC(K);
		if(C.empty())
			break;
		generateL(K);
		K++;
	}
	printFrequentItemset(outputFile);
}

int main() {
	char* inputFile = "sampleInput.txt";
	char* outputFile = "sampleOutput.txt";
	char* bruteForceOutput = "bruteForce.txt";
	if(strcmp(inputFile, "") == 0)
		inputFile = createDatabase();
	minSupport = 22.0;
	minConfidence = 50.0;
	option = 0;
	aprioriProcess(inputFile, outputFile);
	//bruteForceFrequentItemset(inputFile, bruteForceOutput);
	generateAssociationRules();
	return 0;
}

