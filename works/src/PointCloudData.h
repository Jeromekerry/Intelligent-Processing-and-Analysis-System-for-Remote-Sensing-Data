#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unordered_map> //哈希表
#include <numeric>       //包括求和函数accumulate
#include <stdexcept>
#include <memory> //智能指针

#include "DataObject.h"

struct Point3D
{
    double x, y, z;     // 三维坐标
    double intensity;   // 回波强度
    int classification; // 分类标签

    Point3D(double x = 0, double y = 0, double z = 0,
            double intensity = 0, int cls = 0) : x(x), y(y), z(z), intensity(intensity), classification(cls) {}

    // 运算符重载
    Point3D operator+(const Point3D &other) const
    {
        return Point3D(
            x + other.x,
            y + other.y,
            z + other.z,
            (intensity + other.intensity) / 2.0, // 强度取平均更合理
            classification                       // 分类标签不参与加减
        );
    }
    Point3D operator-(const Point3D &other) const
    {
        return Point3D(
            x - other.x,
            y - other.y,
            z - other.z,
            std::abs(intensity - other.intensity), // 强度取差值绝对值
            classification                         // 分类标签不参与加减
        );
    }
    double distanceTo(const Point3D &other) const
    {
        return sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y) + (z - other.z) * (z - other.z));
    }
};

class PointCloudData : public DataObject
{
private:
    std::vector<Point3D> points;
    double minX, maxX, minY, maxY, minZ, maxZ; // 边界框

    void updateBounds()
    {
        if (points.empty())
        {
            minX = minY = minZ = 0.0;
            maxX = maxY = maxZ = 0.0;
            return;
        }

        minX = minY = minZ = 1e9;
        maxX = maxY = maxZ = -1e9;
        for (const auto &p : points)
        {
            minX = std::min(minX, p.x);
            minY = std::min(minY, p.y);
            minZ = std::min(minZ, p.z);
            maxX = std::max(maxX, p.x);
            maxY = std::max(maxY, p.y);
            maxZ = std::max(maxZ, p.z);
        }

        // 同步更新DataObject的size（点云大小按点数估算，单位MB）
        updateSize(points.size() * sizeof(Point3D) / (1024.0 * 1024.0));
    }

public:
    PointCloudData(const std::string &id, const std::string &name, const std::string &path) : DataObject(id, name, path), minX(0), maxX(0), minY(0), maxY(0), minZ(0), maxZ(0) {}
    PointCloudData(const PointCloudData &other) : DataObject(other), points(other.points), minX(other.minX), maxX(other.maxX), minY(other.minY), maxY(other.maxY), minZ(other.minZ), maxZ(other.maxZ) {}
    PointCloudData &operator=(const PointCloudData &other)
    {
        if (this != &other)
        {
            DataObject::operator=(other);
            points = other.points;
            minX = other.minX;
            maxX = other.maxX;
            minY = other.minY;
            maxY = other.maxY;
            minZ = other.minZ;
            maxZ = other.maxZ;
        }
        return *this;
    }
    ~PointCloudData() override {}

    // 实现基类虚函数
    void display() const override
    {
        accessCount++;
        std::cout << "=== PointCloudData ===" << std::endl;
        std::cout << "ID: " << getId() << std::endl;
        std::cout << "Name: " << getName() << std::endl;
        std::cout << "Path: " << getPath() << std::endl;
        std::cout << "Point Count: " << getPointCount() << std::endl;
        std::cout << "Bounds (X): [" << minX << ", " << maxX << "]" << std::endl;
        std::cout << "Bounds (Y): [" << minY << ", " << maxY << "]" << std::endl;
        std::cout << "Bounds (Z): [" << minZ << ", " << maxZ << "]" << std::endl;
    }

    DataObject *clone() const override // 如果使用需要释放。
    {
        return new PointCloudData(*this);
    }

    std::shared_ptr<DataObject> cloneShared() const // 使用智能指针。
    {
        return std::make_shared<PointCloudData>(*this);
    }

    bool exportData(const std::string &format) const override
    {
        accessCount++;
        if (format != "txt" && format != "pcd")
        {
            std::cerr << "Error: Unsupported export format " << format << std::endl;
            return false;
        }
        std::ofstream file("./output/" + getName() + "." + format);
        if (!file)
        {
            std::cout << "Failed to create txt file!" << std::endl;
            return false;
        }

        if (format == "txt")
        {
            file << "PointCloudData: " << getName() << "\n";
            file << "Point Count: " << getPointCount() << "\n";
            file << "X Y Z Intensity Classification\n";
            for (const auto &p : points)
            {
                file << p.x << " " << p.y << " " << p.z << " "
                     << p.intensity << " " << p.classification << "\n";
            }
        }
        else if (format == "pcd")
        { // 补充PCD格式（点云标准格式）  ai辅助
            file << "# .PCD v0.7 - Point Cloud Data file format\n";
            file << "VERSION 0.7\n";
            file << "FIELDS x y z intensity classification\n";
            file << "SIZE 8 8 8 8 4\n";
            file << "TYPE F F F F I\n";
            file << "COUNT 1 1 1 1 1\n";
            file << "WIDTH " << points.size() << "\n";
            file << "HEIGHT 1\n";
            file << "VIEWPOINT 0 0 0 1 0 0 0\n";
            file << "POINTS " << points.size() << "\n";
            file << "DATA ascii\n";
            for (const auto &p : points)
            {
                file << p.x << " " << p.y << " " << p.z << " "
                     << p.intensity << " " << p.classification << "\n";
            }
        }
        else
        {
            std::cerr << "Error: Unsupported export format " << format << std::endl;
            file.close();
            return false;
        }

        file.close();
        std::cout << "Successfully exported data in " << format << " format." << std::endl;
        return true;
    }

    std::string getType() const override { return "PointCloud"; }

    // 运算符重载
    PointCloudData operator+(const PointCloudData &other) const
    {
        PointCloudData res(this->getId() + "_merged", this->getName() + "_merged", this->getPath());
        res.addPoints(this->points);
        res.addPoints(other.points);
        return res;
    }
    PointCloudData &operator+=(const PointCloudData &other)
    {
        this->addPoints(other.points);
        return *this;
    }
    Point3D &operator[](int index)
    {
        if (index < 0 || (size_t)index >= points.size()) // 这样比较防止size_t转成int时导致溢出成负值
        {
            throw std::out_of_range("Point index out of bounds");
        }
        accessCount++;
        return points[index];
    }
    const Point3D &operator[](int index) const
    {
        if (index < 0 || (size_t)index >= points.size())
        {
            throw std::out_of_range("Point index out of bounds");
        }
        accessCount++;
        return points[index];
    }

    // 点云操作
    void addPoint(const Point3D &point) // 这里不使用updateBounds，使得时间复杂度由O(n)变成O(1)。
    {
        accessCount++;
        points.push_back(point);
        if (points.size() == 1)
        {
            minX = maxX = point.x;
            minY = maxY = point.y;
            minZ = maxZ = point.z;
        }
        else
        {
            // 三元运算符一步比较，O(1) 超快
            minX = (point.x < minX) ? point.x : minX;
            maxX = (point.x > maxX) ? point.x : maxX;

            minY = (point.y < minY) ? point.y : minY;
            maxY = (point.y > maxY) ? point.y : maxY;

            minZ = (point.z < minZ) ? point.z : minZ;
            maxZ = (point.z > maxZ) ? point.z : maxZ;
        }
        updateSize(points.size() * sizeof(Point3D) / (1024.0 * 1024.0));
    }

    void addPoints(const std::vector<Point3D> &newpoints)
    {
        accessCount++;
        points.insert(points.end(), newpoints.begin(), newpoints.end());
        updateBounds();
    }
    int getPointCount() const
    {
        accessCount++;
        return (int)points.size();
    }

    // 点云滤波(此处两个函数不懂其中的知识点，使用ai写的代码)
    PointCloudData voxelFilter(double voxelSize) const
    { // 体素(voxel)下采样（保留体素内中心点）
        if (voxelSize <= 0)
            throw std::invalid_argument("Voxel size must be positive");

        PointCloudData res(getId() + "_voxel", getName() + "_voxel", getPath());

        // 哈希表：key = 体素坐标，value = 该体素所有点
        std::unordered_map<std::string, std::vector<Point3D>> voxelGroups;

        for (const auto &p : points)
        {
            // 修复：负数坐标必须用 floor
            int vx = static_cast<int>(floor(p.x / voxelSize));
            int vy = static_cast<int>(floor(p.y / voxelSize));
            int vz = static_cast<int>(floor(p.z / voxelSize));

            std::string key = std::to_string(vx) + "_" + std::to_string(vy) + "_" + std::to_string(vz);
            voxelGroups[key].push_back(p);
        }

        // 每个体素生成一个中心点（分类取最多的）
        for (const auto &group : voxelGroups)
        {
            const auto &pts = group.second;

            double sumX = 0, sumY = 0, sumZ = 0, sumIntensity = 0;

            // 分类投票统计
            std::unordered_map<int, int> clsCount;
            for (const auto &p : pts)
            {
                sumX += p.x;
                sumY += p.y;
                sumZ += p.z;
                sumIntensity += p.intensity;
                clsCount[p.classification]++;
            }

            // 找最多的分类
            int bestCls = pts[0].classification;
            int maxCnt = 0;
            for (const auto &pair : clsCount)
            {
                if (pair.second > maxCnt)
                {
                    maxCnt = pair.second;
                    bestCls = pair.first;
                }
            }

            // 生成中心点
            double cnt = pts.size();
            Point3D center(
                sumX / cnt,
                sumY / cnt,
                sumZ / cnt,
                sumIntensity / cnt,
                bestCls);

            res.addPoint(center);
        }

        return res;
    }

    PointCloudData statisticalOutlierRemoval(int k, double stdDev) const
    { // 统计离群点移除（K近邻+标准差）
        if (k <= 0 || stdDev <= 0)
            throw std::invalid_argument("k and stdDev must be positive");

        int n = (int)points.size();
        if (n <= k)
            return *this;

        PointCloudData res(getId() + "_sor", getName() + "_sor", getPath());
        std::vector<double> meanDists;

        for (int i = 0; i < n; i++)
        {
            const auto &p = points[i];
            std::vector<double> dists;

            for (int j = 0; j < n; j++)
            {
                if (i == j)
                    continue;
                dists.push_back(p.distanceTo(points[j]));
            }

            // 排序取最近 k 个
            std::sort(dists.begin(), dists.end());
            double sum = 0.0;
            for (int d = 0; d < k; d++)
                sum += dists[d];

            meanDists.push_back(sum / k);
        }

        // 计算距离均值和标准差
        double avg = 0.0;
        for (double d : meanDists)
            avg += d;
        avg /= n;

        double var = 0.0;
        for (double d : meanDists)
            var += (d - avg) * (d - avg);
        var /= n;
        double sigma = sqrt(var);

        // 保留正常点
        double limit = avg + stdDev * sigma;
        for (int i = 0; i < n; i++)
        {
            if (meanDists[i] < limit)
                res.addPoint(points[i]);
        }

        return res;
    }

    // 统计信息
    double getAverageHeight() const
    { // 平均高度，z轴
        accessCount++;
        if (points.empty())
            return 0.0;

        double sumZ = 0;
        for (const auto &p : points)
        {
            sumZ += p.z;
        }
        return sumZ / points.size();
    }
    double getHeightRange() const
    { // z轴范围
        accessCount++;
        return maxZ - minZ;
    }
    void printStatistics() const
    {
        accessCount++;
        if (points.empty())
        {
            std::cout << "Point cloud is empty!" << std::endl;
            return;
        }

        std::cout << "=== Point Cloud Statistics ===" << std::endl;
        std::cout << "Total Points: " << points.size() << std::endl;
        std::cout << "Average Height (Z): " << getAverageHeight() << std::endl;
        std::cout << "Height Range (Z): " << getHeightRange() << " (Min: " << minZ << ", Max: " << maxZ << ")" << std::endl;
        std::cout << "X Range: " << maxX - minX << " (Min: " << minX << ", Max: " << maxX << ")" << std::endl;
        std::cout << "Y Range: " << maxY - minY << " (Min: " << minY << ", Max: " << maxY << ")" << std::endl;

        // 统计分类标签分布
        std::unordered_map<int, int> clsCount;
        double avgIntensity = 0;
        for (const auto &p : points)
        {
            clsCount[p.classification]++; // 对应标签类里个数加1
            avgIntensity += p.intensity;
        }
        avgIntensity /= points.size();

        std::cout << "Average Intensity: " << avgIntensity << std::endl;
        std::cout << "Classification Distribution:" << std::endl;
        for (const auto &pair : clsCount)
        {
            std::cout << "  Class " << pair.first << ": " << pair.second << " points ("
                      << (pair.second * 100.0 / points.size()) << "%)" << std::endl;
        }
    }

    // 获取器
    const std::vector<Point3D> &getPoints() const
    {
        accessCount++;
        return points;
    }
    double getMinX() const
    {
        accessCount++;
        return minX;
    }
    double getMaxX() const
    {
        accessCount++;
        return maxX;
    }
    double getMinY() const
    {
        accessCount++;
        return minY;
    }
    double getMaxY() const
    {
        accessCount++;
        return maxY;
    }
    double getMinZ() const
    {
        accessCount++;
        return minZ;
    }
    double getMaxZ() const
    {
        accessCount++;
        return maxZ;
    }
};