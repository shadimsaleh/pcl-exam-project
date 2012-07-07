#ifndef PCQC_H
#define PCQC_H

#include "registration.h"
#include "componentSelection.h"
#include "componentMatch.h"
#include <QString>
#include <QColor>

class Pcqc
{
public:
    //INIT
    Pcqc();
    bool loadTargetCloud(QString path); // returns true if file was loaded correctly
    bool loadSourceCloud(QString path); // returns true if file was loaded correctly
    //GETTERS
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr getTargetCloud();
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr getTargetCloudColorSeg();
    QColor* getPointColor(int pointIndex, bool isFirstStep);
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr getTargetCloudClusterSeg();
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr getSourceCloud();
    //SETTERS
    void setClusterSegThreshold(int threshold);
    void setColorSegThreshold(int threshold);
    //FUNCTIONS
    void clusterSegmentation(int selectedPointIndex, bool isFirstStep);
    void colorSegmentation(int selectedPointIndex, bool isFirstStep);

private:
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr sourceCloud;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr targetCloud;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr targetCloudColorSeg;
    int colThreshold;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr targetCloudClusterSeg;
    int cluThreshold;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr targetCloudTemp;
    pcl::PointIndices::Ptr newComponentPointIndices;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr registeredCloud;
};

#endif // PCQC_H
