// demprj.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include <vector>
#include <iomanip>
#include <io.h>
#include <string>
#include <windows.h>

#include <fstream>

#include<math.h>
///gdal头文件
#include "..\include\gdal.h"
#include "..\include\gdal_priv.h"
#include "..\include\ogr_srs_api.h"
#include "..\include\cpl_string.h"
#include "..\include\cpl_conv.h"


using namespace std;

#define BYTE short      //方便数据类型的修改
#define GRIDNET_NUM_5  58


#define MINNUTE_UNIT (60)

void getFiles(string, vector<string>&);

void getFiles(string path, vector<string>& files)
{
	intptr_t    hFile = 0;
	struct _finddata_t fileinfo;
	string p;

	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
		do {
			if ((fileinfo.attrib & _A_SUBDIR)) {
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else {
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}

		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
} 

//获取1度*1度文件中格点的高程
void getEveryTifAltitude()
{
	int num_iamge_size = 0;
	
	double dProjX = 0;//100.499;
	double dProjY = 0;//26.667; 
	int latStart = 0, lonStart = 0;

	int gridpointIndex = 0;
	double gridAttitude[400][2] = { 0 };


	BYTE* pafScanblock1;  //开辟缓存区
	GDALDataset* poDataset;   //GDAL数据集
	GDALAllRegister();  //注册所有的驱动
	BYTE* pafScanblock2 = (BYTE*)CPLMalloc(sizeof(BYTE) * (1) * (1));
	BYTE elevation;

	//获取文件名
	vector<string> files;

	//dem文件在demtif目录下
	getFiles("D:\\example\\demget\\demget_proj\\demtif", files);
	cout << "get file:" << files.size() << endl;
	// print the files get
	for (int i = 0; i < files.size(); ++i)
	{
		cout << files[i] << endl;

		size_t nPos1 = files[i].find_last_of("\\");
		string  filename = files[i].substr(nPos1 + 1, files[i].size() - nPos1);
		//cout << "filename:" << filename << endl;
		size_t nPos2 = filename.find("N");
		size_t nPos3 = filename.find_last_of("_");
		string latlonstr = filename.substr(nPos2, nPos3 - nPos2);
		//cout << "latlonstr:" << latlonstr << endl;
		sscanf((char*)latlonstr.c_str(), "N%dE%d", &latStart, &lonStart);

		cout << "latStart=" << latStart << "lonStart=" << lonStart << endl;


		//读取当前文件中格点的高程  
		poDataset = (GDALDataset*)GDALOpen(files[i].c_str(), GA_ReadOnly);
		if (poDataset == NULL)
		{
			cout << "fail in open files!!!" << endl;
			return;
		}
		//获取图像波段
		GDALRasterBand* poBand1;
		poBand1 = poDataset->GetRasterBand(1);

		//获取图像的尺寸
		int nImgSizeX = poDataset->GetRasterXSize();
		int nImgSizeY = poDataset->GetRasterYSize();
		cout << "nImgSizeX：" << nImgSizeX << ", nImgSizeY:" << nImgSizeY << endl;


		double adfGeoTransform[6];
		poDataset->GetGeoTransform(adfGeoTransform);

		double dTemp = adfGeoTransform[1] * adfGeoTransform[5] - adfGeoTransform[2] * adfGeoTransform[4];
		double dCol = 0.0, dRow = 0.0;
		int atitudeindex = 0;

		//3分*3分为一个小格点，1度范围内400个格点
		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				dProjX = lonStart + (double)(i * 3) / MINNUTE_UNIT;
				dProjY = latStart + (double)(j * 3) / MINNUTE_UNIT;

				gridpointIndex = (int)(fmod(dProjY, 10) * 60 / 3 + 0.5) * 200 + (int)(fmod(dProjX, 10) * 60 / 3 + 0.5);

				dCol = (adfGeoTransform[5] * (dProjX - adfGeoTransform[0]) -

					adfGeoTransform[2] * (dProjY - adfGeoTransform[3])) / dTemp;

				dRow = (adfGeoTransform[1] * (dProjY - adfGeoTransform[3]) -

					adfGeoTransform[4] * (dProjX - adfGeoTransform[0])) / dTemp;

				int dc = (int32_t)(dCol);

				int dr = (int32_t)(dRow);
				//cout << "dProjX=" << dProjX << ",dProjY=" << dProjY << "dCol=" << dc << ", dRow=" << dr << endl;
				memset(pafScanblock2, 0, sizeof(BYTE) * (1) * (1));
				poBand1->RasterIO(GF_Read, dc, dr, 1, 1, pafScanblock2, 1, 1, GDALDataType(poBand1->GetRasterDataType()), 0, 0);
				elevation = *pafScanblock2;
				gridAttitude[atitudeindex][0] = gridpointIndex;
				gridAttitude[atitudeindex][1] = elevation;
				if (gridAttitude[atitudeindex][1] > 0.0001)
					cout << "gridpointIndex  =" << gridpointIndex << ", gridAttitude = " << setprecision(15) << gridAttitude[atitudeindex][1] << endl;
			
			}
		}

		delete poDataset;
	}
}




int main(int argc, char* argv[])
{
	//根据点的经纬度获取这个点的dem高程
	getEveryTifAltitude();

	system("pause");
	return 0;
}