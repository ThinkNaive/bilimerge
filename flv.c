#include "bilimerge.h"

#pragma pack(push, 1)
typedef struct HEADER // use Big Endian, need correction
{
    char Tag[3];              // "FLV"            0x46,0x4c,0x56
    uint8_t Rev;              // version          0x01
    uint8_t Type;             // stream type      audio:0x04 video:0x01 a+v:0x05
    uint32_t Length;          // Header length    0x09
    uint32_t PreviousTagSize; // default          0x00
} Header;

typedef struct PREVTAG
{
    uint8_t Tag;              // audio:0x08 video:0x09 script:0x12 default:reserved
    uint32_t BufferSize : 24; // Buffer size
    uint32_t TimeStamp : 24;
    uint8_t TimeStampExt;
    uint32_t StreamsID : 24;  // default 0x00
} PrevTag;

typedef struct TAG
{
    PrevTag prev;
    uint8_t *Buffer;          // data region
    uint32_t PreviousTagSize; // total length, exclude self = 11+(BufferSize)
} Tag;
#pragma pack(pop)

double UlongToDouble(void *src)
{
    return *(double *)src;
}

uint64_t DoubleToUlong(void *src)
{
    return *(uint64_t *)src;
}

// 规整形变量BE/LE转换，size取1,2,3...byte
void EdianReverse(void *var, int size)
{
    uint8_t e;
    while (size > 1)
    {
        e = *(uint8_t *)var;
        *(uint8_t *)var = *(uint8_t *)(var + size - 1);
        *(uint8_t *)(var + size - 1) = e;
        ++var;
        size -= 2;
    }
}

// 非规整形变量（指针不在变量处）BE/LE转换
uint64_t BitFieldReverse(uint64_t var, int size)
{
    EdianReverse(&var, size);
    return var;
}

int flvmerge(int argc, char *argv[])
{
    FILE *fo = fopen(argv[1], "wb");
    uint64_t LastDuration = 0;
    int DurationLocation;

    for (int i = 2; i < argc; ++i)
    {
        FILE *f = fopen(argv[i], "rb");
        uint64_t CurrentDuration;
        uint32_t LastTimeStampMillisecond = (uint32_t)(UlongToDouble(&LastDuration) * 1000);
        Header head;
        fread(&head, sizeof(Header), 1, f);
        EdianReverse(&head.Length, sizeof(uint32_t));
        if (head.Tag[0] != 'F' && head.Tag[1] != 'L' && head.Tag[2] != 'V' && head.Length != 9)
            return -1;

        while (!feof(f))
        {
            Tag tag;
            size_t sz = fread(&tag.prev, sizeof(PrevTag), 1, f);
            tag.prev.BufferSize = BitFieldReverse(tag.prev.BufferSize, 3);
            tag.prev.StreamsID = BitFieldReverse(tag.prev.StreamsID, 3);
            // 这里，需要对timestampext加入高位
            tag.prev.TimeStamp = BitFieldReverse(tag.prev.TimeStamp, 3);
            tag.prev.TimeStamp += (uint32_t)tag.prev.TimeStampExt << 24;
            tag.prev.TimeStamp += LastTimeStampMillisecond;

            int pos;
            tag.Buffer = (uint8_t *)malloc(sizeof(uint8_t) * tag.prev.BufferSize);
            fread(tag.Buffer, sizeof(uint8_t), tag.prev.BufferSize, f);
            if (tag.prev.Tag == 0x12)
            {
                // kmp find position
                pos = kmp(tag.Buffer, (uint8_t *)"duration", tag.prev.BufferSize, 8);
                pos += 9;
                // read value to CurrentDuration
                double duration = UlongToDouble(tag.Buffer + pos);
                EdianReverse(&duration, sizeof(double));
                CurrentDuration = DoubleToUlong(&duration);
            }

            fread(&tag.PreviousTagSize, sizeof(uint32_t), 1, f);
            EdianReverse(&tag.PreviousTagSize, sizeof(uint32_t));

            // output
            if (i == 2 && tag.prev.Tag == 0x12) // 合并文件第一个文件，需要写入文件头以及脚本帧
            {
                // Header
                EdianReverse(&head.Length, sizeof(uint32_t));
                fwrite(&head, sizeof(Header), 1, fo);
                // 这里，duration总时长未处理，需最后处理
                DurationLocation = sizeof(Header) + sizeof(PrevTag) + pos;
            }

            if ((i == 2 && tag.prev.Tag == 0x12) || tag.prev.Tag != 0x12) // 忽略剩下文件的脚本帧，写入音频视频Tag
            {
                tag.prev.BufferSize = BitFieldReverse(tag.prev.BufferSize, 3);
                tag.prev.TimeStampExt = tag.prev.TimeStamp >> 24;
                tag.prev.TimeStamp = BitFieldReverse(tag.prev.TimeStamp, 3);
                tag.prev.StreamsID = BitFieldReverse(tag.prev.StreamsID, 3);
                fwrite(&tag.prev, sizeof(PrevTag), 1, fo);
                tag.prev.BufferSize = BitFieldReverse(tag.prev.BufferSize, 3);
                fwrite(tag.Buffer, sizeof(uint8_t), tag.prev.BufferSize, fo);
                EdianReverse(&tag.PreviousTagSize, sizeof(uint32_t));
                fwrite(&tag.PreviousTagSize, sizeof(uint32_t), 1, fo);
            }

            free(tag.Buffer);
        }
        double duration = UlongToDouble(&LastDuration) + UlongToDouble(&CurrentDuration);
        LastDuration = DoubleToUlong(&duration);
        fclose(f);
    }
    // 处理Duration
    fseek(fo, DurationLocation, SEEK_SET);
    EdianReverse(&LastDuration, sizeof(uint64_t));
    fwrite(&LastDuration, sizeof(uint64_t), 1, fo);
    
    fclose(fo);
    return 0;
}