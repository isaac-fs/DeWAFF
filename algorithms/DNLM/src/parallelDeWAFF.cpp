/*
 * main.cpp
 *
 *  Modified on: Nov 07, 2016
 *      Authors: manzumbado, davidp
 */
#include "parallelDeWAFF.hpp"
#include <ctime>
using namespace std;
  

int main(int argc, char* argv[]){
    ParallelDeWAFF deWAFF;

    //Check input arguments
    if (argc != 5){
        cout << "ERROR: Not enough parameters" << endl;
        deWAFF.help();
        return -1;
    }else{

    //Open input video file
    const string inputFile = argv[1];
    double sigma_r = atof(argv[2]);
    double sigma_s = atof(argv[3]);
    double lambda = atof(argv[4]);
    // Find extension point
    string::size_type pAt = inputFile.find_last_of('.');

    // Form the new name with container
    const string outputFile = inputFile.substr(0, pAt) + "_DeNLM.jpg";

    //Create the Laplacian of Gaussian mask once
    NoAdaptiveLaplacian* nAL = deWAFF.getNAL();
    Mat h =  Tools::fspecialLoG(17, 0.005);
    nAL->setMask(-h);
    Mat U,F1;
    U = imread(inputFile, CV_LOAD_IMAGE_COLOR);
    //Read one frame from input video
        if(!U.data){
            cout << "Could not read image from file." << endl;
            return -1;
        }
        //time start
        //clock_t begin = clock();
        F1 = deWAFF.processImage(U, sigma_r, sigma_s, lambda);
        //clock_t end = clock();
        //time end

    //Write image to output file.
    imwrite(outputFile, F1);
    //double elapsed_secs =  ((double) (end - begin)) / CLOCKS_PER_SEC;
    //cout << "Time to process an image: "  << elapsed_secs << endl;
    return 0;
    }
}

NoAdaptiveLaplacian* ParallelDeWAFF::getNAL(){
    return &(this->nal);
}

void ParallelDeWAFF::help(){
    cout
        << "------------------------------------------------------------------------------" << endl
        << "Usage:"                                                                         << endl
        << "./ParallelDeNLM inputImageName"                                                << endl
        << "------------------------------------------------------------------------------" << endl
        << endl;
}

//used parameters for the paper CONCAPAN 2016
Mat ParallelDeWAFF::processImage(const Mat& U, double sigma_r, double sigma_s, double lambda){
    //Set parameters for processing
    int wRSize = 21;
    int wSize_n=1;
    //double sigma_s = wRSize/1.5;
    //int sigma_r = 3; //13
    //int lambda = 1.7;



    Mat fDeceivedNLM = filterDeceivedNLM(U, wRSize, wSize_n, sigma_s, sigma_r, lambda);

    return fDeceivedNLM;
}

//Input image must be from 0 to 255
Mat ParallelDeWAFF::filterDeceivedNLM(const Mat& U, int wSize, int wSize_n, double sigma_s, int sigma_r, int lambda){
    Mat Unorm;
    //The image has to have values from 0 to 1
    U.convertTo(Unorm,CV_32FC3,1.0/255.0);
    //[L, alfaMat, Vnorm] = adaptiveLaplacian(Unorm, amps, trap1, trap2);

    Mat L = this->nal.noAdaptiveLaplacian(Unorm, lambda);
    Mat F = this->nlmfd.nlmfilterDeceived(Unorm, L, wSize, wSize_n, sigma_s, sigma_r);

    //putting back everything
    F.convertTo(F,CV_8UC1,255);
    return F;
}
