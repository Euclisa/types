#include <gtest/gtest.h>
#include <fstream>
#include "uint.hpp"


TEST(uint_test,addition_1024_test)
{
    std::string test_cases_dir(PROJECT_ROOT);
    test_cases_dir += "/tests/cases/";
    std::string test_samples_file_name(test_cases_dir+"uint1024_samples.txt");
    std::string test_res_file_name(test_cases_dir+"uint1024_add.txt");
    std::ifstream fin_samples(test_samples_file_name),fin_res(test_res_file_name);
    while(!fin_samples.eof() and !fin_res.eof())
    {
        lrf::uint1024_t a,b,res,res_actual;
        fin_samples >> a >> b;
        fin_res >> res_actual;
        if(fin_samples.eof() or fin_res.eof()) break;
        res = a+b;
        ASSERT_EQ(res_actual,res);
    }
}


TEST(uint_test,inplace_addition_1024_test)
{
    std::string test_cases_dir(PROJECT_ROOT);
    test_cases_dir += "/tests/cases/";
    std::string test_samples_file_name(test_cases_dir+"uint1024_samples.txt");
    std::string test_res_file_name(test_cases_dir+"uint1024_add.txt");
    std::ifstream fin_samples(test_samples_file_name),fin_res(test_res_file_name);
    while(!fin_samples.eof() and !fin_res.eof())
    {
        lrf::uint1024_t a,b,res_actual;
        fin_samples >> a >> b;
        fin_res >> res_actual;
        if(fin_samples.eof() or fin_res.eof()) break;
        a += b;
        ASSERT_EQ(res_actual,a);
    }
}


TEST(uint_test,subtraction_1024_test)
{
    std::string test_cases_dir(PROJECT_ROOT);
    test_cases_dir += "/tests/cases/";
    std::string test_samples_file_name(test_cases_dir+"uint1024_samples.txt");
    std::string test_res_file_name(test_cases_dir+"uint1024_sub.txt");
    std::ifstream fin_samples(test_samples_file_name),fin_res(test_res_file_name);
    while(!fin_samples.eof() and !fin_res.eof())
    {
        lrf::uint1024_t a,b,res,res_actual;
        fin_samples >> a >> b;
        fin_res >> res_actual;
        if(fin_samples.eof() or fin_res.eof()) break;
        res = a-b;
        ASSERT_EQ(res_actual,res);
    }
}


TEST(uint_test,inplace_subtraction_1024_test)
{
    std::string test_cases_dir(PROJECT_ROOT);
    test_cases_dir += "/tests/cases/";
    std::string test_samples_file_name(test_cases_dir+"uint1024_samples.txt");
    std::string test_res_file_name(test_cases_dir+"uint1024_sub.txt");
    std::ifstream fin_samples(test_samples_file_name),fin_res(test_res_file_name);
    while(!fin_samples.eof() and !fin_res.eof())
    {
        lrf::uint1024_t a,b,res_actual;
        fin_samples >> a >> b;
        fin_res >> res_actual;
        if(fin_samples.eof() or fin_res.eof()) break;
        a -= b;
        ASSERT_EQ(res_actual,a);
    }
}

/*
TEST(uint_test,multiplication_1024_test)
{
    std::string test_cases_dir(PROJECT_ROOT);
    test_cases_dir += "/tests/cases/";
    std::string test_samples_file_name(test_cases_dir+"uint1024_samples.txt");
    std::string test_res_file_name(test_cases_dir+"uint1024_mul.txt");
    std::ifstream fin_samples(test_samples_file_name),fin_res(test_res_file_name);
    while(!fin_samples.eof() and !fin_res.eof())
    {
        lrf::uint1024_t a,b,res,res_actual;
        fin_samples >> a >> b;
        fin_res >> res_actual;
        if(fin_samples.eof() or fin_res.eof()) break;
        res = a*b;
        ASSERT_EQ(res_actual,res);
    }
}


TEST(uint_test,multiplication_32768_test)
{
    std::string test_cases_dir(PROJECT_ROOT);
    test_cases_dir += "/tests/cases/";
    std::string test_samples_file_name(test_cases_dir+"uint32768_samples.txt");
    std::string test_res_file_name(test_cases_dir+"uint32768_mul.txt");
    std::ifstream fin_samples(test_samples_file_name),fin_res(test_res_file_name);
    while(!fin_samples.eof() and !fin_res.eof())
    {
        lrf::_uint<32768> a,b,res,res_actual;
        fin_samples >> a >> b;
        fin_res >> res_actual;
        if(fin_samples.eof() or fin_res.eof()) break;
        res = a*b;
        ASSERT_EQ(res_actual,res);
    }
}
*/
