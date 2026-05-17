#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <limits>
#include <unordered_map>
#include <stdexcept>

namespace RemoteSensingAlgorithms
{
    // 植被指数
    inline double calculateNDVI(double red, double nir)
    {
        double sum = red + nir;
        if (sum == 0.0)
            return 0.0;
        return (nir - red) / sum;
    }
    inline double calculateNDWI(double green, double nir)
    {
        double sum = green + nir;
        if (sum == 0.0)
            return 0.0;
        return (green - nir) / sum;
    }
    inline double calculateSAVI(double red, double nir, double L = 0.5)
    {
        double sum = nir + red + L;
        if (sum == 0.0)
            return 0.0;
        return ((nir - red) * (1.0 + L)) / sum;
    }
    inline double calculateEVI(double blue, double red, double nir)
    {
        double denominator = nir + 6.0 * red - 7.5 * blue + 1.0;
        if (denominator == 0.0)
            return 0.0;
        return 2.5 * (nir - red) / denominator;
    }

    // 水体指数
    inline double calculateMNDWI(double green, double swir)
    {
        double sum = green + swir;
        if (sum == 0.0)
            return 0.0;
        return (green - swir) / sum;
    }
    inline double calculateAWEI(double blue, double green, double nir, double swir1, double swir2)
    {
        double numerator = 4.0 * (green - swir1);
        double denominator = 0.25 * nir + 2.75 * swir2;
        if (denominator == 0.0)
            return 0.0;
        return numerator / denominator;
    }

    // 分类算法
    template <typename T> // 聚类
    std::vector<int> kMeansClustering(const std::vector<T> &data, int k, int maxIter = 100)
    {
        int n = (int)data.size();
        if (n == 0 || k <= 0 || k > n)
        {
            throw std::invalid_argument("Invalid k or empty data");
        }

        // 随机初始化中心点
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, n - 1);

        std::vector<T> centroids(k);
        std::vector<bool> selected(n, false);
        for (int i = 0; i < k; i++)
        {
            int idx;
            do
            {
                idx = dis(gen);
            } while (selected[idx]);
            selected[idx] = true;
            centroids[i] = data[idx];
        }

        std::vector<int> labels(n, 0);

        for (int iter = 0; iter < maxIter; iter++)
        {
            bool changed = false;

            // 分配步骤：每个点分配到最近的中心
            for (int i = 0; i < n; i++)
            {
                double minDist = std::numeric_limits<double>::max();
                int bestLabel = 0;

                for (int j = 0; j < k; j++)
                {
                    double dist = std::abs((double)data[i] - (double)centroids[j]);
                    if (dist < minDist)
                    {
                        minDist = dist;
                        bestLabel = j;
                    }
                }

                if (labels[i] != bestLabel)
                {
                    labels[i] = bestLabel;
                    changed = true;
                }
            }

            // 更新步骤：重新计算中心点
            std::vector<double> sums(k, 0.0);
            std::vector<int> counts(k, 0);

            for (int i = 0; i < n; i++)
            {
                sums[labels[i]] += (double)data[i];
                counts[labels[i]]++;
            }

            for (int j = 0; j < k; j++)
            {
                if (counts[j] > 0)
                {
                    centroids[j] = (T)(sums[j] / counts[j]);
                }
            }

            // 如果标签不再变化，提前收敛
            if (!changed)
                break;
        }

        return labels;
    }

    // 图像处理:卷积
    template <typename T>
    std::vector<std::vector<T>> convolve(const std::vector<std::vector<T>> &image,
                                         const std::vector<std::vector<double>> &kernel)
    {
        int imgH = (int)image.size();
        if (imgH == 0)
            return {};
        int imgW = (int)image[0].size();

        int kH = (int)kernel.size();
        if (kH == 0)
            return image;
        int kW = (int)kernel[0].size();

        int padH = kH / 2;
        int padW = kW / 2;

        std::vector<std::vector<T>> result(imgH, std::vector<T>(imgW, 0));

        for (int i = 0; i < imgH; i++)
        {
            for (int j = 0; j < imgW; j++)
            {
                double sum = 0.0;
                for (int ki = 0; ki < kH; ki++)
                {
                    for (int kj = 0; kj < kW; kj++)
                    {
                        int ii = i + ki - padH;
                        int jj = j + kj - padW;

                        // 边界处理：越界时用镜像填充
                        if (ii < 0)
                            ii = -ii - 1;
                        if (ii >= imgH)
                            ii = 2 * imgH - ii - 1;
                        if (jj < 0)
                            jj = -jj - 1;
                        if (jj >= imgW)
                            jj = 2 * imgW - jj - 1;

                        sum += (double)image[ii][jj] * kernel[ki][kj];
                    }
                }
                result[i][j] = (T)sum;
            }
        }
        return result;
    }

    // 统计分析
    template <typename T> //// 计算信息熵
    double calculateEntropy(const std::vector<T> &data)
    {
        if (data.empty())
            return 0.0;

        // 统计频率
        std::unordered_map<T, int> freq;
        for (const auto &val : data)
        {
            freq[val]++;
        }

        int n = (int)data.size();
        double entropy = 0.0;

        for (const auto &pair : freq)
        {
            double p = (double)pair.second / n;
            if (p > 0.0)
            {
                entropy -= p * std::log2(p);
            }
        }
        return entropy;
    }

    template <typename T> // 直方图
    std::vector<double> calculateHistogram(const std::vector<T> &data, int bins)
    {
        if (data.empty() || bins <= 0)
            return {};

        // 找到最小最大值
        T minVal = *std::min_element(data.begin(), data.end());
        T maxVal = *std::max_element(data.begin(), data.end());

        if (minVal == maxVal)
        {
            std::vector<double> hist(bins, 0.0);
            hist[0] = (double)data.size();
            return hist;
        }

        std::vector<double> histogram(bins, 0.0);
        double range = (double)(maxVal - minVal);
        double binWidth = range / bins;

        for (const auto &val : data)
        {
            int binIndex = (int)(((double)(val - minVal)) / binWidth);
            if (binIndex >= bins)
                binIndex = bins - 1;
            histogram[binIndex]++;
        }

        // 归一化为概率
        for (auto &count : histogram)
        {
            count /= (double)data.size();
        }

        return histogram;
    }
}