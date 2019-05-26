#include "iostream"
#include "BTree.hpp"
#include "cmath"
using namespace std;

int main() {
	sjtu::BTree<int,int> btree;
	for (int i = 0; i < 20000; ++i) {
		int val = rand();
		btree.insert(val, val);
	}

	cout << btree.cnt << endl;

	return 0;
}