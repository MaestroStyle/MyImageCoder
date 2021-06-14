#include <iostream>
#include <fstream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include <Coder.h>

using namespace std;
using namespace cv;

bool imgRead(const string& filename, string& data);
bool imgRead(const string& filename, char* data, int64& size);
bool imgRead(const string& filename, Mat& img);
bool imgWrite(const string& filename, const char* data, int64 size);
bool imgWrite(const string& filename, const string& data);
bool imgWrite(const string& filename, const Mat& img);

int main()
{
    string fimg_name = "F:/Studies_TSU/DataCompression/data_compression/img.bmp";
    string fimg_name_res = "F:/Studies_TSU/DataCompression/data_compression/result/res_img.mybmp";
    string fimg_name_decode = "F:/Studies_TSU/DataCompression/data_compression/result/decode_img.bmp";

    Mat img;
    imgRead(fimg_name, img);
    if(img.type() != CV_8UC3){
        cout << "image isn't color!" << endl;
        return 0;
    }
    int64 img_size = img.total() * img.elemSize();
    cout << fimg_name << endl;
    cout << "image size = " << img_size << " bytes" << endl;

    Coder coder;
    string comp_data = coder.encode(img);
    cout << "compressed image size = " << comp_data.size() << " bytes" << endl;
    cout << "compression ratio = " <<
            static_cast<double>(img_size) / static_cast<double>(comp_data.size()) <<
            endl;
    imgWrite(fimg_name_res, comp_data);

    imgRead(fimg_name_res, comp_data);
    cout << fimg_name_res << endl;
    cout << "image size = " << comp_data.length() << " bytes";
    Mat decode_img = coder.decode(comp_data);
    imgWrite(fimg_name_decode, decode_img);
#ifdef RESIZE
    resize(decode_img, decode_img, Size(200, 200), 0, 0, INTER_AREA);
#endif
    imshow("compressed image", decode_img);
    waitKey();
    destroyAllWindows();

    cout << endl;
    return 0;
}

bool imgRead(const string& filename, string& data){
    ifstream file_img(filename, ios::in | ios::binary | ios::ate);
    if(!file_img.is_open()){
        cout << "file didn't open!" << endl;
        return false;
    }
    cout << "file opened!" << endl;

    data.clear();

    int64 size = file_img.tellg();
    char* _data = new char[size];
    file_img.seekg(0, ios::beg);
    file_img.read(_data, size);

    file_img.close();
    data.append(_data, size);
    delete[] _data;
    _data = nullptr;

    return true;
}

bool imgRead(const string& filename, char* data, int64& size){
    ifstream file_img(filename, ios::in | ios::binary | ios::ate);
    if(file_img.is_open()){
        size = file_img.tellg();
        data = new char[size];
        file_img.seekg(0, ios::beg);
        file_img.read(data, size);
        file_img.close();

        return true;
    }
    return false;
}
bool imgRead(const string& filename, Mat& img){
    img = imread(filename);
    return true;
}
bool imgWrite(const string& filename, const Mat& img){
    imwrite(filename, img);
    return true;
}
bool imgWrite(const string& filename, const char* data, int64 size){
    ofstream file_img(filename, ios::out | ios::binary);
    if(file_img.is_open()){
        file_img.write(data, size);
        file_img.close();
        return true;
    }
    return false;
}
bool imgWrite(const string& filename, const string& data){
    ofstream file_img(filename, ios::out | ios::binary);
    if(file_img.is_open()){
        file_img << data;
        file_img.close();
        return true;
    }
    return false;
}
