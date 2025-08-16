#include "KosmosBase.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

bool IniFile::load(const std::string &filename)
{
    data.clear();
    std::ifstream file(filename);
    if (!file)
        return false;
    std::string line, section;
    while (std::getline(file, line))
    {
        trim(line);
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;
        if (line.front() == '[' && line.back() == ']')
        {
            section = line.substr(1, line.size() - 2);
            trim(section);
        }
        else
        {
            auto eq = line.find('=');
            if (eq == std::string::npos)
                continue;
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            trim(key);
            trim(value);
            data[section][key] = value;
        }
    }
    return true;
}

bool IniFile::save(const std::string &filename) const
{
    std::ofstream file(filename);
    if (!file)
        return false;
    for (const auto &sec : data)
    {
        if (!sec.first.empty())
            file << "[" << sec.first << "]\n";
        for (const auto &kv : sec.second)
        {
            file << kv.first << " = " << kv.second << "\n";
        }
        file << "\n";
    }
    return true;
}

std::string IniFile::getString(const std::string &section, const std::string &key, const std::string &def) const
{
    auto s = data.find(section);
    if (s != data.end())
    {
        auto k = s->second.find(key);
        if (k != s->second.end())
            return k->second;
    }
    return def;
}

int IniFile::getInt(const std::string &section, const std::string &key, int def) const
{
    try
    {
        return std::stoi(getString(section, key));
    }
    catch (...)
    {
        return def;
    }
}

float IniFile::getFloat(const std::string &section, const std::string &key, float def) const
{
    try
    {
        return std::stof(getString(section, key));
    }
    catch (...)
    {
        return def;
    }
}

void IniFile::setString(const std::string &section, const std::string &key, const std::string &value)
{
    data[section][key] = value;
}

void IniFile::setInt(const std::string &section, const std::string &key, int value)
{
    data[section][key] = std::to_string(value);
}

void IniFile::setFloat(const std::string &section, const std::string &key, float value)
{
    data[section][key] = std::to_string(value);
}

void IniFile::trim(std::string &s)
{
    auto notSpace = [](int ch)
    { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}
