#include "DateProcessor.h"

bool IsLeapYear(int Year)
{
    if ((Year % 4 == 0 && Year % 100 != 0) || Year % 400 == 0)
        return 1;
    return 0;
}

bool IsDateValid(int y, int m, int d) 
{
    if (y < 1970 || y > 2038) return false;
    if (y == 2038 && (m > 2 || (m == 2 && d > 28))) return false;

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
        IntDate = -1;
        return;
    }
    int MonthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    IntDate = (Year - 1970) * 365 + (Year - 1969) / 4 - (Year - 1901) / 100 + (Year - 1601) / 400;
    for (int i = 0; i < Month - 1; ++i) IntDate += MonthDays[i];
    if (Month > 2 && IsLeapYear(Year)) ++IntDate;
    IntDate += Day - 1;
}

void IntDate2StrDate(int IntDate, char *StrDate) 
{
    int Year = 1970;
    while (IntDate >= 365) 
    {
        if (IsLeapYear(Year)) 
        {
            if (IntDate >= 366) 
            {
                IntDate -= 366;
                Year++;
            }
        } 
        else 
        {
            IntDate -= 365;
            Year++;
        }
    }
    int MonthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (IsLeapYear(Year)) MonthDays[1] = 29;
    int Month = 1;
    while (IntDate >= MonthDays[Month - 1] && Month <= 12) 
    {
        IntDate -= MonthDays[Month - 1];
        Month++;
    }
    int Day = IntDate + 1;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << Year << "-" << std::setw(2) << Month << "-" << std::setw(2) << Day;
    strncpy(StrDate, oss.str().c_str(), oss.str().length() + 1);
}

std::string IntDate2StrDate2(int IntDate)
{
    int Year = 1970;
    while (IntDate >= 365) {
        if (IsLeapYear(Year)) {
            if (IntDate > 365) {
                IntDate -= 366;
                Year++;
            }
        } else {
            IntDate -= 365;
            Year++;
        }
    }
    int MonthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (IsLeapYear(Year)) MonthDays[1] = 29;
    
    int Month = 1;
    while (IntDate >= MonthDays[Month - 1]) {
        IntDate -= MonthDays[Month - 1];
        Month++;
    }
    int Day = IntDate + 1;
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << Year << "-"
        << std::setw(2) << Month << "-"
        << std::setw(2) << Day;
    return oss.str();
}

bool CheckDate(int IntDate) 
{
    return IntDate >= 0;
}
