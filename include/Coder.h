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
    string encode(const Mat& img) const;
    Mat decode(const string& comp_data) const;

    void setBlockSize(int size);
    int getBlockSize() const;

    void setCompDiffMax(int comp_diff_max);
    int getCompDiffMax() const;

    void setDiffMax(int diff_max);
    int getDiffMax() const;

private:
    int _block_side_size = 0;
    int _comp_diff_max = 0;
    int _diff_max = 0;

    void _encodeBlock(stringstream& buf, const Mat& block) const;
    void _decodeBlock(stringstream& buf, Mat& block) const;
    void _writeInt(stringstream& buf, int num) const;
    int _readInt(stringstream& buf) const;
    void _writeBlockType(stringstream& buf, const BlockType& type) const;
    BlockType _readBlockType(stringstream& buf) const;
    void _writeColor(stringstream& buf, const Scalar& color) const;
    Scalar _readColor(stringstream& buf) const;
    string _rleEncode(const Mat& block) const;
    void _rleDecode(const string& data, Mat& block) const;
    string _jpegEncode(const Mat& block) const;
    void _jpegDecode(const string& data, Mat& block) const;
    string _deflateEncode(const Mat& block) const;
    void _deflateDecode(const string& data, Mat& block) const;
    string _readData(stringstream& buf, int length) const;

    void _writeBlock(stringstream& buf, const Mat& block) const;
    void _readBlock(stringstream& buf, Mat& block) const;

    bool _isContinuousTone(const Mat& block) const;
    bool _isContinuousPixel(uchar r1, uchar g1, uchar b1, uchar r2, uchar g2, uchar b2) const;
    Scalar _calcMeanPixValue(const Mat& block) const;
};

