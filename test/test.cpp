#include "vector.h"

int main() {
  easystl::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  if (v.size() != 2 || v[0] != 1 || v[1] != 2) {
    return 1;
  }
  return 0;
}
