#include "uint.hpp"
#include <iostream>

int main()
{
    lrf::_uint<512> a("58b15388ec3762dbff27caef606e84b6c0a7fd2a88728bb2cbb87f0c4a6a803b"),b("2eb4dfbea2db4bc15634488d486e3ee0da783fd834eebe2fc59b23dd25c0bf"),c(a*b);
    std::cout << c << '\n';
}