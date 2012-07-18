#include "pcqc.h"

Pcqc::Pcqc()
{
    targetCloud.reset (new pcl::PointCloud<pcl::PointXYZRGB>);
    sourceCloud.reset (new pcl::PointCloud<pcl::PointXYZRGB>);
    newComponentCloud.reset (new pcl::PointCloud<pcl::PointXYZRGB>);
    colThreshold = 0;
    cluThreshold = 0;
    newComponentPointIndices.reset (new pcl::PointIndices);
    registeredCloud.reset (new pcl::PointCloud<pcl::PointXYZRGB>);
}


pcl::PointCloud<pcl::PointXYZRGB>::Ptr
Pcqc::voxelCloud (pcl::PointCloud<pcl::PointXYZRGB>::Ptr input, double leafSize){
cout << "Voxelling... "<<flush;
pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered (new pcl::PointCloud<pcl::PointXYZRGB>);
pcl::VoxelGrid<pcl::PointXYZRGB> sor;
sor.setInputCloud (input);
sor.setLeafSize (leafSize,leafSize,leafSize);
sor.filter (*cloud_filtered);
return cloud_filtered;
}

void
Pcqc::segmentation (pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr source,
pcl::PointCloud<pcl::PointXYZRGB>::Ptr segmented)
{
  cout << "Plane segmentation... " <<flush;
  // fit plane and keep points above that plane
  pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
  pcl::PointIndices::Ptr inliers (new pcl::PointIndices);
  // Create the segmentation object
  pcl::SACSegmentation<pcl::PointXYZRGB> seg;
  // Optional
  seg.setOptimizeCoefficients (true);
  // Mandatory
  seg.setModelType (pcl::SACMODEL_PLANE);
  seg.setMethodType (pcl::SAC_RANSAC);
  seg.setDistanceThreshold (3);

  seg.setInputCloud (source);
  seg.segment (*inliers, *coefficients);

  pcl::ExtractIndices<pcl::PointXYZRGB> extract;
  extract.setInputCloud (source);
  extract.setIndices (inliers);
  extract.setNegative (true);

  extract.filter (*segmented);
  vector<int> indices;
  pcl::removeNaNFromPointCloud(*segmented, *segmented, indices);
}


void
Pcqc::removeOutliers(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud){
    cout << "Removing outliers... " << flush;
pcl::StatisticalOutlierRemoval<pcl::PointXYZRGB> sor;
sor.setInputCloud (cloud);
sor.setMeanK (50);
sor.setStddevMulThresh (1.0);
sor.filter (*cloud);
cout << "OK! " << cloud->size() << " points Loaded."<<endl;
}

bool
Pcqc::loadTargetCloud(QString path)
{
    const std::string stdpath = path.toStdString();
    if(pcl::io::loadPCDFile(stdpath, *targetCloud) == 0 )
    {
        vector<int> indices;
        pcl::removeNaNFromPointCloud(*targetCloud, *targetCloud, indices);
        targetCloud=voxelCloud(targetCloud,0.4);
        segmentation(targetCloud,targetCloud);
        removeOutliers(targetCloud);
            return true;
    }
    else return false;
}

bool
Pcqc::loadSourceCloud(QString path)
{
    const std::string stdpath = path.toStdString();
    if(pcl::io::loadPCDFile(stdpath, *sourceCloud) == 0)
    {
        vector<int> indices;
        pcl::removeNaNFromPointCloud(*sourceCloud, *sourceCloud, indices);
        sourceCloud = voxelCloud(sourceCloud,0.4); // alleggerisce il calcolo, da capire se peggiora il risultato o meno.
        segmentation(sourceCloud,sourceCloud); // segmentazione del piano principale.
        removeOutliers(sourceCloud);
        return true;
    }
    else return false;
}

//GETTERS
pcl::PointCloud<pcl::PointXYZRGB>::Ptr Pcqc::getTargetCloud()
{
    return targetCloud;
}

pcl::PointCloud<pcl::PointXYZRGB>::Ptr Pcqc::getSourceCloud()
{
    return sourceCloud;
}

pcl::PointCloud<pcl::PointXYZRGB>::Ptr Pcqc::getRegisteredCloud()
{
    return registeredCloud;
}

QColor* Pcqc::getPointColor(int pointIndex)
{
    QColor *color = new QColor;
    color->setRed(targetCloud->at(pointIndex).r);
    color->setGreen(targetCloud->at(pointIndex).g);
    color->setBlue(targetCloud->at(pointIndex).b);
    return color;
}

pcl::PointCloud<pcl::PointXYZRGB>::Ptr Pcqc::getNewComponentCloud()
{
    return newComponentCloud;
}

pcl::PointCloud<pcl::PointXYZRGB>::Ptr Pcqc::getComponentCloud(QString componentName)
{
    Component component = componentsList.value(componentName);
    return component.getComponentCloud();
}

//SETTERS
void Pcqc::setClusterSegThreshold(int threshold)
{
    cluThreshold = threshold/1000;
}

void Pcqc::setColorSegThreshold(int threshold)
{
    colThreshold = threshold;
}

//FUNCTIONS

void Pcqc::colorIndices(pcl::PointCloud<pcl::PointXYZRGB>::Ptr input, pcl::PointIndices::Ptr indices, int r, int g, int b)
{
  //  cout << "colorIndices... " << flush;
    for (int i =0; i<indices->indices.size();i++)
    {
                int pointN= indices->indices.at(i);
                input->at(pointN).r=r;
                input->at(pointN).g=g;
                input->at(pointN).b=b;
    }
   // cout << "colored" << indices->indices.size() << flush;
}

void Pcqc::colorComponents(pcl::PointCloud<pcl::PointXYZRGB>::Ptr input, int r, int g, int b)
{ //NON FUNZIONA!! COLORA SOLO L'ULTIMO COMPONENTE INSERITO
  //  cout << "colorComponents... " << flush;
    QMapIterator<QString, Component> iter(componentsList);
    Component currentComponent;
    while (iter.hasNext())
    {
        currentComponent = iter.next().value();
        colorIndices(input,currentComponent.getIndices(),r,g,b);
    }

}

void Pcqc::componentSelection(int selectedPointIndex)
{
    lastClickedPointIndex = selectedPointIndex; // save the last clicked point index
    cout << "Component Segmentation... "<<flush; // DEBUG PRINT
    pcl::copyPointCloud(*targetCloud, *newComponentCloud); // start from a new copy of the cloud
    newComponentPointIndices->indices.clear();// and a new point indices

    pcl::PointIndices::Ptr tempClusterIndices(new pcl::PointIndices);
    segmentCluster(newComponentCloud, tempClusterIndices, selectedPointIndex, cluThreshold );

    pcl::PointIndices::Ptr tempColorIndices(new pcl::PointIndices);
    segmentColor(newComponentCloud, tempColorIndices, selectedPointIndex, colThreshold );

    intersectIndices(tempClusterIndices,tempColorIndices,newComponentPointIndices);

    colorIndices(newComponentCloud, newComponentPointIndices,0,255,0);
    cout << "OK! Selected "<< newComponentPointIndices->indices.size() <<" points for this component\n"<<flush; // DEBUG PRINT
}

bool Pcqc::componentSegmentation()
{
    pcl::copyPointCloud(*targetCloud, *newComponentPointIndices, *newComponentCloud);
    return true;
}

bool Pcqc::componentSave(QString componentName)
{
    if(componentsList.find(componentName) == componentsList.end())
    {
        pcl::PointIndices::Ptr indicesToSave(new pcl::PointIndices);
        indicesToSave->indices = newComponentPointIndices->indices;
        Component newComponent(targetCloud, indicesToSave, lastClickedPointIndex, cluThreshold, colThreshold);
        componentsList.insert(componentName, newComponent); // se non è già presente quella chiave, aggiungo il componente, altrimenti no
        return true;
    }
    else return false;
}

bool Pcqc::componentDelete(QString componentName)
{
    if(componentsList.find(componentName) != componentsList.end())
    {
        componentsList.remove(componentName); // se è presente quella chiave, cancello l'item, altrimenti no
        return true;
    }
    else return false;
}

void Pcqc::findSourceComponent()
{
    QMapIterator<QString,Component> iter(componentsList);
    Component toFind=iter.next().value();
    pcl::PointIndices::Ptr toPrint (new pcl::PointIndices);
    componentMatch(registeredCloud,targetCloud,toFind.getIndices(),toFind.getGeneratingIndex(),toFind.getClusterThreshold(),toFind.getColorThreshold(),toPrint);
    cout <<"\n Coloro quisti punti "<<toPrint->indices.size()<<endl;
    colorIndices(registeredCloud,toPrint,255,255,0);
}

void Pcqc::registration()
{
    registerSourceToTarget(sourceCloud, targetCloud, registeredCloud, 1, 0);
}
