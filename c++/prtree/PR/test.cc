#include <stdio.h>
#include <iostream>
#include <vector>
#include <list>
using namespace std;

int main() {
list<int> a;
a.push_back(1);;
a.push_back(2);
a.push_back(3);
list<int>::iterator it = a.begin();
it++;
a.insert(it, 4);
it=a.begin();
while (it!=a.end()) {
 cout << *it << " ";
  it++;
}
}
