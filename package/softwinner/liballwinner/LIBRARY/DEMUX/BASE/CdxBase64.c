#include <CdxBase64.h>

#include <CdxTypes.h>
#include <CdxBuffer.h>
#include <CdxMemory.h>

CdxBufferT *CdxDecodeBase64(AwPoolT *pool, const cdx_char *s)
{
    cdx_size n;
    cdx_size padding = 0, outLen = 0, j = 0, i = 0;
    cdx_uint32 accum = 0;
    CdxBufferT *buffer = NULL;
    cdx_uint8 *out;
    
    CDX_CHECK(s);
    n = strlen(s);

    if ((n % 4) != 0)
    {
        return NULL;
    }

    if (n >= 1 && s[n - 1] == '=')
    {
        padding = 1;

        if (n >= 2 && s[n - 2] == '=')
        {
            padding = 2;
        }
    }

    outLen = 3 * n / 4 - padding;

    buffer = CdxBufferCreate(pool, outLen, NULL, 0);
    CDX_FORCE_CHECK(buffer);

    out = CdxBufferGetData(buffer);
    
    for (i = 0; i < n; ++i) {
        cdx_char c = s[i];
        cdx_uint32 value;
        if (c >= 'A' && c <= 'Z') {
            value = c - 'A';
        }
        else if (c >= 'a' && c <= 'z') {
            value = 26 + c - 'a';
        } 
        else if (c >= '0' && c <= '9') {
            value = 52 + c - '0';
        } 
        else if (c == '+') {
            value = 62;
        } 
        else if (c == '/') {
            value = 63;
        } 
        else if (c != '=') {
            return NULL;
        }
        else {
            if (i < n - padding) {
                return NULL;
            }
            value = 0;
        }

        accum = (accum << 6) | value;

        if (((i + 1) % 4) == 0) {
            out[j++] = (accum >> 16);

            if (j < outLen) { out[j++] = (accum >> 8) & 0xff; } 
            if (j < outLen) { out[j++] = accum & 0xff; }

            accum = 0;
        }
    }
    
    CdxBufferSetRange(buffer, 0, outLen);
    return buffer;
}

static char CdxEncode6Bit(cdx_uint32 x) {
    if (x <= 25) {
        return 'A' + x;
    }
    else if (x <= 51) {
        return 'a' + x - 26;
    } 
    else if (x <= 61) {
        return '0' + x - 52;
    } 
    else if (x == 62) {
        return '+';
    } 
    else {
        return '/';
    }
}

cdx_char *CdxEncodeBase64(AwPoolT *pool, const cdx_void *_data, cdx_size size) 
{
    const cdx_uint8 *data = (const cdx_uint8 *)_data;
    cdx_size i, n, outSizeMax, pos = 0;
    cdx_char *out;

    n = (size / 3) * 3;
    outSizeMax = (n * 4 / 3) + 8;
    out = Palloc(pool, outSizeMax);
    CDX_FORCE_CHECK(out);
    memset(out, 0x00, outSizeMax);
    
    for (i = 0; i < n; i += 3) {
        cdx_uint8 x1 = data[i];
        cdx_uint8 x2 = data[i + 1];
        cdx_uint8 x3 = data[i + 2];

        out[pos++] = CdxEncode6Bit(x1 >> 2);
        out[pos++] = CdxEncode6Bit((x1 << 4 | x2 >> 4) & 0x3f);
        out[pos++] = CdxEncode6Bit((x2 << 2 | x3 >> 6) & 0x3f);
        out[pos++] = CdxEncode6Bit(x3 & 0x3f);
    }
    
    switch (size % 3) {
        case 0:
            break;
        case 2:
        {
            cdx_uint8 x1 = data[i];
            cdx_uint8 x2 = data[i + 1];
            out[pos++] = CdxEncode6Bit(x1 >> 2);
            out[pos++] = CdxEncode6Bit((x1 << 4 | x2 >> 4) & 0x3f);
            out[pos++] = CdxEncode6Bit((x2 << 2) & 0x3f);
            out[pos++] = '=';
            break;
        }
        default:
        {
            cdx_uint8 x1 = data[i];
            out[pos++] = CdxEncode6Bit(x1 >> 2);
            out[pos++] = CdxEncode6Bit((x1 << 4) & 0x3f);
            out[pos++] = '=';
            out[pos++] = '=';
            break;
        }
    }
    return out;
}

