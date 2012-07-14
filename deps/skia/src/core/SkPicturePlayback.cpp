
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkTypeface.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"
#include <new>

/*  Define this to spew out a debug statement whenever we skip the remainder of
    a save/restore block because a clip... command returned false (empty).
 */
#define SPEW_CLIP_SKIPPINGx

SkPicturePlayback::SkPicturePlayback() {
    this->init();
}

SkPicturePlayback::SkPicturePlayback(const SkPictureRecord& record) {
#ifdef SK_DEBUG_SIZE
    size_t overallBytes, bitmapBytes, matricesBytes,
    paintBytes, pathBytes, pictureBytes, regionBytes;
    int bitmaps = record.bitmaps(&bitmapBytes);
    int matrices = record.matrices(&matricesBytes);
    int paints = record.paints(&paintBytes);
    int paths = record.paths(&pathBytes);
    int pictures = record.pictures(&pictureBytes);
    int regions = record.regions(&regionBytes);
    SkDebugf("picture record mem used %zd (stream %zd) ", record.size(),
             record.streamlen());
    if (bitmaps != 0)
        SkDebugf("bitmaps size %zd (bitmaps:%d) ", bitmapBytes, bitmaps);
    if (matrices != 0)
        SkDebugf("matrices size %zd (matrices:%d) ", matricesBytes, matrices);
    if (paints != 0)
        SkDebugf("paints size %zd (paints:%d) ", paintBytes, paints);
    if (paths != 0)
        SkDebugf("paths size %zd (paths:%d) ", pathBytes, paths);
    if (pictures != 0)
        SkDebugf("pictures size %zd (pictures:%d) ", pictureBytes, pictures);
    if (regions != 0)
        SkDebugf("regions size %zd (regions:%d) ", regionBytes, regions);
    if (record.fPointWrites != 0)
        SkDebugf("points size %zd (points:%d) ", record.fPointBytes, record.fPointWrites);
    if (record.fRectWrites != 0)
        SkDebugf("rects size %zd (rects:%d) ", record.fRectBytes, record.fRectWrites);
    if (record.fTextWrites != 0)
        SkDebugf("text size %zd (text strings:%d) ", record.fTextBytes, record.fTextWrites);

    SkDebugf("\n");
#endif
#ifdef SK_DEBUG_DUMP
    record.dumpMatrices();
    record.dumpPaints();
#endif

    record.validate();
    const SkWriter32& writer = record.writeStream();
    init();
    if (writer.size() == 0)
        return;

    {
        size_t size = writer.size();
        void* buffer = sk_malloc_throw(size);
        writer.flatten(buffer);
        fReader.setMemory(buffer, size);    // fReader owns buffer now
    }

    // copy over the refcnt dictionary to our reader
    //
    fRCPlayback.reset(&record.fRCSet);
    fTFPlayback.reset(&record.fTFSet);

    fBitmapCount = record.getBitmaps().unflattenDictionary(fBitmaps, &fRCPlayback);
    fMatrixCount = record.getMatrices().unflattenDictionary(fMatrices);
    fPaintCount = record.getPaints().unflattenDictionary(fPaints, &fRCPlayback, &fTFPlayback);
    fRegionCount = record.getRegions().unflattenDictionary(fRegions);

    SkASSERT(fBitmapCount == record.getBitmaps().count());
    SkASSERT(fMatrixCount == record.getMatrices().count());
    SkASSERT(fPaintCount == record.getPaints().count());
    SkASSERT(fRegionCount == record.getRegions().count());

    fPathHeap = record.fPathHeap;
    SkSafeRef(fPathHeap);

    const SkTDArray<SkPicture* >& pictures = record.getPictureRefs();
    fPictureCount = pictures.count();
    if (fPictureCount > 0) {
        fPictureRefs = SkNEW_ARRAY(SkPicture*, fPictureCount);
        for (int i = 0; i < fPictureCount; i++) {
            fPictureRefs[i] = pictures[i];
            fPictureRefs[i]->ref();
        }
    }

#ifdef SK_DEBUG_SIZE
    int overall = fPlayback->size(&overallBytes);
    bitmaps = fPlayback->bitmaps(&bitmapBytes);
    paints = fPlayback->paints(&paintBytes);
    paths = fPlayback->paths(&pathBytes);
    pictures = fPlayback->pictures(&pictureBytes);
    regions = fPlayback->regions(&regionBytes);
    SkDebugf("playback size %zd (objects:%d) ", overallBytes, overall);
    if (bitmaps != 0)
        SkDebugf("bitmaps size %zd (bitmaps:%d) ", bitmapBytes, bitmaps);
    if (paints != 0)
        SkDebugf("paints size %zd (paints:%d) ", paintBytes, paints);
    if (paths != 0)
        SkDebugf("paths size %zd (paths:%d) ", pathBytes, paths);
    if (pictures != 0)
        SkDebugf("pictures size %zd (pictures:%d) ", pictureBytes, pictures);
    if (regions != 0)
        SkDebugf("regions size %zd (regions:%d) ", regionBytes, regions);
    SkDebugf("\n");
#endif
}

SkPicturePlayback::SkPicturePlayback(const SkPicturePlayback& src) {
    this->init();

    // copy the data from fReader
    {
        size_t size = src.fReader.size();
        void* buffer = sk_malloc_throw(size);
        memcpy(buffer, src.fReader.base(), size);
        fReader.setMemory(buffer, size);
    }

    fBitmapCount = src.fBitmapCount;
    fBitmaps = SkNEW_ARRAY(SkBitmap, fBitmapCount);
    for (int i = 0; i < fBitmapCount; i++) {
        fBitmaps[i] = src.fBitmaps[i];
    }

    fMatrixCount = src.fMatrixCount;
    fMatrices = SkNEW_ARRAY(SkMatrix, fMatrixCount);
    memcpy(fMatrices, src.fMatrices, fMatrixCount * sizeof(SkMatrix));

    fPaintCount = src.fPaintCount;
    fPaints = SkNEW_ARRAY(SkPaint, fPaintCount);
    for (int i = 0; i < fPaintCount; i++) {
        fPaints[i] = src.fPaints[i];
    }

    fPathHeap = src.fPathHeap;
    SkSafeRef(fPathHeap);

    fPictureCount = src.fPictureCount;
    fPictureRefs = SkNEW_ARRAY(SkPicture*, fPictureCount);
    for (int i = 0; i < fPictureCount; i++) {
        fPictureRefs[i] = src.fPictureRefs[i];
        fPictureRefs[i]->ref();
    }

    fRegionCount = src.fRegionCount;
    fRegions = SkNEW_ARRAY(SkRegion, fRegionCount);
    for (int i = 0; i < fRegionCount; i++) {
        fRegions[i] = src.fRegions[i];
    }
}

void SkPicturePlayback::init() {
    fBitmaps = NULL;
    fMatrices = NULL;
    fPaints = NULL;
    fPathHeap = NULL;
    fPictureRefs = NULL;
    fRegions = NULL;
    fBitmapCount = fMatrixCount = fPaintCount = fPictureCount =
    fRegionCount = 0;

    fFactoryPlayback = NULL;
}

SkPicturePlayback::~SkPicturePlayback() {
    sk_free((void*) fReader.base());

    SkDELETE_ARRAY(fBitmaps);
    SkDELETE_ARRAY(fMatrices);
    SkDELETE_ARRAY(fPaints);
    SkDELETE_ARRAY(fRegions);

    SkSafeUnref(fPathHeap);

    for (int i = 0; i < fPictureCount; i++) {
        fPictureRefs[i]->unref();
    }
    SkDELETE_ARRAY(fPictureRefs);

    SkDELETE(fFactoryPlayback);
}

void SkPicturePlayback::dumpSize() const {
    SkDebugf("--- picture size: ops=%d bitmaps=%d [%d] matrices=%d [%d] paints=%d [%d] paths=%d regions=%d\n",
             fReader.size(),
             fBitmapCount, fBitmapCount * sizeof(SkBitmap),
             fMatrixCount, fMatrixCount * sizeof(SkMatrix),
             fPaintCount, fPaintCount * sizeof(SkPaint),
             fPathHeap ? fPathHeap->count() : 0,
             fRegionCount);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define PICT_READER_TAG     SkSetFourByteTag('r', 'e', 'a', 'd')
#define PICT_FACTORY_TAG    SkSetFourByteTag('f', 'a', 'c', 't')
#define PICT_TYPEFACE_TAG   SkSetFourByteTag('t', 'p', 'f', 'c')
#define PICT_PICTURE_TAG    SkSetFourByteTag('p', 'c', 't', 'r')

// This tag specifies the size of the ReadBuffer, needed for the following tags
#define PICT_BUFFER_SIZE_TAG     SkSetFourByteTag('a', 'r', 'a', 'y')
// these are all inside the ARRAYS tag
#define PICT_BITMAP_BUFFER_TAG  SkSetFourByteTag('b', 't', 'm', 'p')
#define PICT_MATRIX_BUFFER_TAG  SkSetFourByteTag('m', 't', 'r', 'x')
#define PICT_PAINT_BUFFER_TAG   SkSetFourByteTag('p', 'n', 't', ' ')
#define PICT_PATH_BUFFER_TAG    SkSetFourByteTag('p', 't', 'h', ' ')
#define PICT_REGION_BUFFER_TAG  SkSetFourByteTag('r', 'g', 'n', ' ')

// Always write this guy last (with no length field afterwards)
#define PICT_EOF_TAG     SkSetFourByteTag('e', 'o', 'f', ' ')

#include "SkStream.h"

static void writeTagSize(SkFlattenableWriteBuffer& buffer, uint32_t tag,
                         uint32_t size) {
    buffer.write32(tag);
    buffer.write32(size);
}

static void writeTagSize(SkWStream* stream, uint32_t tag,
                         uint32_t size) {
    stream->write32(tag);
    stream->write32(size);
}

static void writeFactories(SkWStream* stream, const SkFactorySet& rec) {
    int count = rec.count();

    writeTagSize(stream, PICT_FACTORY_TAG, count);

    SkAutoSTMalloc<16, SkFlattenable::Factory> storage(count);
    SkFlattenable::Factory* array = (SkFlattenable::Factory*)storage.get();
    rec.copyToArray(array);

    for (int i = 0; i < count; i++) {
        const char* name = SkFlattenable::FactoryToName(array[i]);
//        SkDebugf("---- write factories [%d] %p <%s>\n", i, array[i], name);
        if (NULL == name || 0 == *name) {
            stream->writePackedUInt(0);
        } else {
            uint32_t len = strlen(name);
            stream->writePackedUInt(len);
            stream->write(name, len);
        }
    }
}

static void writeTypefaces(SkWStream* stream, const SkRefCntSet& rec) {
    int count = rec.count();

    writeTagSize(stream, PICT_TYPEFACE_TAG, count);

    SkAutoSTMalloc<16, SkTypeface*> storage(count);
    SkTypeface** array = (SkTypeface**)storage.get();
    rec.copyToArray((SkRefCnt**)array);

    for (int i = 0; i < count; i++) {
        array[i]->serialize(stream);
    }
}

void SkPicturePlayback::flattenToBuffer(SkOrderedWriteBuffer& buffer) const {
    int i;

    if (fBitmapCount > 0) {
        writeTagSize(buffer, PICT_BITMAP_BUFFER_TAG, fBitmapCount);
        for (i = 0; i < fBitmapCount; i++) {
            fBitmaps[i].flatten(buffer);
        }
    }
    
    if (fMatrixCount > 0) {
        writeTagSize(buffer, PICT_MATRIX_BUFFER_TAG, fMatrixCount);
        buffer.writeMul4(fMatrices, fMatrixCount * sizeof(SkMatrix));
    }
    
    if (fPaintCount > 0) {
        writeTagSize(buffer, PICT_PAINT_BUFFER_TAG, fPaintCount);
        for (i = 0; i < fPaintCount; i++) {
            fPaints[i].flatten(buffer);
        }
    }
    
    if (fPathHeap && fPathHeap->count() > 0) {
        int count = fPathHeap->count();
        writeTagSize(buffer, PICT_PATH_BUFFER_TAG, count);
        if (count > 0) {
            fPathHeap->flatten(buffer);
        }
    }
    
    if (fRegionCount > 0) {
        writeTagSize(buffer, PICT_REGION_BUFFER_TAG, fRegionCount);
        for (i = 0; i < fRegionCount; i++) {
            buffer.getWriter32()->writeRegion(fRegions[i]);
        }
    }
}

void SkPicturePlayback::serialize(SkWStream* stream) const {
    writeTagSize(stream, PICT_READER_TAG, fReader.size());
    stream->write(fReader.base(), fReader.size());

    if (fPictureCount > 0) {
        writeTagSize(stream, PICT_PICTURE_TAG, fPictureCount);
        for (int i = 0; i < fPictureCount; i++) {
            fPictureRefs[i]->serialize(stream);
        }
    }

    // Write some of our data into a writebuffer, and then serialize that
    // into our stream
    {
        SkRefCntSet  typefaceSet;
        SkFactorySet factSet;

        SkOrderedWriteBuffer buffer(1024);

        buffer.setFlags(SkFlattenableWriteBuffer::kCrossProcess_Flag);
        buffer.setTypefaceRecorder(&typefaceSet);
        buffer.setFactoryRecorder(&factSet);

        this->flattenToBuffer(buffer);

        // We have to write these to sets into the stream *before* we write
        // the buffer, since parsing that buffer will require that we already
        // have these sets available to use.
        writeFactories(stream, factSet);
        writeTypefaces(stream, typefaceSet);

        writeTagSize(stream, PICT_BUFFER_SIZE_TAG, buffer.size());
        buffer.writeToStream(stream);
    }

    stream->write32(PICT_EOF_TAG);
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return the corresponding SkFlattenableReadBuffer flags, given a set of
 *  SkPictInfo flags.
 */
static uint32_t pictInfoFlagsToReadBufferFlags(uint32_t pictInfoFlags) {
    static const struct {
        uint32_t    fSrc;
        uint32_t    fDst;
    } gSD[] = {
        { SkPictInfo::kCrossProcess_Flag,   SkFlattenableReadBuffer::kCrossProcess_Flag },
        { SkPictInfo::kScalarIsFloat_Flag,  SkFlattenableReadBuffer::kScalarIsFloat_Flag },
        { SkPictInfo::kPtrIs64Bit_Flag,     SkFlattenableReadBuffer::kPtrIs64Bit_Flag },
    };
    
    uint32_t rbMask = 0;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gSD); ++i) {
        if (pictInfoFlags & gSD[i].fSrc) {
            rbMask |= gSD[i].fDst;
        }
    }
    return rbMask;
}

bool SkPicturePlayback::parseStreamTag(SkStream* stream, const SkPictInfo& info,
                                       uint32_t tag, size_t size) {
    /*
     *  By the time we encounter BUFFER_SIZE_TAG, we need to have already seen
     *  its dependents: FACTORY_TAG and TYPEFACE_TAG. These two are not required
     *  but if they are present, they need to have been seen before the buffer.
     *
     *  We assert that if/when we see either of these, that we have not yet seen
     *  the buffer tag, because if we have, then its too-late to deal with the
     *  factories or typefaces.
     */
    bool haveBuffer = false;

    switch (tag) {
        case PICT_READER_TAG: {
            void* storage = sk_malloc_throw(size);
            stream->read(storage, size);
            fReader.setMemory(storage, size);
        } break;
        case PICT_FACTORY_TAG: {
            SkASSERT(!haveBuffer);
            fFactoryPlayback = SkNEW_ARGS(SkFactoryPlayback, (size));
            for (size_t i = 0; i < size; i++) {
                SkString str;
                int len = stream->readPackedUInt();
                str.resize(len);
                stream->read(str.writable_str(), len);
                fFactoryPlayback->base()[i] = SkFlattenable::NameToFactory(str.c_str());
            }
        } break;
        case PICT_TYPEFACE_TAG: {
            SkASSERT(!haveBuffer);
            fTFPlayback.setCount(size);
            for (size_t i = 0; i < size; i++) {
                SkSafeUnref(fTFPlayback.set(i, SkTypeface::Deserialize(stream)));
            }
        } break;
        case PICT_PICTURE_TAG: {
            fPictureCount = size;
            fPictureRefs = SkNEW_ARRAY(SkPicture*, fPictureCount);
            for (int i = 0; i < fPictureCount; i++) {
                fPictureRefs[i] = SkNEW_ARGS(SkPicture, (stream));
            }
        } break;
        case PICT_BUFFER_SIZE_TAG: {
            SkAutoMalloc storage(size);
            stream->read(storage.get(), size);
            
            SkOrderedReadBuffer buffer(storage.get(), size);
            buffer.setFlags(pictInfoFlagsToReadBufferFlags(info.fFlags));

            fFactoryPlayback->setupBuffer(buffer);
            fTFPlayback.setupBuffer(buffer);
            
            while (!buffer.eof()) {
                tag = buffer.readU32();
                size = buffer.readU32();
                if (!this->parseBufferTag(buffer, tag, size)) {
                    return false;
                }
            }
            haveBuffer = true;
        } break;
    }
    return true;    // success
}

bool SkPicturePlayback::parseBufferTag(SkOrderedReadBuffer& buffer,
                                       uint32_t tag, size_t size) {
    switch (tag) {
        case PICT_BITMAP_BUFFER_TAG: {
            fBitmapCount = size;
            fBitmaps = SkNEW_ARRAY(SkBitmap, fBitmapCount);
            for (int i = 0; i < fBitmapCount; i++) {
                fBitmaps[i].unflatten(buffer);
            }
        } break;
        case PICT_MATRIX_BUFFER_TAG:
            fMatrixCount = size;
            fMatrices = SkNEW_ARRAY(SkMatrix, fMatrixCount);
            buffer.read(fMatrices, fMatrixCount * sizeof(SkMatrix));
            break;
        case PICT_PAINT_BUFFER_TAG: {
            fPaintCount = size;
            fPaints = SkNEW_ARRAY(SkPaint, fPaintCount);
            for (int i = 0; i < fPaintCount; i++) {
                fPaints[i].unflatten(buffer);
            }
        } break;
        case PICT_PATH_BUFFER_TAG:
            if (size > 0) {
                fPathHeap = SkNEW_ARGS(SkPathHeap, (buffer));
            }
            break;
        case PICT_REGION_BUFFER_TAG: {
            fRegionCount = size;
            fRegions = SkNEW_ARRAY(SkRegion, fRegionCount);
            for (int i = 0; i < fRegionCount; i++) {
                buffer.getReader32()->readRegion(&fRegions[i]);
            }
        } break;
    }
    return true;    // success
}

SkPicturePlayback::SkPicturePlayback(SkStream* stream, const SkPictInfo& info,
                                     bool* isValid) {
    this->init();

    *isValid = false;   // wait until we're done parsing to mark as true
    for (;;) {
        uint32_t tag = stream->readU32();
        if (PICT_EOF_TAG == tag) {
            break;
        }

        uint32_t size = stream->readU32();
        if (!this->parseStreamTag(stream, info, tag, size)) {
            return; // we're invalid
        }
    }
    *isValid = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef SPEW_CLIP_SKIPPING
struct SkipClipRec {
    int     fCount;
    size_t  fSize;

    SkipClipRec() {
        fCount = 0;
        fSize = 0;
    }

    void recordSkip(size_t bytes) {
        fCount += 1;
        fSize += bytes;
    }
};
#endif

void SkPicturePlayback::draw(SkCanvas& canvas) {
#ifdef ENABLE_TIME_DRAW
    SkAutoTime  at("SkPicture::draw", 50);
#endif

#ifdef SPEW_CLIP_SKIPPING
    SkipClipRec skipRect, skipRegion, skipPath;
#endif

#ifdef SK_BUILD_FOR_ANDROID
    SkAutoMutexAcquire autoMutex(fDrawMutex);
#endif

    TextContainer text;
    fReader.rewind();

    while (!fReader.eof()) {
        switch (fReader.readInt()) {
            case CLIP_PATH: {
                const SkPath& path = getPath();
                uint32_t packed = getInt();
                SkRegion::Op op = ClipParams_unpackRegionOp(packed);
                bool doAA = ClipParams_unpackDoAA(packed);
                size_t offsetToRestore = getInt();
                if (!canvas.clipPath(path, op, doAA) && offsetToRestore) {
#ifdef SPEW_CLIP_SKIPPING
                    skipPath.recordSkip(offsetToRestore - fReader.offset());
#endif
                    fReader.setOffset(offsetToRestore);
                }
            } break;
            case CLIP_REGION: {
                const SkRegion& region = getRegion();
                uint32_t packed = getInt();
                SkRegion::Op op = ClipParams_unpackRegionOp(packed);
                size_t offsetToRestore = getInt();
                if (!canvas.clipRegion(region, op) && offsetToRestore) {
#ifdef SPEW_CLIP_SKIPPING
                    skipRegion.recordSkip(offsetToRestore - fReader.offset());
#endif
                    fReader.setOffset(offsetToRestore);
                }
            } break;
            case CLIP_RECT: {
                const SkRect& rect = fReader.skipT<SkRect>();
                uint32_t packed = getInt();
                SkRegion::Op op = ClipParams_unpackRegionOp(packed);
                bool doAA = ClipParams_unpackDoAA(packed);
                size_t offsetToRestore = getInt();
                if (!canvas.clipRect(rect, op, doAA) && offsetToRestore) {
#ifdef SPEW_CLIP_SKIPPING
                    skipRect.recordSkip(offsetToRestore - fReader.offset());
#endif
                    fReader.setOffset(offsetToRestore);
                }
            } break;
            case CONCAT:
                canvas.concat(*getMatrix());
                break;
            case DRAW_BITMAP: {
                const SkPaint* paint = getPaint();
                const SkBitmap& bitmap = getBitmap();
                const SkPoint& loc = fReader.skipT<SkPoint>();
                canvas.drawBitmap(bitmap, loc.fX, loc.fY, paint);
            } break;
            case DRAW_BITMAP_RECT: {
                const SkPaint* paint = getPaint();
                const SkBitmap& bitmap = getBitmap();
                const SkIRect* src = this->getIRectPtr();   // may be null
                const SkRect& dst = fReader.skipT<SkRect>();     // required
                canvas.drawBitmapRect(bitmap, src, dst, paint);
            } break;
            case DRAW_BITMAP_MATRIX: {
                const SkPaint* paint = getPaint();
                const SkBitmap& bitmap = getBitmap();
                const SkMatrix* matrix = getMatrix();
                canvas.drawBitmapMatrix(bitmap, *matrix, paint);
            } break;
            case DRAW_BITMAP_NINE: {
                const SkPaint* paint = getPaint();
                const SkBitmap& bitmap = getBitmap();
                const SkIRect& src = fReader.skipT<SkIRect>();
                const SkRect& dst = fReader.skipT<SkRect>();
                canvas.drawBitmapNine(bitmap, src, dst, paint);
            } break;
            case DRAW_CLEAR:
                canvas.clear(getInt());
                break;
            case DRAW_DATA: {
                size_t length = getInt();
                canvas.drawData(fReader.skip(length), length);
                // skip handles padding the read out to a multiple of 4
            } break;
            case DRAW_PAINT:
                canvas.drawPaint(*getPaint());
                break;
            case DRAW_PATH: {
                const SkPaint& paint = *getPaint();
                canvas.drawPath(getPath(), paint);
            } break;
            case DRAW_PICTURE:
                canvas.drawPicture(getPicture());
                break;
            case DRAW_POINTS: {
                const SkPaint& paint = *getPaint();
                SkCanvas::PointMode mode = (SkCanvas::PointMode)getInt();
                size_t count = getInt();
                const SkPoint* pts = (const SkPoint*)fReader.skip(sizeof(SkPoint) * count);
                canvas.drawPoints(mode, count, pts, paint);
            } break;
            case DRAW_POS_TEXT: {
                const SkPaint& paint = *getPaint();
                getText(&text);
                size_t points = getInt();
                const SkPoint* pos = (const SkPoint*)fReader.skip(points * sizeof(SkPoint));
                canvas.drawPosText(text.text(), text.length(), pos, paint);
            } break;
            case DRAW_POS_TEXT_TOP_BOTTOM: {
                const SkPaint& paint = *getPaint();
                getText(&text);
                size_t points = getInt();
                const SkPoint* pos = (const SkPoint*)fReader.skip(points * sizeof(SkPoint));
                const SkScalar top = fReader.readScalar();
                const SkScalar bottom = fReader.readScalar();
                if (!canvas.quickRejectY(top, bottom, SkCanvas::kAA_EdgeType)) {
                    canvas.drawPosText(text.text(), text.length(), pos, paint);
                }
            } break;
            case DRAW_POS_TEXT_H: {
                const SkPaint& paint = *getPaint();
                getText(&text);
                size_t xCount = getInt();
                const SkScalar constY = getScalar();
                const SkScalar* xpos = (const SkScalar*)fReader.skip(xCount * sizeof(SkScalar));
                canvas.drawPosTextH(text.text(), text.length(), xpos, constY,
                                    paint);
            } break;
            case DRAW_POS_TEXT_H_TOP_BOTTOM: {
                const SkPaint& paint = *getPaint();
                getText(&text);
                size_t xCount = getInt();
                const SkScalar* xpos = (const SkScalar*)fReader.skip((3 + xCount) * sizeof(SkScalar));
                const SkScalar top = *xpos++;
                const SkScalar bottom = *xpos++;
                const SkScalar constY = *xpos++;
                if (!canvas.quickRejectY(top, bottom, SkCanvas::kAA_EdgeType)) {
                    canvas.drawPosTextH(text.text(), text.length(), xpos,
                                        constY, paint);
                }
            } break;
            case DRAW_RECT: {
                const SkPaint& paint = *getPaint();
                canvas.drawRect(fReader.skipT<SkRect>(), paint);
            } break;
            case DRAW_SPRITE: {
                const SkPaint* paint = getPaint();
                const SkBitmap& bitmap = getBitmap();
                int left = getInt();
                int top = getInt();
                canvas.drawSprite(bitmap, left, top, paint);
            } break;
            case DRAW_TEXT: {
                const SkPaint& paint = *getPaint();
                getText(&text);
                SkScalar x = getScalar();
                SkScalar y = getScalar();
                canvas.drawText(text.text(), text.length(), x, y, paint);
            } break;
            case DRAW_TEXT_TOP_BOTTOM: {
                const SkPaint& paint = *getPaint();
                getText(&text);
                const SkScalar* ptr = (const SkScalar*)fReader.skip(4 * sizeof(SkScalar));
                // ptr[0] == x
                // ptr[1] == y
                // ptr[2] == top
                // ptr[3] == bottom
                if (!canvas.quickRejectY(ptr[2], ptr[3],
                                         SkCanvas::kAA_EdgeType)) {
                    canvas.drawText(text.text(), text.length(), ptr[0], ptr[1],
                                    paint);
                }
            } break;
            case DRAW_TEXT_ON_PATH: {
                const SkPaint& paint = *getPaint();
                getText(&text);
                const SkPath& path = getPath();
                const SkMatrix* matrix = getMatrix();
                canvas.drawTextOnPath(text.text(), text.length(), path,
                                      matrix, paint);
            } break;
            case DRAW_VERTICES: {
                const SkPaint& paint = *getPaint();
                DrawVertexFlags flags = (DrawVertexFlags)getInt();
                SkCanvas::VertexMode vmode = (SkCanvas::VertexMode)getInt();
                int vCount = getInt();
                const SkPoint* verts = (const SkPoint*)fReader.skip(
                                                    vCount * sizeof(SkPoint));
                const SkPoint* texs = NULL;
                const SkColor* colors = NULL;
                const uint16_t* indices = NULL;
                int iCount = 0;
                if (flags & DRAW_VERTICES_HAS_TEXS) {
                    texs = (const SkPoint*)fReader.skip(
                                                    vCount * sizeof(SkPoint));
                }
                if (flags & DRAW_VERTICES_HAS_COLORS) {
                    colors = (const SkColor*)fReader.skip(
                                                    vCount * sizeof(SkColor));
                }
                if (flags & DRAW_VERTICES_HAS_INDICES) {
                    iCount = getInt();
                    indices = (const uint16_t*)fReader.skip(
                                                    iCount * sizeof(uint16_t));
                }
                canvas.drawVertices(vmode, vCount, verts, texs, colors, NULL,
                                    indices, iCount, paint);
            } break;
            case RESTORE:
                canvas.restore();
                break;
            case ROTATE:
                canvas.rotate(getScalar());
                break;
            case SAVE:
                canvas.save((SkCanvas::SaveFlags) getInt());
                break;
            case SAVE_LAYER: {
                const SkRect* boundsPtr = getRectPtr();
                const SkPaint* paint = getPaint();
                canvas.saveLayer(boundsPtr, paint, (SkCanvas::SaveFlags) getInt());
                } break;
            case SCALE: {
                SkScalar sx = getScalar();
                SkScalar sy = getScalar();
                canvas.scale(sx, sy);
            } break;
            case SET_MATRIX:
                canvas.setMatrix(*getMatrix());
                break;
            case SKEW: {
                SkScalar sx = getScalar();
                SkScalar sy = getScalar();
                canvas.skew(sx, sy);
            } break;
            case TRANSLATE: {
                SkScalar dx = getScalar();
                SkScalar dy = getScalar();
                canvas.translate(dx, dy);
            } break;
            default:
                SkASSERT(0);
        }
    }

#ifdef SPEW_CLIP_SKIPPING
    {
        size_t size =  skipRect.fSize + skipPath.fSize + skipRegion.fSize;
        SkDebugf("--- Clip skips %d%% rect:%d path:%d rgn:%d\n",
             size * 100 / fReader.offset(), skipRect.fCount, skipPath.fCount,
             skipRegion.fCount);
    }
#endif
//    this->dumpSize();
}

void SkPicturePlayback::abort() {
    fReader.skip(fReader.size() - fReader.offset());
}

///////////////////////////////////////////////////////////////////////////////

#if 0
uint32_t SkPicturePlayback::flatten(void* storage) const {
    SkWBuffer buffer(storage);
    buffer.write32(fBitmapCount);
    int index;
    for (index = 0; index < fBitmapCount; index++) {
        const SkBitmap& bitmap = fBitmaps[index];
        uint32_t size = bitmap.flatten(NULL, true);
        buffer.write32(size);
        void* local = buffer.skip(size);
        bitmap.flatten(local, true);
    }
    buffer.write32(fPaintCount);
    for (index = 0; index < fPaintCount; index++) {
        SkFlattenableWriteBuffer flatWrite;
        const SkPaint& paint = fPaints[index];
        SkFlatPaint::Write(&flatWrite, paint);
        uint32_t size = flatWrite.pos();
        buffer.write32(size);
        void* local = buffer.skip(size);
        flatWrite.reset(local);
        SkFlatPaint::Write(&flatWrite, paint);
    }
    buffer.write32(fPathCount);
    for (index = 0; index < fPathCount; index++) {
        const SkPath& path = fPaths[index];
        uint32_t size = path.flatten(NULL);
        buffer.write32(size);
        void* local = buffer.skip(size);
        path.flatten(local);
    }

#if 0
    buffer.write32(fPictureCount);
    for (index = 0; index < fPictureCount; index++) {
        const SkPicture& picture = fPictures[index];
        uint32_t size = picture.flatten(NULL);
        buffer.write32(size);
        void* local = buffer.skip(size);
        picture.flatten(local);
    }
#endif

    buffer.write32(fRegionCount);
    for (index = 0; index < fRegionCount; index++) {
        const SkRegion& region = fRegions[index];
        size_t size = region.computeBufferSize();
        buffer.write32(size);
        void* local = buffer.skip(size);
        region.writeToBuffer(local);
    }
    fReader.rewind();
    size_t length = fReader.size();
    buffer.write32(length);
    memcpy(buffer.skip(length), fReader.base(), length);
    return (uint32_t) buffer.pos();
}

void SkPicturePlayback::unflatten(const void* storage) {
    SkRBuffer buffer(storage);
    int index;
    fBitmapCount = buffer.readU32();
    fBitmaps = new SkBitmap[fBitmapCount];
    for (index = 0; index < fBitmapCount; index++) {
        uint32_t size = buffer.readU32();
        const void* local = buffer.skip(size);
        fBitmaps[index].unflatten(local);
    }
    fPaintCount = buffer.readU32();
    fPaints = new SkPaint[fPaintCount];
    for (index = 0; index < fPaintCount; index++) {
        uint32_t size = buffer.readU32();
        const void* local = buffer.skip(size);
        SkFlatPaint::Read(local, &fPaints[index]);
    }
    fPathCount = buffer.readU32();
    fPaths = new SkPath[fPathCount];
    for (index = 0; index < fPathCount; index++) {
        uint32_t size = buffer.readU32();
        const void* local = buffer.skip(size);
        fPaths[index].unflatten(local);
    }

#if 0
    fPictureCount = buffer.readU32();
    fPictures = new SkPicture[fPictureCount];
    for (index = 0; index < fPictureCount; index++) {
        uint32_t size = buffer.readU32();
        const void* local = buffer.skip(size);
        fPictures[index].unflatten(local);
    }
#endif

    fRegionCount = buffer.readU32();
    fRegions = new SkRegion[fRegionCount];
    for (index = 0; index < fRegionCount; index++) {
        uint32_t size = buffer.readU32();
        const void* local = buffer.skip(size);
        fRegions[index].readFromBuffer(local);
    }
    int32_t length = buffer.readS32();
    const void* stream = buffer.skip(length);
    fReader.setMemory(stream, length);
}
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG_SIZE
int SkPicturePlayback::size(size_t* sizePtr) {
    int objects = bitmaps(sizePtr);
    objects += paints(sizePtr);
    objects += paths(sizePtr);
    objects += pictures(sizePtr);
    objects += regions(sizePtr);
    *sizePtr = fReader.size();
    return objects;
}

int SkPicturePlayback::bitmaps(size_t* size) {
    size_t result = 0;
    for (int index = 0; index < fBitmapCount; index++) {
     //   const SkBitmap& bitmap = fBitmaps[index];
        result += sizeof(SkBitmap); // bitmap->size();
    }
    *size = result;
    return fBitmapCount;
}

int SkPicturePlayback::paints(size_t* size) {
    size_t result = 0;
    for (int index = 0; index < fPaintCount; index++) {
    //    const SkPaint& paint = fPaints[index];
        result += sizeof(SkPaint); // paint->size();
    }
    *size = result;
    return fPaintCount;
}

int SkPicturePlayback::paths(size_t* size) {
    size_t result = 0;
    for (int index = 0; index < fPathCount; index++) {
        const SkPath& path = fPaths[index];
        result += path.flatten(NULL);
    }
    *size = result;
    return fPathCount;
}

int SkPicturePlayback::regions(size_t* size) {
    size_t result = 0;
    for (int index = 0; index < fRegionCount; index++) {
    //    const SkRegion& region = fRegions[index];
        result += sizeof(SkRegion); // region->size();
    }
    *size = result;
    return fRegionCount;
}
#endif

#ifdef SK_DEBUG_DUMP
void SkPicturePlayback::dumpBitmap(const SkBitmap& bitmap) const {
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "BitmapData bitmap%p = {", &bitmap);
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "{kWidth, %d}, ", bitmap.width());
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "{kHeight, %d}, ", bitmap.height());
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "{kRowBytes, %d}, ", bitmap.rowBytes());
//        start here;
    SkDebugf("%s{0}};\n", pBuffer);
}

void dumpMatrix(const SkMatrix& matrix) const {
    SkMatrix defaultMatrix;
    defaultMatrix.reset();
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "MatrixData matrix%p = {", &matrix);
    SkScalar scaleX = matrix.getScaleX();
    if (scaleX != defaultMatrix.getScaleX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kScaleX, %g}, ", SkScalarToFloat(scaleX));
    SkScalar scaleY = matrix.getScaleY();
    if (scaleY != defaultMatrix.getScaleY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kScaleY, %g}, ", SkScalarToFloat(scaleY));
    SkScalar skewX = matrix.getSkewX();
    if (skewX != defaultMatrix.getSkewX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kSkewX, %g}, ", SkScalarToFloat(skewX));
    SkScalar skewY = matrix.getSkewY();
    if (skewY != defaultMatrix.getSkewY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kSkewY, %g}, ", SkScalarToFloat(skewY));
    SkScalar translateX = matrix.getTranslateX();
    if (translateX != defaultMatrix.getTranslateX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTranslateX, %g}, ", SkScalarToFloat(translateX));
    SkScalar translateY = matrix.getTranslateY();
    if (translateY != defaultMatrix.getTranslateY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTranslateY, %g}, ", SkScalarToFloat(translateY));
    SkScalar perspX = matrix.getPerspX();
    if (perspX != defaultMatrix.getPerspX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kPerspX, %g}, ", SkFractToFloat(perspX));
    SkScalar perspY = matrix.getPerspY();
    if (perspY != defaultMatrix.getPerspY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kPerspY, %g}, ", SkFractToFloat(perspY));
    SkDebugf("%s{0}};\n", pBuffer);
}

void dumpPaint(const SkPaint& paint) const {
    SkPaint defaultPaint;
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "PaintPointers paintPtrs%p = {", &paint);
    const SkTypeface* typeface = paint.getTypeface();
    if (typeface != defaultPaint.getTypeface())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTypeface, %p}, ", typeface);
    const SkPathEffect* pathEffect = paint.getPathEffect();
    if (pathEffect != defaultPaint.getPathEffect())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kPathEffect, %p}, ", pathEffect);
    const SkShader* shader = paint.getShader();
    if (shader != defaultPaint.getShader())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kShader, %p}, ", shader);
    const SkXfermode* xfermode = paint.getXfermode();
    if (xfermode != defaultPaint.getXfermode())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kXfermode, %p}, ", xfermode);
    const SkMaskFilter* maskFilter = paint.getMaskFilter();
    if (maskFilter != defaultPaint.getMaskFilter())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kMaskFilter, %p}, ", maskFilter);
    const SkColorFilter* colorFilter = paint.getColorFilter();
    if (colorFilter != defaultPaint.getColorFilter())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kColorFilter, %p}, ", colorFilter);
    const SkRasterizer* rasterizer = paint.getRasterizer();
    if (rasterizer != defaultPaint.getRasterizer())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kRasterizer, %p}, ", rasterizer);
    const SkDrawLooper* drawLooper = paint.getLooper();
    if (drawLooper != defaultPaint.getLooper())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kDrawLooper, %p}, ", drawLooper);
    SkDebugf("%s{0}};\n", pBuffer);
    bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "PaintScalars paintScalars%p = {", &paint);
    SkScalar textSize = paint.getTextSize();
    if (textSize != defaultPaint.getTextSize())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTextSize, %g}, ", SkScalarToFloat(textSize));
    SkScalar textScaleX = paint.getTextScaleX();
    if (textScaleX != defaultPaint.getTextScaleX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTextScaleX, %g}, ", SkScalarToFloat(textScaleX));
    SkScalar textSkewX = paint.getTextSkewX();
    if (textSkewX != defaultPaint.getTextSkewX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTextSkewX, %g}, ", SkScalarToFloat(textSkewX));
    SkScalar strokeWidth = paint.getStrokeWidth();
    if (strokeWidth != defaultPaint.getStrokeWidth())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kStrokeWidth, %g}, ", SkScalarToFloat(strokeWidth));
    SkScalar strokeMiter = paint.getStrokeMiter();
    if (strokeMiter != defaultPaint.getStrokeMiter())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kStrokeMiter, %g}, ", SkScalarToFloat(strokeMiter));
    SkDebugf("%s{0}};\n", pBuffer);
    bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
        "PaintInts = paintInts%p = {", &paint);
    unsigned color = paint.getColor();
    if (color != defaultPaint.getColor())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kColor, 0x%x}, ", color);
    unsigned flags = paint.getFlags();
    if (flags != defaultPaint.getFlags())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kFlags, 0x%x}, ", flags);
    int align = paint.getTextAlign();
    if (align != defaultPaint.getTextAlign())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kAlign, 0x%x}, ", align);
    int strokeCap = paint.getStrokeCap();
    if (strokeCap != defaultPaint.getStrokeCap())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kStrokeCap, 0x%x}, ", strokeCap);
    int strokeJoin = paint.getStrokeJoin();
    if (strokeJoin != defaultPaint.getStrokeJoin())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kAlign, 0x%x}, ", strokeJoin);
    int style = paint.getStyle();
    if (style != defaultPaint.getStyle())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kStyle, 0x%x}, ", style);
    int textEncoding = paint.getTextEncoding();
    if (textEncoding != defaultPaint.getTextEncoding())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "{kTextEncoding, 0x%x}, ", textEncoding);
    SkDebugf("%s{0}};\n", pBuffer);

    SkDebugf("PaintData paint%p = {paintPtrs%p, paintScalars%p, paintInts%p};\n",
        &paint, &paint, &paint, &paint);
}

void SkPicturePlayback::dumpPath(const SkPath& path) const {
    SkDebugf("path dump unimplemented\n");
}

void SkPicturePlayback::dumpPicture(const SkPicture& picture) const {
    SkDebugf("picture dump unimplemented\n");
}

void SkPicturePlayback::dumpRegion(const SkRegion& region) const {
    SkDebugf("region dump unimplemented\n");
}

int SkPicturePlayback::dumpDrawType(char* bufferPtr, char* buffer, DrawType drawType) {
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "k%s, ", DrawTypeToString(drawType));
}

int SkPicturePlayback::dumpInt(char* bufferPtr, char* buffer, char* name) {
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:%d, ", name, getInt());
}

int SkPicturePlayback::dumpRect(char* bufferPtr, char* buffer, char* name) {
    const SkRect* rect = fReader.skipRect();
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:{l:%g t:%g r:%g b:%g}, ", name, SkScalarToFloat(rect.fLeft),
        SkScalarToFloat(rect.fTop),
        SkScalarToFloat(rect.fRight), SkScalarToFloat(rect.fBottom));
}

int SkPicturePlayback::dumpPoint(char* bufferPtr, char* buffer, char* name) {
    SkPoint pt;
    getPoint(&pt);
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:{x:%g y:%g}, ", name, SkScalarToFloat(pt.fX),
        SkScalarToFloat(pt.fY));
}

void SkPicturePlayback::dumpPointArray(char** bufferPtrPtr, char* buffer, int count) {
    char* bufferPtr = *bufferPtrPtr;
    const SkPoint* pts = (const SkPoint*)fReadStream.getAtPos();
    fReadStream.skip(sizeof(SkPoint) * count);
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "count:%d {", count);
    for (int index = 0; index < count; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "{x:%g y:%g}, ", SkScalarToFloat(pts[index].fX),
        SkScalarToFloat(pts[index].fY));
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "} ");
    *bufferPtrPtr = bufferPtr;
}

int SkPicturePlayback::dumpPtr(char* bufferPtr, char* buffer, char* name, void* ptr) {
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:%p, ", name, ptr);
}

int SkPicturePlayback::dumpRectPtr(char* bufferPtr, char* buffer, char* name) {
    char result;
    fReadStream.read(&result, sizeof(result));
    if (result)
        return dumpRect(bufferPtr, buffer, name);
    else
        return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
            "%s:NULL, ", name);
}

int SkPicturePlayback::dumpScalar(char* bufferPtr, char* buffer, char* name) {
    return snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - buffer),
        "%s:%d, ", name, getScalar());
}

void SkPicturePlayback::dumpText(char** bufferPtrPtr, char* buffer) {
    char* bufferPtr = *bufferPtrPtr;
    int length = getInt();
    bufferPtr += dumpDrawType(bufferPtr, buffer);
    fReadStream.skipToAlign4();
    char* text = (char*) fReadStream.getAtPos();
    fReadStream.skip(length);
    bufferPtr += dumpInt(bufferPtr, buffer, "length");
    int limit = DUMP_BUFFER_SIZE - (bufferPtr - buffer) - 2;
    length >>= 1;
    if (limit > length)
        limit = length;
    if (limit > 0) {
        *bufferPtr++ = '"';
        for (int index = 0; index < limit; index++) {
            *bufferPtr++ = *(unsigned short*) text;
            text += sizeof(unsigned short);
        }
        *bufferPtr++ = '"';
    }
    *bufferPtrPtr = bufferPtr;
}

#define DUMP_DRAWTYPE(drawType) \
    bufferPtr += dumpDrawType(bufferPtr, buffer, drawType)

#define DUMP_INT(name) \
    bufferPtr += dumpInt(bufferPtr, buffer, #name)

#define DUMP_RECT_PTR(name) \
    bufferPtr += dumpRectPtr(bufferPtr, buffer, #name)

#define DUMP_POINT(name) \
    bufferPtr += dumpRect(bufferPtr, buffer, #name)

#define DUMP_RECT(name) \
    bufferPtr += dumpRect(bufferPtr, buffer, #name)

#define DUMP_POINT_ARRAY(count) \
    dumpPointArray(&bufferPtr, buffer, count)

#define DUMP_PTR(name, ptr) \
    bufferPtr += dumpPtr(bufferPtr, buffer, #name, (void*) ptr)

#define DUMP_SCALAR(name) \
    bufferPtr += dumpScalar(bufferPtr, buffer, #name)

#define DUMP_TEXT() \
    dumpText(&bufferPtr, buffer)

void SkPicturePlayback::dumpStream() {
    SkDebugf("RecordStream stream = {\n");
    DrawType drawType;
    TextContainer text;
    fReadStream.rewind();
    char buffer[DUMP_BUFFER_SIZE], * bufferPtr;
    while (fReadStream.read(&drawType, sizeof(drawType))) {
        bufferPtr = buffer;
        DUMP_DRAWTYPE(drawType);
        switch (drawType) {
            case CLIP_PATH: {
                DUMP_PTR(SkPath, &getPath());
                DUMP_INT(SkRegion::Op);
                DUMP_INT(offsetToRestore);
                } break;
            case CLIP_REGION: {
                DUMP_PTR(SkRegion, &getRegion());
                DUMP_INT(SkRegion::Op);
                DUMP_INT(offsetToRestore);
            } break;
            case CLIP_RECT: {
                DUMP_RECT(rect);
                DUMP_INT(SkRegion::Op);
                DUMP_INT(offsetToRestore);
                } break;
            case CONCAT:
                DUMP_PTR(SkMatrix, getMatrix());
                break;
            case DRAW_BITMAP: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_PTR(SkBitmap, &getBitmap());
                DUMP_SCALAR(left);
                DUMP_SCALAR(top);
                } break;
            case DRAW_PAINT:
                DUMP_PTR(SkPaint, getPaint());
                break;
            case DRAW_PATH: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_PTR(SkPath, &getPath());
                } break;
            case DRAW_PICTURE: {
                DUMP_PTR(SkPicture, &getPicture());
                } break;
            case DRAW_POINTS: {
                DUMP_PTR(SkPaint, getPaint());
                (void)getInt(); // PointMode
                size_t count = getInt();
                fReadStream.skipToAlign4();
                DUMP_POINT_ARRAY(count);
                } break;
            case DRAW_POS_TEXT: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_TEXT();
                size_t points = getInt();
                fReadStream.skipToAlign4();
                DUMP_POINT_ARRAY(points);
                } break;
            case DRAW_POS_TEXT_H: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_TEXT();
                size_t points = getInt();
                fReadStream.skipToAlign4();
                DUMP_SCALAR(top);
                DUMP_SCALAR(bottom);
                DUMP_SCALAR(constY);
                DUMP_POINT_ARRAY(points);
                } break;
            case DRAW_RECT: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_RECT(rect);
                } break;
            case DRAW_SPRITE: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_PTR(SkBitmap, &getBitmap());
                DUMP_SCALAR(left);
                DUMP_SCALAR(top);
                } break;
            case DRAW_TEXT: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_TEXT();
                DUMP_SCALAR(x);
                DUMP_SCALAR(y);
                } break;
            case DRAW_TEXT_ON_PATH: {
                DUMP_PTR(SkPaint, getPaint());
                DUMP_TEXT();
                DUMP_PTR(SkPath, &getPath());
                DUMP_PTR(SkMatrix, getMatrix());
                } break;
            case RESTORE:
                break;
            case ROTATE:
                DUMP_SCALAR(rotate);
                break;
            case SAVE:
                DUMP_INT(SkCanvas::SaveFlags);
                break;
            case SAVE_LAYER: {
                DUMP_RECT_PTR(layer);
                DUMP_PTR(SkPaint, getPaint());
                DUMP_INT(SkCanvas::SaveFlags);
                } break;
            case SCALE: {
                DUMP_SCALAR(sx);
                DUMP_SCALAR(sy);
                } break;
            case SKEW: {
                DUMP_SCALAR(sx);
                DUMP_SCALAR(sy);
                } break;
            case TRANSLATE: {
                DUMP_SCALAR(dx);
                DUMP_SCALAR(dy);
                } break;
            default:
                SkASSERT(0);
        }
        SkDebugf("%s\n", buffer);
    }
}

void SkPicturePlayback::dump() const {
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    int index;
    if (fBitmapCount > 0)
        SkDebugf("// bitmaps (%d)\n", fBitmapCount);
    for (index = 0; index < fBitmapCount; index++) {
        const SkBitmap& bitmap = fBitmaps[index];
        dumpBitmap(bitmap);
    }
    if (fBitmapCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Bitmaps bitmaps = {");
    for (index = 0; index < fBitmapCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "bitmap%p, ", &fBitmaps[index]);
    if (fBitmapCount > 0)
        SkDebugf("%s0};\n", pBuffer);

    if (fMatrixCount > 0)
        SkDebugf("// matrices (%d)\n", fMatrixCount);
    for (index = 0; index < fMatrixCount; index++) {
        const SkMatrix& matrix = fMatrices[index];
        dumpMatrix(matrix);
    }
    bufferPtr = pBuffer;
    if (fMatrixCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Matrices matrices = {");
    for (index = 0; index < fMatrixCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "matrix%p, ", &fMatrices[index]);
    if (fMatrixCount > 0)
        SkDebugf("%s0};\n", pBuffer);

    if (fPaintCount > 0)
        SkDebugf("// paints (%d)\n", fPaintCount);
    for (index = 0; index < fPaintCount; index++) {
        const SkPaint& paint = fPaints[index];
        dumpPaint(paint);
    }
    bufferPtr = pBuffer;
    if (fPaintCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Paints paints = {");
    for (index = 0; index < fPaintCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "paint%p, ", &fPaints[index]);
    if (fPaintCount > 0)
        SkDebugf("%s0};\n", pBuffer);

    for (index = 0; index < fPathCount; index++) {
        const SkPath& path = fPaths[index];
        dumpPath(path);
    }
    bufferPtr = pBuffer;
    if (fPathCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Paths paths = {");
    for (index = 0; index < fPathCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "path%p, ", &fPaths[index]);
    if (fPathCount > 0)
        SkDebugf("%s0};\n", pBuffer);

    for (index = 0; index < fPictureCount; index++) {
        dumpPicture(*fPictureRefs[index]);
    }
    bufferPtr = pBuffer;
    if (fPictureCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Pictures pictures = {");
    for (index = 0; index < fPictureCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "picture%p, ", fPictureRefs[index]);
    if (fPictureCount > 0)
        SkDebugf("%s0};\n", pBuffer);

    for (index = 0; index < fRegionCount; index++) {
        const SkRegion& region = fRegions[index];
        dumpRegion(region);
    }
    bufferPtr = pBuffer;
    if (fRegionCount > 0)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "Regions regions = {");
    for (index = 0; index < fRegionCount; index++)
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer),
            "region%p, ", &fRegions[index]);
    if (fRegionCount > 0)
        SkDebugf("%s0};\n", pBuffer);

    const_cast<SkPicturePlayback*>(this)->dumpStream();
}

#endif
