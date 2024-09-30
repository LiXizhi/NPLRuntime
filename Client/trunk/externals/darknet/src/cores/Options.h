#ifndef __DARKNET_OPTIONS_H__
#define __DARKNET_OPTIONS_H__
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
namespace darknet
{
    class Options
    {
    public:
        Options(const std::string &name = "") : m_name(name), m_delimiter('|') {}

        inline const std::string &GetName() { return m_name; }
        inline void SetName(const std::string &name) { m_name = name; }
        inline void SetDelimiter(char delimiter) { m_delimiter = delimiter; }
        inline char GetDelimiter() { return m_delimiter; }
        inline std::unordered_map<std::string, std::string> *GetOptions() { return &m_options; }
        inline std::vector<std::string> *GetLines() { return &m_lines; }

    public:
        void Clear()
        {
            m_options.clear();
            m_lines.clear();
        }

        bool GetOption(const std::string &key, bool default_value)
        {
            auto it = m_options.find(key);
            if (it == m_options.end()) return default_value;
            auto value = it->second;

            // clang-format off
        std::transform(value.begin(), value.end(), value.begin(), [](char c) -> char { return std::tolower(c); });
            // clang-format on

            if (value == "false" || value == "0") return false;
            if (value == "true" || value == "1") return true;

            return default_value;
        }

        int GetOption(const std::string &key, int default_value)
        {
            if (key.find(m_delimiter) != std::string::npos) return GetOption(*split(key), default_value);

            auto it = m_options.find(key);
            if (it == m_options.end()) return default_value;

            return std::stoi(it->second);
        }

        int GetOption(const std::vector<std::string> &keys, int default_value)
        {
            for (auto &key : keys)
            {
                auto it = m_options.find(key);
                if (it == m_options.end()) continue;
                return std::stoi(it->second);
            }
            return default_value;
        }

        float GetOption(const std::string &key, float default_value)
        {
            if (key.find("|") != std::string::npos) return GetOption(*split(key, m_delimiter), default_value);

            auto it = m_options.find(key);
            if (it == m_options.end()) return default_value;
            return std::stof(it->second);
        }

        float GetOption(const std::vector<std::string> &keys, float default_value)
        {
            for (auto key : keys)
            {
                auto it = m_options.find(key);
                if (it == m_options.end()) continue;
                return std::stof(it->second);
            }
            return default_value;
        }

        std::string GetOption(const std::string &key, const char *default_value)
        {
            auto it = m_options.find(key);
            if (it == m_options.end()) return default_value;
            return it->second;
        }

        void SetOption(const std::string &key, const std::string &value) { m_options.insert_or_assign(key, value); }
        void SetOption(const std::string &key, int value) { m_options.insert_or_assign(key, std::to_string(value)); }
        void SetOption(const std::string &key, float value) { m_options.insert_or_assign(key, std::to_string(value)); }

        void ParseOption(const std::string &key, const std::string &value, std::vector<int> &results)
        {
            std::istringstream ss(GetOption(key, value.c_str()));
            while (ss.good())
            {
                std::string substr;
                std::getline(ss, substr, ',');
                results.push_back(std::stoi(substr));
            }
        }

        void ParseOption(const std::string &key, const std::string &value, std::vector<float> &results)
        {
            std::istringstream ss(GetOption(key, value.c_str()));
            while (ss.good())
            {
                std::string substr;
                std::getline(ss, substr, ',');
                results.push_back(std::stof(substr));
            }
        }

    public:
        static std::shared_ptr<std::vector<std::shared_ptr<Options>>> Load(const std::string &path)
        {
            std::ifstream ifs(path);
            if (!ifs) return nullptr;
            return Load(ifs);
        }

        static std::shared_ptr<std::vector<std::shared_ptr<Options>>> Load(std::istream &is)
        {
            auto options_list                = std::make_shared<std::vector<std::shared_ptr<Options>>>();
            std::shared_ptr<Options> options = nullptr;
            std::string line;

            while (std::getline(is, line))
            {
                line = Trim(line);

                // 空行
                if (line.empty()) continue;

                // 注释行
                if (line.front() == '#' || line.front() == ';') continue;

                // 选项节
                if (line.front() == '[' && line.back() == ']')
                {
                    options = std::make_shared<Options>(line.substr(1, line.size() - 2));
                    options_list->push_back(options);
                    continue;
                }

                // 未配置首个节名
                if (options == nullptr)
                {
                    options = std::make_shared<Options>();
                    options_list->push_back(options);
                }

                auto pos = line.find("=");
                if (pos == std::string::npos)
                {
                    if (!line.empty())
                    {
                        options->GetLines()->push_back(line);
                    }
                }
                else
                {
                    auto key   = Trim(line.substr(0, pos));
                    auto value = Trim(line.substr(pos + 1, line.size() - pos - 1));
                    options->GetOptions()->insert_or_assign(key, value);
                }
            }

            return options_list;
        }

    protected:
        static std::vector<std::string> *split(const std::string &strings, const char delimiter = '|')
        {
            static std::vector<std::string> s_string_array;
            std::stringstream ss(strings);
            std::string item;

            s_string_array.clear();
            while (std::getline(ss, item, '|'))
            {
                s_string_array.push_back(item);
            }

            return &s_string_array;
        }

    protected:
        std::string m_name;
        std::unordered_map<std::string, std::string> m_options;
        std::vector<std::string> m_lines;
        char m_delimiter;

    private:
        // 移除首尾空白
        static std::string Trim(const std::string &text)
        {
            // 查找第一个非空白字符的位置
            size_t start = text.find_first_not_of(" \t\n\r");
            // 如果字符串全为空白字符，则返回空字符串
            if (start == std::string::npos) return "";
            // 查找最后一个非空白字符的位置
            size_t end = text.find_last_not_of(" \t\n\r");
            // 提取非空白字符部分
            return text.substr(start, end - start + 1);
        }
    };
}
#endif