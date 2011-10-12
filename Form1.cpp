#include "stdafx.h"
#include "Form1.h"
#include <stdio.h>
#include <opencv2\opencv.hpp>
#include <Windows.h>
#include <MSR_NuiApi.h>

using namespace first_kinect;

#define	CAMERA_WIDTH	640
#define	CAMERA_HEIGHT	480

bool	capFlag = false;
IplImage*	img_camera = 0;
HANDLE	cameraEvent = NULL;
HANDLE	cameraHandle = NULL;


System::Void Form1::Form1_Load_1(System::Object^ sender, System::EventArgs^ e){

	HRESULT	hr = S_OK;

	hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
	if (hr != S_OK) {
		MessageBox::Show("error - NuiInitialize");
		return;
	}

    cameraEvent = CreateEvent(0, true, false, 0);
    hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR,
            NUI_IMAGE_RESOLUTION_640x480,0,2,
            cameraEvent,&cameraHandle);
    if (hr != S_OK){
        MessageBox::Show("error - NuiImageStreamOpen(camera)");
        return;
    }
     
    img_camera = cvCreateImage(cvSize(CAMERA_WIDTH, CAMERA_HEIGHT),
            IPL_DEPTH_8U,4);
	cvNamedWindow("Camera View", CV_WINDOW_AUTOSIZE);

	MessageBox::Show("test");

}

System::Void Form1::btnStart_Click(System::Object^ sender, System::EventArgs^ e){
    HRESULT hr = S_OK;

    btnStop->Focus();

    capFlag = true;
    while (capFlag == true ){
        WaitForSingleObject(cameraEvent, INFINITE);

        const NUI_IMAGE_FRAME* pImageFrame = 0;
        hr = NuiImageStreamGetNextFrame(cameraHandle, 0, &pImageFrame);
        if (hr != S_OK){
            MessageBox::Show("error - NuiImageStreamGetNextFrame");
            capFlag = false;
            return;
        }
        NuiImageBuffer* pTexture = pImageFrame->pFrameTexture;

        KINECT_LOCKED_RECT LockedRect;
        pTexture->LockRect(0,&LockedRect,NULL,0);
        BYTE* pBuffer = (BYTE*)LockedRect.pBits;
        memcpy(img_camera->imageData, pBuffer, img_camera->imageSize);
        cvShowImage("Camera View",img_camera);

        NuiImageStreamReleaseFrame(cameraHandle,pImageFrame);

        Application::DoEvents();
    }
}

System::Void Form1::btnStop_Click_1(System::Object^ sender, System::EventArgs^ e){
    capFlag = false;
}

System::Void Form1::Form1_FormClosed(System::Object^ sender,System::Windows::Forms::FormClosedEventArgs^ e){
    capFlag = false;
    NuiShutdown();
    CloseHandle(cameraEvent);
    cvReleaseImage(&img_camera);
    cvDestroyAllWindows();
}
