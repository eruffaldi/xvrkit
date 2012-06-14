/// cl /EHsc /LD /O2 xvrsnap.cpp && upx xvrsnap.dll
/// TODO: optimize memory usage for multiple shots
/// TODO: test save texture
#include <windows.h>
#include <comdef.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string>
#include <vector>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "opengl32.lib")
#include <gdiplus.h>
using namespace Gdiplus;

typedef void *(*XVR_CallBack)(int);		
XVR_CallBack XVRGet;

namespace Tesla
{
	/**
	 * GLSaveImage is a tool for single images or the frame buffer
	 */
	class GLSaveImage
	{
	public:
		GLSaveImage(const char * format = "image/png");
		~GLSaveImage();

		/// find a name
		std::string findName(std::string prefix);

		/// current texture
		std::string saveTexture(std::string filename, bool autoName = true,int owidth = 0, int oheight= 0,bool flipped = true);

		/// save the buffer
		std::string saveBuffer(std::string filename, bool autoName = true);

	private:
		void save(void * bmp, std::string filename,int w,int h);
		bool incremental_; /// filename incremented if existing
		std::string format_;
		void * data_;
		int width_,height_;

	};
}

static int GetEncoderClsid(const char* xformat, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);
   _bstr_t format(xformat);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

namespace Tesla
{
	GLSaveImage::GLSaveImage(const char * format) : format_(format)
	{
		// Initialize GDI+.
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}

	GLSaveImage::~GLSaveImage()
	{

	}

	/// test file
	std::string GLSaveImage::findName(std::string prefix)
	{

		std::string ext = format_.substr(6); // image/
		std::string f = prefix + "." + ext;
		int count = 0;
		for(int i = 0; i < 10000; i++)
		{
            FILE * fp;
			char x[20];
			itoa(i,x,10);
			f = prefix + x + "." + ext;

			
			if((fp = fopen(f.c_str(),"rb")) == 0)
				return f;
			fclose(fp);		
		}
		throw("no filename");
	}

	/// current texture
	std::string GLSaveImage::saveTexture(std::string filename, bool autoName,int owidth, int oheight,bool flipped )
	{
		GLint w = 0;
		GLint  h = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
		width_ = w;
		height_ = h;
		if(w == 0 || h == 0)
			throw("empty texture");

		GLint iw = owidth == 0 ? w : owidth;
		GLint  ih = oheight== 0 ? h : oheight;
		
		Bitmap img(w,h);
		BitmapData bd;
		Rect rc(0,0,w,h);
		img.LockBits(&rc,ImageLockModeWrite,PixelFormat32bppARGB, &bd);
		if(bd.Scan0 == 0)
			throw("lock");
		glGetTexImage(GL_TEXTURE_2D,0,GL_BGRA_EXT, GL_UNSIGNED_BYTE, bd.Scan0);
		img.UnlockBits(&bd);
		std::string rname = autoName ? findName(filename):filename;
		save(&img, rname,iw,flipped ? -ih : ih);
		return rname;
	}

	/// save the buffer
	std::string GLSaveImage::saveBuffer(std::string filename, bool autoName)
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		GLint w = viewport[2];
		GLint h = viewport[3];
		width_ = w;
		height_ = h;

		Bitmap img(w,h);
		BitmapData bd;
		Rect rc(0,0,w,h);
		img.LockBits(&rc,ImageLockModeWrite,PixelFormat32bppARGB, &bd);
		if(bd.Scan0 == 0)
			throw("lock");
		glReadPixels(0,0,w,h,GL_BGRA_EXT,GL_UNSIGNED_BYTE,bd.Scan0);
		img.UnlockBits(&bd);
		std::string rname = autoName ? findName(filename):filename;
		save(&img, rname,w,-h);
		return rname;
	}

	void GLSaveImage::save(void * x, std::string filename,int w, int h)
	{
		Bitmap * bmp = (Bitmap*)x;
		int sw = bmp->GetWidth();
		int sh = bmp->GetHeight();
		bool flip = h < 0;
		h = abs(h);

		// standard case no transformation
		if(sw == w && sh == h)
		{
			if(flip)
				bmp->RotateFlip(RotateNoneFlipY);

			CLSID   encoderClsid;
			if(GetEncoderClsid(format_.c_str(), &encoderClsid) == -1)
				throw("encoder");
			_bstr_t xx = filename.c_str();
			bmp->Save(xx,&encoderClsid,NULL);
		}
		else
		{
			// new size, maybe a cropping and a flipping
			Bitmap bmp2(w,h);
			Graphics g(&bmp2);
			if(flip)
			{
				Point pf[] = {
					Point(0,h),
					Point(w,h),
					Point(0,0)
				};
				g.DrawImage(bmp,pf,3,0,sh-h,w,h,UnitPixel,0,0,0);
			}
			else
				g.DrawImage(bmp,0,0,0,0,w,h,UnitPixel);
			CLSID   encoderClsid;
			if(GetEncoderClsid(format_.c_str(), &encoderClsid) == -1)
				throw("encoder");
			_bstr_t xx = filename.c_str();
			bmp2.Save(xx,&encoderClsid,NULL);
		}
	}

}

static std::vector<unsigned char> snapdata;
static std::string snapLastNameD;
static std::string snapLastFormatD;
static int snapHeight;
static int snapWidth;

enum SnapWhatInfo { SW_WIDTH,SW_HEIGHT,SW_SIZE};

extern "C"
{

int __declspec(dllexport) snapInfo(int what)
{
	switch(what)
	{
	case SW_WIDTH: return snapWidth;
	case SW_HEIGHT: return snapHeight;
	case SW_SIZE: return snapdata.size();
	default:
		return -1;
	}
}

int __declspec(dllexport) snapData(char * data, int size)
{
	if(size < snapdata.size())
		return snapdata.size();
	else
	{
		memcpy(data,&snapdata[0],snapdata.size());
		return snapdata.size();
	}
}

__declspec(dllexport) const char *  snapLastName()
{
	return snapLastNameD.c_str();
}

__declspec(dllexport) const char * snapLastFormat()
{
	return snapLastFormatD.c_str();
}

/// snapshot of the viewport to file, to the clipboard, to a string
void __declspec(dllexport) snap(const char * filename)
{
	if(strcmp(filename,"<memory>") == 0)
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		GLint w = viewport[2];
		GLint h = viewport[3];
		
		snapdata.resize(w*h*4);
		glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,&snapdata[0]);
		snapLastFormatD = "RGBA";
		snapLastNameD = "<memory>";
		snapWidth = w;
		snapHeight = h;
	}
	else if(filename[0] == 0)
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		GLint w = viewport[2];
		GLint h = viewport[3];
		snapdata.resize(0);
		
		if(OpenClipboard(NULL))
		{
			EmptyClipboard();
			HANDLE hDIB = GlobalAlloc(GHND, sizeof(BITMAPINFO)+w*h*4);
			LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

			// use pixmap, owidth, oheight, format, components
			BITMAPINFOHEADER bi;

			memset(&bi, 0, sizeof(bi));
			bi.biSize = sizeof(bi);
			bi.biWidth = w;
			bi.biHeight = h;
			bi.biPlanes = 1;
			bi.biBitCount = 32;
			bi.biCompression = BI_RGB;
			bi.biSizeImage = w*h*4;
			*lpbi = bi;
			unsigned char * px = ((unsigned char*)lpbi)+sizeof(bi);
			glReadPixels(0,0,w,h,GL_BGRA_EXT,GL_UNSIGNED_BYTE,px);
			GlobalUnlock(hDIB);
			SetClipboardData(CF_DIB,hDIB);
			CloseClipboard();
			snapLastFormatD = "bmp";
			snapLastNameD = "<clipboard>";
			snapWidth = w;
			snapHeight = h;
		}			
		else
		{
			snapWidth = 0;
			snapHeight = 0;
			snapLastFormatD = "";
			snapLastNameD = "";
		}
	}
	else
	{
		Tesla::GLSaveImage saver("image/png");
		snapLastNameD = saver.saveBuffer(filename,true);
		snapLastFormatD = "png";
		snapdata.resize(0);
		snapWidth = 0;
		snapHeight = 0;
		printf("snap %s %s\n",filename,snapLastNameD.c_str());
	}
}

}

extern "C" __declspec(dllexport) const char * _meta(int id)
{
	if (id != 0) return "";
	return "int snapInfo(int what) // returns info about snap: 0=width 1=height 2=size\n"
        "int snapData(outstring data, int size) // return the data of the snap from memory buffer\n"
        "string snapLastName() // return the last filename generated\n"
        "string snapLastFormat() // return last format\n"
        "void snap(string output) // output prefix, or <memory> for snapData or <clipboard>\n"
    ;
}

extern "C" __declspec(dllexport) char *  __XVR_INIT (void *XVR_pointer)
{
	XVRGet = (XVR_CallBack) XVR_pointer;
	return NULL;
}
