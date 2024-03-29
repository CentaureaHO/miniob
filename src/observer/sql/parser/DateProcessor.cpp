#include "DateProcessor.h"
#include <iostream>
#define MinInt 0x80000000

bool IsLeapYear(int Year)
{
    if ((Year % 4 == 0 && Year % 100 != 0) || Year % 400 == 0)
        return 1;
    return 0;
}

bool IsDateValid(int y, int m, int d) 
{
    if (y > 5000000) return 0;
    int mon[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (IsLeapYear(y)) mon[2] = 29;

    return m > 0 && m <= 12 && d > 0 && d <= mon[m];
}

void StrDate2IntDate(const char *StrDate, int &IntDate) 
{
    int Year, Month, Day;
    sscanf(StrDate, "%d-%d-%d", &Year, &Month, &Day);
    if (!IsDateValid(Year, Month, Day)) 
    {
        IntDate = MinInt;
        return;
    }

    bool isBefore1970 = Year < 1970;
    int totalDays = 0;
    if (isBefore1970) for (int y = 1969; y > Year; --y) totalDays -= (IsLeapYear(y) ? 366 : 365);
    else for (int y = 1970; y < Year; ++y) totalDays += (IsLeapYear(y) ? 366 : 365);

    int MonthDays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (IsLeapYear(Year)) MonthDays[2] = 29;

    for (int i = 0; i < Month - 1; ++i) totalDays += MonthDays[i + 1];
    totalDays += Day - 1;

    if (isBefore1970) totalDays = totalDays - 365 - (IsLeapYear(Year) ? 1 : 0);

    IntDate = totalDays;
}

std::string IntDate2StrDate(int IntDate) 
{
    int Year = 1970;
    while (IntDate < 0 || IntDate >= (IsLeapYear(Year) ? 366 : 365)) 
    {
        if (IntDate < 0) 
        {
            Year--;
            IntDate += IsLeapYear(Year) ? 366 : 365;
        } 
        else 
        {
            IntDate -= IsLeapYear(Year) ? 366 : 365;
            Year++;
        }
    }

    int daysInMonth[] = {31, IsLeapYear(Year) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int Month = 0;
    while (IntDate >= daysInMonth[Month]) 
    {
        IntDate -= daysInMonth[Month];
        Month++;
    }
    int Day = IntDate + 1;
    Month += 1;
    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << Year << "-"
        << std::setw(2) << std::setfill('0') << Month << "-"
        << std::setw(2) << std::setfill('0') << Day;

    return oss.str();
}

bool CheckDate(int IntDate) 
{
    return IntDate != MinInt;
}
