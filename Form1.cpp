#include "stdafx.h"
#include "Form1.h"
#include <stdio.h>
#include <iostream>
#include <opencv2\opencv.hpp>
#include <Windows.h>
#include <MSR_NuiApi.h>

using namespace first_kinect;

using namespace System;
using namespace System::IO::Ports;

//---------------------------------------------------------------
// define
//---------------------------------------------------------------
#define		CAMERA_WIDTH	640		// カメラ画像・幅
#define		CAMERA_HEIGHT	480		// カメラ画像・高さ
#define		DEPTH_WIDTH		320		// 奥行き画像・幅
#define		DEPTH_HEIGHT	240		// 奥行き画像・高さ
#define		EVENTS			2		// イベントオブジェクトの個数

//-------------------------------------------------------------------
// globals
//-------------------------------------------------------------------
bool		capFlag = false;		// カメラ入力・オフ
IplImage*	img_camera = 0;			// カメラ画像
IplImage*	img_depth = 0;			// 奥行き画像
IplImage*	img_tmp1 = 0;
IplImage*	img_gscale = 0;
RGBQUAD		depthRGB[DEPTH_WIDTH*DEPTH_HEIGHT];	// 奥行きカラー情報

HANDLE		handles[EVENTS];		// イベントオブジェクト
HANDLE		cameraEvent = NULL;		// イベントオブジェクト（RGBカメラ）
HANDLE		cameraHandle = NULL;	// RGBカメラハンドル
HANDLE		depthEvent = NULL;		// イベントオブジェクト（IRカメラ）
HANDLE		depthHandle = NULL;		// IRカメラハンドル

double		gR = 0;
double		gG = 0;
double		gB = 0;

int			iR = 0;
int			iG = 0;
int			iB = 0;

//-------------------------------------------------------------------
// 起動時
//-------------------------------------------------------------------
System::Void Form1::Form1_Load_1(System::Object^  sender, System::EventArgs^  e){
	DE = gcnew DepthEngine();	// DepthEngineクラス
	HRESULT		hr = S_OK;

	// Kinect初期化
	hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_COLOR); 
//					   NUI_INITIALIZE_FLAG_USES_SKELETON);
	if (hr != S_OK) {
		MessageBox::Show("error - NuiInitialize");
		return;
	}

	// RGBカメラハンドルの取得
	cameraEvent = CreateEvent(0, true, false, 0);
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR,
			NUI_IMAGE_RESOLUTION_640x480,0,2,cameraEvent,&cameraHandle);
	if (hr != S_OK) {MessageBox::Show("error - NuiImageStreamOpen(camera)");return;}

	// IRカメラハンドルの取得
	depthEvent = CreateEvent(0, true, false, 0);
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
			NUI_IMAGE_RESOLUTION_320x240,0,2,depthEvent,&depthHandle);
	if (hr != S_OK) {MessageBox::Show("error - NuiImageStreamOpen(depth)");return;}

	// イベントオブジェクトの構成
	handles[0] = cameraEvent;
	handles[1] = depthEvent;
	
	// カラー画像用のウィンドウ
	img_camera = cvCreateImage(cvSize(CAMERA_WIDTH, CAMERA_HEIGHT),IPL_DEPTH_8U,4);
	cvNamedWindow("Camera View", CV_WINDOW_AUTOSIZE);

	// 奥行き画像用のウィンドウ
	img_depth = cvCreateImage(cvSize(DEPTH_WIDTH, DEPTH_HEIGHT),IPL_DEPTH_8U,4);
	cvNamedWindow("Depth View", CV_WINDOW_AUTOSIZE);

	// テンポラリ
	img_tmp1 = cvCreateImage(cvSize(CAMERA_WIDTH, CAMERA_HEIGHT),IPL_DEPTH_8U,4);
	img_gscale = cvCreateImage(cvSize(CAMERA_WIDTH, CAMERA_HEIGHT),IPL_DEPTH_8U,1);

}

//-------------------------------------------------------------------
// [スタート]ボタン
//-------------------------------------------------------------------
System::Void Form1::btnStart_Click(System::Object^  sender, System::EventArgs^  e){

	HRESULT		hr = S_OK;

	// [ストップ]ボタンにフォーカスをセット
	btnStop->Focus();

	//カメラ入力開始
	capFlag = true;
	while (capFlag == true){

		// すべてのデータの更新を待つ
		WaitForMultipleObjects(EVENTS, handles, true, INFINITE);

		//----------
		// 奥行き画像を入力
		const NUI_IMAGE_FRAME*  pDepthFrame = 0;
		hr = NuiImageStreamGetNextFrame(depthHandle, 0, &pDepthFrame);
		if (hr != S_OK) {
			MessageBox::Show("error - NuiImageStreamGetNextFrame(depth)");
			capFlag = false;
			return;
		}
		NuiImageBuffer* pDepthTexture = pDepthFrame->pFrameTexture;

		// 奥行き画像をロック
		KINECT_LOCKED_RECT LockedRectDepth;
		pDepthTexture->LockRect(0, &LockedRectDepth, NULL, 0);
		BYTE* pBufferDepth = (BYTE*)LockedRectDepth.pBits;

		// 奥行きをカラーに変換
		RGBQUAD* rgbrun = depthRGB;
		DE->Depth2Color(pBufferDepth, rgbrun);

		// ウィンドウに出力
		memcpy(img_depth->imageData, depthRGB, img_depth->imageSize);
		cvShowImage("Depth View", img_depth);

		//----------
		// カラー画像を入力
		const NUI_IMAGE_FRAME*  pImageFrame = 0;
		hr = NuiImageStreamGetNextFrame(cameraHandle, 0, &pImageFrame);
		if (hr != S_OK) {
			MessageBox::Show("error - NuiImageStreamGetNextFrame(camera)");
			capFlag = false;
			return;
		}
		NuiImageBuffer* pTexture = pImageFrame->pFrameTexture;

		// ウィンドウに出力
		KINECT_LOCKED_RECT LockedRect;
		pTexture->LockRect(0, &LockedRect, NULL, 0);
		BYTE* pBuffer = (BYTE*)LockedRect.pBits;

		// ここから追記
		//memcpy(img_camera->imageData, pBuffer, img_camera->imageSize);

		memcpy(img_camera->imageData, pBuffer, img_camera->imageSize);
		cvResize(img_depth,img_tmp1);
		cvCvtColor(img_tmp1,img_gscale,CV_BGR2GRAY);

		cvCopy(img_tmp1,img_camera,img_gscale);

		// ヒストグラム用
		gR = 1.0;
		gG = 1.0;
		gB = 1.0;

		for(int y=0; y<img_camera->height; y++){
			for(int x=0; x<img_camera->width; x++){
				double tB,tG,tR;
				tB = img_camera->imageData[img_camera->widthStep*y+x*4];
				tG = img_camera->imageData[img_camera->widthStep*y+x*4+1];
				tR = img_camera->imageData[img_camera->widthStep*y+x*4+2];

				if(tB == tG && tG == tR){
				}else{
					gB = gB + tB;
					gG = gG + tG;
					gR = gR + tR;
				}
			}
		}

		double g[] = {gB,gG,gR};
		double gmin = g[0];

		for(int i=0; i<3; i++){
			if(g[i]<gmin){
				gmin = g[i];
			}
		}

		cvRectangle(img_camera, cvPoint(0,0), cvPoint(cvRound(300.0*(gR-gmin)/(gR+gG+gB-gmin)),100), CV_RGB(255,0,0),2);
		cvRectangle(img_camera, cvPoint(0,100), cvPoint(cvRound(300.0*(gG-gmin)/(gR+gG+gB-gmin)),200), CV_RGB(0,255,0),2);
		cvRectangle(img_camera, cvPoint(0,200), cvPoint(cvRound(300.0*(gB-gmin)/(gR+gG+gB-gmin)),300), CV_RGB(0,0,255),2);
		cvShowImage("Camera View", img_camera);

		iR = cvRound(10.0*(gR-gmin)/(gR+gG+gB-gmin));
		iG = cvRound(10.0*(gG-gmin)/(gR+gG+gB-gmin));
		iB = cvRound(10.0*(gB-gmin)/(gR+gG+gB-gmin));

		//----------
		// 画像の開放

		// 奥行き画像を解放
		NuiImageStreamReleaseFrame(depthHandle, pDepthFrame);

		// カメラ画像を解放
		NuiImageStreamReleaseFrame(cameraHandle, pImageFrame);

		//----------
 		// イベントの監視
		Application::DoEvents();
	}
}

//-------------------------------------------------------------------
// [ストップ]ボタン
//-------------------------------------------------------------------
System::Void Form1::btnStop_Click_1(System::Object^  sender, System::EventArgs^  e){
	// カメラ入力・オフ
	capFlag = false;

	this->serialPort1->PortName = "COM3";
	this->serialPort1->Open();
	this->serialPort1->Write(iR.ToString("D"));
	this->serialPort1->Close();
}

//-------------------------------------------------------------------
// 終了時
//-------------------------------------------------------------------
System::Void Form1::Form1_FormClosed(
	System::Object^  sender, System::Windows::Forms::FormClosedEventArgs^  e){
	
	// カメラ入力・オフ
	capFlag = false;

	// Kinect終了処理
	NuiShutdown();
	CloseHandle(cameraEvent);
	CloseHandle(depthEvent);

	// OpenCV終了処理
	cvReleaseImage(&img_camera);
	cvReleaseImage(&img_depth);
	cvDestroyAllWindows();
}
