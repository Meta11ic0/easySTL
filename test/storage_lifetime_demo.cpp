#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

struct Tracked {
  inline static int constructions = 0;
  inline static int destructions = 0;

  Tracked() {
    ++constructions;
    std::cout << "  construct: " << constructions << '\n';
  }

  ~Tracked() {
    ++destructions;
    std::cout << "  destroy: " << destructions << '\n';
  }
};

int main() {
  try {
    std::cout << "1. vector reserve(4)\n";
    bool reserve_checked = false;
    {
      std::vector<Tracked> values;
      std::cout << "  before: size=" << values.size()
                << ", capacity=" << values.capacity() << '\n';
      values.reserve(4);
      std::cout << "  after: size=" << values.size()
                << ", capacity=" << values.capacity()
                << ", constructions=" << Tracked::constructions << '\n';
      reserve_checked = values.size() == 0 && values.capacity() >= 4
                        && Tracked::constructions == 0;
    }
    std::cout << "  after scope: destructions=" << Tracked::destructions
              << '\n';
    if (!reserve_checked || Tracked::destructions != 0) {
      std::cerr << "reserve check failed: expected size=0, capacity>=4, "
                   "and no construction or destruction\n";
      return 1;
    }

    std::cout << "\n2. new Tracked[4]\n";
    Tracked::constructions = 0;
    Tracked::destructions = 0;
    Tracked* array = new Tracked[4];
    const bool array_constructed = Tracked::constructions == 4;
    delete[] array;
    if (!array_constructed || Tracked::destructions != 4) {
      std::cerr << "array check failed: expected 4 constructions "
                   "and 4 destructions\n";
      return 1;
    }

    std::cout << "\n3. allocate 4 positions, construct 2 objects\n";
    Tracked::constructions = 0;
    Tracked::destructions = 0;
    std::allocator<Tracked> alloc;
    Tracked* storage = alloc.allocate(4);
    std::cout << "  after allocate(4): constructions="
              << Tracked::constructions << '\n';
    const bool allocation_checked = Tracked::constructions == 0
                                    && Tracked::destructions == 0;

    std::size_t constructed = 0;
    try {
      for (; constructed < 2; ++constructed) {
        std::construct_at(storage + constructed);
      }
    } catch (...) {
      while (constructed > 0) {
        std::destroy_at(storage + --constructed);
      }
      alloc.deallocate(storage, 4);
      throw;
    }
    std::cout << "  after constructing 2 objects: constructions="
              << Tracked::constructions
              << ", destructions=" << Tracked::destructions << '\n';
    const bool construction_checked = Tracked::constructions == 2
                                      && Tracked::destructions == 0;

    while (constructed > 0) {
      std::destroy_at(storage + --constructed);
    }
    std::cout << "  after destroying 2 objects: destructions="
              << Tracked::destructions << ", storage still held\n";
    const bool destruction_checked = Tracked::destructions == 2;
    alloc.deallocate(storage, 4);
    std::cout << "  deallocate(storage, 4): storage returned\n";
    if (!allocation_checked) {
      std::cerr << "allocation check failed: expected no construction "
                   "or destruction after allocate(4)\n";
      return 1;
    }
    if (!construction_checked) {
      std::cerr << "construction check failed: expected 2 constructions "
                   "and no destruction\n";
      return 1;
    }
    if (!destruction_checked) {
      std::cerr << "destruction check failed: expected 2 destructions\n";
      return 1;
    }
  } catch (const std::exception& error) {
    std::cerr << "Experiment failed: " << error.what() << '\n';
    return 1;
  }

  std::cout << "\nAll checks passed\n";
  return 0;
}
