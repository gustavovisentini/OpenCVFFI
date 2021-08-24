//
//  OpenCVWrapper.m
//  Leaf Measure iOS
//
//  Created by Gustavo Visentini on 19/07/21.
//  Created by Gustavo Lidani on 23/08/21.
//

#ifdef __cplusplus
#undef NO
#undef YES
#include <opencv2/opencv.hpp>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include <opencv2/imgproc/imgproc_c.h>
#include "stdlib.h"

using namespace std;
using namespace cv;

double areaMoeda = 3.14;
int blurIndex = 1;

// Avoiding name mangling
extern "C"
{
    // Função para fazer log no console, basedo no tipo do dispositivo (android faz o log de uma forma diferente do iOS)
    void platform_log(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
#ifdef __ANDROID__
        __android_log_vprint(ANDROID_LOG_VERBOSE, "ndk", fmt, args);
#else
        vprintf(fmt, args);
#endif
        va_end(args);
    }

    __attribute__((visibility("default"))) __attribute__((used))
    const char *
    version()
    {
        return CV_VERSION;
    }

    __attribute__((visibility("default"))) __attribute__((used)) char *process_image(char *inputImagePath, char *outputImagePath)
    {
        platform_log("Running pre process...");

        double maiorFolha = -1000000;
        double menorFolha = 10000000;

        cv::Mat imagem_original, imagem_gray, imagem_threshold, img, img_hsv;
        cv::Point circlePoint;

        imagem_original = imread(inputImagePath, IMREAD_GRAYSCALE);

        img = imagem_original;

        //resize image for 0,6%
        // cv::resize(img, img, cv::Size(img.cols * 0.5, img.rows * 0.5), 0, 0, CV_INTER_LINEAR);
        cv::resize(img, img, cv::Size(img.cols * 0.5, img.rows * 0.5), 0, 0, INTER_LINEAR);

        //adjust brigthness and contrast and blur image
        cv::GaussianBlur(img, img, cv::Size(0, 0), blurIndex);

        // cv::cvtColor(img, img_hsv, COLOR_BGR2HSV);
        std::vector<cv::Mat> hsv;

        // cv::split(img_hsv, hsv);
        cv::split(img, hsv);

        cv::Mat img_thr;
        cv::threshold(hsv[1], img_thr, 64, 255, cv::THRESH_BINARY);

        std::vector<std::vector<cv::Point>> cnts;
        std::vector<cv::Vec4i> hier;
        cv::findContours(img_thr.clone(), cnts, hier, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

        // Iterate found contours
        std::vector<cv::Point> cnt_centers;
        std::vector<double> cnt_areas;

        double ref_area = -1;
        Mat aux = img.clone();

        vector<Vec3f> circles;

        HoughCircles(img_thr.clone(), circles, HOUGH_GRADIENT, 1,
                     img_thr.rows / 16, // change this value to detect circles with different distances to each other
                     100, 30, 0, 0      // change the last two parameters
                                        // (min_radius & max_radius) to detect larger circles
        );

        if (circles.size() == 0 || circles.size() > 1)
            return "\0";

        cout << "\n Circulos Detectados " << circles.size() << endl
             << endl;

        cv::Mat maskCir = hsv[0].clone().setTo(cv::Scalar(0));
        cv::Point center(cvRound(circles[0][0]), cvRound(circles[0][1])); //aqui o centro

        int radius = cvRound(circles[0][2]);
        cv::circle(maskCir, center, radius, cv::Scalar(255), cv::FILLED);

        cv::Point moeda;
        double distance = 9999999;

        //procurando a moeda e selecionando moedas e folhas
        for (int i = 0; i < cnts.size(); i++)
        {
            // Current contour
            std::vector<cv::Point> cnt = cnts[i];
            std::vector<cv::Point> cntAux;

            // If contour is too small, discard
            if (cnt.size() < 100)
                continue;

            //Calculate and store center (just for visualization) and area of contour
            cv::Moments m = cv::moments(cnt);
            cnt_centers.push_back(cv::Point(m.m10 / m.m00 - 30, m.m01 / m.m00));
            cv::Point instantPoint(m.m10 / m.m00 - 30, m.m01 / m.m00);
            cnt_areas.push_back(cv::contourArea(cnt));
            //std::cout<<"Tamanho do objeto: "<< cnt_areas[i] << "\n";

            // Check H channel, whether the contour's image parts are mostly green
            cv::Mat mask = hsv[0].clone().setTo(cv::Scalar(0));
            cv::drawContours(mask, cnts, i, cv::Scalar(255), cv::FILLED);
            //double h_mean = cv::mean(hsv[0], mask)[0];
            //cout<< "meam - " << h_mean << "\n";
            aux = mask;

            // cout << "Moeda: " << moeda << endl;
            // cout << "Centro: " << center << endl;

            double res = cv::norm(cv::Mat(instantPoint), cv::Mat(center));
            // cout << endl
            //      << res;

            if (res < distance)
            {
                distance = res;
                cout << "Entrou" << endl;
                moeda.x = instantPoint.x;
                moeda.y = instantPoint.y;
                ref_area = cv::contourArea(cnt);
                //cout<<endl<<moeda<<endl;
            }
        }

        // cout << "\n\n\nValores\nMoeda: " << moeda << "\nMoeda Circles " << center << "\nDistancia Selecionada: " << distance << endl;

        // Iterate all contours again
        double total_areaIAF = 0;
        for (int i = 0; i < cnt_centers.size(); i++)
        {
            // Calculate actual object area
            double area = cnt_areas[i] / ref_area * areaMoeda;
            // cout << "Area : " << area << " - Pos. " << i << " - " << cnt_centers[i] << endl;

            total_areaIAF += area;

            //selecionando as folhas
            int tam = cnt_areas[i] / ref_area * areaMoeda;
            if (tam != areaMoeda)
            {
                if (tam > maiorFolha)
                    maiorFolha = tam;
                if (tam < menorFolha)
                    menorFolha = tam;
            }

            // Put area on image w.r.t. the contour's center
            //cv::putText(img, std::to_string(area), cnt_centers[i], cv::QT_FONT_BLACK, 2, cv::Scalar(255));
        }

        total_areaIAF = total_areaIAF - areaMoeda;
        // cout << "\n\n Total Área: " << total_areaIAF << endl;
        cv::circle(img, center, radius, cv::Scalar(255, 0, 0), cv::FILLED);
        //cv::putText(img, std::to_string(total_areaIAF), cv::Point(30, 30), cv::QT_FONT_BLACK,2, cv::Scalar(255));

        unsigned long int qtdeFolhas = cnt_areas.size() - 1;

        // stringstream stream;
        // stream << total_areaIAF << "|" << qtdeFolhas << "|" << menorFolha << "|" << maiorFolha;

        return "Oi blz?";
    }
}

// imagem_original = imread(inputImagePath, IMREAD_GRAYSCALE);

// Mat threshed, withContours;

// vector<vector<Point>> contours;
// vector<Vec4i> hierarchy;

// adaptiveThreshold(input, threshed, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 77, 6);
// findContours(threshed, contours, hierarchy, RETR_TREE, CHAIN_APPROX_TC89_L1);

// cvtColor(threshed, withContours, COLOR_GRAY2BGR);
// drawContours(withContours, contours, -1, Scalar(0, 255, 0), 4);

// imwrite(outputImagePath, withContours);