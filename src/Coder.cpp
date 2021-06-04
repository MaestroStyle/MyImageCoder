#include "Coder.h"
#include <iostream>

Coder::Coder(int block_size, int comp_diff_max, int diff_max){
    _block_side_size = block_size;
    _comp_diff_max = comp_diff_max;
    _diff_max = diff_max;
}
void Coder::setBlockSize(int size){
    _block_side_size = size;
}
int Coder::getBlockSize(){
    return _block_side_size;
}
void Coder::setCompDiffMax(int comp_diff_max){
    _comp_diff_max = comp_diff_max;
}
int Coder::getCompDiffMax(){
    return _comp_diff_max;
}
void Coder::setDiffMax(int diff_max){
    _diff_max = diff_max;
}
int Coder::getDiffMax(){
    return _diff_max;
}
string Coder::encode(const Mat& img){
    cout << "encode" << endl;
    stringstream buf;

    _writeInt(buf, img.rows);
    _writeInt(buf, img.cols);

//    int width = cols % _block_side_size;
    Mat copy_img = img.clone();
    for(int y = 0; y <= img.rows - _block_side_size; y += _block_side_size)
        for(int x = 0; x <= img.cols - _block_side_size; x += _block_side_size){
            Mat block = img(Range(y, y + _block_side_size), Range(x, x + _block_side_size));
            if(_isContinuousTone(block)){
                _writeBlockType(buf, BlockType::SingleTone);

#ifdef JPEG
                string data = _jpegEncode(block);
                int length_block = static_cast<int>(data.length());
                _writeInt(buf, length_block);
                buf << data;
#else
                Scalar color = _calcMeanPixValue(block);
                _writeColor(buf, color);
#endif

                copy_img(Range(y, y + _block_side_size), Range(x, x + _block_side_size)) = Scalar(0, 0, 255);
            }
            else {
#ifdef DEFLATE
                _writeBlockType(buf, BlockType::Rle);
                string data = _deflateEncode(block);
                int length_block = static_cast<int>(data.length());
                _writeInt(buf, length_block);
                buf << data;
#else
                _writeBlockType(buf, BlockType::Rle);
                string data = _rleEncode(block);
                int length_block = static_cast<int>(data.length());
                _writeInt(buf, length_block);
                buf << data;
#endif
//                if(img.cols /_block_side_size != 0){

//                    for(int y = 0; y < )
//                }
            }
        }
#ifdef RESIZE
    resize(copy_img, copy_img,Size(200, 200), 0, 0, INTER_AREA);
#endif
    imshow("continuous blocks", copy_img);
    waitKey();
    destroyAllWindows();
    return buf.str();
}
Mat Coder::decode(const string& comp_data){
    stringstream buf(comp_data);
    int rows = _readInt(buf);
    int cols = _readInt(buf);

    Mat img(rows, cols, CV_8UC3);
    for(int y = 0; y <= img.rows - _block_side_size; y += _block_side_size)
        for(int x = 0; x <= img.cols - _block_side_size; x += _block_side_size){
            Mat block = img(Range(y, y + _block_side_size), Range(x, x + _block_side_size));

            BlockType type = _readBlockType(buf);
            if(type == BlockType::SingleTone){
#ifdef JPEG
                int length_block = _readInt(buf);
                string data = _readData(buf, length_block);
                _jpegDecode(data, block);
#else
                block = _readColor(buf);
#endif
            }
            else {
#ifdef DEFLATE
                int length_block = _readInt(buf);
                string data = _readData(buf, length_block);
                _deflateDecode(data, block);
#else
                int length_block = _readInt(buf);
                string data = _readData(buf, length_block);
                _rleDecode(data, block);
#endif

            }
        }
    return img;
}
Scalar Coder::_calcMeanPixValue(const Mat& block){
    int r_sum = 0;
    int g_sum = 0;
    int b_sum = 0;
    int channels = block.channels();
    for(int y = 0; y < block.rows; y++)
        for(int x = 0; x < block.cols; x++){
            r_sum += block.at<uchar>(y, x * channels);
            g_sum += block.at<uchar>(y, x * channels + 1);
            b_sum += block.at<uchar>(y, x * channels + 2);
        }
    int total_num_pixels = block.rows * block.cols;
    uchar r_mean = r_sum / total_num_pixels;
    uchar g_mean = g_sum / total_num_pixels;
    uchar b_mean = b_sum / total_num_pixels;

    return Scalar(r_mean, g_mean, b_mean);
}
bool Coder::_isContinuousTone(const Mat &block){
    int channels = block.channels();
    for(int y = 0; y < block.rows - 1; y++)
        for(int x = 0; x < block.cols - 1; x++){
            uchar r00 = block.at<uchar>(y, x * channels);
            uchar g00 = block.at<uchar>(y, x * channels + 1);
            uchar b00 = block.at<uchar>(y, x * channels + 2);

            uchar r01 = block.at<uchar>(y, (x + 1) * channels);
            uchar g01 = block.at<uchar>(y, (x + 1) * channels + 1);
            uchar b01 = block.at<uchar>(y, (x + 1) * channels + 2);

            uchar r10 = block.at<uchar>(y + 1, x * channels);
            uchar g10 = block.at<uchar>(y + 1, x * channels + 1);
            uchar b10 = block.at<uchar>(y + 1, x * channels + 2);

            if(!(_isContinuousPixel(r00, g00, b00, r01, g01, b01) && _isContinuousPixel(r00, g00, b00, r10, g10, b10)))
                return false;
        }
    return true;
}
bool Coder::_isContinuousPixel(uchar r1, uchar g1, uchar b1, uchar r2, uchar g2, uchar b2){
    if(abs(r1 - r2) < _comp_diff_max &&
            abs(g1 - g2) < _comp_diff_max &&
            abs(b1 - b2) < _comp_diff_max &&
            abs(r1 + g1 + b1 - r2 - g2 - b2) < _diff_max)
        return true;
    return false;
}
void Coder::_writeInt(stringstream& buf, int num){
    char _num[4];
    int dec = 255;
    _num[0] = (num >> 24 ) & dec;
    _num[1] = (num >> 16) & dec;
    _num[2] = (num >> 8) & dec;
    _num[3] = num & dec;
    buf.write(_num, 4);
}
int Coder::_readInt(stringstream& buf){
    char block_data[4];
    buf.read(block_data, 4);
    int num = static_cast<uchar>(block_data[0]);
    num <<= 8;
    num |= static_cast<uchar>(block_data[1]);
    num <<= 8;
    num |= static_cast<uchar>(block_data[2]);
    num <<= 8;
    num |= static_cast<uchar>(block_data[3]);
    return num;
}
void Coder::_writeBlockType(stringstream& buf, const BlockType& type){
    char _type[1];
    _type[0] = static_cast<char>(type);
    buf.write(_type, 1);
}
BlockType Coder::_readBlockType(stringstream& buf){
    char type[1];
    buf.read(type, 1);
    return static_cast<BlockType>(type[0]);
}
void Coder::_writeColor(stringstream& buf, const Scalar& color){
    char _color[3];
    _color[0] = static_cast<char>(color[0]);
    _color[1] = static_cast<char>(color[1]);
    _color[2] = static_cast<char>(color[2]);
    buf.write(_color, 3);
}
Scalar Coder::_readColor(stringstream& buf){
    char color[3];
    buf.read(color, 3);

    return Scalar(static_cast<uchar>(color[0]), static_cast<uchar>(color[1]), static_cast<uchar>(color[2]));
}
string Coder::_rleEncode(const Mat& block){
    string data;

    auto it_cur = block.begin<Vec<uchar, 3>>();
    auto it_prev = it_cur++;
    auto it_end = block.end<Vec<uchar,3>>();

    data.push_back((*it_prev)[0]);
    data.push_back((*it_prev)[1]);
    data.push_back((*it_prev)[2]);
    uchar i = 1;
    for(; it_cur != it_end; ++it_prev, ++it_cur, ++i)
        if(*it_prev != *it_cur || i >= 255){
            data.push_back(i);
            data.push_back((*it_cur)[0]);
            data.push_back((*it_cur)[1]);
            data.push_back((*it_cur)[2]);
            i = 0;
        }
    data.push_back(i);
    return data;
}
void Coder::_rleDecode(const string& data, Mat& block){
    auto it_cur = block.begin<Vec<uchar, 3>>();
    auto it_end = block.end<Vec<uchar, 3>>();

    for(int i = 0; i < data.length() && it_cur != it_end; i += 4){
        Vec<uchar, 3> pix(data[i], data[i + 1], data[i + 2]);
        uchar pix_count = data[i + 3];
        for(int y = 0; y < pix_count && it_cur != it_end; ++it_cur, ++y)
            *it_cur = pix;
    }
}
string Coder::_readData(stringstream& buf, int length){
    char * _data = new char[length];
    buf.read(_data, length);
    string data(_data, length);
    delete[] _data;
    _data = nullptr;
    return data;
}
void Coder::_writeBlock(stringstream& buf, const Mat& block){
    auto it_dst = block.begin<Vec<uchar, 3>>();
    auto it_dst_end = block.end<Vec<uchar,3>>();
    for(; it_dst != it_dst_end; ++it_dst){
        Vec<uchar, 3> pix = *it_dst;
        char _pix[3];
        _pix[0] = pix[0];
        _pix[1] = pix[1];
        _pix[2] = pix[2];
        buf.write(_pix, 3);
    }
}
void Coder::_readBlock(stringstream& buf, Mat& block){
    auto it_dst = block.begin<Vec<uchar, 3>>();
    auto it_dst_end = block.end<Vec<uchar,3>>();
    for(; it_dst != it_dst_end; ++it_dst){
        char pix[3];
        buf.read(pix, 3);
        *it_dst = Vec<uchar, 3>(pix[0], pix[1], pix[2]);
    }
}
string Coder::_jpegEncode(const Mat& block){
    vector<uchar> buf;
//    vector<int> params;
//    params.push_back(cv::IMWRITE_JPEG_QUALITY );
//    params.push_back(30);

    imencode(".jpeg", block, buf);//, params);

    string data((const char*)buf.data(), buf.size());
    return data;
}
void Coder::_jpegDecode(const string& data, Mat& block){
    std::vector<uchar> buf(data.begin(), data.end());
    cv::Mat decode_block = cv::imdecode(buf, cv::IMREAD_COLOR);

    auto it_dst = block.begin<Vec<uchar, 3>>();
    auto it_dst_end = block.end<Vec<uchar,3>>();
    auto it_src = decode_block.begin<Vec<uchar, 3>>();
    auto it_src_end = decode_block.end<Vec<uchar, 3>>();
    for(; it_dst != it_dst_end || it_src != it_src_end; ++it_dst, ++it_src){
        *it_dst = *it_src;
    }
}
string Coder::_deflateEncode(const Mat& block){
    vector<uchar> buf;
//    vector<int> params;
//    params.push_back(cv::IMWRITE_JPEG_QUALITY );
//    params.push_back(30);

    imencode(".png", block, buf);//, params);

    string data((const char*)buf.data(), buf.size());
    return data;
}
void Coder::_deflateDecode(const string& data, Mat& block){
    std::vector<uchar> buf(data.begin(), data.end());
    cv::Mat decode_block = cv::imdecode(buf, cv::IMREAD_COLOR);

    auto it_dst = block.begin<Vec<uchar, 3>>();
    auto it_dst_end = block.end<Vec<uchar,3>>();
    auto it_src = decode_block.begin<Vec<uchar, 3>>();
    auto it_src_end = decode_block.end<Vec<uchar, 3>>();
    for(; it_dst != it_dst_end || it_src != it_src_end; ++it_dst, ++it_src){
        *it_dst = *it_src;
    }
}
