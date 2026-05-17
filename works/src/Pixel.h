#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

template <typename T>
class Pixel
{
private:
    T red, green, blue, nir; // 多光谱波段
    T thermal;               // 热红外波段
    double timestamp;        // 采集时间戳,例如202501011230
    mutable int accessCount; // 访问计数（mutable演示）

public:
    // 构造函数（支持默认参数）
    Pixel(T r = 0, T g = 0, T b = 0, T n = 0, T t = 0, double ts = 0) : red(r), green(g), blue(b), nir(n), thermal(t), timestamp(ts), accessCount(0) {}

    // 拷贝构造函数
    Pixel(const Pixel<T> &other) : red(other.red), green(other.green), blue(other.blue), nir(other.nir), thermal(other.thermal), timestamp(other.timestamp), accessCount(0) {}

    // 析构函数
    ~Pixel() {}

    // 赋值运算符
    Pixel<T> &operator=(const Pixel<T> &other)
    {
        if (this == &other)
        {
            std::cout << "Self-assignment ignored" << std::endl;
            return *this;
        }
        red = other.red;
        green = other.green;
        blue = other.blue;
        nir = other.nir;
        thermal = other.thermal;
        timestamp = other.timestamp;

        return *this;
    }

    // 算术运算符重载
    Pixel<T> operator+(const Pixel<T> &other) const
    {
        Pixel<T> res;
        res.red = red + other.red;
        res.green = green + other.green;
        res.blue = blue + other.blue;
        res.nir = nir + other.nir;
        res.thermal = thermal + other.thermal;
        res.timestamp = this->timestamp; // 保留了原时间戳。
        return res;
    }

    Pixel<T> operator-(const Pixel<T> &other) const
    {
        Pixel<T> res;
        res.red = red - other.red;
        res.green = green - other.green;
        res.blue = blue - other.blue;
        res.nir = nir - other.nir;
        res.thermal = thermal - other.thermal;
        res.timestamp = this->timestamp;
        return res;
    }

    Pixel<T> operator*(T factor) const // 仅支持右×数字。
    {
        Pixel<T> res;
        res.red = red * factor;
        res.green = green * factor;
        res.blue = blue * factor;
        res.nir = nir * factor;
        res.thermal = thermal * factor;
        res.timestamp = timestamp;
        return res;
    }

    Pixel<T> operator/(T divisor) const
    {
        if (divisor == 0)
        {
            throw std::runtime_error("Divisor cannot be zero");
        }
        Pixel<T> res;
        res.red = red / divisor;
        res.green = green / divisor;
        res.blue = blue / divisor;
        res.nir = nir / divisor;
        res.thermal = thermal / divisor;
        res.timestamp = timestamp;
        return res;
    }

    Pixel<T> &operator+=(const Pixel<T> &other)
    {
        red += other.red;
        green += other.green;
        blue += other.blue;
        nir += other.nir;
        thermal += other.thermal;

        return *this;
    }

    Pixel<T> &operator-=(const Pixel<T> &other)
    {
        red -= other.red;
        green -= other.green;
        blue -= other.blue;
        nir -= other.nir;
        thermal -= other.thermal;

        return *this;
    }

    // 比较运算符
    bool operator==(const Pixel<T> &other) const
    {
        accessCount++;
        return this->red == other.red && this->blue == other.blue && this->green == other.green && this->nir == other.nir && this->thermal == other.thermal && this->timestamp == other.timestamp;
    }
    bool operator!=(const Pixel<T> &other) const
    {
        accessCount++;
        return !(*this == other); // 复用==，更简洁。
    }

    // 类型转换运算符
    operator T() const
    {
        accessCount++;
        return getBrightness();
    } // 转换为亮度值
    operator std::string() const
    {
        accessCount++;
        std::stringstream ss;

        ss << "red:" << red << "  "
           << "green:" << green << "  "
           << "blue:" << blue << "  "
           << "nir:" << nir << "  "
           << "thermal:" << thermal << "  "
           << "timestamp:" << timestamp;

        return ss.str();
    } // 转换为字符串

    // 下标运算符（访问波段）
    T &operator[](int band)
    {
        accessCount++; // 访问波段时计数
        switch (band)
        {
        case 0:
            return red;
        case 1:
            return green;
        case 2:
            return blue;
        case 3:
            return nir;
        case 4:
            return thermal;
        default:
            throw std::out_of_range("the index is wrong");
        }
    }

    const T &operator[](int band) const
    {
        accessCount++;
        switch (band)
        {
        case 0:
            return red;
        case 1:
            return green;
        case 2:
            return blue;
        case 3:
            return nir;
        case 4:
            return thermal;
        default:
            throw std::out_of_range("the index is wrong");
        }
    }

    // 友元函数（流输出）
    template <typename U> // 不能用T，因为友元函数不是类的成员，不能用。
    friend std::ostream &operator<<(std::ostream &os, const Pixel<U> &pixel)
    {
        pixel.accessCount++;
        os << "R:" << pixel.red << " G:" << pixel.green
           << " B:" << pixel.blue << " NIR:" << pixel.nir
           << " Thermal:" << pixel.thermal;
        return os;
    }

    // 遥感指数计算
    double getNDVI() const
    {
        accessCount++;
        double sum = (double)nir + (double)red;
        if (sum == 0)
        {
            return 0.0;
        }
        return ((double)nir - (double)red) / sum;
    } // 归一化植被指数
    double getNDWI() const
    {
        accessCount++;
        double sum = (double)green + (double)nir;
        if (sum == 0)
        {
            return 0.0;
        }
        return ((double)green - (double)nir) / sum;
    } // 归一化水体指数
    double getSAVI(double L) const
    {
        accessCount++;
        double sum = (double)nir + (double)red + L;
        if (sum == 0)
        {
            return 0.0;
        }
        return (((double)nir - (double)red) * (1 + L)) / sum;
    } // 土壤调节植被指数
    double getBrightness() const
    {
        accessCount++;
        return ((double)red + (double)green + (double)blue + (double)nir) / 4;
    } // 亮度值
    double getTemperature() const
    {
        accessCount++;
        return (double)thermal / 100.0;
    } // 亮温（基于热红外）

    // 获取器
    T getRed() const
    {
        accessCount++;
        return red;
    }
    T getGreen() const
    {
        accessCount++;
        return green;
    }
    T getBlue() const
    {
        accessCount++;
        return blue;
    }
    T getNir() const
    {
        accessCount++;
        return nir;
    }
    T getThermal() const
    {
        accessCount++;
        return thermal;
    }
    int getAccessCount() const { return accessCount; }

    // 设置器
    void setRed(T r) { red = r; }
    void setGreen(T g) { green = g; }
    void setBlue(T b) { blue = b; }
    void setNir(T n) { nir = n; }
    void setThermal(T t) { thermal = t; }
};