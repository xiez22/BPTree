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

int main() {
	sjtu::BTree<int,int> btree;

	vector<int> to_push;
	
	for (int i = 0; i < 1000000; ++i) {
		auto temp = rand();
		to_push.push_back(temp);
	}

	cout << "Testing insert...";
	int cnt = 0;
	for (auto p : to_push) {
		btree.insert(p, p);
	}
	cout << "\nChecking...";

	for (auto p : to_push) {
		if (btree.find(p)==btree.cend()) {
			cout << "   Error!\n";
			break;
		}
	}

	int last_one = -1;
	for (auto p : btree) {
		if (p.second <= last_one) {
			cout << "   Error!\n";
			break;
		}
		last_one = p.second;
	}

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