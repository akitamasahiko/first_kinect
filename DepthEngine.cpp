#include "StdAfx.h"
#include "DepthEngine.h"

//----------------------------------------------------------------
// define
//----------------------------------------------------------------
#define	DEPTH_WIDTH			320		// ���s���摜�E��
#define	DEPTH_HEIGHT		240		// ���s���摜�E����
#define	DEPTH_RANGE_MIN		800		// ��ԋ߂��Ƃ���i800mm��0.8m�j
#define	DEPTH_RANGE_MAX		4000	// ��ԉ����Ƃ���i4000mm��4m�j

//----------------------------------------------------------------
DepthEngine::DepthEngine(void)
{
}


//----------------------------------------------------------------
// ���s���摜���J���[�ɕϊ�
//----------------------------------------------------------------
void DepthEngine::Depth2Color(BYTE* depth, RGBQUAD* buf){
	RGBQUAD	c;			// �F���
	int		lv = 0;		// 0�`255
	BYTE	iv = 0;		// 0�`255

	// �摜�f�[�^�̐擪�A�h���X
	USHORT* pRun = (USHORT *)depth;

	for (int y = 0; y < DEPTH_HEIGHT; y++) {
		for (int x = 0; x < DEPTH_WIDTH; x++) {			

			// ���i��ID���擾
			USHORT id = *pRun & 7;

			// �������擾
			USHORT RealDepth = (*pRun & 0xfff8) >> 3;
/*
			// �����̃`�F�b�N
			if ((RealDepth <= 0) || (RealDepth > DEPTH_RANGE_MAX)) {
				iv = 0;
			}
			else {
				// ������0�`255�ɕϊ�
				lv = 256 * (RealDepth - DEPTH_RANGE_MIN) / DEPTH_RANGE_MAX;
				if (lv > 255) {
					lv = 255;
				}
				else if (lv < 0) {
					lv = 0;
				}
	
				// Kinect�ɋ߂����𔒐F�ɂ���
				iv = 255 - (BYTE)lv;
			}


			// RGB�l���Z�b�g
			switch( id ){
				case 0:	// ���[�U�[�Ȃ�
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

			// �|�C���^��i�߂�
			buf++;
			pRun++;
		}
	}
}

