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
int Coder::getBlockSize() const{
    return _block_side_size;
}
void Coder::setCompDiffMax(int comp_diff_max){
    _comp_diff_max = comp_diff_max;
}
int Coder::getCompDiffMax() const {
    return _comp_diff_max;
}
void Coder::setDiffMax(int diff_max){
    _diff_max = diff_max;
}
int Coder::getDiffMax() const {
    return _diff_max;
}
string Coder::encode(const Mat& img) const {
    cout << "encode" << endl;
    stringstream buf;

    _writeInt(buf, img.rows);
    _writeInt(buf, img.cols);

    int width_corner_block = img.cols % _block_side_size;
    int height_corner_block = img.rows % _block_side_size;
#ifdef DEBUG
    Mat copy_img = img.clone();
#endif
    for(int y = 0; y <= img.rows - _block_side_size; y += _block_side_size){
        for(int x = 0; x <= img.cols - _block_side_size; x += _block_side_size){
            Mat block = img(Range(y, y + _block_side_size), Range(x, x + _block_side_size));
            _encodeBlock(buf, block);
        }
        if(width_corner_block != 0){
            Mat block = img(Range(y, y + _block_side_size), Range(img.cols - width_corner_block, img.cols));
            _encodeBlock(buf, block);
        }
    }
    if(height_corner_block != 0){
        for(int x = 0; x <= img.cols - _block_side_size; x += _block_side_size){
            Mat block = img(Range(img.rows - height_corner_block, img.rows), Range(x, x + _block_side_size));
            _encodeBlock(buf, block);
        }
        if(width_corner_block != 0){
            Mat block = img(Range(img.rows - height_corner_block, img.rows), Range(img.cols - width_corner_block, img.cols));
            _encodeBlock(buf, block);
        }
    }
#ifdef DEBUG
#ifdef RESIZE
    resize(copy_img, copy_img,Size(200, 200), 0, 0, INTER_AREA);
#endif
    imshow("continuous blocks", copy_img);
    waitKey();
    destroyAllWindows();
#endif
    return buf.str();
}
Mat Coder::decode(const string& comp_data) const {
    stringstream buf(comp_data);
    int rows = _readInt(buf);
    int cols = _readInt(buf);

    Mat img(rows, cols, CV_8UC3);
    int width_corner_block = img.cols % _block_side_size;
    int height_corner_block = img.rows % _block_side_size;
    for(int y = 0; y <= img.rows - _block_side_size; y += _block_side_size){
        for(int x = 0; x <= img.cols - _block_side_size; x += _block_side_size){
            Mat block = img(Range(y, y + _block_side_size), Range(x, x + _block_side_size));
            _decodeBlock(buf, block);
        }
        if(width_corner_block != 0){
            Mat block = img(Range(y, y + _block_side_size), Range(img.cols - width_corner_block, img.cols));
            _decodeBlock(buf, block);
        }
    }
    if(height_corner_block != 0){
        for(int x = 0; x <= img.cols - _block_side_size; x += _block_side_size){
            Mat block = img(Range(img.rows - height_corner_block, img.rows), Range(x, x + _block_side_size));
            _decodeBlock(buf, block);
        }
        if(width_corner_block != 0){
            Mat block = img(Range(img.rows - height_corner_block, img.rows), Range(img.cols - width_corner_block, img.cols));
            _decodeBlock(buf, block);
        }
    }
    return img;
}
void Coder::_encodeBlock(stringstream &buf, const Mat &block) const{
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
#ifdef DEBUG
//        copy_img(Range(y, y + _block_side_size), Range(x, x + _block_side_size)) = Scalar(0, 0, 255);
#endif
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
    }
}
void Coder::_decodeBlock(stringstream &buf, Mat &block) const{
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
Scalar Coder::_calcMeanPixValue(const Mat& block) const {
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
bool Coder::_isContinuousTone(const Mat &block) const {
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
bool Coder::_isContinuousPixel(uchar r1, uchar g1, uchar b1, uchar r2, uchar g2, uchar b2) const {
    if(abs(r1 - r2) < _comp_diff_max &&
            abs(g1 - g2) < _comp_diff_max &&
            abs(b1 - b2) < _comp_diff_max &&
            abs(r1 + g1 + b1 - r2 - g2 - b2) < _diff_max)
        return true;
    return false;
}
void Coder::_writeInt(stringstream& buf, int num) const {
    char _num[4];
    int dec = 255;
    _num[0] = (num >> 24 ) & dec;
    _num[1] = (num >> 16) & dec;
    _num[2] = (num >> 8) & dec;
    _num[3] = num & dec;
    buf.write(_num, 4);
}
int Coder::_readInt(stringstream& buf) const {
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
void Coder::_writeBlockType(stringstream& buf, const BlockType& type) const {
    char _type[1];
    _type[0] = static_cast<char>(type);
    buf.write(_type, 1);
}
BlockType Coder::_readBlockType(stringstream& buf) const {
    char type[1];
    buf.read(type, 1);
    return static_cast<BlockType>(type[0]);
}
void Coder::_writeColor(stringstream& buf, const Scalar& color) const {
    char _color[3];
    _color[0] = static_cast<char>(color[0]);
    _color[1] = static_cast<char>(color[1]);
    _color[2] = static_cast<char>(color[2]);
    buf.write(_color, 3);
}
Scalar Coder::_readColor(stringstream& buf) const {
    char color[3];
    buf.read(color, 3);

    return Scalar(static_cast<uchar>(color[0]), static_cast<uchar>(color[1]), static_cast<uchar>(color[2]));
}
string Coder::_rleEncode(const Mat& block) const {
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
void Coder::_rleDecode(const string& data, Mat& block) const {
    auto it_cur = block.begin<Vec<uchar, 3>>();
    auto it_end = block.end<Vec<uchar, 3>>();

    for(int i = 0; i < data.length() && it_cur != it_end; i += 4){
        Vec<uchar, 3> pix(data[i], data[i + 1], data[i + 2]);
        uchar pix_count = data[i + 3];
        for(int y = 0; y < pix_count && it_cur != it_end; ++it_cur, ++y)
            *it_cur = pix;
    }
}
string Coder::_readData(stringstream& buf, int length) const {
    char * _data = new char[length];
    buf.read(_data, length);
    string data(_data, length);
    delete[] _data;
    _data = nullptr;
    return data;
}
void Coder::_writeBlock(stringstream& buf, const Mat& block) const {
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
void Coder::_readBlock(stringstream& buf, Mat& block) const {
    auto it_dst = block.begin<Vec<uchar, 3>>();
    auto it_dst_end = block.end<Vec<uchar,3>>();
    for(; it_dst != it_dst_end; ++it_dst){
        char pix[3];
        buf.read(pix, 3);
        *it_dst = Vec<uchar, 3>(pix[0], pix[1], pix[2]);
    }
}
string Coder::_jpegEncode(const Mat& block) const {
    vector<uchar> buf;
//    vector<int> params;
//    params.push_back(cv::IMWRITE_JPEG_QUALITY );
//    params.push_back(30);

    imencode(".jpeg", block, buf);//, params);

    string data((const char*)buf.data(), buf.size());
    return data;
}
void Coder::_jpegDecode(const string& data, Mat& block) const {
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
string Coder::_deflateEncode(const Mat& block) const {
    string raw_data;

    auto it_cur = block.begin<Vec<uchar, 3>>();
    auto it_end = block.end<Vec<uchar,3>>();
    for(; it_cur != it_end; ++it_cur){
            raw_data.push_back((*it_cur)[0]);
            raw_data.push_back((*it_cur)[1]);
            raw_data.push_back((*it_cur)[2]);
        }

    int size_block = static_cast<int>(block.total() * block.elemSize());
    uchar* comp_data = new uchar[size_block];
    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;

    defstream.avail_in = (uInt)size_block + 1; // size of input, string + terminator
    defstream.next_in = (Bytef *)raw_data.data();//block.data; // input char array
    defstream.avail_out = (uInt)size_block; // size of output
    defstream.next_out = (Bytef *)comp_data; // output char array

    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);
    string data((const char*)comp_data, strlen((const char*)comp_data));
    delete[] comp_data;
    comp_data = nullptr;
    return data;
}
void Coder::_deflateDecode(const string& data, Mat& block) const {
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;

    int size_block = static_cast<int>(block.total() * block.elemSize());
    uchar* decode_data = new uchar[size_block];
    infstream.avail_in = (uInt)data.length(); // size of input
    infstream.next_in = (Bytef *)data.data(); // input char array
    infstream.avail_out = (uInt)size_block; // size of output
    infstream.next_out = (Bytef *)decode_data; // output char array

    // the actual DE-compression work.
    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);

    auto it_dst = block.begin<Vec<uchar, 3>>();
    auto it_dst_end = block.end<Vec<uchar,3>>();
    for(int i = 0; it_dst != it_dst_end || i < size_block; ++it_dst, i += 3)
        *it_dst = Vec<uchar, 3>(decode_data[i], decode_data[i + 1], decode_data[i + 2]);

    delete[] decode_data;
    decode_data = nullptr;
}
