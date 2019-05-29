#include "iostream"
#include "BTree.hpp"
#include "cmath"
#include "vector"
#include "ctime"
using namespace std;

template <class T>
void print_all(T& btree) {
	for (auto p : btree) {
		cout << p.second << " ";
	}
	cout << endl;
}

bool visited[100009] = { 0 };
bool erased[100009] = { 0 };

int main() {
	sjtu::BTree<int,int> btree;

	vector<int> to_push;
	
	for (int i = 0; i < 100000; ++i) {
		to_push.push_back(i);
	}

	cout << "Testing insert...";
	int cnt = 0;
	for (auto p : to_push) {
		btree.insert(p, p);
	}
	cout << "\nChecking...";

	cout << "   Finished!\nTesting erase...";
	cnt = 0;
	for (auto p : to_push) {
		//cout << cnt << ":";
		btree.erase(p);
	}
	cout << "   Finished!\nChecking...";
	print_all(btree);
	if (btree.empty()) {
		cout << "    Well Done!!!";
	}
	else {
		cout << "    Errors occured!!!";
	}

	return 0;
}