
#include "BitmapData.h"
#include "Platform.h"
#include "SkImageDecoder.h"

void BitmapData::load()
{
    std::string buf;
    Platform::readFile(path(), &buf);
    SkImageDecoder::DecodeMemory(buf.data(), buf.size(), &_bitmap,
        SkBitmap::kARGB_8888_Config, SkImageDecoder::kDecodePixels_Mode);
}

void BitmapData::unload()
{

}
