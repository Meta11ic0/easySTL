#include <vector>
#include <iostream>
#include "test.h"
#include "vector.h"

void VectorTest()
{
  std::cout << "[----------------- vector test -----------------]\n";
  int a[] = { 1,2,3,4,5 };
  easystl::vector<int> v1;
  easystl::vector<int> v2(10);
  easystl::vector<int> v3(10, 1);
  easystl::vector<int> v4(a + 0, a + 5);
  easystl::vector<int> v5(v2);
  easystl::vector<int> v6(v2);
  easystl::vector<int> v7{ 1,2,3,4,5,6,7,8,9 };
  easystl::vector<int> v8, v9, v10;
  v8 = v3;
  v9 = v3;
  v10 = { 1,2,3,4,5,6,7,8,9 };
  
  FUN_AFTER(v1, v1.assign(8, 8));
  FUN_AFTER(v1, v1.assign(a, a + 5));
  FUN_AFTER(v1, v1.push_back(6));
  FUN_AFTER(v1, v1.insert(v1.end(), 7));
  FUN_AFTER(v1, v1.insert(v1.begin() + 3, 2, 3));
  FUN_AFTER(v1, v1.insert(v1.begin(), a + 0, a + 5));
  FUN_AFTER(v1, v1.pop_back());
  FUN_AFTER(v1, v1.erase(v1.begin()));
  FUN_AFTER(v1, v1.erase(v1.begin(), v1.begin() + 2));
  FUN_AFTER(v1, v1.swap(v9));
  FUN_VALUE(v1.size());
  FUN_VALUE(v1.capacity());
  FUN_VALUE(*v1.begin());
  FUN_VALUE(*(v1.end() - 1));
  FUN_VALUE(v1.front());
  FUN_VALUE(v1.back());
  FUN_VALUE(v1[0]);
  int* p = v1.data();
  *p = 10;
  *++p = 20;
  p[1] = 30;
  std::cout << " After change v1.data() :" << "\n";
  COUT(v1);
  std::cout << std::boolalpha;
  FUN_VALUE(v1.empty());
  std::cout << std::noboolalpha;
  FUN_VALUE(v1.size());
  FUN_VALUE(v1.capacity());
  FUN_AFTER(v1, v1.resize(10));
  FUN_VALUE(v1.size());
  FUN_VALUE(v1.capacity());
  FUN_AFTER(v1, v1.resize(6, 6));
  FUN_VALUE(v1.size());
  FUN_VALUE(v1.capacity());
  FUN_AFTER(v1, v1.clear());
  FUN_VALUE(v1.size());
  FUN_VALUE(v1.capacity());
  std::cout << "[----------------- End -----------------]\n";
}


