/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Longda on 2010
//

#include "common/lang/string.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "common/log/log.h"
namespace common
{

    char* strip(char* str_)
    {
        if (str_ == NULL || *str_ == 0)
        {
            LOG_ERROR("The augument is invalid!");
            return str_;
        }

        char* head = str_;
        while (isspace(*head)) ++head;

        char* last = str_ + strlen(str_) - 1;
        while (isspace(*last) && last != str_) --last;
        *(last + 1) = 0;
        return head;
    }

    void strip(std::string& str)
    {
        size_t head = 0;

        while (isspace(str[head])) { ++head; }

        size_t tail = str.size() - 1;
        while (isspace(str[tail]) && tail != head) { --tail; }

        str = str.substr(head, (tail - head) + 1);
    }

    // Translation functions with templates are defined in the header file
    std::string size_to_pad_str(int size, int pad)
    {
        std::ostringstream ss;
        ss << std::setw(pad) << std::setfill('0') << size;
        return ss.str();
    }

    std::string& str_to_upper(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), (int (*)(int)) & std::toupper);
        return s;
    }

    std::string& str_to_lower(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), (int (*)(int)) & std::tolower);
        return s;
    }

    void split_string(const std::string& str, std::string delim, std::set<std::string>& results)
    {
        int         cut_at;
        std::string tmp_str(str);
        while ((cut_at = tmp_str.find_first_of(delim)) != (signed)tmp_str.npos)
        {
            if (cut_at > 0) { results.insert(tmp_str.substr(0, cut_at)); }
            tmp_str = tmp_str.substr(cut_at + 1);
        }

        if (tmp_str.length() > 0) { results.insert(tmp_str); }
    }

    void split_string(const std::string& str, std::string delim, std::vector<std::string>& results)
    {
        int         cut_at;
        std::string tmp_str(str);
        while ((cut_at = tmp_str.find_first_of(delim)) != (signed)tmp_str.npos)
        {
            if (cut_at > 0) { results.push_back(tmp_str.substr(0, cut_at)); }
            tmp_str = tmp_str.substr(cut_at + 1);
        }

        if (tmp_str.length() > 0) { results.push_back(tmp_str); }
    }

    void split_string(char* str, char dim, std::vector<char*>& results, bool keep_null)
    {
        char* p = str;
        char* l = p;
        while (*p)
        {
            if (*p == dim)
            {
                *p++ = 0;
                if (p - l > 1 || keep_null) results.push_back(l);
                l = p;
            }
            else
                ++p;
        }
        if (p - l > 0 || keep_null) results.push_back(l);
        return;
    }

    void merge_string(std::string& str, std::string delim, std::vector<std::string>& source, size_t result_len)
    {

        std::ostringstream ss;
        if (source.empty())
        {
            str = ss.str();
            return;
        }

        if (result_len == 0 || result_len > source.size()) { result_len = source.size(); }

        for (unsigned int i = 0; i < result_len; i++)
        {
            if (i == 0) { ss << source[i]; }
            else { ss << delim << source[i]; }
        }

        str = ss.str();
        return;
    }

    void replace(std::string& str, const std::string& old, const std::string& new_str)
    {
        if (old.compare(new_str) == 0) { return; }

        if (old == "") { return; }

        if (old.length() > str.length()) { return; }

        std::string result;

        size_t index;
        size_t last_index = 0;

        while ((index = str.find(old, last_index)) != std::string::npos)
        {
            result += str.substr(last_index, index - last_index);
            result += new_str;
            last_index = index + old.length();
        }

        result += str.substr(last_index, str.length() - last_index + 1);

        str = result;

        return;
    }

    char* bin_to_hex(const char* s, const int len, char* hex_buff)
    {
        int            new_len = 0;
        unsigned char* end = (unsigned char*)s + len;
        for (unsigned char* p = (unsigned char*)s; p < end; p++)
        {
            new_len += snprintf(hex_buff + new_len, 3, "%02x", *p);
        }

        hex_buff[new_len] = '\0';
        return hex_buff;
    }

    char* hex_to_bin(const char* s, char* bin_buff, int* dest_len)
    {
        char  buff[3];
        char* src;
        int   src_len;
        char* p_dest;
        char* p_dest_end;

        src_len = strlen(s);
        if (src_len == 0)
        {
            *dest_len = 0;
            bin_buff[0] = '\0';
            return bin_buff;
        }

        *dest_len = src_len / 2;
        src = (char*)s;
        buff[2] = '\0';

        p_dest_end = bin_buff + (*dest_len);
        for (p_dest = bin_buff; p_dest < p_dest_end; p_dest++)
        {
            buff[0] = *src++;
            buff[1] = *src++;
            *p_dest = (char)strtol(buff, NULL, 16);
        }

        *p_dest = '\0';
        return bin_buff;
    }

    bool is_blank(const char* s)
    {
        if (s == nullptr) { return true; }
        while (*s != '\0')
        {
            if (!isspace(*s)) { return false; }
            s++;
        }
        return true;
    }

    /**
     * 获取子串
     * 从s中提取下标为n1~n2的字符组成一个新字符串，然后返回这个新串的首地址
     *
     * @param s
     * @param n1
     * @param n2
     * @return
     */
    char* substr(const char* s, int n1, int n2)
    {
        char* sp = (char*)malloc(sizeof(char) * (n2 - n1 + 2));
        int   i, j = 0;
        for (i = n1; i <= n2; i++) { sp[j++] = s[i]; }
        sp[j] = 0;
        return sp;
    }

    /**
     * double to string
     * @param v
     * @return
     */
    std::string double_to_str(double v)
    {
        char buf[256];
        snprintf(buf, sizeof(buf), "%.2f", v);
        size_t len = strlen(buf);
        while (buf[len - 1] == '0') { len--; }
        if (buf[len - 1] == '.') { len--; }

        return std::string(buf, len);
    }

/**
 * 为日期类型设计的格式转换与非法输入判断
 */
#define MinInt 0x80000000
    bool IsLeapYear(int Year)
    {
        if ((Year % 4 == 0 && Year % 100 != 0) || Year % 400 == 0) return 1;
        return 0;
    }

    bool IsDateValid(int y, int m, int d)
    {
        if (y > 5000000) return 0;
        int MonthDays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (IsLeapYear(y)) MonthDays[2] = 29;

        return m > 0 && m <= 12 && d > 0 && d <= MonthDays[m];
    }

    void StrDate2IntDate(const char* StrDate, int& IntDate)
    {
        int Year, Month, Day;
        sscanf(StrDate, "%d-%d-%d", &Year, &Month, &Day);
        if (!IsDateValid(Year, Month, Day))
        {
            IntDate = MinInt;
            return;
        }

        bool IsBefore1970 = Year < 1970;
        IntDate = 0;
        if (IsBefore1970)
            for (int y = 1969; y > Year; --y) IntDate -= (IsLeapYear(y) ? 366 : 365);
        else
            for (int y = 1970; y < Year; ++y) IntDate += (IsLeapYear(y) ? 366 : 365);

        int MonthDays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (IsLeapYear(Year)) MonthDays[2] = 29;

        for (int i = 0; i < Month - 1; ++i) IntDate += MonthDays[i + 1];
        IntDate += Day - 1;

        if (IsBefore1970) IntDate = IntDate - 365 - (IsLeapYear(Year) ? 1 : 0);
    }

    std::string IntDate2StrDate(int IntDate)
    {
        int Year = 1970;
        while (IntDate < 0 || IntDate >= (IsLeapYear(Year) ? 366 : 365))
        {
            if (IntDate < 0)
            {
                --Year;
                IntDate += IsLeapYear(Year) ? 366 : 365;
            }
            else
            {
                IntDate -= IsLeapYear(Year) ? 366 : 365;
                ++Year;
            }
        }

        int MonthDays[] = {31, IsLeapYear(Year) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int Month = 0;
        while (IntDate >= MonthDays[Month])
        {
            IntDate -= MonthDays[Month];
            ++Month;
        }
        int Day = IntDate + 1;
        Month += 1;
        std::ostringstream oss;
        oss << std::setw(4) << std::setfill('0') << Year << "-" << std::setw(2) << std::setfill('0') << Month << "-"
            << std::setw(2) << std::setfill('0') << Day;

        return oss.str();
    }

    bool CheckDate(int IntDate) { return IntDate != MinInt; }
#undef MinInt
}  // namespace common
