#include <iostream>
using namespace std;
#include <array>
#include <vector>
using Block8x8 = std::array<int, 8 * 8>;
using Block16x16 = std::array<int, 16 * 16>;
enum BLOCKTYPE
{
    Y_BLOCK,
    Cr_BLOCK,
    Cb_BLOCK,
};

template <BLOCKTYPE TYPE>
struct TypeBlockSelector;

template <>
struct TypeBlockSelector<Y_BLOCK>
{
    using type = Block16x16;
};

template <>
struct TypeBlockSelector<Cb_BLOCK>
{
    using type = Block8x8;
};

template <>
struct TypeBlockSelector<Cr_BLOCK>
{
    using type = Block8x8;
};

int main(int argc, char *argv[])
{
    // int a = 0;
    // std::vector<int> vec{1, 2, 3};
    // std::array<int, 3> arr;
    // arr[0] = 0;
    // std::copy(arr.begin(), arr.begin() + 3, vec.begin());
    // std::cout << arr[0] << arr[1] << arr[2] << std::endl;
    std::vector<int> vec1{1, 2};
    std::vector<int> vec2{3, 4};
    vec1 = vec2;
    for (int i = 0; i < vec1.size(); i++)
    {
        std::cout << vec1[i] << std::endl;
    }
    return 0;
}
