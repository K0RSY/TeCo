#include <vector>
#include <iostream>

int main(int argc, char const *argv[])
{
    std::vector<std::vector<int>> test;

    for (int i = 0; i < 10; i++)
    {
        std::vector<int> sex;
        test.push_back(sex);
    }
    
    test[2].push_back(10);

    return 0;
}
