#include <iostream>
#include <vector>

// 用构造、析构日志观察对象是否已经创建。
struct ConstructionProbe {
  inline static int constructions = 0;
  inline static int destructions = 0;

  ConstructionProbe() {
    ++constructions;
    std::cout << "  ConstructionProbe()\n";
  }

  ~ConstructionProbe() {
    ++destructions;
    std::cout << "  ~ConstructionProbe()\n";
  }
};

int main() {
  std::cout << "1. Empty vector: reserve(4)\n";
  {
    std::vector<ConstructionProbe> probes;
    probes.reserve(4);
    std::cout << "  size=" << probes.size()
              << ", capacity=" << probes.capacity()
              << ", constructions=" << ConstructionProbe::constructions << '\n';
    if (probes.size() != 0 || probes.capacity() < 4 ||
        ConstructionProbe::constructions != 0) return 1;
  }  // 没有元素需要析构，vector 只释放预留的存储空间。
  std::cout << "  destructions=" << ConstructionProbe::destructions << '\n';
  if (ConstructionProbe::destructions != 0) return 1;

  std::cout << "\n2. new ConstructionProbe[4]\n";
  ConstructionProbe* array = new ConstructionProbe[4];
  std::cout << "  constructions=" << ConstructionProbe::constructions << '\n';
  delete[] array;  // 析构4个对象，再释放数组的存储空间。
  std::cout << "  destructions=" << ConstructionProbe::destructions << '\n';
  if (ConstructionProbe::constructions != 4 ||
      ConstructionProbe::destructions != 4) return 1;

  std::cout << "\nAll checks passed.\n";
  return 0;
}
