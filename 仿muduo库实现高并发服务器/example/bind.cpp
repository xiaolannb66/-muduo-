#include <iostream>
#include <string>
#include <vector>
#include <functional>

void print(const std::string &str, int num)
{
    std::cout << str << num << std::endl;
}

int main()
{
    using Task = std::function<void()>;
    std::vector<Task> arry;
    arry.push_back(std::bind(print, "hello", 10));
    arry.push_back(std::bind(print, "leihou", 20));
    arry.push_back(std::bind(print, "nihao", 30));
    arry.push_back(std::bind(print, "bitejiuyeke", 40));

    for (auto &f:arry) {
        f();
    }
    return 0;
}