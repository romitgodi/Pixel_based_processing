// Source.cpp : Defines the entry point for the console application.
//

#include <afxwin.h>  // necessary for MFC to work properly
#include "Header.h"
#include "../../src/blepo.h"
#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace blepo;

void computeRegionProperties(int count, const ImgInt &imgLabel, ImgBgr &imgFile_B3)
{
	double moment_Reg00 = 0, moment_Reg01 = 0, moment_Reg10 = 0, moment_Reg11 = 0, moment_Reg02 = 0, moment_Reg20 = 0;
	int pixel;

	for (int y = 0; y < imgLabel.Height(); ++y)
	{
		for (int x = 0; x < imgLabel.Width(); ++x)
		{
			pixel = imgLabel(x, y);
			if (pixel == count)
			{
				moment_Reg00++;
				moment_Reg10 += x;
				moment_Reg01 += y;
				moment_Reg11 += x*y;
				moment_Reg02 += y*y;
				moment_Reg20 += x*x;
			}
		}
	}

	cout << "Regular moments: " << endl << moment_Reg00 << endl << moment_Reg01 << endl << moment_Reg10 << endl << moment_Reg11 << endl << moment_Reg02 << endl << moment_Reg20 << endl << endl;

	double x_Central = 0, y_Central = 0;
	x_Central = moment_Reg10 / moment_Reg00;
	y_Central = moment_Reg01 / moment_Reg00;

	double moment_Cent00 = 0, moment_Cent01 = 0, moment_Cent10 = 0, moment_Cent11 = 0, moment_Cent02 = 0, moment_Cent20 = 0;

	moment_Cent00 = moment_Reg00;
	moment_Cent11 = moment_Reg11 - (x_Central*moment_Reg01);
	moment_Cent02 = moment_Reg02 - (y_Central*moment_Reg01);
	moment_Cent20 = moment_Reg20 - (x_Central*moment_Reg10);

	cout << "Central moments: " << endl << moment_Cent00 << endl << moment_Cent01 << endl << moment_Cent10 << endl << moment_Cent11 << endl << moment_Cent02 << endl << moment_Cent20 << endl << endl;

	double comp = 0;
	comp = moment_Cent11 / moment_Cent00;
	cout << "Compactness: " << comp << endl << endl;

	double ecct = 0, num = 0, den = 0;
	num = 2 * (sqrt(pow((moment_Cent20 - moment_Cent02), 2) + 4 * pow(moment_Cent11, 2)));
	den = moment_Cent20 + moment_Cent02 + (sqrt(pow((moment_Cent20 - moment_Cent02), 2) + 4 * pow(moment_Cent11, 2)));
	ecct = sqrt(num / den);
	cout << "Eccentricity: " << ecct << endl << endl;

	double direction = 0;
	direction = atan2(2 * moment_Cent11, (moment_Cent20 - moment_Cent02));
	cout << "Direction: " << direction << endl << endl;

	int type = 0;
	if ((moment_Cent00 > 5000 && moment_Cent00 < 5500) && (ecct > 0.9 && ecct < 0.95))
	{
		type = 1;
		cout << "Banana" << endl << endl;
	}
	else if ((moment_Cent00 > 3000 && moment_Cent00 < 4000) && (ecct > 0.2 && ecct < 0.4))
	{
		type = 3;
		cout << "Grapefruit" << endl << endl;
	}
	else if ((moment_Cent00 > 5500 && moment_Cent00 < 6500) && (ecct > 0.2 && ecct < 0.38))
	{
		type = 2;
		cout << "Apple" << endl << endl;
	}

	ImgBgr imgFile_Color;
	for (int y = 0; y < imgLabel.Height(); ++y)
	{
		for (int x = 0; x < imgLabel.Width(); ++x)
		{
			if (imgLabel(x, y) == count)
			{
				Bgr& pix = imgFile_B3(x, y);
				if (type == 1)
				{
					pix.b = 255;
					pix.g = 0;
					pix.r = 0;
				}
				else if (type == 2)
				{
					pix.b = 0;
					pix.g = 255;
					pix.r = 0;
				}
				else if (type == 3)
				{
					pix.b = 0;
					pix.g = 0;
					pix.r = 255;
				}
				FloodFill4(imgFile_B3, x, y, pix, &imgFile_Color);
			}
		}
	}
}

int main(int argc, const char* argv[], const char* envp[])
{
	// Initialize MFC and return if failure
	HMODULE hModule = ::GetModuleHandle(NULL);
	if (hModule == NULL || !AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
	{
		printf("Fatal Error: MFC initialization failed (hModule = %x)\n", (int)hModule);
		return 1;
	}

	try
	{
		// Load filename 1.
		ImgGray imgFile_Original;

		// Define image path for loading
		CString path("../../images/");
		CString imgPath_1 = path + CString(argv[1]);

		Load(imgPath_1, &imgFile_Original);

		// Check for proper file size
		if (imgFile_Original.Width() == 0 || imgFile_Original.Height() == 0)
		{
			printf("Unable to load file 1. Improper file size!\n");
			exit(0);
		}

		// Display the original image
		Figure fig1;
		fig1.SetTitle("Original image");
		fig1.Draw(imgFile_Original);

		ImgBinary imgFile_LowThreshold;

		Threshold(imgFile_Original, MinThreshold, &imgFile_LowThreshold);

		// Display the low threshold image
		Figure fig2;
		fig2.SetTitle("Low threshold image");
		fig2.Draw(imgFile_LowThreshold);

		ImgBinary imgFile_HighThreshold;

		Threshold(imgFile_Original, MaxThreshold, &imgFile_HighThreshold);

		// Display the high threshold image
		Figure fig3;
		fig3.SetTitle("High threshold image");
		fig3.Draw(imgFile_HighThreshold);

		ImgBinary imgFile_temp(imgFile_Original.Width(), imgFile_Original.Height());

		for (int y = 0; y < imgFile_Original.Height(); ++y)
		{
			for (int x = 0; x < imgFile_Original.Width(); ++x)
			{
				if (imgFile_temp(x, y) == 1)
				{
					imgFile_temp(x, y) = 0;
				}
			}
		}

		// Double Thresholding and Floodfill
		for (int y = 0; y < imgFile_Original.Height(); ++y)
		{
			for (int x = 0; x < imgFile_Original.Width(); ++x)
			{
				if (imgFile_Original(x, y) > MinThreshold)
				{
					imgFile_LowThreshold(x, y) = 1;
				}
			}
		}

		for (int y = 0; y < imgFile_Original.Height(); ++y)
		{
			for (int x = 0; x < imgFile_Original.Width(); ++x)
			{
				if (imgFile_Original(x, y) > MaxThreshold)
				{
					FloodFill4(imgFile_LowThreshold, x, y, 1, &imgFile_temp);
				}
			}
		}

		ImgBinary imgFile_1, imgFile_2;

		Dilate3x3(imgFile_temp, &imgFile_1);
		Erode3x3(imgFile_1, &imgFile_2);

		// Display the floodfill image
		Figure fig4;
		fig4.SetTitle("Floodfill image");
		fig4.Draw(imgFile_temp);

		// Display the noise removed image
		Figure fig5;
		fig5.SetTitle("Noise removed image");
		fig5.Draw(imgFile_2);

		// Run connected component to count number of items
		ImgInt imgFile_Label;
		std::vector<ConnectedComponentProperties<ImgBinary::Pixel>> imgProp;
		ConnectedComponents4(imgFile_2, &imgFile_Label, &imgProp);

		ImgBgr imgFile_B1, imgFile_B2;

		Convert(imgFile_2, &imgFile_B1);
		int label = imgProp.size() - 1;
		for (int i = 1; i <= label; ++i)
		{
			cout << "Item " << i << endl;
			computeRegionProperties(i, imgFile_Label, imgFile_B1);
		}

		// Draw boundaries for all items
		ImgGray imgFile_Temp1, imgFile_Temp2, imgFile_Temp3;
		Convert(imgFile_B1, &imgFile_Temp3);
		Erode3x3(imgFile_Temp3, &imgFile_Temp2);
		Xor(imgFile_Temp3, imgFile_Temp2, &imgFile_Temp1);

		Convert(imgFile_Temp1, &imgFile_B1);
		Convert(imgFile_Original, &imgFile_B2);

		for (int y = 0; y < imgFile_Original.Height(); ++y)
		{
			for (int x = 0; x < imgFile_Original.Width(); ++x)
			{
				Bgr& pix_1 = imgFile_B1(x, y);
				Bgr& pix_2 = imgFile_B2(x, y);
				if (pix_1.b == 25)
				{
					pix_2.b = 0;
					pix_2.g = 255;
					pix_2.r = 255;
				}
				if (pix_1.b == 76)
				{
					pix_2.b = 0;
					pix_2.g = 0;
					pix_2.r = 255;
				}
				if (pix_1.b == 153)
				{
					pix_2.b = 0;
					pix_2.g = 255;
					pix_2.r = 0;
				}
			}
		}

		// Image after classification
		Figure fig6;
		fig6.SetTitle("Output");
		fig6.Draw(imgFile_B2);

		// Loop forever until user presses Ctrl-C in terminal window.
		EventLoop();
	}

	catch (const Exception& e)
	{
		e.Display();    // display exception 
	}
	return 0;
}