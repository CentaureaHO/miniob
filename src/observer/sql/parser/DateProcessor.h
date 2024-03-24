#pragma once
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

extern bool IsLeapYear(int Year);
extern void StrDate2IntDate(const char *StrDate, int &IntDate);
extern void IntDate2StrDate(int IntDate, char *StrDate);
extern std::string IntDate2StrDate2(int IntDate);
