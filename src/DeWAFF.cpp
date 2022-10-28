/*
 * bfilterDeceived.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: davidp
 */

#include "DeWAFF.hpp"

Mat deWAFF::filter(const Mat& originalImage, const Mat& USMImage, int w, double sigma_d, int sigma_r){
    //Pre-compute Gaussian domain weights.
    Mat_<float> X,Y,X2,Y2, XY2, G;
    Tools::meshGrid(Range(-w,w),Range(-w,w),X,Y);
    pow(X-Y,2,XY2); // (X-Y)^2
    double std2 = pow(sigma_d,2);
    XY2.convertTo(G,CV_32F,-1/(2*std2)); // -(1/2*std)(X-Y)^2
    exp(G,G); // e^(-(1/2*std)(X-Y)^2)

    //Apply bilateral filter.
    Mat B = Mat(originalImage.size(),originalImage.type());
    Mat F, H, I, L, dL, da, db;
    int iMin, iMax, jMin, jMax;
    std::vector<Mat> channels(3);
	Vec<float,3> pixel;
	double normF;

	#pragma omp target // OpenMP 4 pragma. Supported in GCC 5
	#pragma omp parallel for\
			private(I,iMin,iMax,jMin,jMax,pixel,channels,dL,da,db,H,F,normF,L)\
			shared(originalImage,B,G,USMImage,w,sigma_d,sigma_r)
    for(int i = 0; i < originalImage.rows; i++){
       for(int j = 0; j < originalImage.cols; j++){
             // Extract local region.
             iMin = max(i - w,0);
             iMax = min(i + w,originalImage.rows-1);
             jMin = max(j - w,0);
             jMax = min(j + w,originalImage.cols-1);

             // Compute Gaussian range weights in the three channels
             I = originalImage(Range(iMin,iMax), Range(jMin,jMax));
             split(I,channels);
             pixel = originalImage.at<Vec3f>(i,j);
             pow(channels[0] - pixel.val[0], 2, dL);
             pow(channels[1] - pixel.val[1], 2, da);
             pow(channels[2] - pixel.val[2], 2, db);
             exp((dL + da + db) / (-2 * std2),H);

             //Calculate bilateral filter response.
             F = H.mul(G(Range(iMin-i+w, iMax-i+w), Range(jMin-j+w, jMax-j+w)));
             normF = sum(F).val[0];

             //The laplacian deceive consists on weighting the gaussian function with
             //the original image, and using the image values of the laplacian image.
             L = USMImage(Range(iMin,iMax), Range(jMin,jMax));
             split(L,channels);
             B.at<Vec3f>(i,j)[0] = (sum(sum(F.mul(channels[0])))/normF).val[0];
             B.at<Vec3f>(i,j)[2] = (sum(sum(F.mul(channels[2])))/normF).val[0];
             B.at<Vec3f>(i,j)[1] = (sum(sum(F.mul(channels[1])))/normF).val[0];
       }
    }

    return B;
}
