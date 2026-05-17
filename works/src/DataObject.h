#pragma once

#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <stdexcept>

class DataExporter;

class DataObject
{
    friend class DataExporter;
    friend bool exportToCSV(const DataObject &data, const std::string &filename);
    friend bool exportToJSON(const DataObject &data, const std::string &filename);

protected:
    std::string id;                              // 唯一标识符,在判断时只用id即可
    std::string name;                            // 数据名称
    std::string path;                            // 存储路径
    double size;                                 // 数据大小(MB)
    time_t createTime;                           // 创建时间
    std::map<std::string, std::string> metadata; // 元数据字典
    mutable int accessCount;                     // 访问计数

public:
    // 构造函数与析构函数
    DataObject(const std::string &id, const std::string &name, const std::string &path) : id(id), name(name), path(path), size(0.0), accessCount(0)
    {
        createTime = time(NULL);
        totalObjects++;
    }

    virtual ~DataObject() {}

    // 拷贝构造函数（深拷贝）
    DataObject(const DataObject &other) : id(other.id), name(other.name), path(other.path), size(other.size), createTime(other.createTime), accessCount(0), metadata(other.metadata)
    {
        totalObjects++;
    }

    // 赋值运算符
    DataObject &operator=(const DataObject &other)
    {
        if (this == &other)
        {
            std::cout << "Self-assignment ignored" << std::endl;
            return *this;
        }
        this->id = other.id;
        this->name = other.name;
        this->path = other.path;
        this->size = other.size;
        this->createTime = other.createTime;
        this->metadata = other.metadata;
        this->accessCount = 0;

        return *this;
    }

    // 纯虚函数（使DataObject成为抽象类）
    virtual void display() const = 0;
    virtual DataObject *clone() const = 0;
    virtual bool exportData(const std::string &format) const = 0;

    // 虚函数（可被子类重写）
    virtual std::string getType() const { return "DataObject"; }
    virtual double getQualityScore() const { return 100.0; } //
    virtual bool isValid() const { return true; }

    // 运算符重载
    bool operator==(const DataObject &other) const { return id == other.id; } // id时唯一标识符
    bool operator!=(const DataObject &other) const { return id != other.id; }
    bool operator<(const DataObject &other) const { return size < other.size; }

    // 类型转换运算符
    operator std::string() const { return name + " (" + id + ")"; }
    operator double() const { return size; }

    // 获取器（const成员函数）
    std::string getId() const
    {
        accessCount++;
        return id;
    }
    std::string getName() const
    {
        accessCount++;
        return name;
    }
    std::string getPath() const
    {
        accessCount++;
        return path;
    }
    double getSize() const
    {
        accessCount++;
        return size;
    }
    time_t getCreateTime() const
    {
        accessCount++;
        return createTime;
    }
    int getAccessCount() const
    {
        return accessCount;
    }

    // 元数据操作
    void addMetadata(const std::string &key, const std::string &value)
    {
        metadata[key] = value;
        accessCount++;
    }
    std::string getMetadata(const std::string &key) const
    {
        accessCount++;
        auto it = metadata.find(key);
        if (it != metadata.end())
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error("Metadata key not found: " + key);
        }
    }
    bool hasMetadata(const std::string &key) const
    {
        accessCount++;
        return metadata.find(key) != metadata.end();
    }
    void removeMetadata(const std::string &key)
    {
        accessCount++;
        auto it = metadata.find(key);
        if (it != metadata.end())
        {
            metadata.erase(key);
        }
    }

    // 静态成员（所有对象共享）
    static int getTotalObjects()
    {
        return totalObjects;
    }
    static void resetTotalObjects()
    {
        totalObjects = 0;
    }

protected:
    void updateSize(double newSize)
    {
        size = newSize;
        accessCount++;
    }
    void setPath(const std::string &newPath)
    {
        path = newPath;
        accessCount++;
    }

private:
    static int totalObjects; // 统计创建的对象总数
};
