#include "StdAfx.h"
#include "DepthEngine.h"

//----------------------------------------------------------------
// define
//----------------------------------------------------------------
#define	DEPTH_WIDTH			320		// 奥行き画像・幅
#define	DEPTH_HEIGHT		240		// 奥行き画像・高さ
#define	DEPTH_RANGE_MIN		800		// 一番近いところ（800mm＝0.8m）
#define	DEPTH_RANGE_MAX		4000	// 一番遠いところ（4000mm＝4m）

//----------------------------------------------------------------
DepthEngine::DepthEngine(void)
{
}


//----------------------------------------------------------------
// 奥行き画像をカラーに変換
//----------------------------------------------------------------
void DepthEngine::Depth2Color(BYTE* depth, RGBQUAD* buf){
	RGBQUAD	c;			// 色情報
	int		lv = 0;		// 0～255
	BYTE	iv = 0;		// 0～255

	// 画像データの先頭アドレス
	USHORT* pRun = (USHORT *)depth;

	for (int y = 0; y < DEPTH_HEIGHT; y++) {
		for (int x = 0; x < DEPTH_WIDTH; x++) {			

			// 骨格のIDを取得
			USHORT id = *pRun & 7;

			// 距離を取得
			USHORT RealDepth = (*pRun & 0xfff8) >> 3;
/*
			// 距離のチェック
			if ((RealDepth <= 0) || (RealDepth > DEPTH_RANGE_MAX)) {
				iv = 0;
			}
			else {
				// 距離を0～255に変換
				lv = 256 * (RealDepth - DEPTH_RANGE_MIN) / DEPTH_RANGE_MAX;
				if (lv > 255) {
					lv = 255;
				}
				else if (lv < 0) {
					lv = 0;
				}
	
				// Kinectに近い方を白色にする
				iv = 255 - (BYTE)lv;
			}


			// RGB値をセット
			switch( id ){
				case 0:	// ユーザーなし
					c.rgbRed = iv;
					c.rgbGreen = iv;
					c.rgbBlue = iv;
					break;
				case 1:
					c.rgbRed = iv;
					break;
				case 2:
					c.rgbGreen = iv;
					break;
				case 3:
					c.rgbRed = iv / 4;
					c.rgbGreen = iv;
					c.rgbBlue = iv;
					break;
				case 4:
					c.rgbRed = iv;
					c.rgbGreen = iv;
					c.rgbBlue = iv / 4;
					break;
				case 5:
					c.rgbRed = iv;
					c.rgbGreen = iv / 4;
					c.rgbBlue = iv;
					break;
				case 6:
					c.rgbRed = iv / 2;
					c.rgbGreen = iv / 2;
					c.rgbBlue = iv;
					break;
				default:
					c.rgbRed = iv;
					c.rgbGreen = iv;
					c.rgbBlue = iv;
					break;
			}
*/
			if( id == 0 ){
				c.rgbRed = 255;
				c.rgbGreen = 255;
				c.rgbBlue = 255;
			}else{
				c.rgbRed = 0;
				c.rgbGreen = 0;
				c.rgbBlue = 0;
			}

			*buf = c;

			// ポインタを進める
			buf++;
			pRun++;
		}
	}
}

