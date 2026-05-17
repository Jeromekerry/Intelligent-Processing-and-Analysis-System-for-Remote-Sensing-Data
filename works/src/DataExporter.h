#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <ctime>

#include "DataObject.h"
#include "SatelliteImage.h"
#include "PointCloudData.h"

// 前置声明
bool exportToCSV(const DataObject &data, const std::string &filename);
bool exportToJSON(const DataObject &data, const std::string &filename);

class DataExporter
{
private:
    std::string exportPath;
    int exportCount;

public:
    DataExporter(const std::string &path = "./Export/") : exportPath(path), exportCount(0)
    {
        if (!exportPath.empty() && exportPath.back() != '/' && exportPath.back() != '\\')
        {
            exportPath += '/';
        }
    }
    ~DataExporter() {}

    // 友元函数声明（将作为DataObject的友元）

    friend bool exportToCSV(const DataObject &data, const std::string &filename);

    friend bool exportToJSON(const DataObject &data, const std::string &filename);

    // 批量导出
    template <typename T>
    int batchExport(const std::vector<T> &dataList, const std::string &format)
    {
        if (format != "csv" && format != "json")
        {
            throw std::invalid_argument(format + " is not supported,please use 'csv' or 'json'");
        }
        int successCount = 0; // exportCount 是历史累计（多次调用 batchExport 会累加），而 successCount 是本次调用的成功数。
        for (const auto &data : dataList)
        {
            std::string filename = exportPath + data.getName() + "_" + std::to_string(std::time(nullptr)) + "." + format; // 加时间戳,防止批量导出多个同名对象时，文件被覆盖。
            bool success = false;
            if (format == "csv")
            {
                success = exportToCSV(data, filename);
            }
            else
            {
                success = exportToJSON(data, filename);
            }
            if (success)
            {
                successCount++;
            }
        }
        exportCount += successCount;
        return successCount;
    }

    // 统计
    void printExportStats() const
    {
        std::cout << "===Exporter Statistics===" << std::endl;
        std::cout << "Total export path: " << exportPath << std::endl;
        std::cout << "Number of successful exports: " << exportCount << std::endl;
    }
};

// 全局友元函数

bool exportToCSV(const DataObject &data, const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Failed to create CSV file " << filename << std::endl;
        return false;
    }
    file << "General Information\n"
         << "ID," << data.id << "\n"
         << "Name," << data.name << "\n"
         << "Path," << data.path << "\n"
         << "Type," << data.getType() << "\n"
         << "Size(MB)," << data.size << "\n"
         << "Access Count," << data.accessCount << "\n"
         << "\n";

    std::string type = data.getType();

    if (type == "PointCloud")
    {
        const PointCloudData *pc = dynamic_cast<const PointCloudData *>(&data);
        if (pc)
        {
            file << "Point Cloud Detail\n"
                 << "Point Count," << pc->getPointCount() << "\n"
                 << "X Range," << pc->getMinX() << "," << pc->getMaxX() << "\n"
                 << "Y Range," << pc->getMinY() << "," << pc->getMaxY() << "\n"
                 << "Z Range," << pc->getMinZ() << "," << pc->getMaxZ() << "\n"
                 << "Average Height," << pc->getAverageHeight() << "\n"
                 << "\nCoordinates(X,Y,Z,Intensity,Classification)\n";
            const auto &points = pc->getPoints();
            for (const auto &p : points)
            {
                file << p.x << "," << p.y << "," << p.z << "," << p.intensity << "," << p.classification << "\n";
            }
        }
    }
    else if (type == "SatelliteImage")
    {
        const SatelliteImage *img = dynamic_cast<const SatelliteImage *>(&data);
        if (img)
        {
            file << "Image Detail\n"
                 << "Width," << img->getWidth() << "\n"
                 << "Height," << img->getHeight() << "\n"
                 << "Bands," << img->getBands() << "\n"
                 << "Sensor," << img->getSensorType() << "\n"
                 << "Cloud Cover(%)," << img->getCloudCover() << "\n"
                 << "Quality Score," << img->getQualityScore() << "\n"
                 << "\nStatistics(Mean,Min,Max,StdDev)\n"
                 << img->getMeanValue() << ","
                 << img->getMinValue() << ","
                 << img->getMaxValue() << ","
                 << img->getStdDev() << "\n";
        }
    }
    else
    {
        file << "Data Content,No specific detail handler\n";
    }
    file.close();
    std::cout << "Successfully exported CSV file: " << filename << std::endl;
    return true;
}

bool exportToJSON(const DataObject &data, const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Failed to create JSON file " << filename << std::endl;
        return false;
    }
    file << "{\n";
    file << "  \"basic_info\": {\n";
    file << "    \"id\": \"" << data.id << "\",\n";
    file << "    \"name\": \"" << data.name << "\",\n";
    file << "    \"path\": \"" << data.path << "\",\n";
    file << "    \"type\": \"" << data.getType() << "\",\n";
    file << "    \"size_mb\": " << data.size << ",\n";
    file << "    \"access_count\": " << data.accessCount << "\n";
    file << "  }";
    std::string type = data.getType();
    if (type == "PointCloud")
    {
        const PointCloudData *pc = dynamic_cast<const PointCloudData *>(&data);
        if (pc)
        {
            file << ",\n  \"point_cloud_detail\": {\n";
            file << "    \"point_count\": " << pc->getPointCount() << ",\n";
            file << "    \"bounds\": {\n";
            file << "      \"x\": [" << pc->getMinX() << ", " << pc->getMaxX() << "],\n";
            file << "      \"y\": [" << pc->getMinY() << ", " << pc->getMaxY() << "],\n";
            file << "      \"z\": [" << pc->getMinZ() << ", " << pc->getMaxZ() << "]\n";
            file << "    },\n";
            file << "    \"average_height\": " << pc->getAverageHeight() << "\n";
            file << "  }";
        }
    }
    else if (type == "SatelliteImage")
    {
        const SatelliteImage *img = dynamic_cast<const SatelliteImage *>(&data);
        if (img)
        {
            file << ",\n  \"satellite_image_detail\": {\n";
            file << "    \"width\": " << img->getWidth() << ",\n";
            file << "    \"height\": " << img->getHeight() << ",\n";
            file << "    \"bands\": " << img->getBands() << ",\n";
            file << "    \"sensor\": \"" << img->getSensorType() << "\",\n";
            file << "    \"cloud_cover\": " << img->getCloudCover() << ",\n";
            file << "    \"quality_score\": " << img->getQualityScore() << "\n";
            file << "  }";
        }
    }
    else
    {
        file << ",\n  \"data_content\": \"No specific detail handler\"";
    }
    file << "\n}\n";
    file.close();
    std::cout << "Successfully exported JSON file: " << filename << std::endl;
    return true;
}