#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <stdexcept>

// 这个文件涉及大量迭代器的使用

template <typename T> // 这个模板T是DataObject或其的子类。
class DataManager
{
private:
    std::vector<T> dataList;                           // 数据列表
    std::map<std::string, std::vector<T>> categoryMap; // 按类别分组
    std::set<T> uniqueData;                            // 唯一数据集合
    mutable int operationCount;                        // 操作计数

    void updateCategory(const T &data)
    {
        std::string type = data.getType();
        categoryMap[type].push_back(data);
    }
    void rebuildIndex()
    {
        categoryMap.clear();
        for (const auto &data : dataList)
        {
            updateCategory(data);
        }
    }

public:
    // 构造函数
    DataManager() : operationCount(0) {}
    ~DataManager() {}

    // 禁止拷贝（单例模式可选）

    DataManager &operator=(const DataManager &) = delete;

    // 基本操作
    void addData(const T &data)
    {
        if (contains(data.getId()))
        {
            throw std::runtime_error("This data already exists");
        }
        dataList.push_back(data);
        updateCategory(data);
        operationCount++;
    }

    void removeData(const std::string &id)
    {
        auto it = std::find_if(dataList.begin(), dataList.end(), [&id](const T &data)
                               { return data.getId() == id; });
        if (it == dataList.end())
        {
            throw std::out_of_range("Data with ID [" + id + "] not found!");
        }
        dataList.erase(it);
        rebuildIndex();
        operationCount++;
    }
    T findData(const std::string &id) const
    {
        auto it = std::find_if(dataList.begin(), dataList.end(), [&id](const T &data)
                               { return data.getId() == id; });
        if (it == dataList.end())
        {
            throw std::out_of_range("Data with ID [" + id + "] not found!");
        }
        operationCount++;
        return *it;
    }
    bool contains(const std::string &id) const
    {
        return std::any_of(dataList.begin(), dataList.end(), [&id](const T &data)
                           { return data.getId() == id; });
    }
    void clear()
    {
        dataList.clear();
        categoryMap.clear();
        uniqueData.clear();
        operationCount++;
    }

    // 批量操作
    void addDataList(const std::vector<T> &newdataList)
    {
        int addcount = 0;
        for (const auto &data : newdataList)
        {
            if (contains(data.getId()))
            {
                std::cout << "Warning: Data ID [" << data.getId() << "] duplicate,skipped" << std::endl;
                continue;
            }
            dataList.push_back(data);
            addcount++;
        }
        rebuildIndex();
        operationCount += addcount;
    }
    std::vector<T> getDataByType(const std::string &type) const
    {
        operationCount++;
        auto it = categoryMap.find(type);
        if (it == categoryMap.end())
        {
            return {};
        }
        return it->second;
    }

    // 排序
    std::vector<T> sortBySize(bool ascending = true) const
    { // true是从小到大
        std::vector<T> sortedList = dataList;
        std::sort(sortedList.begin(), sortedList.end(), [ascending](const T &a, const T &b)
                  {
            if(ascending)
                return a.getSize() < b.getSize();
            else
                return a.getSize() > b.getSize(); });
        operationCount++;
        return sortedList;
    }
    std::vector<T> sortByName() const
    {
        std::vector<T> sortedList = dataList;
        std::sort(sortedList.begin(), sortedList.end(), [](const T &a, const T &b)
                  { return a.getName() < b.getName(); });
        operationCount++;
        return sortedList;
    }
    std::vector<T> sortByTime() const
    {
        std::vector<T> sortedList = dataList;
        std::sort(sortedList.begin(), sortedList.end(), [](const T &a, const T &b)
                  { return a.getCreateTime() < b.getCreateTime(); });
        operationCount++;
        return sortedList;
    }

    // 过滤（函数式编程）
    std::vector<T> filter(std::function<bool(const T &)> condition) const
    {
        std::vector<T> filteredList;
        std::copy_if(dataList.begin(), dataList.end(), std::back_inserter(filteredList), condition);
        operationCount++;
        return filteredList;
    }

    // 统计
    double getTotalSize() const
    {
        double total = 0.0;
        for (const auto &data : dataList)
        {
            total += data.getSize();
        }
        operationCount++;
        return total;
    }

    int getCount() const
    {
        operationCount++;
        return (int)dataList.size();
    }

    int getUniqueCount() const
    {
        std::set<std::string> uniqueIds;
        for (const auto &data : dataList)
        {
            uniqueIds.insert(data.getId());
        }
        return (int)uniqueIds.size();
    }

    std::map<std::string, int> getTypeStatistics() const // 按类型统计,value是数据条数
    {
        std::map<std::string, int> stats;
        for (const auto &entry : categoryMap)
        {
            stats[entry.first] = (int)(entry.second.size());
        }
        operationCount++;
        return stats;
    }
    void printStatistics() const
    {
        std::cout << "=== DataManager Statistics ===" << std::endl;
        std::cout << "Total Operation Count: " << operationCount << std::endl;
        std::cout << "Total Data Count: " << getCount() << std::endl;
        std::cout << "Total Data Size (MB): " << getTotalSize() << std::endl;
        std::cout << "Unique Data Count: " << getUniqueCount() << std::endl;
        std::cout << "Type Distribution:" << std::endl;

        auto typeStats = getTypeStatistics();
        for (const auto &entry : typeStats)
        {
            std::cout << "  " << entry.first << ": " << entry.second << std::endl;
        }
        operationCount++;
    }

    // 运算符重载
    T &operator[](int index)
    {
        if (index < 0 || (size_t)index >= dataList.size())
        {
            throw std::out_of_range("Index out of bounds!");
        }
        operationCount++;
        return dataList[index];
    }

    const T &operator[](int index) const
    {
        if (index < 0 || (size_t)index >= dataList.size())
        {
            throw std::out_of_range("Index out of bounds!");
        }
        operationCount++;
        return dataList[index];
    }
    DataManager<T> operator+(const DataManager<T> &other) const
    {
        DataManager<T> merged;
        merged.addDataList(this->dataList);
        merged.addDataList(other.dataList);
        return merged;
    }
    DataManager<T> &operator+=(const DataManager<T> &other)
    {
        this->addDataList(other.dataList);
        return *this;
    }

    // 迭代器支持
    typename std::vector<T>::iterator begin() { return dataList.begin(); }
    typename std::vector<T>::iterator end() { return dataList.end(); }
    typename std::vector<T>::const_iterator begin() const { return dataList.begin(); }
    typename std::vector<T>::const_iterator end() const { return dataList.end(); }

    // 模板方法：通用处理
    template <typename Func>
    void forEach(Func func) const
    {
        std::for_each(dataList.begin(), dataList.end(), func);
        operationCount++;
    }

    template <typename ResultType>
    std::vector<ResultType> transform(std::function<ResultType(const T &)> func) const
    {
        std::vector<ResultType> transformed;
        std::transform(dataList.begin(), dataList.end(),
                       std::back_inserter(transformed), func);
        operationCount++;
        return transformed;
    }

    // 静态方法
    static DataManager<T> merge(const DataManager<T> &a, const DataManager<T> &b)
    {
        return a + b;
    }
};