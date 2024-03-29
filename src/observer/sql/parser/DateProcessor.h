#pragma once
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

extern bool IsLeapYear(int Year);
extern void StrDate2IntDate(const char *StrDate, int &IntDate);
extern std::string IntDate2StrDate(int IntDate);
extern bool CheckDate(int IntDate);