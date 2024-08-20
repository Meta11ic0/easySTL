#ifndef MYTINYSTL_TEST_H_
#define MYTINYSTL_TEST_H_

#include <iostream>
#include <string>

// 输出通过提示
#define PASSED    std::cout << "[ PASSED ]\n"

// 遍历输出容器
#define COUT(container) do {                             \
  std::string con_name = #container;                     \
  std::cout << " " << con_name << " :";                  \
  for (auto it : container)                              \
    std::cout << " " << it;                              \
  std::cout << "\n";                                     \
} while(0)

#define STR_COUT(str) do {                               \
  std::string str_name = #str;                           \
  std::cout << " " << str_name << " : " << str << "\n";  \
} while(0)

// 输出容器调用函数后的结果
#define FUN_AFTER(con, fun) do {                         \
  std::string fun_name = #fun;                           \
  std::cout << " After " << fun_name << " :\n";          \
  fun;                                                   \
  COUT(con);                                             \
} while(0)

#define STR_FUN_AFTER(str, fun) do {                     \
  std::string fun_name = #fun;                           \
  std::cout << " After " << fun_name << " :\n";          \
  fun;                                                   \
  STR_COUT(str);                                         \
} while(0)

// 输出容器调用函数的值
#define FUN_VALUE(fun) do {                              \
  std::string fun_name = #fun;                           \
  std::cout << " " << fun_name << " : " << fun << "\n";  \
} while(0)


#endif // !MYTINYSTL_TEST_H_

