#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstdlib> // srand, rand
#include <ctime>   // time

#include "Pixel.h"
#include "DataObject.h"
#include "SatelliteImage.h"
#include "PointCloudData.h"
#include "DataManager.h"
#include "DataExporter.h"
#include "Algorithms.h"

using namespace std;

int DataObject::totalObjects = 0;

// 辅助函数
void printLine(const string &title)
{
    cout << "\n======== " << title << " ========\n";
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // 随机图像生成时使用。

    cout << "=== Remote Sensing Data Processing System ===" << endl;
    cout << "Total DataObjects created: " << DataObject::getTotalObjects() << endl;

    // 1.测试Pixel--------------------------------------------------------------------------------------------
    printLine("Pixel Test");

    // 构造函数
    Pixel<double> p1(0.3, 0.5, 0.2, 0.8, 300.0, 202601011200);
    Pixel<double> p2(0.1, 0.4, 0.3, 0.6, 250.0, 202601011200);
    Pixel<double> p3(p2);

    // 友元函数，流输出。
    cout << "Pixel p1: " << p1 << endl;
    cout << "Pixel p2: " << p2 << endl;
    cout << "Pixel p3: " << p3 << endl;

    //()的重载
    cout << "p1 brightness: " << (double)p1 << endl;
    cout << "(string)p2" << (string)p2 << endl;

    // 遥感指数计算
    cout << "p1 NDVI: " << p1.getNDVI() << endl;
    cout << "p1 NDWI: " << p1.getNDWI() << endl;
    cout << "p1 SAVI(L=0.5): " << p1.getSAVI(0.5) << endl;

    // 运算符重载
    Pixel<double> p4 = p1 + p2;
    cout << "p1 + p2: " << p4 << endl;

    Pixel<double> p5 = p1 * 2.0;
    cout << "p1 * 2: " << p5 << endl;

    p1 += p2;
    cout << "p1 += p2: " << p1 << endl;
    cout << "p1 == p2: " << (p1 == p2 ? "true" : "false") << endl;
    cout << "p1[0](Red): " << p1[0] << ", p1[3](NIR): " << p1[3] << endl;

    // 2.测试SatelliteImage-------------------------------------------------------------------------------------
    printLine("SatelliteImage Test");

    SatelliteImage img1("img001", "Beijing_Landsat", "./data/", 5, 5, 5, "Landsat8", 2459000);
    cout << "Created SatelliteImage: " << (string)img1 << endl;
    cout << "Type: " << img1.getType() << endl;
    cout << "Quality Score: " << img1.getQualityScore() << endl;
    cout << "Is Valid: " << (img1.isValid() ? "Yes" : "No") << endl;

    // Set pixel values
    for (int i = 0; i < img1.getHeight(); i++)
    {
        for (int j = 0; j < img1.getWidth(); j++)
        {
            img1[i][j].setRed(0.1 + i * 0.05);
            img1[i][j].setGreen(0.2 + j * 0.05);
            img1[i][j].setBlue(0.15);
            img1[i][j].setNir(0.5 + (i + j) * 0.05);
            img1[i][j].setThermal(250.0 + i * 10);
        }
    }

    img1.refreshStatistics();
    img1.display();
    img1.printStatistics();

    // Test sub-image
    SatelliteImage subImg = img1.getSubImage(1, 1, 3, 3);
    cout << "SubImage size: " << subImg.getWidth() << "x" << subImg.getHeight() << endl;

    // Test band extraction
    SatelliteImage redBand = img1.extractBand(0);
    cout << "Red band extracted: " << redBand.getWidth() << "x" << redBand.getHeight() << ", bands: " << redBand.getBands() << endl;

    // 生成掩模图
    SatelliteImage img2 = SatelliteImage::createConstantImage("img002", 5, 5, Pixel<double>(0.1, 0.1, 0.1, 0.1, 100.0));

    // 测试随机图像生成
    SatelliteImage imgRandom = SatelliteImage::createRandomImage("img_random", 5, 5);
    imgRandom.refreshStatistics();
    cout << "Random image created: " << imgRandom.getId() << endl;
    cout << "Random pixel [0][0] red: " << imgRandom[0][0].getRed() << endl;
    imgRandom.printStatistics();

    // 运算符重载
    SatelliteImage imgAdd = img1 + img2;
    imgAdd.refreshStatistics();
    cout << "img1 + img2 created: " << imgAdd.getId() << endl;
    imgAdd.printStatistics();
    img1 += img2;
    cout << "img1 += img2 done" << endl;

    // Test NDVI calculation
    vector<vector<double>> ndviMap = img1.calculateNDVI();
    cout << "NDVI map size: " << ndviMap.size() << "x" << ndviMap[0].size() << endl;
    cout << "NDVI[0][0]: " << ndviMap[0][0] << endl;

    // 波段操作
    std::vector<double> bandov = img1.getBandValues(4);
    for (const auto &b : bandov)
    {
        cout << b << "  ";
    }
    cout << endl;

    // Test export
    img1.exportData("txt");

    // 3.测试PointCloudData---------------------------------------------------------------------------------
    printLine("PointCloudData Test");

    PointCloudData cloud1("cloud001", "Building_Scan", "./data/");
    cout << "Created PointCloud: " << (string)cloud1 << endl; // 继承自 DataObject
    cout << "Type: " << cloud1.getType() << endl;

    // Add points
    cloud1.addPoint(Point3D(1.0, 2.0, 3.0, 100.0, 1));
    cloud1.addPoint(Point3D(2.0, 3.0, 4.0, 150.0, 1));
    cloud1.addPoint(Point3D(5.0, 6.0, 7.0, 200.0, 2));
    cloud1.addPoint(Point3D(1.5, 2.5, 3.5, 120.0, 1));
    cloud1.addPoint(Point3D(10.0, 10.0, 10.0, 50.0, 3));

    cloud1.display();
    cloud1.printStatistics();

    cout << "Average Height: " << cloud1.getAverageHeight() << endl;
    cout << "Height Range: " << cloud1.getHeightRange() << endl;

    // Test point access
    cout << "Point[0]: (" << cloud1[0].x << ", " << cloud1[0].y << ", " << cloud1[0].z << ")" << endl;
    cout << "Point[2]: (" << cloud1[2].x << ", " << cloud1[2].y << ", " << cloud1[2].z << ")" << endl;
    cout << "distance of P0 and P2" << cloud1[0].distanceTo(cloud1[2]) << endl;
    Point3D Pa = cloud1[0] + cloud1[2];
    cout << "Pa =P0 + P2 :  (" << Pa.x << ", " << Pa.y << ", " << Pa.z << Pa.intensity << ")" << endl;

    // Test voxel filter
    PointCloudData voxelCloud = cloud1.voxelFilter(2.0);
    cout << "After voxel filter: " << voxelCloud.getPointCount() << " points" << endl;

    // Test export
    cloud1.exportData("txt");
    cloud1.exportData("pcd");

    // 4.测试DataManager--------------------------------------------------------------------------------------
    printLine("DataManager Template Class Test");

    DataManager<SatelliteImage> imgManager; // 在T = SatelliteImage的基础上操作。
    cout << "DataManager created" << endl;

    SatelliteImage img3("img003", "Shanghai_Sentinel", "./data/", 10, 10, 5, "Sentinel2", 2459050);
    for (int i = 0; i < img3.getHeight(); i++)
    {
        for (int j = 0; j < img3.getWidth(); j++)
        {
            img3[i][j].setRed(0.2 + i * 0.03);
            img3[i][j].setGreen(0.3 + j * 0.03);
            img3[i][j].setBlue(0.25);
            img3[i][j].setNir(0.6 + (i + j) * 0.03);
            img3[i][j].setThermal(280.0 + i * 5);
        }
    }
    img3.refreshStatistics();
    SatelliteImage img4("img004", "Guangzhou_Landsat", "./data/", 8, 8, 5, "Landsat8", 2459100);
    for (int i = 0; i < img4.getHeight(); i++)
    {
        for (int j = 0; j < img4.getWidth(); j++)
        {
            img4[i][j].setRed(0.15 + i * 0.04);
            img4[i][j].setGreen(0.25 + j * 0.04);
            img4[i][j].setBlue(0.2);
            img4[i][j].setNir(0.55 + (i + j) * 0.04);
            img4[i][j].setThermal(260.0 + i * 8);
        }
    }
    img4.refreshStatistics();
    SatelliteImage img5("img005", "Shenzhen_Sentinel", "./data/", 12, 12, 5, "Sentinel2", 2459150);
    for (int i = 0; i < img5.getHeight(); i++)
    {
        for (int j = 0; j < img5.getWidth(); j++)
        {
            img5[i][j].setRed(0.25 + i * 0.02);
            img5[i][j].setGreen(0.35 + j * 0.02);
            img5[i][j].setBlue(0.3);
            img5[i][j].setNir(0.7 + (i + j) * 0.02);
            img5[i][j].setThermal(300.0 + i * 4);
        }
    }
    img5.refreshStatistics();

    imgManager.addData(img3);
    imgManager.addData(img4);
    imgManager.addData(img5);

    cout << "Added 3 images, total count: " << imgManager.getCount() << endl;

    // Test find
    try
    {
        SatelliteImage found = imgManager.findData("img004");
        cout << "Found image: " << found.getName() << endl;
    }
    catch (const exception &e)
    {
        cout << "Find error: " << e.what() << endl;
    }

    // Test sort
    vector<SatelliteImage> sortedBySize = imgManager.sortBySize(true);
    cout << "Sorted by size (ascending):" << endl;
    for (const auto &img : sortedBySize)
    {
        cout << "  " << img.getName() << ": " << img.getSize() << " MB" << endl;
    }

    // Test filter
    vector<SatelliteImage> sentinelImages = imgManager.filter([](const SatelliteImage &img)
                                                              { return img.getSensorType() == "Sentinel2"; });
    cout << "Filtered Sentinel2 images: " << sentinelImages.size() << endl;

    // Test type statistics
    auto typeStats = imgManager.getTypeStatistics();
    cout << "Type statistics:" << endl;
    for (const auto &pair : typeStats)
    {
        cout << "  " << pair.first << ": " << pair.second << endl;
    }

    // Test operator[]
    cout << "imgManager[0]: " << imgManager[0].getName() << endl;

    // Test merge
    DataManager<SatelliteImage> imgManager2;
    imgManager2.addData(SatelliteImage::createRandomImage("img006", 5, 5));
    DataManager<SatelliteImage> merged = DataManager<SatelliteImage>::merge(imgManager, imgManager2);
    cout << "Merged manager count: " << merged.getCount() << endl;

    imgManager.printStatistics();

    // 5.测试DataExporter----------------------------------------------------------------------------------------
    printLine("DataExporter Test");

    DataExporter exporter("./output/test_export/");
    cout << "Exporter created" << endl;

    // Export single image
    vector<SatelliteImage> imgList = {img3, img4};

    int exported = exporter.batchExport(imgList, "csv");
    cout << "Exported " << exported << " CSV files" << endl;

    exported = exporter.batchExport(imgList, "json");
    cout << "Exported " << exported << " JSON files" << endl;

    exporter.printExportStats();

    // 6.测试Algorithms-------------------------------------------------------------------------------------
    printLine("Algorithms Test");

    // 计算遥感指标
    double ndvi = RemoteSensingAlgorithms::calculateNDVI(0.3, 0.8);
    double ndwi = RemoteSensingAlgorithms::calculateNDWI(0.5, 0.3);
    double savi = RemoteSensingAlgorithms::calculateSAVI(0.3, 0.8, 0.5);
    double evi = RemoteSensingAlgorithms::calculateEVI(0.1, 0.3, 0.8);
    double mndwi = RemoteSensingAlgorithms::calculateMNDWI(0.5, 0.2);

    cout << "NDVI: " << ndvi << endl;
    cout << "NDWI: " << ndwi << endl;
    cout << "SAVI: " << savi << endl;
    cout << "EVI: " << evi << endl;
    cout << "MNDWI: " << mndwi << endl;

    // 直方图测试
    vector<int> histData = {1, 2, 2, 3, 3, 3, 4, 4, 5};
    vector<double> hist = RemoteSensingAlgorithms::calculateHistogram(histData, 5);
    cout << "Histogram: ";
    for (double h : hist)
    {
        cout << h << " ";
    }
    cout << endl;

    // 测试熵
    double entropy = RemoteSensingAlgorithms::calculateEntropy(histData);
    cout << "Entropy: " << entropy << endl;

    // 测试卷积
    vector<vector<double>> testImage = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}};
    vector<vector<double>> kernel = {
        {0, 1, 0},
        {1, -4, 1},
        {0, 1, 0}};
    auto convolved = RemoteSensingAlgorithms::convolve(testImage, kernel);
    cout << "Convolved image center: " << convolved[1][1] << endl;

    // 7.测试多态的正确性---------------------------------------------------------------------------------------
    printLine("Polymorphism Test");

    vector<DataObject *> dataObjects;
    dataObjects.push_back(new SatelliteImage("poly_img", "PolyTest", "./", 3, 3, 5, "TestSensor"));
    PointCloudData *pc = new PointCloudData("poly_cloud", "PolyCloud", "./");
    pc->addPoint(Point3D(1.0, 2.0, 3.0, 100.0, 1));
    pc->addPoint(Point3D(4.0, 5.0, 6.0, 200.0, 2));
    dataObjects.push_back(pc);

    for (auto *obj : dataObjects)
    {
        cout << "Object ID: " << obj->getId() << ", Type: " << obj->getType() << endl;
        obj->display();
    }

    // 单独测试clone函数，两种指针的测试：----------------------------------------------------------------------
    printLine("Polymorphism & Memory Management Test");

    // 传统指针版本：
    cout << "Raw Pointer Version (Manual Delete)" << endl;
    {
        DataObject *rawObj = new SatelliteImage("raw_img", "RawTest", "./", 3, 3, 5, "TestSensor");
        rawObj->display();
        DataObject *rawCloned = rawObj->clone();

        cout << "Raw cloned type: " << rawCloned->getType() << endl;
        rawCloned->display();

        delete rawCloned;
        delete rawObj;
        cout << "Raw pointers manually deleted" << endl;
    }

    // 智能指针版本:
    cout << "\n--- Smart Pointer Version (Auto Release) ---" << endl;
    {

        std::shared_ptr<PointCloudData> smartObj = std::make_shared<PointCloudData>("smart_cloud", "SmartTest", "./");
        smartObj->addPoint(Point3D(10.0, 20.0, 30.0, 150.0, 1));
        smartObj->display();

        std::shared_ptr<DataObject> smartCloned =
            std::dynamic_pointer_cast<PointCloudData>(smartObj)->cloneShared();
        cout << "Smart cloned type: " << smartCloned->getType() << endl;
        smartCloned->display();
    }
    cout << "Smart pointers auto-released" << endl;

    //----------------------------------------------------------------------------------------------------------
    printLine("Summary");

    cout << "Total DataObjects created: " << DataObject::getTotalObjects() << endl;
    cout << "All tests completed successfully!" << endl;

    return 0;
}