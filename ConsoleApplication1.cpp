#include "stdafx.h"
#include "afxwin.h"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv.hpp"  
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>  
#include "template_matching.h"
#include "MyDrawing.h"
#include "math.h"

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <Wingdi.h>


//#include <atlimage.h> //CImage defined here
//#include <afxwin.h> //CBitmap defined here

using namespace cv;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PI 3.14159265

int total_image_number = 0;

int target_contour_detected = 0;
int target_inside_detected = 0;
int target_inside_turn_detected = 0;

Mat src; Mat src_gray;  Mat im_gray; Mat im_bw2;
Mat img; Mat template_reticle; Mat img_captured; Mat img_to_show;
Mat Roi_img;

int CG_col_prev[20] = {0};
int CG_row_prev[20] = {0};

//*****Time keeping initialization
_int64 countspersec, curr_time, prev_time;
double secpercount;
float totalTime = 0;
float totalTime_temp = 0;
//*****Time keeping initialization finished


RNG rng(12345);

HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
COORD CursorPosition;

/// Function header
void MatchingMethod(Mat source_image, Mat template_image, Mat result ,template_matching_row_and_col_result &row_and_col_result);
void CaptureScreen(char *filename);
void CaptureScreen_DOI_1(char *filename);
void CaptureScreen_DOI_2(char *filename);
void RGB2MatData(Mat image, int row, int col, int R, int G, int B);
int IsPossibleRed(Vec4b color);
int IsInPossibleRedLine(Mat img, int x, int y);
int IsInPossibleRedLineDiff(Mat img, int x, int y);
int MaximumInt(int a, int b);
int MinimumInt(int a, int b);
void gotoXY(int x, int y);
//void MyDrawThread();

int ROI_width = 384;
int ROI_height = 216;

int IsWideSearch = 1;

Mat ImageTest(ROI_height, ROI_width, CV_8UC3, Scalar(0,0,0));

int SquareRootInt[500000]={0};

template_matching_row_and_col_result MatchingResult;

void gotoXY(int x, int y) 
{ 
	CursorPosition.X = x; 
	CursorPosition.Y = y; 
	SetConsoleCursorPosition(console,CursorPosition); 
}

int MaximumInt(int a, int b) // for int only. Faster than fmax() and fmin() methods
{
	if (a >= b) 
		return a;
	else 
		return b;
}

int MinimumInt(int a, int b)
{
	if (a >= b) 
		return b;
	else 
		return a;
}

int SignInt(int a)
{
	if (a > 0)
		return 1;
	else if (a < 0)
		return -1;
	else
		return 0;
}

int IsPossibleRed(Vec4b color)
{
    // These parameters are tuned with machine learning (tensorflow + keras). Still being updated over time.
	if(  (color[0] > 25 && color[0] < 180 && color[1] > 25 && color[1] < 150 && 
	      (color[0]-color[1])<40 && (color[2] > 150 || (color[2] <= 150 && (color[2]-color[1]) > 50 && (color[2]-color[0]) > 40)) && 
		   (color[2]-color[1]) > 20 && ((color[1] < 80 && (color[0]-color[1]) > 15) || (color[1] >= 80  && (color[0]-color[1]) > 5 )) && 
		   ((color[2] < 170 && color[0] > 150 && color[1] > 140) || (color[2] >= 170))) || ((color[2] <= 150 && (color[2]-color[1]) > 50 && (color[2]-color[0]) > 40))  )  //&& (color[2]-color[1]) > 20 maybe work better in this case
	//if(  (color[2]) > 130 && ( (  (color[2]) > 186 && abs(color[0]-color[1]*11/10+7)<10  ) ||  (   (color[2]) < 170 && abs(color[0]-color[1]*18/10+45)<10   )  ||  (  (color[2]) > 179 && abs(color[0]-color[1]*13/10+23)<10  )  ||  (  (color[2]) > 186 && abs(color[0]-color[1]*13/10+45)<10  )  ||  (  (color[2]) < 182 && abs(color[0]-color[1]*15/10+25)<10  )   )  )
	/*
	if(  (color[2]) > 130 && ( (  (color[2])>186 && color[0]-color[1]>-3 && color[0]-color[1]<11 && color[0]>81 && color[0]<182) 
		||  (  (color[2]) < 170 && color[0]-color[1]>-9 && color[0]-color[1]<14 && color[0]>43 && color[0]<85)  
		||  (  (color[2]) > 179 && color[0]-color[1]>0 && color[0]-color[1]<23 && color[0]>76 && color[0]<167)  
		||  (  (color[2]) > 186 && color[0]-color[1]>-28 && color[0]-color[1]<14 && color[0]>44 && color[0]<141)  
		||  (  (color[2]) < 182 && color[0]-color[1]>-4 && color[0]-color[1]<19 && color[0]>51 && color[0]<133)   )  )
	*/
	{
		return 1;
	}	
    else
	{
		return 0;
	}
}

int IsInPossibleRedLineDiff(Mat img, int x, int y)
{
	int jump = 4;

	Vec4b color_0 = img.at<Vec4b>(Point(x,y));
	Vec4b color_up = img.at<Vec4b>(Point(x,y-jump));
	Vec4b color_down = img.at<Vec4b>(Point(x,y+jump));
	Vec4b color_left = img.at<Vec4b>(Point(x-jump,y));
	Vec4b color_right = img.at<Vec4b>(Point(x+jump,y));

	int horizontal = 0;
	int vertical = 0;

	int redness_horizontal_left = color_0[2] - color_left[2] + color_left[0] - color_0[0] + color_left[1] - color_0[1];
	int redness_horizontal_right = color_0[2] - color_right[2] + color_right[0] - color_0[0] + color_right[1] - color_0[1];
	int redness_horizontal = redness_horizontal_left + redness_horizontal_right;

	int redness_vertical_up = color_0[2] - color_up[2] + color_up[0] - color_0[0] + color_up[1] - color_0[1];
	int redness_vertical_down = color_0[2] - color_down[2] + color_down[0] - color_0[0] + color_down[1] - color_0[1];
	int redness_vertical = redness_vertical_up + redness_vertical_down;

	int redness_score = 0;

	if (redness_horizontal >= redness_vertical)
	{
		horizontal = 1;
		redness_score = redness_horizontal;
	}
	else
	{
		vertical = 1;
		redness_score = redness_vertical;
	}

	int threshold_redness=160;

	if (redness_score>threshold_redness && horizontal==1 &&  redness_horizontal_left>0 && redness_horizontal_right>0 && ((color_0[2]-color_left[2]>0 && color_0[2]-color_right[2]>0) || color_0[2]>150) )
		return 1;
	else if (redness_score>threshold_redness && vertical==1 &&  redness_vertical_up>0 && redness_vertical_down>0 && ((color_0[2]-color_up[2]>0 && color_0[2]-color_down[2]>0) || color_0[2]>150) )
		return 1;
	else
		return 0;
}

int IsInPossibleRedLine(Mat img, int x, int y)
{
	int score = 0;
	//Vec4b color_1;
	//Vec4b color_2;
	//Vec4b color_3;
	/*
	color_1=img.at<Vec4b>(Point(x-1,y)); color_2=img.at<Vec4b>(Point(x+1,y));  
	if(IsPossibleRed(color_1) && IsPossibleRed(color_2)) {score++;}

	color_1=img.at<Vec4b>(Point(x,y-1)); color_2=img.at<Vec4b>(Point(x,y+1));  
	if(IsPossibleRed(color_1) && IsPossibleRed(color_2)) {score++;}

	color_1=img.at<Vec4b>(Point(x-1,y-1)); color_2=img.at<Vec4b>(Point(x+1,y+1));  
	if(IsPossibleRed(color_1) && IsPossibleRed(color_2)) {score++;}

	color_1=img.at<Vec4b>(Point(x-1,y+1)); color_2=img.at<Vec4b>(Point(x+1,y-1));  
	if(IsPossibleRed(color_1) && IsPossibleRed(color_2)) {score++;}

	if(score>=2) {return 0;}
	if(score<2) {score=0;}
	*/
	//color_1=img.at<Vec4b>(Point(x,y+1)); color_2=img.at<Vec4b>(Point(x,y+2)); //color_3=img.at<Vec4b>(Point(x,y+3));  
	if(IsInPossibleRedLineDiff(img, x, y + 1) && IsInPossibleRedLineDiff(img, x, y + 2)) 
	{
		score++;
	}

	//color_1=img.at<Vec4b>(Point(x+1,y+1)); color_2=img.at<Vec4b>(Point(x+2,y+2)); //color_3=img.at<Vec4b>(Point(x+3,y+3));   
	if(IsInPossibleRedLineDiff(img, x + 1, y + 1) && IsInPossibleRedLineDiff(img, x + 2, y + 2))
	{
		score++;
	}

	if(score == 2)
	{
		return 0;
	}

	//color_1=img.at<Vec4b>(Point(x+1,y)); color_2=img.at<Vec4b>(Point(x+2,y)); //color_3=img.at<Vec4b>(Point(x+3,y));  
	if(IsInPossibleRedLineDiff(img, x + 1, y) && IsInPossibleRedLineDiff(img, x + 2, y))
	{
		score++;
	}

	if(score == 2) {return 0;}

	//color_1=img.at<Vec4b>(Point(x+1,y-1)); color_2=img.at<Vec4b>(Point(x+2,y-2)); //color_3=img.at<Vec4b>(Point(x+3,y-3));   
	if(IsInPossibleRedLineDiff(img, x + 1, y - 1) && IsInPossibleRedLineDiff(img, x + 2,y - 2))
	{
		score++;
	}

	if(score >= 2)
	{
		return 0;
	}
	
	if(score < 2)
	{
		score=0;
	}

	//color_1=img.at<Vec4b>(Point(x,y-1)); color_2=img.at<Vec4b>(Point(x,y-2)); //color_3=img.at<Vec4b>(Point(x,y-3));   
	if(IsInPossibleRedLineDiff(img, x, y - 1) && IsInPossibleRedLineDiff(img, x, y - 2))
	{
		score++;
	}

	//color_1=img.at<Vec4b>(Point(x-1,y-1)); color_2=img.at<Vec4b>(Point(x-2,y-2)); //color_3=img.at<Vec4b>(Point(x-3,y-3));   
	if(IsInPossibleRedLineDiff(img, x - 1, y - 1) && IsInPossibleRedLineDiff(img, x - 2, y - 2))
	{
		score++;
	}

	if(score == 2)
	{
		return 0;
	}
		
	//color_1=img.at<Vec4b>(Point(x-1,y)); color_2=img.at<Vec4b>(Point(x-2,y)); //color_3=img.at<Vec4b>(Point(x-3,y));  
	if(IsInPossibleRedLineDiff(img, x - 1, y) && IsInPossibleRedLineDiff(img, x - 2,y)) 
	{
		score++;
	}

	if(score == 2)
	{
		return 0;
	}

	//color_1=img.at<Vec4b>(Point(x-1,y+1)); color_2=img.at<Vec4b>(Point(x-2,y+2)); //color_3=img.at<Vec4b>(Point(x-3,y+3));   
	if(IsInPossibleRedLineDiff(img, x - 1, y + 1) && IsInPossibleRedLineDiff(img, x - 2, y + 2))
	{
		score++;
	}

	if(score >= 2)
	{
		return 0;
	}
	
	if(score < 2)
	{
		return 1;
	}
}

BOOL PeekandPump()
{
    static MSG msg;

    while(::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
        if(!AfxGetApp()->PumpMessage())
        {
            ::PostQuitMessage(0);
            return FALSE;
        }    
    }

    return TRUE;
}


int _tmain(int argc, _TCHAR* argv[])
{
	MatchingResult.col = 0;
	MatchingResult.row = 0;
	template_reticle = imread("reticle.jpg");

	char *filename_original = "D:\\Screeshot.bmp"; // On D:\\ by default.

	int image_number = 0;

	for(int ii = 0; ii < 500000; ii++)
    {
		SquareRootInt[ii] = int(sqrt(ii));
    } // fast square root table

	//*****Time keeping initialization
	QueryPerformanceFrequency((LARGE_INTEGER*)&countspersec);//time keeping code
	secpercount = 1.0 / (double)countspersec; //Time record
	QueryPerformanceCounter( ( LARGE_INTEGER* ) &prev_time ); //Time record 

	std::ofstream output_image_number;
	output_image_number.open("D:\\read_and_write_test_4.txt"); // On D:\\ by default.
	//*****Time keeping initialization finished

	for(int i = 1; i < 9999999999999; i++)  //
	{
		//*****Time record
		QueryPerformanceCounter( ( LARGE_INTEGER* ) &curr_time ); //Time record
		totalTime_temp = totalTime + (curr_time - prev_time) * secpercount;  //Time record   
		//*****Time record

		
		total_image_number = i;
		
		std::string address = " ";
							
		image_number += 1;

		address="D:\\screenshots\\";   //folder to store images captured
		std::string imagename;
		std::ostringstream image_number_to_string;

		image_number_to_string << image_number;
		imagename = image_number_to_string.str();
		address += imagename;
		address += ".jpg";			
		char*filename2 = const_cast<char*>(address.c_str());

		CaptureScreen(filename2);	

		//*****Time record update
		QueryPerformanceCounter( ( LARGE_INTEGER* ) &curr_time ); //Time record
		totalTime = totalTime + (curr_time - prev_time) * secpercount;  //Time record
		
		QueryPerformanceCounter( ( LARGE_INTEGER* ) &prev_time ); 

		//output_image_number<<totalTime<<endl;
		//*****Time record update finished


		//****peek message
		// Check to see if any messages are waiting in the queue
		MSG msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Translate the message and dispatch it to WindowProc()
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If the message is WM_QUIT, exit the while loop
		if(msg.message == WM_QUIT)
		break;
		PeekandPump();
		//****peek finished
	}
	return 0;
}

Mat hwnd2mat(HWND hwnd)
{

    HDC hwindowDC, hwindowCompatibleDC;

    int height, width, srcheight, srcwidth;
    HBITMAP hbwindow;
    Mat src;
    BITMAPINFOHEADER bi;

    hwindowDC = GetDC(hwnd);
    hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC,COLORONCOLOR);  

    RECT windowsize;    // get the height and width of the screen
    GetClientRect(hwnd, &windowsize);

    srcheight = windowsize.bottom;
    srcwidth = windowsize.right;
    height = windowsize.bottom / 5;  //change this to whatever size you want to resize
    width = windowsize.right / 5;

    src.create(height,width,CV_8UC4);
	
    // create a bitmap
    hbwindow = CreateCompatibleBitmap( hwindowDC, width, height);
    bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
    bi.biWidth = width;    
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;    
    bi.biBitCount = 32;    
    bi.biCompression = BI_RGB;    
    bi.biSizeImage = 0;  
    bi.biXPelsPerMeter = 0;    
    bi.biYPelsPerMeter = 0;    
    bi.biClrUsed = 0;    
    bi.biClrImportant = 0;

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);
    // copy from the window device context to the bitmap device context
    StretchBlt( hwindowCompatibleDC, 0,0, width, height, hwindowDC, srcwidth/2-width/2, srcheight/2-height/2, width, height, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC,hbwindow,0,height,src.data,(BITMAPINFO *)&bi,DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow, use streched one
    // avoid memory leak
    DeleteObject (hbwindow); DeleteDC(hwindowCompatibleDC); ReleaseDC(hwnd, hwindowDC);
	
    return src;
}


void RGB2MatData(Mat image, int row, int col, int R, int G, int B)
{
	Vec3b color = image.at<Vec3b>(Point(col, row));

	color[0] = B;
	color[1] = G;
	color[2] = R;

	image.at<Vec3b>(Point(col, row)) = color;
}


void CaptureScreen(char *filename)
{
    //****peek message
	// Check to see if any messages are waiting in the queue
	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        // Translate the message and dispatch it to WindowProc()
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // If the message is WM_QUIT, exit the while loop
    //if(msg.message == WM_QUIT)
    //break;
	PeekandPump();
	//****peek finished


	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    HWND hDesktopWnd = GetDesktopWindow();
    //HDC hDesktopDC = GetDC(hDesktopWnd);
    //HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
    //HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);

	img_captured = hwnd2mat(hDesktopWnd);
	
	int Roi_col = img_captured.cols / 12;  // narrow it down to a small area
	int Roi_row = img_captured.rows / 12; 
	
	Roi_img = img_captured(Rect(img_captured.cols / 2 - Roi_col / 2 - 40 * IsWideSearch, img_captured.rows / 2 - Roi_row / 2 - 20 * IsWideSearch, Roi_col + 80 * IsWideSearch, Roi_row + 40 * IsWideSearch));
	img_captured = Roi_img;
	
	int reticle_x = img.cols / 2; //img.cols/2;  //initialize to an approxomate value. Change it later
	int reticle_y = img.rows / 2; //img.rows/2;  //initialize to an approxomate value. Change it later

	//if(total_image_number<=100 || total_image_number==150 || total_image_number==200) {imwrite(_T(filename),img_captured);}
	//if(total_image_number%30==0) {imwrite(_T(filename),img_captured);}
	
	//Vec4b color = img_captured.at<Vec4b>(Point(104,26));
	
	//imwrite("D:\\screenshots_overwatch_disturbance\\temp.jpg",img_captured);
	//img_captured=imread("D:\\screenshots_overwatch_disturbance\\temp.jpg");
	//imshow("Original Image",img);
   
    img_captured = imread("D:\\screenshots_real\\750.jpg");
    
	//Roi_img=img(Rect(img.cols/2-Roi_col/2,img.rows/2-Roi_row/2-12,Roi_col,Roi_row));
	//img=Roi_img;
	
	
	if ((GetKeyState(0x90) == 1)) //0x90 is numlock
	{
		imwrite("D:\\screenshots_overwatch_disturbance\\temp.jpg", img_captured);
		img = imread("D:\\screenshots_overwatch_disturbance\\temp.jpg");
		Mat Result = img.clone();
		MatchingMethod(img, template_reticle, Result, MatchingResult);	
		reticle_x = MatchingResult.col;
		reticle_y = MatchingResult.row;
	}
	
	imshow("Original Image", img_captured);
    waitKey();

	img = img_captured.clone();
	img_to_show = img_captured.clone();
	//***********threshold****
	/*
	for(int y=0;y<img_captured.rows;y++)
    {
        for(int x=0;x<img_captured.cols;x++)
        {
        Vec4b color = img_captured.at<Vec4b>(Point(x,y));
		//if(x>3 && x<img_captured.cols-3 && y>3 && y<img_captured.rows-3)
		if(x>3 && x<img_captured.cols-3 && y>3 && y<img_captured.rows-3 && IsPossibleRed(color)==1)
        {
			if(IsInPossibleRedLine(img_captured, x, y))
			{
				//if(IsInPossibleRedLine(img_captured, x, y))
				{
				color[0] = 255;
				color[1] = 255;
				color[2] = 255;
				}
			}
		}
        else
        {
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
        }
		img.at<Vec4b>(Point(x,y)) = color;
		}
    }
	*/
	//***********threshold finished****

	//imwrite(_T(filename),img);
	//cv::imshow("BW image",img);
	//waitKey();
	//cvtColor( img, img, CV_BGR2GRAY );

	//threshold(img, img, 128, 255, THRESH_BINARY);
	//imshow("Original Image",img);
	
	/*
	int erosion_type;
	int erosion_size=1;
	int erosion_elem=0;
    if( erosion_elem == 0 ){ erosion_type = MORPH_RECT; }
    else if( erosion_elem == 1 ){ erosion_type = MORPH_CROSS; }
    else if( erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }

	Mat erosion_element = getStructuringElement( erosion_type, Size( 2*erosion_size + 1, 2*erosion_size+1 ),Point( erosion_size, erosion_size ) );
	
	int dilatation_type;
	int dilatation_size=1;
	int dilatation_elem=0;
    if( dilatation_elem == 0 ){ dilatation_type = MORPH_RECT; }
    else if( dilatation_elem == 1 ){ dilatation_type = MORPH_CROSS; }
    else if( dilatation_elem == 2) { dilatation_type = MORPH_ELLIPSE; }

    Mat dilatation_element = getStructuringElement( erosion_type, Size( 2*erosion_size + 1, 2*erosion_size+1 ),Point( erosion_size, erosion_size ) );
	*/

	//dilate(img,img,dilatation_element);
	//imshow("dilatation", img); 
	//waitKey(1);

	//cv::dilate(img,img,dilatation_element);
	//cv::imshow("dilatation", img); 
	//waitKey(1);
	

	int row_sum=0;
	int col_sum=0;
	int white_count=0;
	int distance_square_sum=0;

    for(int y = 8; y <= img.rows - 8; y += 3)
    {
        for(int x = 8; x <= img.cols - 8; x += 3)
        {
        //Vec4b color = img.at<Vec4b>(Point(x,y));
		//Vec4b color_initial = img.at<Vec4b>(Point(x,y));
		/*
		color_initial[0] = 1;
		color_initial[1] = 1;
		color_initial[2] = 1;
		img_to_show.at<Vec4b>(Point(x,y)) = color_initial;
		*/
		//if(color[0] > 250 && color[1] > 250 && color[2] > 250)
		if(!(x >= reticle_x && x <= reticle_x + 6 && y >= reticle_y && y <= reticle_y+6) && IsInPossibleRedLineDiff(img_captured, x, y) == 1)
			{
				for(int y_inner = -1; y_inner < 2; y_inner++)
				{
					for(int x_inner = -1; x_inner < 2; x_inner++)
					{
						if(IsInPossibleRedLine(img_captured, x + x_inner, y + y_inner) == 1)
						{
							row_sum += (y + y_inner);
							col_sum += (x + x_inner);
							white_count += 1;

							/*
							color[0] = 254;
							color[1] = 254;
							color[2] = 254;

							img_to_show.at<Vec4b>(Point(x+x_inner,y+y_inner)) = color;
							*/
						}
				
					}
			
				}
						
			}

		}
	}
	
	//cv::imshow("dilatation", img_to_show); 
	waitKey();

	if (white_count == 0) 
	{
		white_count = 1;
	} //to be safe
	
	int row_CenterGravity = row_sum / white_count;
	int col_CenterGravity = col_sum / white_count;

	for (int iii = 19; iii > 0; iii--)
	{
		CG_col_prev[iii] = CG_col_prev[iii-1];
		CG_row_prev[iii] = CG_row_prev[iii-1];
	}
	CG_col_prev[0] = col_CenterGravity;
	CG_row_prev[0] = row_CenterGravity;

	/*
	for(int y=0;y<img.rows;y++)
    {
        for(int x=0;x<img.cols;x++)
        {
        Vec4b color = img.at<Vec4b>(Point(x,y));
        if(color[0] > 250 && color[1] > 250 && color[2] > 250)
		{
        distance_square_sum=distance_square_sum+((y-row_CenterGravity)*(y-row_CenterGravity)+(x-col_CenterGravity)*(x-col_CenterGravity));
		}
		}
	}
    
	int distance_square_average=distance_square_sum/white_count; //radius
	if (white_count==1) {distance_square_average=0;} //to be safe
	*/

	//circle(img, Point(col_CenterGravity, row_CenterGravity), SquareRootInt[distance_square_average], Scalar(255,129,0), 2, 8);
	//imshow("point", img);
	//imwrite(_T(filename),img);
	//waitKey();

	/*
    for(int y=reticle_y-2;y<=reticle_y+6;y++)
    {
        for(int x=reticle_x-2;x<=reticle_x+7;x++)
        {
        Vec4b color = img.at<Vec4b>(Point(x,y));
        if((color[0] > 250 && color[1] > 250 && color[2] > 250) && (x==reticle_x-2 || x==reticle_x+7 || y==reticle_y-2 || y==reticle_y+6))
			{
			Vec4b color_opposite = img.at<Vec4b>(Point(2*reticle_x+5-x,2*reticle_y+4-y));   //at the opposite.
			if (color_opposite[0] > 250 && color_opposite[1] > 250 && color_opposite[2] > 250)
				{
					target_contour_detected = 1;
				    break;
				}
			}
		}
	}	
	*/

	//Crosshair is at row 54, col 96.

	/*
	int vector_x=96-col_CenterGravity;
	int vector_y=54-row_CenterGravity;
	int vector_norm_square=vector_x*vector_x+vector_y*vector_y;
	if (vector_norm_square == 0) {vector_x=1; vector_y=1;vector_norm_square=2;} //to be safe
	int vector_norm=SquareRootInt[vector_norm_square];
	int vector_x_norm_projection=int(vector_x/vector_norm);
	int vector_y_norm_projection=int(vector_y/vector_norm);

	for(int vector_extend=5; vector_extend<0*SquareRootInt[distance_square_average]; vector_extend++)
	{
	int vector_extend_x=96+vector_extend*vector_x_norm_projection;
	int vector_extend_y=54+vector_extend*vector_y_norm_projection;
	if (vector_extend_x<=0 || vector_extend_x>=img.cols-1 || vector_extend_y<=0 || vector_extend_y>=img.rows-1){break;}
	Vec4b color = img.at<Vec4b>(Point(vector_extend_x,vector_extend_y));
	if((color[0] > 250 && color[1] > 250 && color[2] > 250))
	{target_inside_detected=1;break;}
	}
	*/
    //int extend_left=0;
	//int extend_right=0;
	//int extend_up=0;
	//int extend_down=0;
	//int extend_left_plane[40]={0};
	//int extend_right_plane[40]={0};
	int extend_length = 30;
	int turn_extend_detected[4] = {0};
	int turn_distance[4] = {0};

	/*
	for(int x=1; x<extend_length; x++)
	{
	Vec4b color = img.at<Vec4b>(Point(reticle_x-2-x, reticle_y));
	if((color[0] > 250 && color[1] > 250 && color[2] > 250)) 
		{
			extend_left=1;
			break;
		}
	}

	for(int x=1; x<extend_length; x++)
	{
	Vec4b color = img.at<Vec4b>(Point(reticle_x+7+x, reticle_y));
	if((color[0] > 250 && color[1] > 250 && color[2] > 250)) 
		{
			extend_right=1;
			break;
		}
	}

	for(int y=1; y<extend_length; y++)
	{
	Vec4b color = img.at<Vec4b>(Point(reticle_x, reticle_y-2-y));
	if((color[0] > 250 && color[1] > 250 && color[2] > 250)) 
		{
			extend_up=1;
			break;
		}
	}

	for(int y=1.0; y<extend_length; y++)
	{
	Vec4b color = img.at<Vec4b>(Point(reticle_x, reticle_y+6+y));
	if((color[0] > 250 && color[1] > 250 && color[2] > 250)) 
		{
			extend_down=1;
			break;
		}
	}
	*/

	//**********inside detection********
	/*
	int number_of_turn=0;

	for(float theta=10.0; theta<85.0; theta+=10.0)
	{
		float x_norm_projection=cos(theta/57.3);
		float y_norm_projection=sin(theta/57.3);

		float x_norm_turn_90_projection=-y_norm_projection;
		float y_norm_turn_90_projection=x_norm_projection;

		for(int vec=1; vec<extend_length; vec++)
	    {
	        Vec4b color = img.at<Vec4b>(Point(reticle_x+7+int(vec*x_norm_projection), reticle_y+6+int(vec*y_norm_projection)));
			if((color[0] > 250 && color[1] > 250 && color[2] > 250)) 
			{turn_extend_detected[0]=1;turn_distance[0]=vec;break;}
		}



		for(int vec=1; vec<extend_length; vec++)
	    {
	        Vec4b color = img.at<Vec4b>(Point(reticle_x-2-int(vec*x_norm_projection), reticle_y-2-int(vec*y_norm_projection)));
			if((color[0] > 250 && color[1] > 250 && color[2] > 250)) 
			{turn_extend_detected[1]=1;turn_distance[1]=vec;break;}
		}



		for(int vec=1; vec<extend_length; vec++)
	    {
	        Vec4b color = img.at<Vec4b>(Point(reticle_x+7+int(vec*x_norm_turn_90_projection), reticle_y-2+int(vec*y_norm_turn_90_projection)));
			if((color[0] > 250 && color[1] > 250 && color[2] > 250)) 
			{turn_extend_detected[2]=1;turn_distance[2]=vec;break;}
		}

		if(turn_extend_detected[0]+turn_extend_detected[1]+turn_extend_detected[2] >= 2)
		{
		for(int vec=1; vec<extend_length; vec++)
	    {
	        Vec4b color = img.at<Vec4b>(Point(reticle_x-2-int(vec*x_norm_turn_90_projection), reticle_y+6-int(vec*y_norm_turn_90_projection)));
			if((color[0] > 250 && color[1] > 250 && color[2] > 250)) 
			{turn_extend_detected[3]=1;turn_distance[3]=vec;break;}
		}
		}
		
		number_of_turn++;

		if(turn_extend_detected[0]+turn_extend_detected[1]+turn_extend_detected[2]+turn_extend_detected[3] >= 3)
		{
			if ( (abs(turn_distance[0]-turn_distance[1])<10 && abs(turn_distance[2]-turn_distance[3])<10) || ( (abs(turn_distance[0]-turn_distance[1])>abs(turn_distance[2]-turn_distance[3])/3) && (abs(turn_distance[0]-turn_distance[1])<abs(turn_distance[2]-turn_distance[3])*3) )  )
			target_inside_turn_detected=1;
			break;
		}

	}
	*/
	//**********inside detection finished********

	/*
	for(int number_of_turn=0; number_of_turn<40; number_of_turn++)
	{
		if(extend_left_plane[number_of_turn]==1 && extend_right_plane[number_of_turn]==1){target_inside_turn_detected++;}
	}
	*/

	//if ((extend_left==1 && extend_right==1) && (extend_up==1 && extend_down==1)) {target_inside_detected=1;}
	//target_inside_detected=extend_left+extend_right+extend_up+extend_down;

	//imshow("point", img);
	//waitKey(1);

	//((white_count<(img.rows*img.cols)/10))
	
	//**************mouse click*************
	/*
	if (( target_inside_turn_detected == 1) && (GetKeyState(0x14)==1))  //0x14 is capslock target_inside_detected >= 2 && 
	{
	//Sleep (5);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	Sleep(1);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	//circle(img, Point(col_CenterGravity, row_CenterGravity), sqrt(distance_square_average), Scalar(255,129,0), 2, 8);
	//imshow("point", img);
	}
	
	target_contour_detected = 0;
	target_inside_detected = 0;
	target_inside_turn_detected = 0;
	*/
	//***********mouse click finished************
	if ( (GetKeyState(VK_LBUTTON) & 0x100) != 0 && abs(col_CenterGravity-reticle_x) < (img_captured.cols/2-10) )
	{
		IsWideSearch = 0;
	}
	else
	{
		IsWideSearch = 1;
	}

	if ((GetKeyState(VK_LBUTTON) & 0x100) != 0 && (GetKeyState(0x14)==1) && (white_count>3)) //0x14 is capslock  GetKeyState(VK_LBUTTON) &&   
	{
		float gain = 0.13; // for aiming sensitivity
		float gain_d = 0.05;
		//if(int((totalTime-(curr_time - prev_time) * secpercount)/0.02)<int(totalTime/0.02)) //Image display frame rate setting. Could be commented out
		if (true)
		{
			//mouse_event(MOUSEEVENTF_MOVE,int(gain*(col_CenterGravity-reticle_x))+int(gain_d*(CG_col_prev[0]-CG_col_prev[3])),int(gain*(row_CenterGravity-reticle_y)+int(gain_d*(CG_row_prev[0]-CG_row_prev[3]))),0,0);
			mouse_event(MOUSEEVENTF_MOVE,int(gain*(col_CenterGravity-reticle_x))+2*SignInt(col_CenterGravity-reticle_x),int(gain*(row_CenterGravity-reticle_y)+2*SignInt(row_CenterGravity-reticle_y)),0,0);
		}
	}
	
	//Sleep (1);
}

void CaptureScreen_DOI_1(char *filename) // For test only.
{

	int x, y, bx, by, start_bx, start_by, end_bx, end_by, MAX_WIDTH, MAX_HEIGHT;
	COLORREF pixel;
	HDC hdc;

	int R, G, B;

	MAX_WIDTH = ROI_width;
	MAX_HEIGHT = ROI_height;

	hdc = GetDC(HWND_DESKTOP);
	bx = GetSystemMetrics(SM_CXSCREEN);
	by = GetSystemMetrics(SM_CYSCREEN);
	start_bx = (bx/2) - (MAX_WIDTH/2);
	start_by = (by/2) - (MAX_HEIGHT/2);
	end_bx = (bx/2) + (MAX_WIDTH/2);
	end_by = (by/2) + (MAX_HEIGHT/2);

	for(y = start_by; y < end_by; y++)
	{   
		for(x = start_bx; x < end_bx; x++)
		{
			pixel = GetPixel(hdc, x, y);
			//if(pixel==RGB(255, 0, 0))
			{   
				R=GetRValue(pixel);
				G=GetGValue(pixel);
				B=GetBValue(pixel);

				RGB2MatData(ImageTest, y-start_by, x-start_bx, R, G, B);
				//imwrite(_T(filename),ImageTest);
				/*
				SetCursorPos(x,y);
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				Sleep(50);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(25);
				*/
			}
		}
	}
}




void CaptureScreen_DOI_2(char *filename) // For test only.
{
	HDC hdc, hdcTemp;
    RECT rect;
    BYTE* bitPointer;
    int x, y;
    int red, green, blue, alpha;
	
	
	hdc = GetDC(HWND_DESKTOP);
    GetWindowRect(HWND_DESKTOP, &rect);
    int MAX_WIDTH = rect.right;
    int MAX_HEIGHT = rect.bottom;

    hdcTemp = CreateCompatibleDC(hdc);
    BITMAPINFO bitmap;
    bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
    bitmap.bmiHeader.biWidth = MAX_WIDTH;
    bitmap.bmiHeader.biHeight = MAX_HEIGHT;
    bitmap.bmiHeader.biPlanes = 1;
    bitmap.bmiHeader.biBitCount = 32;
    bitmap.bmiHeader.biCompression = BI_RGB;
    bitmap.bmiHeader.biSizeImage = MAX_WIDTH * 4 * MAX_HEIGHT;
    bitmap.bmiHeader.biClrUsed = 0;
    bitmap.bmiHeader.biClrImportant = 0;
    HBITMAP hBitmap2 = CreateDIBSection(hdcTemp, &bitmap, DIB_RGB_COLORS, (void**)(&bitPointer), NULL, NULL);
    SelectObject(hdcTemp, hBitmap2);
    BitBlt(hdcTemp, 0, 0, MAX_WIDTH, MAX_HEIGHT, hdc, 0, 0, SRCCOPY);

	int x_begin, x_end, y_begin, y_end, height_desired, width_desired;
	
	height_desired=216;
	width_desired=384;
	
	x_begin=MAX_WIDTH/2-width_desired/2;
	y_begin=MAX_HEIGHT/2-height_desired/2;

	x_end=MAX_WIDTH/2+width_desired/2;
	y_end=MAX_HEIGHT/2+height_desired/2;
	//int 

    //for (int i=0; i<(MAX_WIDTH * 4 * MAX_HEIGHT); i+=4)
    for (y=y_begin; y<y_end; y+=1)	
	{
		for (x=x_begin; x<x_end; x+=1)
		{
        int i=(y*MAX_WIDTH+x)*4;
		red = (int)bitPointer[i];
        green = (int)bitPointer[i+1];
        blue = (int)bitPointer[i+2];
        alpha = (int)bitPointer[i+3];

        //x = i / (4 * MAX_HEIGHT);
        //y = i / (4 * MAX_WIDTH);

        if (red == 255 && green == 0 && blue == 0)
        {
            //SetCursorPos(x,y);
            //mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            //Sleep(50);
            //mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            //Sleep(25);
        }
		
		}
    }
}