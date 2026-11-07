#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// std::string 的构造函数不能直接加日志，所以另用这个类型观察构造过程。
struct ConstructionProbe {
  inline static int constructions = 0;
  inline static int destructions = 0;

  ConstructionProbe() {
    ++constructions;
    std::cout << "  ConstructionProbe()\n";
  }

  ConstructionProbe(const ConstructionProbe&) {
    ++constructions;
    std::cout << "  ConstructionProbe(copy)\n";
  }

  ~ConstructionProbe() {
    ++destructions;
    std::cout << "  ~ConstructionProbe()\n";
  }
};

int main() {
  std::cout << "1. std::vector<std::string>: reserve(4)\n";
  std::vector<std::string> reserved_names;
  reserved_names.reserve(4);
  std::cout << "  size=" << reserved_names.size()
            << ", capacity=" << reserved_names.capacity() << '\n';
  if (!reserved_names.empty() || reserved_names.capacity() < 4) return 1;

  // at() 会检查 size；即使已经有存储空间，也没有第0个元素可以读取。
  bool out_of_range = false;
  try {
    static_cast<void>(reserved_names.at(0));  // 只观察是否抛异常，不使用返回值。
  } catch (const std::out_of_range&) {
    out_of_range = true;
    std::cout << "  at(0): out_of_range, no first element\n";
  }
  if (!out_of_range) return 1;

  reserved_names.push_back("Alice");
  reserved_names.push_back("Bob");
  std::cout << "  after push_back: size=" << reserved_names.size()
            << ", values=" << reserved_names[0] << ", " << reserved_names[1]
            << '\n';
  if (reserved_names.size() != 2 || reserved_names[0] != "Alice" ||
      reserved_names[1] != "Bob") return 1;

  std::cout << "\n2. Observe construction calls\n";
  {
    std::vector<ConstructionProbe> probes;
    std::cout << "  before reserve(4)\n";
    probes.reserve(4);
    std::cout << "  after reserve(4): constructions="
              << ConstructionProbe::constructions << '\n';
    if (!probes.empty() || probes.capacity() < 4 ||
        ConstructionProbe::constructions != 0) return 1;

    // 直接在容器内构造两个对象，避免 push_back 临时对象带来的额外日志。
    probes.emplace_back();
    probes.emplace_back();
    std::cout << "  after adding two elements: constructions="
              << ConstructionProbe::constructions << '\n';
    if (probes.size() != 2 || ConstructionProbe::constructions != 2) return 1;
  }  // vector 析构，销毁刚才构造的两个对象。
  if (ConstructionProbe::destructions != 2) return 1;

  std::cout << "\n3. new ConstructionProbe[4]\n";
  const int before_new = ConstructionProbe::constructions;
  const int before_delete = ConstructionProbe::destructions;
  ConstructionProbe* array = new ConstructionProbe[4];
  const int created = ConstructionProbe::constructions - before_new;
  delete[] array;  // 配对清理，不能只观察构造而遗漏释放。
  const int destroyed = ConstructionProbe::destructions - before_delete;
  std::cout << "  array constructed=" << created
            << ", destroyed=" << destroyed << '\n';
  if (created != 4 || destroyed != 4) return 1;

  std::cout << "\nAll checks passed.\n";
  return 0;
}
