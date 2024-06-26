/*
 *  SPDX-FileCopyrightText: 2005 Michael Thaler
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_filters.h"

#include <algorithm>

#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include "kis_convolution_painter.h"
#include "kis_convolution_kernel.h"
#include "kis_pixel_selection.h"
#include <kis_sequential_iterator.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define RINT(x) floor ((x) + 0.5)

KisSelectionFilter::~KisSelectionFilter()
{
}

KUndo2MagicString KisSelectionFilter::name()
{
    return KUndo2MagicString();
}

QRect KisSelectionFilter::changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);
    return rect;
}

void KisSelectionFilter::computeBorder(qint32* circ, qint32 xradius, qint32 yradius)
{
    qint32 i;
    qint32 diameter = xradius * 2 + 1;
    double tmp;

    for (i = 0; i < diameter; i++) {
        if (i > xradius)
            tmp = (i - xradius) - 0.5;
        else if (i < xradius)
            tmp = (xradius - i) - 0.5;
        else
            tmp = 0.0;

        double divisor = (double) xradius;
        if (divisor == 0.0) {
            divisor = 1.0;
        }
        circ[i] = (qint32) RINT(yradius * sqrt(xradius * xradius - tmp * tmp) / divisor);
    }
}

void KisSelectionFilter::rotatePointers(quint8** p, quint32 n)
{
    quint32 i;
    quint8  *p0 = p[0];
    for (i = 0; i < n - 1; i++) {
        p[i] = p[i + 1];
    }
    p[i] = p0;
}

void KisSelectionFilter::computeTransition(quint8* transition, quint8** buf, qint32 width)
{
    qint32 x = 0;

    if (width == 1) {
        if (buf[1][x] > 127 && (buf[0][x] < 128 || buf[2][x] < 128))
            transition[x] = 255;
        else
            transition[x] = 0;
        return;
    }
    if (buf[1][x] > 127) {
        if (buf[0][x] < 128 || buf[0][x + 1] < 128 ||
            buf[1][x + 1] < 128 ||
            buf[2][x] < 128 || buf[2][x + 1] < 128)
            transition[x] = 255;
        else
            transition[x] = 0;
    } else
        transition[x] = 0;
    for (qint32 x = 1; x < width - 1; x++) {
        if (buf[1][x] >= 128) {
            if (buf[0][x - 1] < 128 || buf[0][x] < 128 || buf[0][x + 1] < 128 ||
                buf[1][x - 1] < 128           ||          buf[1][x + 1] < 128 ||
                buf[2][x - 1] < 128 || buf[2][x] < 128 || buf[2][x + 1] < 128)
                transition[x] = 255;
            else
                transition[x] = 0;
        } else
            transition[x] = 0;
    }
    if (buf[1][x] >= 128) {
        if (buf[0][x - 1] < 128 || buf[0][x] < 128 ||
            buf[1][x - 1] < 128 ||
            buf[2][x - 1] < 128 || buf[2][x] < 128)
            transition[x] = 255;
        else
            transition[x] = 0;
    } else
        transition[x] = 0;
}


KUndo2MagicString KisErodeSelectionFilter::name()
{
    return kundo2_i18n("Erode Selection");
}

QRect KisErodeSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    const qint32 radius = 1;
    return rect.adjusted(-radius, -radius, radius, radius);
}

void KisErodeSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    // Erode (radius 1 pixel) a mask (1bpp)
    quint8* buf[3];

    qint32 width = rect.width();
    qint32 height = rect.height();

    quint8* out = new quint8[width];
    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];


    // load top of image
    pixelSelection->readBytes(buf[0] + 1, rect.x(), rect.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy(buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < height; y++) {
        if (y + 1 < height) {
            pixelSelection->readBytes(buf[2] + 1, rect.x(), rect.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        } else {
            memcpy(buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++) {
            qint32 min = 255;

            if (buf[0][x+1] < min) min = buf[0][x+1];
            if (buf[1][x]   < min) min = buf[1][x];
            if (buf[1][x+1] < min) min = buf[1][x+1];
            if (buf[1][x+2] < min) min = buf[1][x+2];
            if (buf[2][x+1] < min) min = buf[2][x+1];

            out[x] = min;
        }

        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, width, 1);
        rotatePointers(buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];
    delete[] out;
}


KUndo2MagicString KisDilateSelectionFilter::name()
{
    return kundo2_i18n("Dilate Selection");
}

QRect KisDilateSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    const qint32 radius = 1;
    return rect.adjusted(-radius, -radius, radius, radius);
}

void KisDilateSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
 {
    // dilate (radius 1 pixel) a mask (1bpp)
    quint8* buf[3];

    qint32 width = rect.width();
    qint32 height = rect.height();

    quint8* out = new quint8[width];
    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];


    // load top of image
    pixelSelection->readBytes(buf[0] + 1, rect.x(), rect.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy(buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < height; y++) {
        if (y + 1 < height) {
            pixelSelection->readBytes(buf[2] + 1, rect.x(), rect.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        } else {
            memcpy(buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++) {
            qint32 max = 0;

            if (buf[0][x+1] > max) max = buf[0][x+1];
            if (buf[1][x]   > max) max = buf[1][x];
            if (buf[1][x+1] > max) max = buf[1][x+1];
            if (buf[1][x+2] > max) max = buf[1][x+2];
            if (buf[2][x+1] > max) max = buf[2][x+1];

            out[x] = max;
        }

        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, width, 1);
        rotatePointers(buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];
    delete[] out;
}


KisBorderSelectionFilter::KisBorderSelectionFilter(qint32 xRadius, qint32 yRadius, bool antialiasing)
  : m_xRadius(xRadius),
    m_yRadius(yRadius),
    m_antialiasing(antialiasing)
{
}

KUndo2MagicString KisBorderSelectionFilter::name()
{
    return kundo2_i18n("Border Selection");
}

QRect KisBorderSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    return rect.adjusted(-m_xRadius, -m_yRadius, m_xRadius, m_yRadius);
}

void KisBorderSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    if (m_xRadius <= 0 || m_yRadius <= 0) return;

    quint8  *buf[3];
    quint8 **density;
    quint8 **transition;

    if (m_xRadius == 1 && m_yRadius == 1) {
        // optimize this case specifically
        quint8* source[3];

        for (qint32 i = 0; i < 3; i++)
            source[i] = new quint8[rect.width()];

        quint8* transition = new quint8[rect.width()];

        pixelSelection->readBytes(source[0], rect.x(), rect.y(), rect.width(), 1);
        memcpy(source[1], source[0], rect.width());
        if (rect.height() > 1)
            pixelSelection->readBytes(source[2], rect.x(), rect.y() + 1, rect.width(), 1);
        else
            memcpy(source[2], source[1], rect.width());

        computeTransition(transition, source, rect.width());
        pixelSelection->writeBytes(transition, rect.x(), rect.y(), rect.width(), 1);

        for (qint32 y = 1; y < rect.height(); y++) {
            rotatePointers(source, 3);
            if (y + 1 < rect.height())
                pixelSelection->readBytes(source[2], rect.x(), rect.y() + y + 1, rect.width(), 1);
            else
                memcpy(source[2], source[1], rect.width());
            computeTransition(transition, source, rect.width());
            pixelSelection->writeBytes(transition, rect.x(), rect.y() + y, rect.width(), 1);
        }

        for (qint32 i = 0; i < 3; i++)
            delete[] source[i];
        delete[] transition;
        return;
    }

    qint32* max = new qint32[rect.width() + 2 * m_xRadius];
    for (qint32 i = 0; i < (rect.width() + 2 * m_xRadius); i++)
        max[i] = m_yRadius + 2;
    max += m_xRadius;

    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[rect.width()];

    transition = new quint8*[m_yRadius + 1];
    for (qint32 i = 0; i < m_yRadius + 1; i++) {
        transition[i] = new quint8[rect.width() + 2 * m_xRadius];
        memset(transition[i], 0, rect.width() + 2 * m_xRadius);
        transition[i] += m_xRadius;
    }
    quint8* out = new quint8[rect.width()];
    density = new quint8*[2 * m_xRadius + 1];
    density += m_xRadius;

    for (qint32 x = 0; x < (m_xRadius + 1); x++) { // allocate density[][]
        density[ x]  = new quint8[2 * m_yRadius + 1];
        density[ x] += m_yRadius;
        density[-x]  = density[x];
    }

    // compute density[][]
    if (m_antialiasing) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_xRadius == m_yRadius && "anisotropic fading is not implemented");
        const qreal maxRadius = 0.5 * (m_xRadius + m_yRadius);
        const qreal minRadius = maxRadius - 1.0;

        for (qint32 x = 0; x < (m_xRadius + 1); x++) {
            double dist;
            quint8 a;

            for (qint32 y = 0; y < (m_yRadius + 1); y++) {

                dist = sqrt(pow2(x) + pow2(y));

                if (dist > maxRadius) {
                    a = 0;
                } else if (dist > minRadius) {
                    a = qRound((1.0 - dist + minRadius) * 255.0);
                } else {
                    a = 255;
                }

                density[ x][ y] = a;
                density[ x][-y] = a;
                density[-x][ y] = a;
                density[-x][-y] = a;
            }
        }

    } else {
        for (qint32 x = 0; x < (m_xRadius + 1); x++) {
            double tmpx, tmpy, dist;
            quint8 a;

            tmpx = x > 0.0 ? x - 0.5 : 0.0;

            for (qint32 y = 0; y < (m_yRadius + 1); y++) {
                tmpy = y > 0.0 ? y - 0.5 : 0.0;

                dist = (pow2(tmpy) / pow2(m_yRadius) +
                        pow2(tmpx) / pow2(m_xRadius));

                a = dist <= 1.0 ? 255 : 0;

                density[ x][ y] = a;
                density[ x][-y] = a;
                density[-x][ y] = a;
                density[-x][-y] = a;
            }
        }
    }

    pixelSelection->readBytes(buf[0], rect.x(), rect.y(), rect.width(), 1);
    memcpy(buf[1], buf[0], rect.width());
    if (rect.height() > 1)
        pixelSelection->readBytes(buf[2], rect.x(), rect.y() + 1, rect.width(), 1);
    else
        memcpy(buf[2], buf[1], rect.width());
    computeTransition(transition[1], buf, rect.width());

    for (qint32 y = 1; y < m_yRadius && y + 1 < rect.height(); y++) { // set up top of image
        rotatePointers(buf, 3);
        pixelSelection->readBytes(buf[2], rect.x(), rect.y() + y + 1, rect.width(), 1);
        computeTransition(transition[y + 1], buf, rect.width());
    }
    for (qint32 x = 0; x < rect.width(); x++) { // set up max[] for top of image
        max[x] = -(m_yRadius + 7);
        for (qint32 j = 1; j < m_yRadius + 1; j++)
            if (transition[j][x]) {
                max[x] = j;
                break;
            }
    }
    for (qint32 y = 0; y < rect.height(); y++) { // main calculation loop
        rotatePointers(buf, 3);
        rotatePointers(transition, m_yRadius + 1);
        if (y < rect.height() - (m_yRadius + 1)) {
            pixelSelection->readBytes(buf[2], rect.x(), rect.y() + y + m_yRadius + 1, rect.width(), 1);
            computeTransition(transition[m_yRadius], buf, rect.width());
        } else
            memcpy(transition[m_yRadius], transition[m_yRadius - 1], rect.width());

        for (qint32 x = 0; x < rect.width(); x++) { // update max array
            if (max[x] < 1) {
                if (max[x] <= -m_yRadius) {
                    if (transition[m_yRadius][x])
                        max[x] = m_yRadius;
                    else
                        max[x]--;
                } else if (transition[-max[x]][x])
                    max[x] = -max[x];
                else if (transition[-max[x] + 1][x])
                    max[x] = -max[x] + 1;
                else
                    max[x]--;
            } else
                max[x]--;
            if (max[x] < -m_yRadius - 1)
                max[x] = -m_yRadius - 1;
        }
        quint8 last_max =  max[0][density[-1]];
        qint32 last_index = 1;
        for (qint32 x = 0 ; x < rect.width(); x++) { // render scan line
            last_index--;
            if (last_index >= 0) {
                last_max = 0;
                for (qint32 i = m_xRadius; i >= 0; i--)
                    if (max[x + i] <= m_yRadius && max[x + i] >= -m_yRadius && density[i][max[x+i]] > last_max) {
                        last_max = density[i][max[x + i]];
                        last_index = i;
                    }
                out[x] = last_max;
            } else {
                last_max = 0;
                for (qint32 i = m_xRadius; i >= -m_xRadius; i--)
                    if (max[x + i] <= m_yRadius && max[x + i] >= -m_yRadius && density[i][max[x + i]] > last_max) {
                        last_max = density[i][max[x + i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
            if (last_max == 0) {
                qint32 i;
                for (i = x + 1; i < rect.width(); i++) {
                    if (max[i] >= -m_yRadius)
                        break;
                }
                if (i - x > m_xRadius) {
                    for (; x < i - m_xRadius; x++)
                        out[x] = 0;
                    x--;
                }
                last_index = m_xRadius;
            }
        }
        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, rect.width(), 1);
    }
    delete [] out;

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];

    max -= m_xRadius;
    delete[] max;

    for (qint32 i = 0; i < m_yRadius + 1; i++) {
        transition[i] -= m_xRadius;
        delete transition[i];
    }
    delete[] transition;

    for (qint32 i = 0; i < m_xRadius + 1 ; i++) {
        density[i] -= m_yRadius;
        delete density[i];
    }
    density -= m_xRadius;
    delete[] density;
}


KisFeatherSelectionFilter::KisFeatherSelectionFilter(qint32 radius)
    : m_radius(radius)
{
}

KUndo2MagicString KisFeatherSelectionFilter::name()
{
    return kundo2_i18n("Feather Selection");
}

QRect KisFeatherSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    return rect.adjusted(-m_radius, -m_radius,
                         m_radius, m_radius);
}

void KisFeatherSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    // compute horizontal kernel
    const uint kernelSize = m_radius * 2 + 1;
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> gaussianMatrix(1, kernelSize);

    const qreal multiplicand = 1.0 / (2.0 * M_PI * m_radius * m_radius);
    const qreal exponentMultiplicand = 1.0 / (2.0 * m_radius * m_radius);

    for (uint x = 0; x < kernelSize; x++) {
        uint xDistance = qAbs((int)m_radius - (int)x);
        gaussianMatrix(0, x) = multiplicand * exp( -(qreal)((xDistance * xDistance) + (m_radius * m_radius)) * exponentMultiplicand );
    }

    KisConvolutionKernelSP kernelHoriz = KisConvolutionKernel::fromMatrix(gaussianMatrix, 0, gaussianMatrix.sum());
    KisConvolutionKernelSP kernelVertical = KisConvolutionKernel::fromMatrix(gaussianMatrix.transpose(), 0, gaussianMatrix.sum());

    KisPaintDeviceSP interm = new KisPaintDevice(pixelSelection->colorSpace());
    interm->prepareClone(pixelSelection);

    KisConvolutionPainter horizPainter(interm);
    horizPainter.setChannelFlags(interm->colorSpace()->channelFlags(false, true));
    horizPainter.applyMatrix(kernelHoriz, pixelSelection, rect.topLeft(), rect.topLeft(), rect.size(), BORDER_REPEAT);
    horizPainter.end();

    KisConvolutionPainter verticalPainter(pixelSelection);
    verticalPainter.setChannelFlags(pixelSelection->colorSpace()->channelFlags(false, true));
    verticalPainter.applyMatrix(kernelVertical, interm, rect.topLeft(), rect.topLeft(), rect.size(), BORDER_REPEAT);
    verticalPainter.end();
}


KisGrowSelectionFilter::KisGrowSelectionFilter(qint32 xRadius, qint32 yRadius)
    : m_xRadius(xRadius)
    , m_yRadius(yRadius)
{
}

KUndo2MagicString KisGrowSelectionFilter::name()
{
    return kundo2_i18n("Grow Selection");
}

QRect KisGrowSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    return rect.adjusted(-m_xRadius, -m_yRadius, m_xRadius, m_yRadius);
}

void KisGrowSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    if (m_xRadius <= 0 || m_yRadius <= 0) return;

    /**
        * Much code resembles Shrink filter, so please fix bugs
        * in both filters
        */

    quint8  **buf;  // caches the region's pixel data
    quint8  **max;  // caches the largest values for each column

    max = new quint8* [rect.width() + 2 * m_xRadius];
    buf = new quint8* [m_yRadius + 1];
    for (qint32 i = 0; i < m_yRadius + 1; i++) {
        buf[i] = new quint8[rect.width()];
    }
    quint8* buffer = new quint8[(rect.width() + 2 * m_xRadius) *(m_yRadius + 1)];
    for (qint32 i = 0; i < rect.width() + 2 * m_xRadius; i++) {
        if (i < m_xRadius)
            max[i] = buffer;
        else if (i < rect.width() + m_xRadius)
            max[i] = &buffer[(m_yRadius + 1) * (i - m_xRadius)];
        else
            max[i] = &buffer[(m_yRadius + 1) * (rect.width() + m_xRadius - 1)];

        for (qint32 j = 0; j < m_xRadius + 1; j++)
            max[i][j] = 0;
    }
    /* offset the max pointer by m_xRadius so the range of the array
        is [-m_xRadius] to [region->w + m_xRadius] */
    max += m_xRadius;

    quint8* out = new quint8[ rect.width()];  // holds the new scan line we are computing

    qint32* circ = new qint32[ 2 * m_xRadius + 1 ]; // holds the y coords of the filter's mask
    computeBorder(circ, m_xRadius, m_yRadius);

    /* offset the circ pointer by m_xRadius so the range of the array
        is [-m_xRadius] to [m_xRadius] */
    circ += m_xRadius;

    memset(buf[0], 0, rect.width());
    for (qint32 i = 0; i < m_yRadius && i < rect.height(); i++) { // load top of image
        pixelSelection->readBytes(buf[i + 1], rect.x(), rect.y() + i, rect.width(), 1);
    }

    for (qint32 x = 0; x < rect.width() ; x++) { // set up max for top of image
        max[x][0] = 0;         // buf[0][x] is always 0
        max[x][1] = buf[1][x]; // MAX (buf[1][x], max[x][0]) always = buf[1][x]
        for (qint32 j = 2; j < m_yRadius + 1; j++) {
            max[x][j] = MAX(buf[j][x], max[x][j-1]);
        }
    }

    for (qint32 y = 0; y < rect.height(); y++) {
        rotatePointers(buf, m_yRadius + 1);
        if (y < rect.height() - (m_yRadius))
            pixelSelection->readBytes(buf[m_yRadius], rect.x(), rect.y() + y + m_yRadius, rect.width(), 1);
        else
            memset(buf[m_yRadius], 0, rect.width());
        for (qint32 x = 0; x < rect.width(); x++) { /* update max array */
            for (qint32 i = m_yRadius; i > 0; i--) {
                max[x][i] = MAX(MAX(max[x][i - 1], buf[i - 1][x]), buf[i][x]);
            }
            max[x][0] = buf[0][x];
        }
        qint32 last_max = max[0][circ[-1]];
        qint32 last_index = 1;
        for (qint32 x = 0; x < rect.width(); x++) { /* render scan line */
            last_index--;
            if (last_index >= 0) {
                if (last_max == 255)
                    out[x] = 255;
                else {
                    last_max = 0;
                    for (qint32 i = m_xRadius; i >= 0; i--)
                        if (last_max < max[x + i][circ[i]]) {
                            last_max = max[x + i][circ[i]];
                            last_index = i;
                        }
                    out[x] = last_max;
                }
            } else {
                last_index = m_xRadius;
                last_max = max[x + m_xRadius][circ[m_xRadius]];
                for (qint32 i = m_xRadius - 1; i >= -m_xRadius; i--)
                    if (last_max < max[x + i][circ[i]]) {
                        last_max = max[x + i][circ[i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
        }
        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, rect.width(), 1);
    }
    /* undo the offsets to the pointers so we can free the malloced memory */
    circ -= m_xRadius;
    max -= m_xRadius;

    delete[] circ;
    delete[] buffer;
    delete[] max;
    for (qint32 i = 0; i < m_yRadius + 1; i++)
        delete[] buf[i];
    delete[] buf;
    delete[] out;
}


KisShrinkSelectionFilter::KisShrinkSelectionFilter(qint32 xRadius, qint32 yRadius, bool edgeLock)
    : m_xRadius(xRadius)
    , m_yRadius(yRadius)
    , m_edgeLock(edgeLock)
{
}

KUndo2MagicString KisShrinkSelectionFilter::name()
{
    return kundo2_i18n("Shrink Selection");
}

QRect KisShrinkSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    return m_edgeLock ? defaultBounds->imageBorderRect() : rect;
}

void KisShrinkSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    if (m_xRadius <= 0 || m_yRadius <= 0) return;

    /*
        pretty much the same as fatten_region only different
        blame all bugs in this function on jaycox@gimp.org
    */
    /* If edge_lock is true  we assume that pixels outside the region
        we are passed are identical to the edge pixels.
        If edge_lock is false, we assume that pixels outside the region are 0
    */
    quint8  **buf;  // caches the region's pixels
    quint8  **max;  // caches the smallest values for each column
    qint32    last_max, last_index;

    max = new quint8* [rect.width() + 2 * m_xRadius];
    buf = new quint8* [m_yRadius + 1];
    for (qint32 i = 0; i < m_yRadius + 1; i++) {
        buf[i] = new quint8[rect.width()];
    }

    qint32 buffer_size = (rect.width() + 2 * m_xRadius + 1) * (m_yRadius + 1);
    quint8* buffer = new quint8[buffer_size];

    if (m_edgeLock)
        memset(buffer, 255, buffer_size);
    else
        memset(buffer, 0, buffer_size);

    for (qint32 i = 0; i < rect.width() + 2 * m_xRadius; i++) {
        if (i < m_xRadius)
            if (m_edgeLock)
                max[i] = buffer;
            else
                max[i] = &buffer[(m_yRadius + 1) * (rect.width() + m_xRadius)];
        else if (i < rect.width() + m_xRadius)
            max[i] = &buffer[(m_yRadius + 1) * (i - m_xRadius)];
        else if (m_edgeLock)
            max[i] = &buffer[(m_yRadius + 1) * (rect.width() + m_xRadius - 1)];
        else
            max[i] = &buffer[(m_yRadius + 1) * (rect.width() + m_xRadius)];
    }
    if (!m_edgeLock)
        for (qint32 j = 0 ; j < m_xRadius + 1; j++) max[0][j] = 0;

    // offset the max pointer by m_xRadius so the range of the array is [-m_xRadius] to [region->w + m_xRadius]
    max += m_xRadius;

    quint8* out = new quint8[rect.width()]; // holds the new scan line we are computing

    qint32* circ = new qint32[2 * m_xRadius + 1]; // holds the y coords of the filter's mask

    computeBorder(circ, m_xRadius, m_yRadius);

    // offset the circ pointer by m_xRadius so the range of the array is [-m_xRadius] to [m_xRadius]
    circ += m_xRadius;

    for (qint32 i = 0; i < m_yRadius && i < rect.height(); i++) // load top of image
        pixelSelection->readBytes(buf[i + 1], rect.x(), rect.y() + i, rect.width(), 1);

    if (m_edgeLock)
        memcpy(buf[0], buf[1], rect.width());
    else
        memset(buf[0], 0, rect.width());


    for (qint32 x = 0; x < rect.width(); x++) { // set up max for top of image
        max[x][0] = buf[0][x];
        for (qint32 j = 1; j < m_yRadius + 1; j++)
            max[x][j] = MIN(buf[j][x], max[x][j-1]);
    }

    for (qint32 y = 0; y < rect.height(); y++) {
        rotatePointers(buf, m_yRadius + 1);
        if (y < rect.height() - m_yRadius)
            pixelSelection->readBytes(buf[m_yRadius], rect.x(), rect.y() + y + m_yRadius, rect.width(), 1);
        else if (m_edgeLock)
            memcpy(buf[m_yRadius], buf[m_yRadius - 1], rect.width());
        else
            memset(buf[m_yRadius], 0, rect.width());

        for (qint32 x = 0 ; x < rect.width(); x++) { // update max array
            for (qint32 i = m_yRadius; i > 0; i--) {
                max[x][i] = MIN(MIN(max[x][i - 1], buf[i - 1][x]), buf[i][x]);
            }
            max[x][0] = buf[0][x];
        }
        last_max =  max[0][circ[-1]];
        last_index = 0;

        for (qint32 x = 0 ; x < rect.width(); x++) { // render scan line
            last_index--;
            if (last_index >= 0) {
                if (last_max == 0)
                    out[x] = 0;
                else {
                    last_max = 255;
                    for (qint32 i = m_xRadius; i >= 0; i--)
                        if (last_max > max[x + i][circ[i]]) {
                            last_max = max[x + i][circ[i]];
                            last_index = i;
                        }
                    out[x] = last_max;
                }
            } else {
                last_index = m_xRadius;
                last_max = max[x + m_xRadius][circ[m_xRadius]];
                for (qint32 i = m_xRadius - 1; i >= -m_xRadius; i--)
                    if (last_max > max[x + i][circ[i]]) {
                        last_max = max[x + i][circ[i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
        }
        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, rect.width(), 1);
    }

    // undo the offsets to the pointers so we can free the malloced memory
    circ -= m_xRadius;
    max -= m_xRadius;

    delete[] circ;
    delete[] buffer;
    delete[] max;
    for (qint32 i = 0; i < m_yRadius + 1; i++)
        delete[] buf[i];
    delete[] buf;
    delete[] out;
}


KUndo2MagicString KisSmoothSelectionFilter::name()
{
    return kundo2_i18n("Smooth Selection");
}

QRect KisSmoothSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    const qint32 radius = 1;
    return rect.adjusted(-radius, -radius, radius, radius);
}

void KisSmoothSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    // Simple convolution filter to smooth a mask (1bpp)
    quint8      *buf[3];

    qint32 width = rect.width();
    qint32 height = rect.height();


    quint8* out = new quint8[width];
    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];


    // load top of image
    pixelSelection->readBytes(buf[0] + 1, rect.x(), rect.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy(buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < height; y++) {
        if (y + 1 < height) {
            pixelSelection->readBytes(buf[2] + 1, rect.x(), rect.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        } else {
            memcpy(buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++) {
            qint32 value = (buf[0][x] + buf[0][x+1] + buf[0][x+2] +
                            buf[1][x] + buf[2][x+1] + buf[1][x+2] +
                            buf[2][x] + buf[1][x+1] + buf[2][x+2]);

            out[x] = value / 9;
        }

        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, width, 1);
        rotatePointers(buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];
    delete[] out;
}


KUndo2MagicString KisInvertSelectionFilter::name()
{
    return kundo2_i18n("Invert Selection");
}

QRect KisInvertSelectionFilter::changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(rect);
    return defaultBounds->bounds();
}

void KisInvertSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    Q_UNUSED(rect);

    const QRect imageRect = pixelSelection->defaultBounds()->bounds();
    const QRect selectionRect = pixelSelection->selectedExactRect();

    /**
     * A special treatment for the user-visible selection inversion:
     *
     * If the selection is fully contained inside the image, then
     * just invert it pixel-by-pixel without changing the default
     * pixel. It makes it selectedExactRect() work a little bit more
     * expected for the user (see bug 457820).
     *
     * If the selection spreads outside the image bounds, then
     * just invert it in a mathematical way adjusting the default
     * pixel.
     */

    if (!imageRect.contains(selectionRect)) {
        pixelSelection->invert();
    } else {
        KisSequentialIterator it(pixelSelection, imageRect);
        while(it.nextPixel()) {
            *(it.rawData()) = MAX_SELECTED - *(it.rawData());
        }
        pixelSelection->crop(imageRect);
        pixelSelection->invalidateOutlineCache();
    }
}

constexpr qint32 KisAntiAliasSelectionFilter::offsets[numSteps];

KUndo2MagicString KisAntiAliasSelectionFilter::name()
{
    return kundo2_i18n("Anti-Alias Selection");
}

bool KisAntiAliasSelectionFilter::getInterpolationValue(qint32 negativeSpanEndDistance,
                                                        qint32 positiveSpanEndDistance,
                                                        qint32 negativePixelDiff,
                                                        qint32 positivePixelDiff,
                                                        qint32 currentPixelDiff,
                                                        bool negativeSpanExtremeValid,
                                                        bool positiveSpanExtremeValid,
                                                        qint32 *interpolationValue) const
{
    // Since we search a limited number of steps in each direction of the
    // current pixel, the end pixel of the span may still belong to the edge.
    // So we check for that, and if that's the case we must not smooth the
    // current pixel 
    const bool pixelDiffLessThanZero = currentPixelDiff < 0;
    quint32 distance;
    if (negativeSpanEndDistance < positiveSpanEndDistance) {
        if (!negativeSpanExtremeValid) {
            return false;
        }
        // The pixel is closer to the negative end
        const bool spanEndPixelDiffLessThanZero = negativePixelDiff < 0;
        if (pixelDiffLessThanZero == spanEndPixelDiffLessThanZero) {
            return false;
        }
        distance = negativeSpanEndDistance;
    } else {
        if (!positiveSpanExtremeValid) {
            return false;
        }
        // The pixel is closer to the positive end
        const bool spanEndPixelDiffLessThanZero = positivePixelDiff < 0;
        if (pixelDiffLessThanZero == spanEndPixelDiffLessThanZero) {
            return false;
        }
        distance = positiveSpanEndDistance;
    }
    const qint32 spanLength = positiveSpanEndDistance + negativeSpanEndDistance;
    *interpolationValue = ((distance << 8) / spanLength) + 128;
    return *interpolationValue >= 0;
}

void KisAntiAliasSelectionFilter::findSpanExtreme(quint8 **scanlines, qint32 x, qint32 pixelOffset,
                                                  qint32 rowMultiplier, qint32 colMultiplier, qint32 direction,
                                                  qint32 pixelAvg, qint32 scaledGradient, qint32 currentPixelDiff,
                                                  qint32 *spanEndDistance, qint32 *pixelDiff, bool *spanExtremeValid) const
{
    *spanEndDistance = 0;
    *spanExtremeValid = true;
    for (qint32 i = 0; i < numSteps; ++i) {
        *spanEndDistance += offsets[i];
        const qint32 row1 = currentScanlineIndex + (direction * *spanEndDistance * rowMultiplier);
        const qint32 col1 = x + horizontalBorderSize + (direction * *spanEndDistance * colMultiplier);
        const qint32 row2 = row1 + pixelOffset * colMultiplier;
        const qint32 col2 = col1 + pixelOffset * rowMultiplier;
        const quint8 *pixel1 = scanlines[row1] + col1;
        const quint8 *pixel2 = scanlines[row2] + col2;
        // Get how different are these edge pixels from the current pixels and
        // stop searching if they are too different
        *pixelDiff = ((*pixel1 + *pixel2) >> 1) - pixelAvg;
        if (qAbs(*pixelDiff) > scaledGradient) {
            // If this is the end of the span then check if the corner belongs
            // to a jagged border or to a right angled part of the shape
            qint32 pixelDiff2;
            if ((currentPixelDiff < 0 && *pixelDiff < 0) || (currentPixelDiff > 0 && *pixelDiff > 0)) {
                const qint32 row3 = row2 + pixelOffset * colMultiplier;
                const qint32 col3 = col2 + pixelOffset * rowMultiplier;
                const quint8 *pixel3 = scanlines[row3] + col3;
                pixelDiff2 = ((*pixel2 + *pixel3) >> 1) - pixelAvg;
            } else {
                const qint32 row3 = row1 - pixelOffset * colMultiplier;
                const qint32 col3 = col1 - pixelOffset * rowMultiplier;
                const quint8 *pixel3 = scanlines[row3] + col3;
                pixelDiff2 = ((*pixel1 + *pixel3) >> 1) - pixelAvg;
            }
            *spanExtremeValid = !(qAbs(pixelDiff2) > scaledGradient);
            break;
        }
    }
}

void KisAntiAliasSelectionFilter::findSpanExtremes(quint8 **scanlines, qint32 x, qint32 pixelOffset,
                                                   qint32 rowMultiplier, qint32 colMultiplier,
                                                   qint32 pixelAvg, qint32 scaledGradient, qint32 currentPixelDiff,
                                                   qint32 *negativeSpanEndDistance, qint32 *positiveSpanEndDistance,
                                                   qint32 *negativePixelDiff, qint32 *positivePixelDiff,
                                                   bool *negativeSpanExtremeValid, bool *positiveSpanExtremeValid) const
{
    findSpanExtreme(scanlines, x, pixelOffset, rowMultiplier, colMultiplier, -1, pixelAvg, scaledGradient,
                    currentPixelDiff, negativeSpanEndDistance, negativePixelDiff, negativeSpanExtremeValid);
    findSpanExtreme(scanlines, x, pixelOffset, rowMultiplier, colMultiplier, 1, pixelAvg, scaledGradient,
                    currentPixelDiff, positiveSpanEndDistance, positivePixelDiff, positiveSpanExtremeValid);
}

void KisAntiAliasSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect &rect)
{
    const quint8 defaultPixel = *pixelSelection->defaultPixel().data();
    // Size of a scanline
    const quint32 bytesPerScanline = rect.width() + 2 * horizontalBorderSize;
    // Size of a scanline padded to a multiple of 8
    const quint32 bytesPerPaddedScanline = ((bytesPerScanline + 7) / 8) * 8;

    // This buffer contains the number of consecutive scanlines needed to
    // process the current scanline
    QVector<quint8> buffer(bytesPerPaddedScanline * numberOfScanlines);

    // These pointers point to the individual scanlines in the buffer
    quint8 *scanlines[numberOfScanlines];
    for (quint32 i = 0; i < numberOfScanlines; ++i) {
        scanlines[i] = buffer.data() + i * bytesPerPaddedScanline;
    }

    // Initialize the scanlines
    // Set the border scanlines on the top
    for (qint32 i = 0; i < verticalBorderSize; ++i) {
        memset(scanlines[i], defaultPixel, bytesPerScanline);
    }
    // Copy the first scanlines of the image
    const quint32 numberOfFirstRows = qMin(rect.height(), numberOfScanlines - verticalBorderSize);
    for (quint32 i = verticalBorderSize; i < verticalBorderSize + numberOfFirstRows; ++i) {
        // Set the border pixels on the left
        memset(scanlines[i], defaultPixel, horizontalBorderSize);
        // Copy the pixel data
        pixelSelection->readBytes(scanlines[i] + horizontalBorderSize, rect.x(), rect.y() + i - verticalBorderSize, rect.width(), 1);
        // Set the border pixels on the right
        memset(scanlines[i] + horizontalBorderSize + rect.width(), defaultPixel, horizontalBorderSize);
    }
    // Set the border scanlines on the bottom
    if (verticalBorderSize + numberOfFirstRows < numberOfScanlines) {
        for (quint32 i = verticalBorderSize + numberOfFirstRows; i < numberOfScanlines; ++i) {
            memset(scanlines[i], defaultPixel, bytesPerScanline);
        }
    }
    // Buffer that contains the current output scanline
    QVector<quint8> antialiasedScanline(rect.width());
    // Main loop
    for (int y = 0; y < rect.height(); ++y)
    {
        // Move to the next scanline
        if (y > 0) {
            // Update scanline pointers
            std::rotate(std::begin(scanlines), std::begin(scanlines) + 1, std::end(scanlines));
            // Copy the next scanline
            if (y < rect.height() - verticalBorderSize) {
                // Set the border pixels on the left
                memset(scanlines[numberOfScanlines - 1], defaultPixel, horizontalBorderSize);
                // Copy the pixel data
                pixelSelection->readBytes(scanlines[numberOfScanlines - 1] + horizontalBorderSize, rect.x(), rect.y() + y + verticalBorderSize, rect.width(), 1);
                // Set the border pixels on the right
                memset(scanlines[numberOfScanlines - 1] + horizontalBorderSize + rect.width(), defaultPixel, horizontalBorderSize);
            } else {
                memset(scanlines[numberOfScanlines - 1], defaultPixel, bytesPerScanline);
            }
        }
        // Process the pixels in the current scanline
        for (int x = 0; x < rect.width(); ++x)
        {
            // Get the current pixel and neighbors
            quint8 *pixelPtrM = scanlines[currentScanlineIndex    ] + x + horizontalBorderSize;
            quint8 *pixelPtrN = scanlines[currentScanlineIndex - 1] + x + horizontalBorderSize;
            quint8 *pixelPtrS = scanlines[currentScanlineIndex + 1] + x + horizontalBorderSize;
            const qint32 pixelNW = *(pixelPtrN - 1);
            const qint32 pixelN  = *(pixelPtrN    );
            const qint32 pixelNE = *(pixelPtrN + 1);
            const qint32 pixelW  = *(pixelPtrM - 1);
            const qint32 pixelM  = *(pixelPtrM    );
            const qint32 pixelE  = *(pixelPtrM + 1);
            const qint32 pixelSW = *(pixelPtrS - 1);
            const qint32 pixelS  = *(pixelPtrS    );
            const qint32 pixelSE = *(pixelPtrS + 1);
            // Get the gradients
            const qint32 rowNSum = (pixelNW >> 2) + (pixelN >> 1) + (pixelNE >> 2);
            const qint32 rowMSum = (pixelW  >> 2) + (pixelM >> 1) + (pixelE  >> 2);
            const qint32 rowSSum = (pixelSW >> 2) + (pixelS >> 1) + (pixelSE >> 2);
            const qint32 colWSum = (pixelNW >> 2) + (pixelW >> 1) + (pixelSW >> 2);
            const qint32 colMSum = (pixelN  >> 2) + (pixelM >> 1) + (pixelS  >> 2);
            const qint32 colESum = (pixelNE >> 2) + (pixelE >> 1) + (pixelSE >> 2);
            const qint32 gradientN = qAbs(rowMSum - rowNSum);
            const qint32 gradientS = qAbs(rowSSum - rowMSum);
            const qint32 gradientW = qAbs(colMSum - colWSum);
            const qint32 gradientE = qAbs(colESum - colMSum);
            // Get the maximum gradient
            const qint32 maxGradientNS = qMax(gradientN, gradientS);
            const qint32 maxGradientWE = qMax(gradientW, gradientE);
            const qint32 maxGradient = qMax(maxGradientNS, maxGradientWE);
            // Return early if the gradient is bellow some threshold (given by
            // the value bellow which the jagged edge is not noticeable)
            if (maxGradient < edgeThreshold) {
                antialiasedScanline[x] = pixelM;
                continue;
            }
            // Collect some info about the pixel and neighborhood
            qint32 neighborPixel, gradient;
            qint32 pixelOffset, rowMultiplier, colMultiplier;
            if (maxGradientNS > maxGradientWE) {
                // Horizontal span
                if (gradientN > gradientS) {
                    // The edge is formed with the top pixel
                    neighborPixel = pixelN;
                    gradient = gradientN;
                    pixelOffset = -1;
                } else {
                    // The edge is formed with the bottom pixel
                    neighborPixel = pixelS;
                    gradient = gradientS;
                    pixelOffset = 1;
                }
                rowMultiplier = 0;
                colMultiplier = 1;
            } else {
                // Vertical span
                if (gradientW > gradientE) {
                    // The edge is formed with the left pixel
                    neighborPixel = pixelW;
                    gradient = gradientW;
                    pixelOffset = -1;
                } else {
                    // The edge is formed with the right pixel
                    neighborPixel = pixelE;
                    gradient = gradientE;
                    pixelOffset = 1;
                }
                rowMultiplier = 1;
                colMultiplier = 0;
            }
            // Find the span extremes
            const qint32 pixelAvg = (neighborPixel + pixelM) >> 1;
            const qint32 currentPixelDiff = pixelM - pixelAvg;
            qint32 negativePixelDiff, positivePixelDiff;
            qint32 negativeSpanEndDistance, positiveSpanEndDistance;
            bool negativeSpanExtremeValid, positiveSpanExtremeValid;
            findSpanExtremes(scanlines, x, pixelOffset,
                             rowMultiplier, colMultiplier,
                             pixelAvg, gradient >> 2, currentPixelDiff,
                             &negativeSpanEndDistance, &positiveSpanEndDistance,
                             &negativePixelDiff, &positivePixelDiff,
                             &negativeSpanExtremeValid, &positiveSpanExtremeValid);
            // Get the interpolation value for this pixel given the span extent
            // and perform linear interpolation between the current pixel and
            // the edge neighbor
            qint32 interpolationValue;
            if (!getInterpolationValue(negativeSpanEndDistance, positiveSpanEndDistance,
                                       negativePixelDiff, positivePixelDiff, currentPixelDiff,
                                       negativeSpanExtremeValid, positiveSpanExtremeValid, &interpolationValue)) {
                antialiasedScanline[x] = pixelM;
            } else {
                antialiasedScanline[x] = neighborPixel + ((pixelM - neighborPixel) * interpolationValue >> 8);
            }
        }
        // Copy the scanline data to the mask
        pixelSelection->writeBytes(antialiasedScanline.data(), rect.x(), rect.y() + y, rect.width(), 1);
    }
}

KisGrowUntilDarkestPixelSelectionFilter::KisGrowUntilDarkestPixelSelectionFilter(qint32 radius,
                                                                                 KisPaintDeviceSP referenceDevice)
    : m_radius(radius)
    , m_referenceDevice(referenceDevice)
{
}

KUndo2MagicString KisGrowUntilDarkestPixelSelectionFilter::name()
{
    return kundo2_i18n("Grow Selection Until Darkest Pixel");
}

QRect KisGrowUntilDarkestPixelSelectionFilter::changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    return rect.adjusted(-m_radius, -m_radius, m_radius, m_radius);
}

void KisGrowUntilDarkestPixelSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect &rect)
{
    // Copy the original selection. We will grow this adaptively until the
    // darkest or mor opaque pixels or until the maximum grow is reached.
    KisPixelSelectionSP mask = new KisPixelSelection(*pixelSelection);
    // Grow the original selection normally. At the end this selection will be
    // masked with the adaptively grown mask. We cannot grow adaptively this
    // selection directly since it may have semi-transparent or soft edges.
    // Those need to be retained in the final selection. This normally grown
    // selection is also used as a stop condition for the adaptive mask, which
    // cannot grow pass the limits of the this normally grown selection.
    KisGrowSelectionFilter growFilter(m_radius, m_radius);
    growFilter.process(pixelSelection, rect);

    const qint32 maskScanLineSize = rect.width();
    const KoColorSpace *referenceColorSpace = m_referenceDevice->colorSpace();
    const qint32 referencePixelSize = referenceColorSpace->pixelSize();
    const qint32 referenceScanLineSize = maskScanLineSize * referencePixelSize;
    // Some buffers to store the working scanlines
    QVector<quint8> maskBuffer(maskScanLineSize * 2);
    QVector<quint8> referenceBuffer(referenceScanLineSize * 2);
    QVector<quint8> selectionBuffer(maskScanLineSize);
    quint8 *maskScanLines[2] = {maskBuffer.data(), maskBuffer.data() + maskScanLineSize};
    quint8 *referenceScanLines[2] = {referenceBuffer.data(), referenceBuffer.data() + referenceScanLineSize};
    quint8 *selectionScanLine = selectionBuffer.data();
    // Helper function to test if a pixel can be selected
    auto testSelectPixel = 
        [referenceColorSpace]
        (quint8 pixelOpacity, quint8 pixelIntensity,
         const quint8 *testMaskPixel, const quint8 *testReferencePixel) -> bool
        {
            if (*testMaskPixel) {
                const quint8 testOpacity = referenceColorSpace->opacityU8(testReferencePixel);
                if (pixelOpacity >= testOpacity) {
                    // Special case for when the neighbor pixel is fully transparent.
                    // In that case do not compare the intensity
                    if (testOpacity == MIN_SELECTED) {
                        return true;
                    }
                    // If the opacity test passes we still have to perform the
                    // intensity test
                    const quint8 testIntensity = referenceColorSpace->intensity8(testReferencePixel);
                    if (pixelIntensity <= testIntensity) {
                        return true;
                    }
                }
            }
            return false;
        };

    // Top-left to bottom-right pass
    // First row
    {
        mask->readBytes(maskScanLines[1], rect.left(), rect.top(), maskScanLineSize, 1);
        m_referenceDevice->readBytes(referenceScanLines[1], rect.left(), rect.top(), maskScanLineSize, 1);
        pixelSelection->readBytes(selectionScanLine, rect.left(), rect.top(), maskScanLineSize, 1);
        quint8 *currentMaskScanLineBegin = maskScanLines[1];
        quint8 *currentMaskScanLineEnd = maskScanLines[1] + maskScanLineSize;
        quint8 *currentReferenceScanLineBegin = referenceScanLines[1];
        quint8 *currentSelectionScanLineBegin = selectionScanLine;
        // First pixel
        ++currentMaskScanLineBegin;
        currentReferenceScanLineBegin += referencePixelSize;
        ++currentSelectionScanLineBegin;
        // Rest of pixels
        while (currentMaskScanLineBegin != currentMaskScanLineEnd) {
            if (*currentMaskScanLineBegin == MIN_SELECTED && *currentSelectionScanLineBegin != MIN_SELECTED) {
                const quint8 currentOpacity = referenceColorSpace->opacityU8(currentReferenceScanLineBegin);
                const quint8 currentIntensity = referenceColorSpace->intensity8(currentReferenceScanLineBegin);

                bool pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                       currentMaskScanLineBegin - 1,
                                                       currentReferenceScanLineBegin - referencePixelSize);
                if (pixelIsSelected) {
                    *currentMaskScanLineBegin = MAX_SELECTED;
                }
            }
            ++currentMaskScanLineBegin;
            currentReferenceScanLineBegin += referencePixelSize;
            ++currentSelectionScanLineBegin;
        }
        mask->writeBytes(maskScanLines[1], rect.left(), rect.top(), maskScanLineSize, 1);
    }
    // Rest of rows
    for (qint32 y = rect.top() + 1; y <= rect.bottom(); ++y) {
        rotatePointers(maskScanLines, 2);
        rotatePointers(referenceScanLines, 2);
        mask->readBytes(maskScanLines[1], rect.left(), y, maskScanLineSize, 1);
        m_referenceDevice->readBytes(referenceScanLines[1], rect.left(), y, maskScanLineSize, 1);
        pixelSelection->readBytes(selectionScanLine, rect.left(), y, maskScanLineSize, 1);
        quint8 *currentMaskScanLineBegin = maskScanLines[1];
        quint8 *currentMaskScanLineEnd = maskScanLines[1] + maskScanLineSize;
        quint8 *currentReferenceScanLineBegin = referenceScanLines[1];
        quint8 *topMaskScanLineBegin = maskScanLines[0];
        quint8 *topReferenceScanLineBegin = referenceScanLines[0];
        quint8 *currentSelectionScanLineBegin = selectionScanLine;
        // First pixel
        {
            if (*currentMaskScanLineBegin == MIN_SELECTED && *currentSelectionScanLineBegin != MIN_SELECTED) {
                const quint8 currentOpacity = referenceColorSpace->opacityU8(currentReferenceScanLineBegin);
                const quint8 currentIntensity = referenceColorSpace->intensity8(currentReferenceScanLineBegin);

                bool pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                       topMaskScanLineBegin,
                                                       topReferenceScanLineBegin);
                if (!pixelIsSelected) {
                    pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                        topMaskScanLineBegin + 1,
                                                        topReferenceScanLineBegin + referencePixelSize);
                }
                if (pixelIsSelected) {
                    *currentMaskScanLineBegin = MAX_SELECTED;
                }
            }
            ++currentMaskScanLineBegin;
            currentReferenceScanLineBegin += referencePixelSize;
            ++topMaskScanLineBegin;
            topReferenceScanLineBegin += referencePixelSize;
            ++currentSelectionScanLineBegin;
        }
        // Rest of pixels
        while (currentMaskScanLineBegin != (currentMaskScanLineEnd - 1)) {
            if (*currentMaskScanLineBegin == MIN_SELECTED && *currentSelectionScanLineBegin != MIN_SELECTED) {
                const quint8 currentOpacity = referenceColorSpace->opacityU8(currentReferenceScanLineBegin);
                const quint8 currentIntensity = referenceColorSpace->intensity8(currentReferenceScanLineBegin);

                bool pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity, 
                                                       topMaskScanLineBegin - 1,
                                                       topReferenceScanLineBegin - referencePixelSize);
                if (!pixelIsSelected) {
                    pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                      topMaskScanLineBegin,
                                                      topReferenceScanLineBegin);
                    if (!pixelIsSelected) {
                        pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                          topMaskScanLineBegin + 1,
                                                          topReferenceScanLineBegin + referencePixelSize);
                        if (!pixelIsSelected) {
                            pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                              currentMaskScanLineBegin - 1,
                                                              currentReferenceScanLineBegin - referencePixelSize);
                        }
                    }
                }
                if (pixelIsSelected) {
                    *currentMaskScanLineBegin = MAX_SELECTED;
                }
            }
            ++currentMaskScanLineBegin;
            currentReferenceScanLineBegin += referencePixelSize;
            ++topMaskScanLineBegin;
            topReferenceScanLineBegin += referencePixelSize;
            ++currentSelectionScanLineBegin;
        }
        // Last pixel
        {
            if (*currentMaskScanLineBegin == MIN_SELECTED && *currentSelectionScanLineBegin != MIN_SELECTED) {
                const quint8 currentOpacity = referenceColorSpace->opacityU8(currentReferenceScanLineBegin);
                const quint8 currentIntensity = referenceColorSpace->intensity8(currentReferenceScanLineBegin);

                bool pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity, 
                                                       topMaskScanLineBegin - 1,
                                                       topReferenceScanLineBegin - referencePixelSize);
                if (!pixelIsSelected) {
                    pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                      topMaskScanLineBegin,
                                                      topReferenceScanLineBegin);
                    if (!pixelIsSelected) {
                        pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                          currentMaskScanLineBegin - 1,
                                                          currentReferenceScanLineBegin - referencePixelSize);
                    }
                }
                if (pixelIsSelected) {
                    *currentMaskScanLineBegin = MAX_SELECTED;
                }
            }
        }
        mask->writeBytes(maskScanLines[1], rect.left(), y, maskScanLineSize, 1);
    }

    // Bottom-right to top-left pass
    // Last row
    {
        mask->readBytes(maskScanLines[1], rect.left(), rect.bottom(), maskScanLineSize, 1);
        m_referenceDevice->readBytes(referenceScanLines[1], rect.left(), rect.bottom(), maskScanLineSize, 1);
        pixelSelection->readBytes(selectionScanLine, rect.left(), rect.bottom(), maskScanLineSize, 1);
        quint8 *currentMaskScanLineBegin = maskScanLines[1] + maskScanLineSize - 1;
        quint8 *currentMaskScanLineEnd = maskScanLines[1] - 1;
        quint8 *currentReferenceScanLineBegin = referenceScanLines[1] + referenceScanLineSize - referencePixelSize;
        quint8 *currentSelectionScanLineBegin = selectionScanLine + maskScanLineSize - 1;
        // Last pixel
        --currentMaskScanLineBegin;
        currentReferenceScanLineBegin -= referencePixelSize;
        --currentSelectionScanLineBegin;
        // Rest of pixels
        while (currentMaskScanLineBegin != currentMaskScanLineEnd) {
            if (*currentMaskScanLineBegin == MIN_SELECTED && *currentSelectionScanLineBegin != MIN_SELECTED) {
                const quint8 currentOpacity = referenceColorSpace->opacityU8(currentReferenceScanLineBegin);
                const quint8 currentIntensity = referenceColorSpace->intensity8(currentReferenceScanLineBegin);

                bool pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                       currentMaskScanLineBegin + 1,
                                                       currentReferenceScanLineBegin + referencePixelSize);
                if (pixelIsSelected) {
                    *currentMaskScanLineBegin = MAX_SELECTED;
                }
            }
            --currentMaskScanLineBegin;
            currentReferenceScanLineBegin -= referencePixelSize;
            --currentSelectionScanLineBegin;
        }
        mask->writeBytes(maskScanLines[1], rect.left(), rect.top(), maskScanLineSize, 1);
    }
    // Rest of rows
    for (qint32 y = rect.bottom() - 1; y >= rect.top(); --y) {
        rotatePointers(maskScanLines, 2);
        rotatePointers(referenceScanLines, 2);
        mask->readBytes(maskScanLines[1], rect.left(), y, maskScanLineSize, 1);
        m_referenceDevice->readBytes(referenceScanLines[1], rect.left(), y, maskScanLineSize, 1);
        pixelSelection->readBytes(selectionScanLine, rect.left(), y, maskScanLineSize, 1);
        quint8 *currentMaskScanLineBegin = maskScanLines[1] + maskScanLineSize - 1;
        quint8 *currentMaskScanLineEnd = maskScanLines[1] - 1;
        quint8 *currentReferenceScanLineBegin = referenceScanLines[1] + referenceScanLineSize - referencePixelSize;
        quint8 *bottomMaskScanLineBegin = maskScanLines[0] + maskScanLineSize - 1;
        quint8 *bottomReferenceScanLineBegin = referenceScanLines[0] + referenceScanLineSize - referencePixelSize;
        quint8 *currentSelectionScanLineBegin = selectionScanLine + maskScanLineSize - 1;
        // Last pixel
        {
            if (*currentMaskScanLineBegin == MIN_SELECTED && *currentSelectionScanLineBegin != MIN_SELECTED) {
                const quint8 currentOpacity = referenceColorSpace->opacityU8(currentReferenceScanLineBegin);
                const quint8 currentIntensity = referenceColorSpace->intensity8(currentReferenceScanLineBegin);

                bool pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                       bottomMaskScanLineBegin,
                                                       bottomReferenceScanLineBegin);
                if (!pixelIsSelected) {
                    pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                      bottomMaskScanLineBegin - 1,
                                                      bottomReferenceScanLineBegin - referencePixelSize);
                }
                if (pixelIsSelected) {
                    *currentMaskScanLineBegin = MAX_SELECTED;
                }
            }
            --currentMaskScanLineBegin;
            currentReferenceScanLineBegin -= referencePixelSize;
            --bottomMaskScanLineBegin;
            bottomReferenceScanLineBegin -= referencePixelSize;
            --currentSelectionScanLineBegin;
        }
        // Rest of pixels
        while (currentMaskScanLineBegin != (currentMaskScanLineEnd + 1)) {
            if (*currentMaskScanLineBegin == MIN_SELECTED && *currentSelectionScanLineBegin != MIN_SELECTED) {
                const quint8 currentOpacity = referenceColorSpace->opacityU8(currentReferenceScanLineBegin);
                const quint8 currentIntensity = referenceColorSpace->intensity8(currentReferenceScanLineBegin);

                bool pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity, 
                                                       bottomMaskScanLineBegin + 1,
                                                       bottomReferenceScanLineBegin + referencePixelSize);
                if (!pixelIsSelected) {
                    pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                      bottomMaskScanLineBegin,
                                                      bottomReferenceScanLineBegin);
                    if (!pixelIsSelected) {
                        pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                          bottomMaskScanLineBegin - 1,
                                                          bottomReferenceScanLineBegin - referencePixelSize);
                        if (!pixelIsSelected) {
                            pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                              currentMaskScanLineBegin + 1,
                                                              currentReferenceScanLineBegin + referencePixelSize);
                        }
                    }
                }
                if (pixelIsSelected) {
                    *currentMaskScanLineBegin = MAX_SELECTED;
                }
            }
            --currentMaskScanLineBegin;
            currentReferenceScanLineBegin -= referencePixelSize;
            --bottomMaskScanLineBegin;
            bottomReferenceScanLineBegin -= referencePixelSize;
            --currentSelectionScanLineBegin;
        }
        // First pixel
        {
            if (*currentMaskScanLineBegin == MIN_SELECTED && *currentSelectionScanLineBegin != MIN_SELECTED) {
                const quint8 currentOpacity = referenceColorSpace->opacityU8(currentReferenceScanLineBegin);
                const quint8 currentIntensity = referenceColorSpace->intensity8(currentReferenceScanLineBegin);

                bool pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity, 
                                                       bottomMaskScanLineBegin + 1,
                                                       bottomReferenceScanLineBegin + referencePixelSize);
                if (!pixelIsSelected) {
                    pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                      bottomMaskScanLineBegin,
                                                      bottomReferenceScanLineBegin);
                    if (!pixelIsSelected) {
                        pixelIsSelected = testSelectPixel(currentOpacity, currentIntensity,
                                                            currentMaskScanLineBegin + 1,
                                                            currentReferenceScanLineBegin + referencePixelSize);
                    }
                }
                if (pixelIsSelected) {
                    *currentMaskScanLineBegin = MAX_SELECTED;
                }
            }
        }
        mask->writeBytes(maskScanLines[1], rect.left(), y, maskScanLineSize, 1);
    }

    // Combine the adaptively grown mask with the normally grown mask. The
    // adaptively grown mask is used as a binary mask to erase some of the
    // pixels of the normally grown mask
    {
        KisSequentialConstIterator it1(mask, rect);
        KisSequentialIterator it2(pixelSelection, rect);
        while (it1.nextPixel() && it2.nextPixel()) {
            *it2.rawData() *= (*it1.rawDataConst() != MIN_SELECTED);
        }
    }
}
