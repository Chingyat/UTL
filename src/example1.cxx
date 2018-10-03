#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utl/vector.hpp>
#include <vector>

//using std::vector;
using utl::vector;

template class utl::vector<long>;
template class utl::vector<char *>;
//template class std::vector<std::unique_ptr<int>>;
template class utl::vector<std::string>;

void equality()
{
    vector<long> vec{1, 2, 3, 4};

    auto vec2 = vec;

    assert(vec2 == vec);
}

void insertibility()
{
    {
        vector<long> vec{1, 2, 3, 5};
        auto it = std::find(vec.begin(), vec.end(), 5);
        vec.insert(it, 4);

        vector<long> vec2{1, 2, 3, 4, 5};
        assert(vec == vec2);
    }
    {
        vector<std::unique_ptr<int>> vec;
        vec.push_back(std::make_unique<int>(32));
        vec.emplace_back(std::make_unique<int>(5));
        auto ptr = std::make_unique<int>(342);
        vec.insert(vec.begin() + 1, std::move(ptr));
        assert(vec.size() == 3);
    }
    {
        vector<std::string> vec;
        vec.emplace(vec.cbegin(), "hello world");
        vec.emplace_back("aaaaa");
        assert(vec[0] == "hello world");
        assert(vec[1] == "aaaaa");
    }
}

void erasability()
{
    {
        vector<std::vector<int>> vec{
            std::vector{1, 3, 4},
            std::vector{0, 0, 0},
        };

        vec.erase(vec.begin());
        assert((vec[0] == std::vector<int>{0, 0, 0}));
    }
    {
        vector<int> vec{1, 1, 2, 2, 3, 3, 1, 2, 3, 1, 2, 3};
        vec.erase(std::remove_if(vec.begin(), vec.end(),
                      [](int x) { return bool(x % 2); }),
            vec.end());

        assert(std::all_of(vec.begin(), vec.end(), [](int x) { return x == 2; }));
    }
}

void resizability()
{
    vector<std::unique_ptr<int>> vec;
    vec.resize(199);
    vec.reserve(1);
    vec.resize(10);
    assert(vec.capacity() >= 199);
    assert(vec.size() == 10);
}

void input_iterator()
{
    {
        std::istringstream iss("1 2 3 4 5 6 7");
        vector vec(std::istream_iterator<int>{iss},
            std::istream_iterator<int>());
        assert(vec.size() == 7);
    }
    {
        std::istringstream iss("1 2 3 4 5 6 7");
        vector<int> vec{1, 2, 3};
        vec.insert(vec.end(), std::istream_iterator<int>{iss},
            std::istream_iterator<int>());
        assert(vec.size() == 10);
    }
    {
        std::istringstream iss("1 2 3 4 5 6 7");
        vector<int> vec{1, 2, 3};
        vec.assign(std::istream_iterator<int>{iss},
            std::istream_iterator<int>());
        assert(vec.size() == 7);
    }
    {
        std::istringstream iss("1 2 3 4 5 6 7");
        vector<std::string> vec{"oeuikaoeuao", "eaueoeu", "ueae"};
        vec.assign(std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>());
        assert(vec.size() == 7);
    }
    {
        std::istringstream iss("1 2 3 4 5 6 7");
        vector<std::string> vec{"oeuikaoeuao", "eaueoeu", "ueae"};
        vec.insert(vec.begin() + 1, std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>());
        assert(vec.size() == 10);
    }
    {
        std::istringstream iss("1 2 ");
        vector<int> vec{1, 2, 3};
        vec.assign(std::istream_iterator<int>{iss},
            std::istream_iterator<int>());
        assert(vec.size() == 2);
    }
}

void swappability()
{
    vector<int> v1{1, 2, 3};
    vector<int> v2{4, 5, 6};

    auto it = v1.begin();

    swap(v1, v2);

    assert((v1 == vector<int>{4, 5, 6}));
    assert((v2 == vector<int>{1, 2, 3}));
    assert(it == v2.begin());
}

void use_after_move()
{
    vector<std::string> vec{"Hello", "World"};
    vector vec2 = std::move(vec);
    vec.resize(20);
    assert(vec.size() == 20);
    assert(vec[0].empty());
    assert(vec2[1] == "World");
}

void assignability()
{
    vector<std::unique_ptr<int>> vec;
    vec.emplace_back();
    vec.emplace_back(std::make_unique<int>(42));
    vec.push_back({});
    vector<std::unique_ptr<int>> v2;
    v2.push_back(std::make_unique<int>(33));
    v2 = std::move(vec);
    vec = vector<std::unique_ptr<int>>(10u);
    assert(v2.size() == 3);
    assert(vec.size() == 10);
    v2.shrink_to_fit();
    assert(v2.capacity() == 3);
}

void sortability()
{
    vector<long> vec;
    std::random_device rd;

    vec.resize(100000);
    std::for_each(std::begin(vec), std::end(vec), [&rd](auto &x) noexcept {
        x = rd();
    });

    std::sort(std::begin(vec), std::end(vec));

    bool is = std::is_sorted(std::begin(vec), std::end(vec));
    assert(is);
}

int main()
{
    equality();
    insertibility();
    erasability();
    resizability();
    input_iterator();
    swappability();
    use_after_move();
    sortability();
}
