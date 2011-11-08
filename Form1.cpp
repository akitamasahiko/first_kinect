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
#define		CAMERA_WIDTH	640		// �J�����摜�E��
#define		CAMERA_HEIGHT	480		// �J�����摜�E����
#define		DEPTH_WIDTH		320		// ���s���摜�E��
#define		DEPTH_HEIGHT	240		// ���s���摜�E����
#define		EVENTS			2		// �C�x���g�I�u�W�F�N�g�̌�

//-------------------------------------------------------------------
// globals
//-------------------------------------------------------------------
bool		capFlag = false;		// �J�������́E�I�t
IplImage*	img_camera = 0;			// �J�����摜
IplImage*	img_depth = 0;			// ���s���摜
IplImage*	img_tmp1 = 0;
IplImage*	img_gscale = 0;
RGBQUAD		depthRGB[DEPTH_WIDTH*DEPTH_HEIGHT];	// ���s���J���[���

HANDLE		handles[EVENTS];		// �C�x���g�I�u�W�F�N�g
HANDLE		cameraEvent = NULL;		// �C�x���g�I�u�W�F�N�g�iRGB�J�����j
HANDLE		cameraHandle = NULL;	// RGB�J�����n���h��
HANDLE		depthEvent = NULL;		// �C�x���g�I�u�W�F�N�g�iIR�J�����j
HANDLE		depthHandle = NULL;		// IR�J�����n���h��

double		gR = 0;
double		gG = 0;
double		gB = 0;

int			iR = 0;
int			iG = 0;
int			iB = 0;

//-------------------------------------------------------------------
// �N����
//-------------------------------------------------------------------
System::Void Form1::Form1_Load_1(System::Object^  sender, System::EventArgs^  e){
	DE = gcnew DepthEngine();	// DepthEngine�N���X
	HRESULT		hr = S_OK;

	// Kinect������
	hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_COLOR); 
//					   NUI_INITIALIZE_FLAG_USES_SKELETON);
	if (hr != S_OK) {
		MessageBox::Show("error - NuiInitialize");
		return;
	}

	// RGB�J�����n���h���̎擾
	cameraEvent = CreateEvent(0, true, false, 0);
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR,
			NUI_IMAGE_RESOLUTION_640x480,0,2,cameraEvent,&cameraHandle);
	if (hr != S_OK) {MessageBox::Show("error - NuiImageStreamOpen(camera)");return;}

	// IR�J�����n���h���̎擾
	depthEvent = CreateEvent(0, true, false, 0);
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
			NUI_IMAGE_RESOLUTION_320x240,0,2,depthEvent,&depthHandle);
	if (hr != S_OK) {MessageBox::Show("error - NuiImageStreamOpen(depth)");return;}

	// �C�x���g�I�u�W�F�N�g�̍\��
	handles[0] = cameraEvent;
	handles[1] = depthEvent;
	
	// �J���[�摜�p�̃E�B���h�E
	img_camera = cvCreateImage(cvSize(CAMERA_WIDTH, CAMERA_HEIGHT),IPL_DEPTH_8U,4);
	cvNamedWindow("Camera View", CV_WINDOW_AUTOSIZE);

	// ���s���摜�p�̃E�B���h�E
	img_depth = cvCreateImage(cvSize(DEPTH_WIDTH, DEPTH_HEIGHT),IPL_DEPTH_8U,4);
	cvNamedWindow("Depth View", CV_WINDOW_AUTOSIZE);

	// �e���|����
	img_tmp1 = cvCreateImage(cvSize(CAMERA_WIDTH, CAMERA_HEIGHT),IPL_DEPTH_8U,4);
	img_gscale = cvCreateImage(cvSize(CAMERA_WIDTH, CAMERA_HEIGHT),IPL_DEPTH_8U,1);

}

//-------------------------------------------------------------------
// [�X�^�[�g]�{�^��
//-------------------------------------------------------------------
System::Void Form1::btnStart_Click(System::Object^  sender, System::EventArgs^  e){

	HRESULT		hr = S_OK;

	// [�X�g�b�v]�{�^���Ƀt�H�[�J�X���Z�b�g
	btnStop->Focus();

	//�J�������͊J�n
	capFlag = true;
	while (capFlag == true){

		// ���ׂẴf�[�^�̍X�V��҂�
		WaitForMultipleObjects(EVENTS, handles, true, INFINITE);

		//----------
		// ���s���摜�����
		const NUI_IMAGE_FRAME*  pDepthFrame = 0;
		hr = NuiImageStreamGetNextFrame(depthHandle, 0, &pDepthFrame);
		if (hr != S_OK) {
			MessageBox::Show("error - NuiImageStreamGetNextFrame(depth)");
			capFlag = false;
			return;
		}
		NuiImageBuffer* pDepthTexture = pDepthFrame->pFrameTexture;

		// ���s���摜�����b�N
		KINECT_LOCKED_RECT LockedRectDepth;
		pDepthTexture->LockRect(0, &LockedRectDepth, NULL, 0);
		BYTE* pBufferDepth = (BYTE*)LockedRectDepth.pBits;

		// ���s�����J���[�ɕϊ�
		RGBQUAD* rgbrun = depthRGB;
		DE->Depth2Color(pBufferDepth, rgbrun);

		// �E�B���h�E�ɏo��
		memcpy(img_depth->imageData, depthRGB, img_depth->imageSize);
		cvShowImage("Depth View", img_depth);

		//----------
		// �J���[�摜�����
		const NUI_IMAGE_FRAME*  pImageFrame = 0;
		hr = NuiImageStreamGetNextFrame(cameraHandle, 0, &pImageFrame);
		if (hr != S_OK) {
			MessageBox::Show("error - NuiImageStreamGetNextFrame(camera)");
			capFlag = false;
			return;
		}
		NuiImageBuffer* pTexture = pImageFrame->pFrameTexture;

		// �E�B���h�E�ɏo��
		KINECT_LOCKED_RECT LockedRect;
		pTexture->LockRect(0, &LockedRect, NULL, 0);
		BYTE* pBuffer = (BYTE*)LockedRect.pBits;

		// ��������ǋL
		//memcpy(img_camera->imageData, pBuffer, img_camera->imageSize);

		memcpy(img_camera->imageData, pBuffer, img_camera->imageSize);
		cvResize(img_depth,img_tmp1);
		cvCvtColor(img_tmp1,img_gscale,CV_BGR2GRAY);

		cvCopy(img_tmp1,img_camera,img_gscale);

		// �q�X�g�O�����p
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
		// �摜�̊J��

		// ���s���摜�����
		NuiImageStreamReleaseFrame(depthHandle, pDepthFrame);

		// �J�����摜�����
		NuiImageStreamReleaseFrame(cameraHandle, pImageFrame);

		//----------
 		// �C�x���g�̊Ď�
		Application::DoEvents();
	}
}

//-------------------------------------------------------------------
// [�X�g�b�v]�{�^��
//-------------------------------------------------------------------
System::Void Form1::btnStop_Click_1(System::Object^  sender, System::EventArgs^  e){
	// �J�������́E�I�t
	capFlag = false;

	this->serialPort1->PortName = "COM3";
	this->serialPort1->Open();
	this->serialPort1->Write(iR.ToString("D"));
	this->serialPort1->Close();
}

//-------------------------------------------------------------------
// �I����
//-------------------------------------------------------------------
System::Void Form1::Form1_FormClosed(
	System::Object^  sender, System::Windows::Forms::FormClosedEventArgs^  e){
	
	// �J�������́E�I�t
	capFlag = false;

	// Kinect�I������
	NuiShutdown();
	CloseHandle(cameraEvent);
	CloseHandle(depthEvent);

	// OpenCV�I������
	cvReleaseImage(&img_camera);
	cvReleaseImage(&img_depth);
	cvDestroyAllWindows();
}
