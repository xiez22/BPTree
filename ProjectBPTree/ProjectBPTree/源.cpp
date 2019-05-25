#include "iostream"
#include "BTree.hpp"
#include "cmath"
using namespace std;

int main() {
	sjtu::BTree<int,int> btree;
	for (int i = 0; i < 100000; ++i) {
		int val = rand();
		btree.insert(val, val);
	}
	for (auto p : btree)
		cout << p.second << endl;

	return 0;
}