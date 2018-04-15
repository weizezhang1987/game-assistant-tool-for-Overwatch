#include "stdafx.h"
#include "opencv2\highgui\highgui.hpp"
#include "opencv.hpp"  
#include <iostream>
#include <opencv2/opencv.hpp>  

using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class template_matching_row_and_col_result
{
	public:
	int col;
	int row;
};

void MatchingMethod( Mat source_image, Mat template_image, Mat result ,template_matching_row_and_col_result &row_and_col_result )  //using CV_TM_SQDIFF_NORMED
{
  /// Source image to display
  Mat img_display;
  source_image.copyTo( img_display );

  /// Create the result matrix
  int result_cols =  source_image.cols - template_image.cols + 1;
  int result_rows = source_image.rows - template_image.rows + 1;

  result.create( result_rows, result_cols, CV_32FC1 );

  /// Do the Matching and Normalize
  matchTemplate( source_image, template_image, result, CV_TM_SQDIFF_NORMED );
  normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

  /// Localizing the best match with minMaxLoc
  double minVal; double maxVal; Point minLoc; Point maxLoc;
  Point matchLoc;

  minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

  /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
  /*
  if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
    { matchLoc = minLoc; }
  else
    { matchLoc = maxLoc; }
  */
  matchLoc = minLoc;

  row_and_col_result.col=matchLoc.x;
  row_and_col_result.row=matchLoc.y;

  /// Show me what you got
  //rectangle( img_display, matchLoc, Point( matchLoc.x + template_image.cols , matchLoc.y + template_image.rows ), Scalar::all(0), 2, 8, 0 );
  //rectangle( result, matchLoc, Point( matchLoc.x + template_image.cols , matchLoc.y + template_image.rows ), Scalar::all(0), 2, 8, 0 );

  //imshow( "image with pipette unidentified", img_display );
  //imshow( "image with pipette identified", result );

  return;
}