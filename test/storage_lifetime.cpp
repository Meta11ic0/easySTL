#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

struct Tracked {
  // static 成员由所有 Tracked 对象共享，不会每创建一个对象就多一份计数。
  // inline 允许直接在类内定义这两个静态变量，无需在类外再写定义。
  inline static int constructions = 0;
  inline static int destructions = 0;

  Tracked() { ++constructions; }

  // 析构只记录对象销毁，不负责释放外部为它准备的存储空间。
  // noexcept 表示这个函数不向外抛出异常，使清理过程可以可靠完成。
  ~Tracked() noexcept { ++destructions; }
};

int main() {
  // 申请空间可能抛出 std::bad_alloc。失败时报告原因，并返回非零。
  try {
    std::cout << "1. std::vector reserve(4)\n";
    bool reserve_checked = false;
    {
      std::vector<Tracked> values;
      std::cout << "  before: size=" << values.size()
                << ", capacity=" << values.capacity() << '\n';

      values.reserve(4);
      std::cout << "  after reserve: size=" << values.size()
                << ", capacity=" << values.capacity()
                << ", constructions=" << Tracked::constructions
                << ", destructions=" << Tracked::destructions << '\n';

      // 标准只保证容量至少为4，不要求恰好为4；预留后仍没有元素。
      reserve_checked = values.size() == 0 && values.capacity() >= 4
                        && Tracked::constructions == 0
                        && Tracked::destructions == 0;
    }  // values 析构并归还存储；没有 Tracked 元素需要析构。

    std::cout << "  after vector destruction: destructions="
              << Tracked::destructions << '\n';
    if (!reserve_checked || Tracked::destructions != 0) {
      std::cerr << "reserve check failed\n";
      return 1;
    }

    // 上一过程已经结束，没有存活的 Tracked 对象，此时才重置计数。
    Tracked::constructions = 0;
    Tracked::destructions = 0;
    std::cout << "\n2. new Tracked[4]\n";
    Tracked* array = new Tracked[4];
    std::cout << "  after new[]: constructions=" << Tracked::constructions
              << ", destructions=" << Tracked::destructions << '\n';

    // const bool 保存当前检查结果，后面销毁数组也不会改变这个结果。
    const bool array_checked = Tracked::constructions == 4
                               && Tracked::destructions == 0;
    // new[] 在申请后构造全部4个对象；delete[] 销毁全部对象并释放数组存储。
    delete[] array;
    std::cout << "  after delete[]: destructions=" << Tracked::destructions
              << '\n';
    if (!array_checked || Tracked::destructions != 4) {
      std::cerr << "array construction or destruction check failed\n";
      return 1;
    }

    Tracked::constructions = 0;
    Tracked::destructions = 0;
    std::cout << "\n3. allocate 4 positions, construct 2 objects\n";
    // 模板参数 Tracked 指定元素类型；allocate 的4是位置数量，不是字节数。
    // storage 的类型是 Tracked*，这里只取得空间，没有调用 Tracked 构造函数。
    std::allocator<Tracked> alloc;
    Tracked* storage = alloc.allocate(4);
    std::cout << "  after allocate: constructions=" << Tracked::constructions
              << ", destructions=" << Tracked::destructions << '\n';
    const bool allocation_checked = Tracked::constructions == 0
                                    && Tracked::destructions == 0;

    // 只记录成功构造的数量。若第2个构造失败，第1个仍需要我们显式销毁。
    std::size_t constructed = 0;
    try {
      for (; constructed < 2; ++constructed) {
        // 指针加1前进 sizeof(Tracked) 字节，指向下一个元素位置。
        // 不提供额外参数，construct_at 在指定位置调用默认构造函数。
        std::construct_at(storage + constructed);
      }
    } catch (...) {
      // ... 捕获任意异常；throw; 在清理后继续抛出原异常。
      // 先减计数再销毁，定位最后一个成功构造的对象，不能销毁失败位置。
      while (constructed > 0) {
        --constructed;
        std::destroy_at(storage + constructed);
      }
      alloc.deallocate(storage, 4);
      throw;
    }

    std::cout << "  after constructing 2: constructions="
              << Tracked::constructions
              << ", destructions=" << Tracked::destructions << '\n';
    const bool construction_checked = Tracked::constructions == 2
                                      && Tracked::destructions == 0;

    while (constructed > 0) {
      --constructed;
      std::destroy_at(storage + constructed);
    }
    std::cout << "  after destroying 2: constructions="
              << Tracked::constructions
              << ", destructions=" << Tracked::destructions
              << ", storage still held\n";
    const bool destruction_checked = Tracked::destructions == 2;

    // 销毁2个对象后，整块存储仍由本过程持有。归还数量必须是原来的4。
    alloc.deallocate(storage, 4);
    std::cout << "  storage returned with deallocate(storage, 4)\n";
    // 从这里起不再通过 storage 访问对象或存储。先清理，再报告检查失败。
    if (!allocation_checked) {
      std::cerr << "allocate must not construct any Tracked objects\n";
      return 1;
    }
    if (!construction_checked || !destruction_checked) {
      std::cerr << "partial construction or destruction check failed\n";
      return 1;
    }

    std::cout << "\nAll checks passed\n";
    return 0;
  } catch (const std::exception& error) {
    // 按 const 引用接收标准异常，读取消息而不复制或修改异常对象。
    std::cerr << "experiment failed: " << error.what() << '\n';
    return 1;
  }
}
