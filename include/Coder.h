#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <sstream>

//#define RESIZE
#define JPEG
#define DEFLATE

using namespace std;
using namespace cv;

enum class BlockType : char {
    SingleTone,
    Rle,
    Edge,
    Bottom,
    Raw
};

class Coder{

public:
    Coder(int bloc_size = 16, int comp_diff_max = 5, int diff_max = 5);

//    vector<char> rleEncode();
    string encode(const Mat& img);
    Mat decode(const string& comp_data);

    void setBlockSize(int size);
    int getBlockSize();

    void setCompDiffMax(int comp_diff_max);
    int getCompDiffMax();

    void setDiffMax(int diff_max);
    int getDiffMax();

private:
    int _block_side_size = 0;
    int _comp_diff_max = 0;
    int _diff_max = 0;

    void _writeInt(stringstream& buf, int num);
    int _readInt(stringstream& buf);
    void _writeBlockType(stringstream& buf, const BlockType& type);
    BlockType _readBlockType(stringstream& buf);
    void _writeColor(stringstream& buf, const Scalar& color);
    Scalar _readColor(stringstream& buf);
    string _rleEncode(const Mat& block);
    void _rleDecode(const string& data, Mat& block);
    string _jpegEncode(const Mat& block);
    void _jpegDecode(const string& data, Mat& block);
    string _deflateEncode(const Mat& block);
    void _deflateDecode(const string& data, Mat& block);
    string _readData(stringstream& buf, int length);

    void _writeBlock(stringstream& buf, const Mat& block);
    void _readBlock(stringstream& buf, Mat& block);

    bool _isContinuousTone(const Mat& block);
    bool _isContinuousPixel(uchar r1, uchar g1, uchar b1, uchar r2, uchar g2, uchar b2);
    Scalar _calcMeanPixValue(const Mat& block);
};

