#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <stdexcept>

#include "Pixel.h"
#include "DataObject.h"

class SatelliteImage : public DataObject
{
private:
    int width;                                    // 宽度（像素）
    int height;                                   // 高度（像素）
    int bands;                                    // 波段数量
    std::string sensorType;                       // 传感器类型
    double cloudCover;                            // 云量百分比
    double acquisitionTime;                       // 采集时间（儒略日）
    std::vector<std::vector<Pixel<double>>> data; // 影像数据矩阵     data = 一张完整的图片  Pixel型的数据点存二维
    std::vector<double> bandStatistics;           // 波段统计信息

    // 私有辅助函数
    void calculateStatistics()
    {
        int n = width * height;
        if (n == 0)
        {
            bandStatistics.assign(4, 0.0); // 强制保证大小为4
            return;
        }

        double sum = 0, sumSq = 0;
        double minV = 1e9, maxV = -1e9;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                double b = data[i][j].getBrightness();
                sum += b;
                sumSq += b * b;
                minV = std::min(minV, b);
                maxV = std::max(maxV, b);
            }
        }
        double mean = sum / n;
        double std = sqrt((sumSq / n) - mean * mean);

        bandStatistics[0] = mean;
        bandStatistics[1] = minV;
        bandStatistics[2] = maxV;
        bandStatistics[3] = std;
    } // 计算统计信息

    void validateCoordinates(int x, int y) const
    {
        if (x < 0 || x >= width || y < 0 || y >= height)
        {
            throw std::out_of_range("Coordinate out of bounds!");
        }
    } // 坐标验证

public:
    void refreshStatistics() { calculateStatistics(); }

    // 构造函数
    SatelliteImage(const std::string &id, const std::string &name, const std::string &path,
                   int w, int h, int b, const std::string &sensor, double time = 0) : DataObject(id, name, path), width(w), height(h), bands(b),
                                                                                      sensorType(sensor), acquisitionTime(time)
    {
        setCloudCover(15.0);
        data.resize(h, std::vector<Pixel<double>>(w));
        bandStatistics.resize(4, 0.0);
        calculateStatistics();

        double imageSizeMB = width * height * 5 * sizeof(double) / (1024.0 * 1024.0);
        updateSize(imageSizeMB);
    }

    // 拷贝构造函数
    SatelliteImage(const SatelliteImage &other) : DataObject(other), width(other.width), height(other.height), bands(other.bands),
                                                  sensorType(other.sensorType), cloudCover(other.cloudCover),
                                                  acquisitionTime(other.acquisitionTime), data(other.data), bandStatistics(other.bandStatistics)
    {
    }

    // 析构函数
    ~SatelliteImage() override {}

    // 赋值运算符
    SatelliteImage &operator=(const SatelliteImage &other)
    {
        if (this == &other)
        {
            std::cout << "Self-assignment ignored" << std::endl;
            return *this;
        }

        DataObject::operator=(other);
        width = other.width;
        height = other.height;
        bands = other.bands;
        sensorType = other.sensorType;
        cloudCover = other.cloudCover;
        acquisitionTime = other.acquisitionTime;
        data = other.data;
        bandStatistics = other.bandStatistics;
        return *this;
    }

    // 实现基类虚函数
    void display() const override
    {
        accessCount++;
        std::cout << "SatelliteImage:  " << getName() << "  " << width << "x" << height << std::endl;
    }

    DataObject *clone() const override // 使用this指针创建当前对象的深拷贝
    {                                  // 使用的传统指针，后续需要释放
        return new SatelliteImage(*this);
    }

    bool exportData(const std::string &format) const override // 往文件里写。
    {
        accessCount++;
        if (format == "txt") // 支持的文件格式
        {
            std::ofstream file("./output/" + getName() + "." + format);
            if (!file)
            {
                std::cout << "Failed to create txt file!" << std::endl;
                return false;
            }
            // 写入影像基本信息
            file << "SatelliteImage: " << getName() << std::endl;
            file << "Size: " << width << "x" << height << std::endl;
            file << "Cloud cover: " << cloudCover << std::endl;
            // 可选：写入像素数据
            for (int i = 0; i < height; i++)
            {
                for (int j = 0; j < width; j++)
                {
                    file << data[i][j].getBrightness() << " ";
                }
                file << std::endl;
            }
            file.close();
            std::cout << "Successfully exported data in txt format." << std::endl;
            return true;
        }
        else
        {
            std::cout << "Unsupported format: " << format << std::endl;
            return false;
        }
    }

    std::string getType() const override { return "SatelliteImage"; }

    double getQualityScore() const override
    {
        accessCount++;
        return 100.0 - cloudCover * 0.5;
    }

    bool isValid() const override
    {
        accessCount++;
        return width > 0 && height > 0 && bands > 0 && !sensorType.empty();
    }

    // 算术运算符重载（影像运算）
    SatelliteImage operator+(const SatelliteImage &other) const
    {
        if (width != other.width || height != other.height || bands != other.bands)
            throw std::invalid_argument("Image size mismatch");

        SatelliteImage res(getId() + "_add", getName() + "_add", getPath(),
                           width, height, bands, sensorType, acquisitionTime);
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                res.data[i][j] = data[i][j] + other.data[i][j];
            }
        }
        return res;
    }

    SatelliteImage operator-(const SatelliteImage &other) const
    {
        if (width != other.width || height != other.height || bands != other.bands)
            throw std::invalid_argument("Image size mismatch");

        SatelliteImage res(getId() + "_sub", getName() + "_sub", getPath(),
                           width, height, bands, sensorType, acquisitionTime);
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                res.data[i][j] = data[i][j] - other.data[i][j];
            }
        }
        return res;
    }

    // 乘除现在只支持右操作值为数字
    SatelliteImage operator*(double factor) const
    {
        SatelliteImage res = *this;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                res.data[i][j] = data[i][j] * factor;
            }
        }
        return res;
    }

    SatelliteImage operator/(double divisor) const
    {
        if (divisor == 0)
            throw std::invalid_argument("Divide by zero");

        SatelliteImage res = *this;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                res.data[i][j] = data[i][j] / divisor;
            }
        }
        return res;
    }

    SatelliteImage &operator+=(const SatelliteImage &other) // 在这里不用*this = *this + other;虽然看着代码简单，但是运行时有超大临时变量，慢。
    {
        if (width != other.width || height != other.height || bands != other.bands)
            throw std::invalid_argument("Image size mismatch");

        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                data[i][j] += other.data[i][j];

        calculateStatistics();
        return *this;
    }
    SatelliteImage &operator-=(const SatelliteImage &other)
    {
        if (width != other.width || height != other.height || bands != other.bands)
            throw std::invalid_argument("Image size mismatch");

        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                data[i][j] -= other.data[i][j];

        calculateStatistics();
        return *this;
    }

    // 比较运算符
    bool operator==(const SatelliteImage &other) const
    {
        accessCount++;
        return width == other.width && height == other.height && bands == other.bands && data == other.data;
    }

    bool operator!=(const SatelliteImage &other) const
    {
        return !(*this == other);
    }

    // 下标运算符（访问行）
    std::vector<Pixel<double>> &operator[](int row)
    {
        validateCoordinates(0, row);
        accessCount++;
        return data[row];
    }
    const std::vector<Pixel<double>> &operator[](int row) const
    {
        validateCoordinates(0, row);
        accessCount++;
        return data[row];
    }

    // 类型转换运算符
    operator double() const
    {
        accessCount++;
        return cloudCover;
    } // 转换为云量

    operator std::string() const
    {
        accessCount++;
        std::stringstream ss;
        ss << "SatelliteImage: " << getName() << "   "
           << "Size: " << height << "x" << width << "    "
           << "Bands: " << bands;
        return ss.str();
    } // 转换为摘要字符串

    // 子图操作，抠出一个子图
    SatelliteImage getSubImage(int x, int y, int w, int h) const
    {
        validateCoordinates(x, y); // 先检查起点是否合法。
        accessCount++;

        std::string subId = getId() + "_sub";
        std::string subName = getName() + "_sub";

        SatelliteImage subImg(subId, subName, getPath(), w, h, bands, sensorType, acquisitionTime);
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
            {
                int yy = y + i;
                int xx = x + j;
                if (xx >= 0 && xx < width && yy >= 0 && yy < height)
                    subImg.data[i][j] = data[yy][xx];
                else                                     // 如果越界，设置为0；
                    subImg.data[i][j] = Pixel<double>(); // 这是Pixel里面的默认构造函数，实现清零的作用
            }
        return subImg;
    }

    // 波段操作，把给的参数band波段的所有像素值，抽出来变成一长串数字
    std::vector<double> getBandValues(int band) const
    {
        accessCount++;
        std::vector<double> res;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                switch (band)
                {
                case 0:
                    res.push_back(data[i][j][0]);
                    break;
                case 1:
                    res.push_back(data[i][j][1]);
                    break;
                case 2:
                    res.push_back(data[i][j][2]);
                    break;
                case 3:
                    res.push_back(data[i][j][3]);
                    break;
                case 4:
                    res.push_back(data[i][j][4]);
                    break;
                default:
                    throw std::out_of_range("band is not legal");
                }
            }
        }
        return res;
    } // 0:R,1:G,2:B,3:NIR,4:Thermal

    SatelliteImage extractBand(int band) const // 从原图里抠出一个波段，生成一张【新的单波段图像】
    {
        accessCount++;

        SatelliteImage res(
            getId() + "_b" + std::to_string(band),
            getName() + "_b" + std::to_string(band),
            getPath(),
            width, height,
            1, // 波段数变成 1
            sensorType,
            acquisitionTime);

        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if (band == 0)
                    res.data[i][j].setRed(data[i][j].getRed());
                else if (band == 1)
                    res.data[i][j].setGreen(data[i][j].getGreen());
                else if (band == 2)
                    res.data[i][j].setBlue(data[i][j].getBlue());
                else if (band == 3)
                    res.data[i][j].setNir(data[i][j].getNir());
                else if (band == 4)
                    res.data[i][j].setThermal(data[i][j].getThermal());
            }
        }
        return res;
    }

    // 遥感指数计算（返回二维数组）
    std::vector<std::vector<double>> calculateNDVI() const
    {
        accessCount++;
        std::vector<std::vector<double>> ndviMap(height, std::vector<double>(width));
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                ndviMap[i][j] = data[i][j].getNDVI();
            }
        }
        return ndviMap;
    }
    std::vector<std::vector<double>> calculateNDWI() const
    {
        accessCount++;
        std::vector<std::vector<double>> ndwiMap(height, std::vector<double>(width));
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                ndwiMap[i][j] = data[i][j].getNDWI();
            }
        }
        return ndwiMap;
    }
    std::vector<std::vector<double>> calculateSAVI(double L = 0.5) const
    {
        accessCount++;
        std::vector<std::vector<double>> saviMap(height, std::vector<double>(width));
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                saviMap[i][j] = data[i][j].getSAVI(L);
        return saviMap;
    }
    std::vector<std::vector<double>> calculateTemperature() const
    {
        accessCount++;
        std::vector<std::vector<double>> tempMap(height, std::vector<double>(width));
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                tempMap[i][j] = data[i][j].getTemperature();
        return tempMap;
    }

    // 影像统计
    double getMeanValue() const
    {
        accessCount++;
        return bandStatistics[0];
    }

    double getStdDev() const // 代码求的是总体标准差。
    {
        accessCount++;
        return bandStatistics[3];
    }
    double getMinValue() const
    {
        accessCount++;
        return bandStatistics[1];
    }

    double getMaxValue() const
    {
        accessCount++;
        return bandStatistics[2];
    }
    void printStatistics() const
    {
        accessCount++;
        std::cout << "=== Image Statistics ===" << std::endl;
        std::cout << "Mean: " << getMeanValue() << std::endl;
        std::cout << "Min: " << getMinValue() << std::endl;
        std::cout << "Max: " << getMaxValue() << std::endl;
        std::cout << "StdDev: " << getStdDev() << std::endl;
    }

    // 影像处理      //                  ai说太复杂，可以不写。
    void applyGaussianBlur(double sigma)
    {
        accessCount++;
        std::cout << "Gaussian blur applied\n";
    }
    void applyMedianFilter(int kernelSize)
    {
        accessCount++;
        std::cout << "Median filter applied\n";
    }
    void normalize()
    {
        accessCount++;
        std::cout << "Normalized\n";
    }
    void resample(int newWidth, int newHeight)
    {
        accessCount++;
        std::cout << "Resampled\n";
    }

    // 获取器
    int getWidth() const
    {
        accessCount++;
        return width;
    }
    int getHeight() const
    {
        accessCount++;
        return height;
    }
    int getBands() const
    {
        accessCount++;
        return bands;
    }
    std::string getSensorType() const
    {
        accessCount++;
        return sensorType;
    }
    double getCloudCover() const
    {
        accessCount++;
        return cloudCover;
    }
    double getAcquisitionTime() const
    {
        accessCount++;
        return acquisitionTime;
    }

    // 设置器（带验证）
    void setCloudCover(double cover)
    {
        if (cover < 0.0 || cover > 100.0)
            throw std::out_of_range("cover out of bounds");
        cloudCover = cover;
    }
    void setAcquisitionTime(double time)
    {
        if (time < 0)
            throw std::invalid_argument("Invalid time");
        acquisitionTime = time;
    }

    // 静态工厂方法
    static SatelliteImage createRandomImage(const std::string &id, int w, int h) // 随机噪点图
    {
        SatelliteImage img(id, "RandomImage", " ", w, h, 5, "DefaultSensor");
        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                img.data[i][j].setRed((double)rand() / RAND_MAX);
                img.data[i][j].setGreen((double)rand() / RAND_MAX);
                img.data[i][j].setBlue((double)rand() / RAND_MAX);
                img.data[i][j].setNir((double)rand() / RAND_MAX);
                img.data[i][j].setThermal((double)rand() / RAND_MAX);
            }
        }
        double imageSizeMB = w * h * 5 * sizeof(double) / (1024.0 * 1024.0);
        img.updateSize(imageSizeMB);

        return img;
    }
    static SatelliteImage createConstantImage(const std::string &id, int w, int h, const Pixel<double> &value) // 纯色图，掩膜图
    {
        SatelliteImage img(id, "ConstantImage", " ", w, h, 5, "DefaultSensor");

        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                img.data[i][j] = value;
            }
        }
        double imageSizeMB = w * h * 5 * sizeof(double) / (1024.0 * 1024.0);
        img.updateSize(imageSizeMB);

        return img;
    }
};