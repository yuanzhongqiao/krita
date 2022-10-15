/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestSvgText.h"

#include <simpletest.h>

#include "SvgParserTestingUtils.h"
#include <text/KoSvgText.h>
#include <text/KoSvgTextProperties.h>
#include <text/KoFontRegistery.h>
#include <text/KoCssTextUtils.h>
#include "KoSvgTextShapeMarkupConverter.h"

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <QFont>

void addProp(SvgLoadingContext &context,
             KoSvgTextProperties &props,
             const QString &attribute,
             const QString &value,
             KoSvgTextProperties::PropertyId id,
             int newValue)
{
    props.parseSvgTextAttribute(context, attribute, value);
    if (props.property(id).toInt() != newValue) {
        qDebug() << "Failed to load the property:";
        qDebug() << ppVar(attribute) << ppVar(value);
        qDebug() << ppVar(newValue);
        qDebug() << ppVar(props.property(id));
        QFAIL("Fail :(");
    }
}

void addProp(SvgLoadingContext &context,
             KoSvgTextProperties &props,
             const QString &attribute,
             const QString &value,
             KoSvgTextProperties::PropertyId id,
             KoSvgText::AutoValue newValue)
{
    props.parseSvgTextAttribute(context, attribute, value);
    if (props.property(id).value<KoSvgText::AutoValue>() != newValue) {
        qDebug() << "Failed to load the property:";
        qDebug() << ppVar(attribute) << ppVar(value);
        qDebug() << ppVar(newValue);
        qDebug() << ppVar(props.property(id));
        QFAIL("Fail :(");
    }
    QCOMPARE(props.property(id), QVariant::fromValue(newValue));
}

void addProp(SvgLoadingContext &context,
             KoSvgTextProperties &props,
             const QString &attribute,
             const QString &value,
             KoSvgTextProperties::PropertyId id,
             qreal newValue)
{
    props.parseSvgTextAttribute(context, attribute, value);
    if (props.property(id).toReal() != newValue) {
        qDebug() << "Failed to load the property:";
        qDebug() << ppVar(attribute) << ppVar(value);
        qDebug() << ppVar(newValue);
        qDebug() << ppVar(props.property(id));
        QFAIL("Fail :(");
    }
}

void TestSvgText::initTestCase()
{
    /// The test initialization function sets Qt::AA_Use96Dpi
    /// application attribute, but it doesn't affect the font
    /// that has already been set as the default application
    /// font.
    qApp->setFont(QFont("sans", 10));
}

void TestSvgText::testTextProperties()
{
    KoDocumentResourceManager resourceManager;
    SvgLoadingContext context(&resourceManager);
    context.pushGraphicsContext();

    KoSvgTextProperties props;

    addProp(context,
            props,
            "writing-mode",
            "tb-rl",
            KoSvgTextProperties::WritingModeId,
            KoSvgText::VerticalRL);
    addProp(context,
            props,
            "writing-mode",
            "rl",
            KoSvgTextProperties::WritingModeId,
            KoSvgText::HorizontalTB);

    // According to https://www.w3.org/TR/css-writing-modes-3/#glyph-orientation
    // glyph-orientation is only to be converted to text orientation in the 0 and 90 cases.

    //   "UAs must ignore and treat as invalid any other values for the glyph-orientation-vertical
    //   property; and treat as invalid the glyph-orientation-horizontal property in its entirety."
    addProp(context,
            props,
            "glyph-orientation-vertical",
            "auto",
            KoSvgTextProperties::TextOrientationId,
            KoSvgText::OrientationMixed);
    addProp(context,
            props,
            "glyph-orientation-vertical",
            "0",
            KoSvgTextProperties::TextOrientationId,
            KoSvgText::OrientationUpright);
    addProp(context,
            props,
            "glyph-orientation-vertical",
            "90",
            KoSvgTextProperties::TextOrientationId,
            KoSvgText::OrientationSideWays);
    // This is confusing, but what now happens is that the tested value is always going to be 'sideways'
    // because the value is ignored.
    int newValueForGlyphOrientation = int(KoSvgText::OrientationSideWays);
    addProp(context,
            props,
            "glyph-orientation-vertical",
            "95",
            KoSvgTextProperties::TextOrientationId,
            newValueForGlyphOrientation);
    addProp(context,
            props,
            "glyph-orientation-vertical",
            "175",
            KoSvgTextProperties::TextOrientationId,
            newValueForGlyphOrientation);
    addProp(context,
            props,
            "glyph-orientation-vertical",
            "280",
            KoSvgTextProperties::TextOrientationId,
            newValueForGlyphOrientation);
    addProp(context,
            props,
            "glyph-orientation-vertical",
            "350",
            KoSvgTextProperties::TextOrientationId,
            newValueForGlyphOrientation);
    addProp(context,
            props,
            "glyph-orientation-vertical",
            "105",
            KoSvgTextProperties::TextOrientationId,
            newValueForGlyphOrientation);

    addProp(context, props, "direction", "rtl", KoSvgTextProperties::DirectionId, KoSvgText::DirectionRightToLeft);
    addProp(context, props, "unicode-bidi", "embed", KoSvgTextProperties::UnicodeBidiId, KoSvgText::BidiEmbed);
    addProp(context, props, "unicode-bidi", "bidi-override", KoSvgTextProperties::UnicodeBidiId, KoSvgText::BidiOverride);


    addProp(context, props, "text-anchor", "middle", KoSvgTextProperties::TextAnchorId, KoSvgText::AnchorMiddle);
    addProp(context,
            props,
            "dominant-baseline",
            "ideographic",
            KoSvgTextProperties::DominantBaselineId,
            KoSvgText::BaselineIdeographic);
    addProp(context,
            props,
            "alignment-baseline",
            "alphabetic",
            KoSvgTextProperties::AlignmentBaselineId,
            KoSvgText::BaselineAlphabetic);
    addProp(context, props, "baseline-shift", "sub", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftSub);
    addProp(context, props, "baseline-shift", "super", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftSuper);
    addProp(context, props, "baseline-shift", "baseline", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftNone);

    addProp(context, props, "baseline-shift", "10%", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftPercentage);
    QCOMPARE(props.property(KoSvgTextProperties::BaselineShiftValueId).toDouble(), 0.1);

    context.currentGC()->textProperties.setProperty(KoSvgTextProperties::FontSizeId, 180.0);

    addProp(context, props, "baseline-shift", "36", KoSvgTextProperties::BaselineShiftModeId, KoSvgText::ShiftPercentage);
    QCOMPARE(props.property(KoSvgTextProperties::BaselineShiftValueId).toDouble(), 3.6);

    addProp(context, props, "kerning", "auto", KoSvgTextProperties::KerningId, KoSvgText::AutoValue());
    addProp(context, props, "kerning", "20", KoSvgTextProperties::KerningId, KoSvgText::AutoValue(20.0));

    addProp(context, props, "letter-spacing", "normal", KoSvgTextProperties::LetterSpacingId, KoSvgText::AutoValue());
    addProp(context, props, "letter-spacing", "20", KoSvgTextProperties::LetterSpacingId, KoSvgText::AutoValue(20.0));

    addProp(context, props, "word-spacing", "normal", KoSvgTextProperties::WordSpacingId, KoSvgText::AutoValue());
    addProp(context, props, "word-spacing", "20", KoSvgTextProperties::WordSpacingId, KoSvgText::AutoValue(20.0));
}

void TestSvgText::testDefaultTextProperties()
{
    KoSvgTextProperties props;

    QVERIFY(props.isEmpty());
    QVERIFY(!props.hasProperty(KoSvgTextProperties::UnicodeBidiId));

    QVERIFY(KoSvgTextProperties::defaultProperties().hasProperty(KoSvgTextProperties::UnicodeBidiId));
    QCOMPARE(KoSvgTextProperties::defaultProperties().property(KoSvgTextProperties::UnicodeBidiId).toInt(), int(KoSvgText::BidiNormal));

    props = KoSvgTextProperties::defaultProperties();

    QVERIFY(props.hasProperty(KoSvgTextProperties::UnicodeBidiId));
    QCOMPARE(props.property(KoSvgTextProperties::UnicodeBidiId).toInt(), int(KoSvgText::BidiNormal));
}

void TestSvgText::testTextPropertiesDifference()
{
    using namespace KoSvgText;

    KoSvgTextProperties props;

    props.setProperty(KoSvgTextProperties::WritingModeId, HorizontalTB);
    props.setProperty(KoSvgTextProperties::DirectionId, DirectionRightToLeft);
    props.setProperty(KoSvgTextProperties::UnicodeBidiId, BidiEmbed);
    props.setProperty(KoSvgTextProperties::TextAnchorId, AnchorEnd);
    props.setProperty(KoSvgTextProperties::DominantBaselineId,
                      BaselineNoChange);
    props.setProperty(KoSvgTextProperties::AlignmentBaselineId,
                      BaselineIdeographic);
    props.setProperty(KoSvgTextProperties::BaselineShiftModeId, ShiftPercentage);
    props.setProperty(KoSvgTextProperties::BaselineShiftValueId, 0.5);
    props.setProperty(KoSvgTextProperties::KerningId, fromAutoValue(AutoValue(10)));
    props.setProperty(KoSvgTextProperties::TextOrientationId,
                      OrientationSideWays);
    props.setProperty(KoSvgTextProperties::LetterSpacingId, fromAutoValue(AutoValue(20)));
    props.setProperty(KoSvgTextProperties::WordSpacingId, fromAutoValue(AutoValue(30)));

    KoSvgTextProperties newProps = props;

    newProps.setProperty(KoSvgTextProperties::KerningId, fromAutoValue(AutoValue(11)));
    newProps.setProperty(KoSvgTextProperties::LetterSpacingId, fromAutoValue(AutoValue(21)));

    KoSvgTextProperties diff = newProps.ownProperties(props);

    QVERIFY(diff.hasProperty(KoSvgTextProperties::KerningId));
    QVERIFY(diff.hasProperty(KoSvgTextProperties::LetterSpacingId));

    QVERIFY(!diff.hasProperty(KoSvgTextProperties::WritingModeId));
    QVERIFY(!diff.hasProperty(KoSvgTextProperties::DirectionId));


}

void TestSvgText::testParseFontStyles()
{
    const QString data =
            "<text x=\"7\" y=\"7\""
            "    font-family=\"Verdana , \'Times New Roman\', serif\" font-size=\"15\" font-style=\"oblique\" fill=\"blue\""
            "    font-stretch=\"extra-condensed\""
            "    font-size-adjust=\"0.56\""
            "    font=\"bold italic large Palatino, serif\"" //we don't support this right now.
            "    font-variant=\"small-caps\" font-weight=\"600\" >"
            "    Hello, out there"
            "</text>";

    QDomDocument doc;
    QVERIFY(doc.setContent(data.toLatin1()));
    QDomElement root = doc.documentElement();

    KoDocumentResourceManager resourceManager;
    SvgLoadingContext context(&resourceManager);
    context.pushGraphicsContext();

    SvgStyles styles = context.styleParser().collectStyles(root);
    context.styleParser().parseFont(styles);

    auto getFont = [&context] () {
        return context.currentGC()->textProperties;
    };

    {
        QStringList expectedFonts = {"Verdana", "Times New Roman", "serif"};
        QCOMPARE(getFont().property(KoSvgTextProperties::FontFamiliesId).toStringList(), expectedFonts);
    }

    QCOMPARE(getFont().property(KoSvgTextProperties::FontSizeId).toReal(), 15.0);
    QCOMPARE(QFont::Style(getFont().property(KoSvgTextProperties::FontStyleId).toInt()), QFont::StyleOblique);
    QCOMPARE(getFont().property(KoSvgTextProperties::FontVariantCapsId).toInt(), KoSvgText::SmallCaps);
    QCOMPARE(getFont().property(KoSvgTextProperties::FontWeightId).toInt(), 600);

    {
        SvgStyles fontModifier;
        fontModifier["font-weight"] = "bolder";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(getFont().property(KoSvgTextProperties::FontWeightId).toInt(), 700);
    }

    {
        SvgStyles fontModifier;
        fontModifier["font-weight"] = "lighter";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(getFont().property(KoSvgTextProperties::FontWeightId).toInt(), 600);
    }

    QCOMPARE(getFont().property(KoSvgTextProperties::FontStretchId).toInt(), int(QFont::ExtraCondensed));

    {
        SvgStyles fontModifier;
        fontModifier["font-stretch"] = "narrower";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(getFont().property(KoSvgTextProperties::FontStretchId).toInt(), int(QFont::UltraCondensed));
    }

    {
        SvgStyles fontModifier;
        fontModifier["font-stretch"] = "wider";
        context.styleParser().parseFont(fontModifier);
        QCOMPARE(getFont().property(KoSvgTextProperties::FontStretchId).toInt(), int(QFont::ExtraCondensed));
    }

    {
        SvgStyles fontModifier;
        fontModifier["text-decoration"] = "underline";
        context.styleParser().parseFont(fontModifier);
        KoSvgText::TextDecorations deco =
            getFont().property(KoSvgTextProperties::TextDecorationLineId).value<KoSvgText::TextDecorations>();
        QCOMPARE(deco.testFlag(KoSvgText::DecorationUnderline), true);
    }

    {
        SvgStyles fontModifier;
        fontModifier["text-decoration"] = "overline";
        context.styleParser().parseFont(fontModifier);
        KoSvgText::TextDecorations deco =
            getFont().property(KoSvgTextProperties::TextDecorationLineId).value<KoSvgText::TextDecorations>();
        QCOMPARE(deco.testFlag(KoSvgText::DecorationOverline), true);
    }

    {
        SvgStyles fontModifier;
        fontModifier["text-decoration"] = "line-through";
        context.styleParser().parseFont(fontModifier);
        KoSvgText::TextDecorations deco =
            getFont().property(KoSvgTextProperties::TextDecorationLineId).value<KoSvgText::TextDecorations>();
        QCOMPARE(deco.testFlag(KoSvgText::DecorationLineThrough), true);
    }

    {
        SvgStyles fontModifier;
        fontModifier["text-decoration"] = " line-through overline";
        context.styleParser().parseFont(fontModifier);
        KoSvgText::TextDecorations deco =
            getFont().property(KoSvgTextProperties::TextDecorationLineId).value<KoSvgText::TextDecorations>();
        QCOMPARE(deco.testFlag(KoSvgText::DecorationUnderline), false);
        QCOMPARE(deco.testFlag(KoSvgText::DecorationLineThrough), true);
        QCOMPARE((deco.testFlag(KoSvgText::DecorationOverline)), true);
    }

}

void TestSvgText::testParseTextStyles()
{
    const QString data =
            "<text x=\"7\" y=\"7\""
            "    font-family=\"Verdana\" font-size=\"15\" font-style=\"oblique\" fill=\"blue\""
            "    writing-mode=\"tb-rl\" "
            "    glyph-orientation-vertical=\"90\" >"
            "    Hello, out there"
            "</text>";

    QDomDocument doc;
    QVERIFY(doc.setContent(data.toLatin1()));
    QDomElement root = doc.documentElement();

    KoDocumentResourceManager resourceManager;
    SvgLoadingContext context(&resourceManager);
    context.pushGraphicsContext();

    SvgStyles styles = context.styleParser().collectStyles(root);
    context.styleParser().parseFont(styles);

    auto getFont = [&context] () {
        return context.currentGC()->textProperties.generateFont();
    };

    QCOMPARE(getFont().family(), QString("Verdana"));

    KoSvgTextProperties &props = context.currentGC()->textProperties;

    QCOMPARE(props.property(KoSvgTextProperties::WritingModeId).toInt(),
             int(KoSvgText::VerticalRL));
    QCOMPARE(props.property(KoSvgTextProperties::TextOrientationId).toInt(),
             int(KoSvgText::OrientationSideWays));
}

#include <text/KoSvgTextShape.h>
#include <text/KoSvgTextChunkShape.h>
#include <text/KoSvgTextChunkShapeLayoutInterface.h>

void TestSvgText::testSimpleText()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/text-test-simple-text.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());


    SvgRenderTester t (data.data());
    t.test_standard("text_simple", QSize(140, 40), 72.0);

    KoShape *shape = t.findShape("testRect");
    KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
    QVERIFY(chunkShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(shape));

    QCOMPARE(chunkShape->shapeCount(), 0);
    QCOMPARE(chunkShape->layoutInterface()->isTextNode(), true);

    QCOMPARE(chunkShape->layoutInterface()->numChars(), 17);
    QCOMPARE(chunkShape->layoutInterface()->nodeText(), QString("Hello, out there!"));

    QVector<KoSvgText::CharTransformation> transform = chunkShape->layoutInterface()->localCharTransformations();
    QCOMPARE(transform.size(), 1);
    QVERIFY(bool(transform[0].xPos));
    QVERIFY(bool(transform[0].yPos));
    QVERIFY(!transform[0].dxPos);
    QVERIFY(!transform[0].dyPos);
    QVERIFY(!transform[0].rotate);

    QCOMPARE(*transform[0].xPos, 7.0);
    QCOMPARE(*transform[0].yPos, 27.0);

    QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
        chunkShape->layoutInterface()->collectSubChunks();

    QCOMPARE(subChunks.size(), 1);
    QCOMPARE(subChunks[0].text.size(), 17);
    //qDebug() << ppVar(subChunks[0].text);
    //qDebug() << ppVar(subChunks[0].transformation);
    //qDebug() << ppVar(subChunks[0].format);

}

inline KoSvgTextChunkShape* toChunkShape(KoShape *shape) {
    KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
    KIS_ASSERT(chunkShape);
    return chunkShape;
}

void TestSvgText::testComplexText()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/text-test-complex-text.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());


    SvgRenderTester t (data.data());
    t.test_standard("text_complex", QSize(370, 56), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

    QCOMPARE(baseShape->shapeCount(), 4);
    QCOMPARE(baseShape->layoutInterface()->isTextNode(), false);
    QCOMPARE(baseShape->layoutInterface()->numChars(), 41);

    {   // chunk 0: "Hello, "
        KoSvgTextChunkShape *chunk = toChunkShape(baseShape->shapes()[0]);

        QCOMPARE(chunk->shapeCount(), 0);
        QCOMPARE(chunk->layoutInterface()->isTextNode(), true);


        QCOMPARE(chunk->layoutInterface()->numChars(), 7);
        QCOMPARE(chunk->layoutInterface()->nodeText(), QString("Hello, "));

        QVector<KoSvgText::CharTransformation> transform = chunk->layoutInterface()->localCharTransformations();
        QCOMPARE(transform.size(), 7);
        QVERIFY(bool(transform[0].xPos));
        QVERIFY(!bool(transform[1].xPos));

        for (int i = 0; i < 7; i++) {
            QVERIFY(!i || bool(transform[i].dxPos));

            if (i) {
                QCOMPARE(*transform[i].dxPos, qreal(i));
            }
        }

        QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
            chunk->layoutInterface()->collectSubChunks();

        QCOMPARE(subChunks.size(), 1); // used to be 7, but we got rid of aggresive subchunking.
        QCOMPARE(subChunks[0].text.size(), 7);
        QCOMPARE(*subChunks[0].transformation.at(0).xPos, 7.0);
        QVERIFY(subChunks[0].transformation.at(0).xPos); //if there's a value it's always set.
    }

    {   // chunk 1: "out"
        KoSvgTextChunkShape *chunk = toChunkShape(baseShape->shapes()[1]);

        QCOMPARE(chunk->shapeCount(), 0);
        QCOMPARE(chunk->layoutInterface()->isTextNode(), true);

        QCOMPARE(chunk->layoutInterface()->numChars(), 3);
        QCOMPARE(chunk->layoutInterface()->nodeText(), QString("out"));

        QVector<KoSvgText::CharTransformation> transform = chunk->layoutInterface()->localCharTransformations();
        QCOMPARE(transform.size(), 2);
        QVERIFY(bool(transform[0].xPos));
        QVERIFY(!bool(transform[1].xPos));

        for (int i = 0; i < 2; i++) {
            QVERIFY(bool(transform[i].dxPos));
            QCOMPARE(*transform[i].dxPos, qreal(i + 7));
        }

        QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
            chunk->layoutInterface()->collectSubChunks();

        QCOMPARE(subChunks.size(), 1);
        QCOMPARE(subChunks[0].text.size(), 3);
    }

    {   // chunk 2: " there "
        KoSvgTextChunkShape *chunk = toChunkShape(baseShape->shapes()[2]);

        QCOMPARE(chunk->shapeCount(), 0);
        QCOMPARE(chunk->layoutInterface()->isTextNode(), true);

        QCOMPARE(chunk->layoutInterface()->numChars(), 7);
        QCOMPARE(chunk->layoutInterface()->nodeText(), QString(" there "));

        QVector<KoSvgText::CharTransformation> transform = chunk->layoutInterface()->localCharTransformations();
        QCOMPARE(transform.size(), 0);

        QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
            chunk->layoutInterface()->collectSubChunks();

        QCOMPARE(subChunks.size(), 1);
        QCOMPARE(subChunks[0].text.size(), 7);
    }

    {   // chunk 3: "cool cdata --> nice work"
        KoSvgTextChunkShape *chunk = toChunkShape(baseShape->shapes()[3]);

        QCOMPARE(chunk->shapeCount(), 0);
        QCOMPARE(chunk->layoutInterface()->isTextNode(), true);

        QCOMPARE(chunk->layoutInterface()->numChars(), 24);
        QCOMPARE(chunk->layoutInterface()->nodeText(), QString("cool cdata --> nice work"));

        QVector<KoSvgText::CharTransformation> transform = chunk->layoutInterface()->localCharTransformations();
        QCOMPARE(transform.size(), 0);

        QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> subChunks =
            chunk->layoutInterface()->collectSubChunks();

        QCOMPARE(subChunks.size(), 1);
        QCOMPARE(subChunks[0].text.size(), 24);
    }
}

/**
 * @brief TestSvgText::testHindiText
 *
 * Test complex text-shaping in Devaganari using FreeSans.
 * Harfbuzz takes care of all of this, but it is a core feature
 * we need to keep an eye on.
 */
void TestSvgText::testHindiText()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/text-test-hindi-text.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/FreeSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("FreeSans").toLatin1());


    SvgRenderTester t (data.data());

    t.setCheckQImagePremultiplied(true);
    t.setFuzzyThreshold(5);

    t.test_standard("text_hindi", QSize(200, 30), 72);
}

/**
 * @brief TestSvgText::testTextBaselineShift
 *
 * This tests the baseline-shift.
 * TODO: Test alignment and dominant baseline?
 */
void TestSvgText::testTextBaselineShift()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/text-test-baseline-shift.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());

    SvgRenderTester t (data.data());

    t.setCheckQImagePremultiplied(true);
    t.test_standard("text_baseline_shift", QSize(180, 40), 72);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

}
/**
 * @brief TestSvgText::testTextSpacing
 *
 * This tests the letter and word spacing CSS properties,
 * as well as the SVG 1.1 kerning property. The latter
 * is considered a on-off function for CSS font-kerning
 * in SVG 2.0, so it will have different result in a SVG
 * 1.1 renderer.
 */
void TestSvgText::testTextSpacing()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/test-text-spacing.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());

    SvgRenderTester t (data.data());
    t.setFuzzyThreshold(5);
    t.test_standard("text_letter_word_spacing", QSize(340, 250), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

}
/**
 * @brief TestSvgText::testTextTabSpacing
 *
 *  Tests tabs being kept as well as tab-size.
 */
void TestSvgText::testTextTabSpacing()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/test-text-tab-spacing.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());

    SvgRenderTester t (data.data());
    t.setFuzzyThreshold(5);
    t.test_standard("text_tab_spacing", QSize(400, 170), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));
}

/**
 * @brief TestSvgText::testTextDecorations
 *
 * tests the text-decorations, but for some reason they don't paint so it's broken :(
 */
void TestSvgText::testTextDecorations()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/test-text-decorations.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());

    SvgRenderTester t (data.data());
    t.setFuzzyThreshold(5);
    t.test_standard("text_decorations", QSize(290, 135), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

}

void TestSvgText::testRightToLeft()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/test-text-right-to-left.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());

    SvgRenderTester t (data.data());
    t.test_standard("text_right_to_left", QSize(500,600), 72.0);

    KoSvgTextChunkShape *baseShape = toChunkShape(t.findShape("testRect"));
    QVERIFY(baseShape);

    // root shape is not just a chunk!
    QVERIFY(dynamic_cast<KoSvgTextShape*>(baseShape));

}

#include <QTextLayout>
#include <QPainter>
#include <QPainterPath>

void TestSvgText::testQtBidi()
{
    // Arabic text sample from Wikipedia:
    // https://ar.wikipedia.org/wiki/%D8%A5%D9%85%D8%A7%D8%B1%D8%A7%D8%AA_%D8%A7%D9%84%D8%B3%D8%A7%D8%AD%D9%84_%D8%A7%D9%84%D9%85%D8%AA%D8%B5%D8%A7%D9%84%D8%AD

    QStringList ltrText;
    ltrText << "aa bb cc dd";
    ltrText << "aa bb حادثتا السفينتين بسين cc dd";
    ltrText << "aa bb \u202ec1c2 d3d4\u202C ee ff";

    QStringList rtlText;
    rtlText << "حادثتا السفينتين «بسين Bassein» و«فايبر Viper»";
    rtlText << "حادثتا السفينتين «بسين aa bb cc dd» و«فايبر Viper»";


    QImage canvas(500,500,QImage::Format_ARGB32);
    QPainter gc(&canvas);
    QPointF pos(15,15);


    QVector<QStringList> textSamples;
    textSamples << ltrText;
    textSamples << rtlText;

    QVector<Qt::LayoutDirection> textDirections;
    textDirections << Qt::LeftToRight;
    textDirections << Qt::RightToLeft;

    for (int i = 0; i < textSamples.size(); i++) {
        Q_FOREACH (const QString str, textSamples[i]) {
            QTextOption option;
            option.setTextDirection(textDirections[i]);
            option.setUseDesignMetrics(true);

            QTextLayout layout;

            layout.setText(str);
            layout.setFont(QFont("serif", 15.0));
            layout.setCacheEnabled(true);
            layout.beginLayout();

            QTextLine line = layout.createLine();
            line.setPosition(pos);
            pos.ry() += 25;
            layout.endLayout();
            layout.draw(&gc, QPointF());
        }
    }

    canvas.save("test_bidi.png");
}

void TestSvgText::testQtDxDy()
{
    QImage canvas(500,500,QImage::Format_ARGB32);
    QPainter gc(&canvas);
    QPointF pos(15,15);

    QTextOption option;
    option.setTextDirection(Qt::LeftToRight);
    option.setUseDesignMetrics(true);
    option.setWrapMode(QTextOption::WrapAnywhere);

    QTextLayout layout;

    layout.setText("aa bb cc dd ee ff");
    layout.setFont(QFont("serif", 15.0));
    layout.setCacheEnabled(true);
    layout.beginLayout();
    layout.setTextOption(option);

    {
        QTextLine line = layout.createLine();
        line.setPosition(pos);
        line.setNumColumns(4);
    }
    pos.ry() += 25;
    pos.rx() += 30;
    {
        QTextLine line = layout.createLine();
        line.setPosition(pos);
    }

    layout.endLayout();
    layout.draw(&gc, QPointF());


    canvas.save("test_dxdy.png");
}

/**
 * @brief testTextOutlineSolid()
 * 
 * Tests whether SVG strokes render correctly for SVG text.
 */
void TestSvgText::testTextOutlineSolid()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/test-text-solid-stroke.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());

    SvgRenderTester t (data.data());
    t.test_standard("text_outline_solid", QSize(30, 30), 72.0);
}

/**
 * @brief testNbspHandling()
 * 
 * Tests whether no-break-spaces (nbsp) are left alone.
 */
void TestSvgText::testNbspHandling()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/test-text-no-break-space.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());

    SvgRenderTester t (data.data());
    t.test_standard("text_nbsp", QSize(30, 30), 72.0);
}

/**
 * @brief testMulticolorText()
 * 
 * Tests whether we can have a piece of text with multiple
 * colors assigned to different parts of the text.
 * 
 * Note: We could make this test a bit more thorough by testing
 * what happens when ligatures straddle a text. According to
 * SVG, all graphemes made up from multiple code-points (ike ligatures)
 * should have the color assigned to the first code-point.
 */
void TestSvgText::testMulticolorText()
{
    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/test-text-multicolor.svg"));
    bool res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QString fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());

    SvgRenderTester t (data.data());
    t.setFuzzyThreshold(5);
    t.test_standard("text_multicolor", QSize(30, 30), 72.0);
}

#include <KoColorBackground.h>

void TestSvgText::testConvertToStrippedSvg()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text transform=\"translate(2)\" id=\"testRect\" x=\"2\" y=\"24\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        S<tspan fill=\"red\">A</tspan><![CDATA[some stuff<><><<<>]]>"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.parser.setResolution(QRectF(QPointF(), QSizeF(30,30)) /* px */, 72.0/* ppi */);
    t.run();

    KoSvgTextShape *baseShape = dynamic_cast<KoSvgTextShape*>(t.findShape("testRect"));
    QVERIFY(baseShape);

    {
        KoColorBackground *bg = dynamic_cast<KoColorBackground*>(baseShape->background().data());
        QVERIFY(bg);
        QCOMPARE(bg->color(), QColor(Qt::blue));
    }

    KoSvgTextShapeMarkupConverter converter(baseShape);

    QString svgText;
    QString stylesText;
    QVERIFY(converter.convertToSvg(&svgText, &stylesText));

    QCOMPARE(stylesText, QString("<defs/>"));
    QCOMPARE(svgText, QString("<text fill=\"#0000ff\" stroke=\"#000000\" stroke-opacity=\"0\" stroke-width=\"0\" stroke-linecap=\"square\" stroke-linejoin=\"bevel\" font-family=\"DejaVu Sans\" font-size=\"15\"><tspan x=\"2\" y=\"24\">S</tspan><tspan fill=\"#ff0000\">A</tspan><tspan>some stuff&lt;&gt;&lt;&gt;&lt;&lt;&lt;&gt;</tspan></text>"));

    // test loading

    svgText = "<text fill=\"#00ff00\" font-family=\"DejaVu Sans\" font-size=\"19\"><tspan x=\"2\" y=\"24\">S</tspan><tspan fill=\"#ff0000\">A</tspan><tspan>some stuff&lt;&gt;&lt;&gt;&lt;&lt;&lt;&gt;</tspan></text>";

    QVERIFY(converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));

    {
        KoColorBackground *bg = dynamic_cast<KoColorBackground*>(baseShape->background().data());
        QVERIFY(bg);
        QCOMPARE(bg->color(), QColor(Qt::green));
    }

    {
        KoSvgTextProperties props = baseShape->textProperties();
        QVERIFY(props.hasProperty(KoSvgTextProperties::FontSizeId));

        const qreal fontSize = props.property(KoSvgTextProperties::FontSizeId).toReal();
        QCOMPARE(fontSize, 19.0);
    }

    QCOMPARE(baseShape->shapeCount(), 3);
}

void TestSvgText::testConvertToStrippedSvgNullOrigin()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text transform=\"translate(2)\" id=\"testRect\" x=\"0\" y=\"0\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        S<tspan fill=\"red\">A</tspan><![CDATA[some stuff<><><<<>]]>"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.parser.setResolution(QRectF(QPointF(), QSizeF(30,30)) /* px */, 72.0/* ppi */);
    t.run();

    KoSvgTextShape *baseShape = dynamic_cast<KoSvgTextShape*>(t.findShape("testRect"));
    QVERIFY(baseShape);

    KoSvgTextShapeMarkupConverter converter(baseShape);

    QString svgText;
    QString stylesText;
    QVERIFY(converter.convertToSvg(&svgText, &stylesText));

    QCOMPARE(stylesText, QString("<defs/>"));
    QCOMPARE(svgText, QString("<text fill=\"#0000ff\" stroke=\"#000000\" stroke-opacity=\"0\" stroke-width=\"0\" stroke-linecap=\"square\" stroke-linejoin=\"bevel\" font-family=\"DejaVu Sans\" font-size=\"15\"><tspan x=\"0\" y=\"0\">S</tspan><tspan fill=\"#ff0000\">A</tspan><tspan>some stuff&lt;&gt;&lt;&gt;&lt;&lt;&lt;&gt;</tspan></text>"));
}

void TestSvgText::testConvertFromIncorrectStrippedSvg()
{
    QScopedPointer<KoSvgTextShape> baseShape(new KoSvgTextShape());

    KoSvgTextShapeMarkupConverter converter(baseShape.data());

    QString svgText;
    QString stylesText;

    svgText = "<text>blah text</text>";
    QVERIFY(converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));
    QCOMPARE(converter.errors().size(), 0);

    svgText = "<text>>><<><blah text</text>";
    QVERIFY(!converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));
    qDebug() << ppVar(converter.errors());
    QCOMPARE(converter.errors().size(), 1);

    svgText = "<notext>blah text</notext>";
    QVERIFY(!converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));
    qDebug() << ppVar(converter.errors());
    QCOMPARE(converter.errors().size(), 1);

    svgText = "<defs/>";
    QVERIFY(!converter.convertFromSvg(svgText, stylesText, QRectF(0,0,30,30), 72.0));
    qDebug() << ppVar(converter.errors());
    QCOMPARE(converter.errors().size(), 1);
}

void TestSvgText::testEmptyTextChunk()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"2\" y=\"24\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        " // no actual text! should not crash!
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    // it just shouldn't assert or fail when seeing an empty text block
    t.parser.setResolution(QRectF(QPointF(), QSizeF(30,30)) /* px */, 72.0/* ppi */);
    t.run();
}

void TestSvgText::testTrailingWhitespace()
{
    QStringList chunkA;
    chunkA << "aaa";
    chunkA << " aaa";
    chunkA << "aaa ";
    chunkA << " aaa ";

    QStringList chunkB;
    chunkB << "bbb";
    chunkB << " bbb";
    chunkB << "bbb ";
    chunkB << " bbb ";

    QStringList linkChunk;
    linkChunk << "";
    linkChunk << " ";
    linkChunk << "<tspan></tspan>";
    linkChunk << "<tspan> </tspan>";


    const QString dataTemplate =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"2\" y=\"24\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        <tspan>%1</tspan>%2<tspan>%3</tspan>"
            "    </text>"

            "</g>"

            "</svg>";

    for (auto itL = linkChunk.constBegin(); itL != linkChunk.constEnd(); ++itL) {
        for (auto itA = chunkA.constBegin(); itA != chunkA.constEnd(); ++itA) {
            for (auto itB = chunkB.constBegin(); itB != chunkB.constEnd(); ++itB) {
                if (itA->rightRef(1) != " " &&
                    itB->leftRef(1) != " " &&
                    *itL != " " &&
                    *itL != linkChunk.last()) continue;

                QString cleanLink = *itL;
                cleanLink.replace('/', '_');

                qDebug() << "Testcase:" << *itA << cleanLink << *itB;

                const QString data = dataTemplate.arg(*itA, *itL, *itB);
                SvgRenderTester t (data);
                t.setFuzzyThreshold(5);
                //t.test_standard(QString("text_trailing_%1_%2_%3").arg(*itA).arg(cleanLink).arg(*itB), QSize(70, 30), 72.0);

                // all files should look exactly the same!
                t.test_standard(QString("text_whitespace"), QSize(70, 30), 72.0);
            }
        }
    }
}

void TestSvgText::testConvertHtmlToSvg()
{
    const QString html =
            "<?xml version=\"1.0\"?>"
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
            "<html>"
              "<head>"
                "<meta name=\"qrichtext\" content=\"1\"/>"
                "<style type=\"text/css\">p, li { white-space: pre-wrap; }</style>"
              "</head>"
              "<body style=\" font-family:'Droid Sans'; font-size:9pt; font-weight:400; font-style:normal;\">"
                "<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "  <span style=\" font-family:'Times'; font-size:20pt;\">Lorem ipsum dolor</span>"
                "</p>"
                "<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">sit am"
                "<span style=\" font-weight:600;\">et, consectetur adipis</span>cing </p>"
                "<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "  <span style=\" font-style:italic;\">elit. </span>"
                "</p>"
              "</body>"
            "</html>";

    KoSvgTextShape shape;
    KoSvgTextShapeMarkupConverter converter(&shape);

    QString svg;
    QString defs;

    converter.convertFromHtml(html, &svg, &defs);


    bool r = converter.convertToSvg(&svg, &defs);

    qDebug() << r << svg << defs;

}

void TestSvgText::testTextWithMultipleRelativeOffsets()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"
            "    <text id=\"testRect\" x=\"10\" y=\"40\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" "
            "        dy=\"0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0\">"
            "        Lorem ipsum dolor sit amet"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.setFuzzyThreshold(5);
    t.test_standard("text_multiple_relative_offsets", QSize(300, 80), 72.0);
}

void TestSvgText::testTextWithMultipleAbsoluteOffsetsArabic()
{
    /**
     * According to the standard, each **absolute** offset defines a
     * new text chunk, therefore, the arabic text must become
     * ltr reordered
     */

    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"
            "    <text id=\"testRect\" x=\"10\" y=\"40\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" "
            "        y=\"40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 40 45 50 55 50 45 40 35 30 25 30 35 \">"
            "        Lo rem اللغة العربية المعيارية الحديثة ip sum"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard("text_multiple_absolute_offsets_arabic", QSize(530, 70), 72.0);
}

void TestSvgText::testTextWithMultipleRelativeOffsetsArabic()
{
    /**
     * According to the standard, **relative** offsets must not define a new
     * text chunk, therefore, the arabic text must be written in native rtl order,
     * even though the individual letters are split.
     */

    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"
            "    <text id=\"testRect\" x=\"10\" y=\"40\""
            "        font-family=\"DejaVu Sans\" font-size=\"15\" "
            "        dy=\"0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0 -3 -3 -3 -3 3 3 3 3 0\">"
            "        Lo rem اللغة العربية المعيارية الحديثة ip sum"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    // we cannot expect more than one failure
#ifndef USE_ROUND_TRIP
    QEXPECT_FAIL("", "WARNING: in Krita relative offsets also define a new text chunk, that doesn't comply with SVG standard and must be fixed", Continue);
    t.test_standard("text_multiple_relative_offsets_arabic", QSize(530, 70), 72.0);
#endif
}

void TestSvgText::testTextOutline()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"test\">"

            "    <rect id=\"boundingRect\" x=\"4\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text id=\"testRect\" x=\"7\" y=\"27\""
            " "
            "        font-family=\"DejaVu Sans\" font-size=\"15\" fill=\"blue\" >"
            "        normal "
            "        <tspan text-decoration=\"line-through\">strikethrough</tspan>"
            "        <tspan text-decoration=\"overline\">overline</tspan>"
            "        <tspan text-decoration=\"underline\">underline</tspan>"
            "    </text>"

            "</g>"

            "</svg>";

    QRect renderRect(0, 0, 450, 40);

    SvgRenderTester t (data);
    t.setFuzzyThreshold(5);
    t.test_standard("text_outline", renderRect.size(), 72.0);

    KoShape *shape = t.findShape("testRect");
    KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape*>(shape);
    QVERIFY(chunkShape);

    KoSvgTextShape *textShape = dynamic_cast<KoSvgTextShape*>(shape);

    QImage canvas(renderRect.size(), QImage::Format_ARGB32);
    canvas.fill(0);
    QPainter gc(&canvas);
    gc.setPen(Qt::NoPen);
    gc.setBrush(Qt::black);
    gc.setRenderHint(QPainter::Antialiasing, true);
    for (KoShape *shape : textShape->textOutline()) {
        KoPathShape *outline = dynamic_cast<KoPathShape *>(shape);
        if (shape) {
            gc.drawPath(outline->outline());
        }
    }

    QVERIFY(TestUtil::checkQImage(canvas, "svg_render", "load_text_outline", "converted_to_path", 3, 5));
}


void testTextFontSizeHelper(QString filename, int dpi, bool pixelSize)
{
    ENTER_FUNCTION() << ppVar(dpi) << ppVar(filename) << ppVar(pixelSize);

    QFont testFont("Chilanka");
    if (!QFontInfo(testFont).exactMatch()) {
        qWarning() << "Chilanka is *not* found! Text rendering might be broken!";
    }

    if (pixelSize) {
        testFont.setPixelSize(20);
    } else {
        testFont.setPointSize(20);
    }

    QTextLayout layout("Chy QTextLayout", testFont);

    QFontMetricsF fontMetrics(testFont);
    int leading = fontMetrics.leading();
    qreal height = 0;
    layout.setCacheEnabled(true);
    layout.beginLayout();
    qreal lineWidth = 1000;
    while (1) {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(lineWidth);
        height += leading;
        line.setPosition(QPointF(0, height));
        height += line.height();
    }
    layout.endLayout();

    ENTER_FUNCTION() << ppVar(layout.boundingRect());
    QImage image(QSize(200, 100), QImage::Format_ARGB32);
    // 72 dpi => ~2834 dpm
    qreal inchesInMeter = 39.37007874;
    qreal dpm = dpi*inchesInMeter;

    image.setDotsPerMeterX((int)dpm);
    image.setDotsPerMeterY((int)dpm);

    ENTER_FUNCTION() << ppVar(image.dotsPerMeterX()) << ppVar(image.dotsPerMeterY()) << ppVar(image.devicePixelRatioF())
                     << ppVar(image.devicePixelRatioFScale()) << ppVar(image.logicalDpiX()) << ppVar(image.logicalDpiY())
                     << ppVar(image.logicalDpiX()) << ppVar(image.physicalDpiX())<< ppVar(image.physicalDpiY());

    image.fill(Qt::white);
    QPainter painter(&image);
    //painter.se
    layout.draw(&painter, QPointF(0, 0));


    QBrush brush(Qt::red);
    QPen pen(Qt::red);
    painter.setBrush(brush);
    painter.setPen(pen);
    painter.drawLine(QPoint(0, 20), QPoint(200, 20));
    painter.drawLine(QPoint(0, 40), QPoint(200, 40));
    painter.drawLine(QPoint(0, 60), QPoint(200, 60));
    painter.drawLine(QPoint(0, 80), QPoint(200, 80));

    QBrush brush2(Qt::blue);
    QPen pen2(Qt::blue);
    painter.setBrush(brush2);
    painter.setPen(pen2);

    painter.setFont(testFont);
    //painter.drawText(QPointF(0, 0), "Chy QPainter");
    painter.drawText(QRectF(0, 40, 200, 100), "Chy QPainter");

    ENTER_FUNCTION() << ppVar(painter.fontMetrics().height()) << ppVar(painter.fontMetrics().xHeight());
    ENTER_FUNCTION() << ppVar(QFontMetrics(testFont).height()) << ppVar(QFontMetrics(testFont).xHeight());



    QString filenameSuffix = (pixelSize ? "pixel_" : "point_") + QString::number(dpi);

    image.save(QString(FILES_OUTPUT_DIR) + '/' + filename + "_" + filenameSuffix + ".png");

}

void TestSvgText::testTextFontSize()
{
    QString filename = "testTextFontSize";

    testTextFontSizeHelper(filename, 72, true);
    testTextFontSizeHelper(filename, 72, false);
    testTextFontSizeHelper(filename, 4*72, true);
    testTextFontSizeHelper(filename, 4*72, false);

    testTextFontSizeHelper(filename, 96, true);
    testTextFontSizeHelper(filename, 96, false);

}

/**
 * @brief TestSvgText::testAddingTestFont
 *
 * This test tests whether we can add a font to the font registery
 * and retrieve it afterwards. Without this, we won't be able to
 * write reliable machine tests given how much of text rendering is
 * font specific.
 */
void TestSvgText::testAddingTestFont()
{
    QString fontName = "Ahem";
    QString fontFileName = "fonts/Ahem/ahem.ttf";
    QString fileName = TestUtil::fetchDataFileLazy(fontFileName);

    bool res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1").arg(fontName).toLatin1());


    QVector<int> lengths;
    const std::vector<FT_FaceUP> faces = KoFontRegistery::instance()->facesForCSSValues({fontName}, lengths);

    res = false;
    for (const FT_FaceUP &face : faces) {
        //qDebug() << face->family_name;
        if (face->family_name == fontName) {
            res = true;
            break;
        }
    }
    QVERIFY2(res, QString("KoFontRegistery could not find the added test font %1").arg(fontName).toLatin1());

}

/**
 * @testUnicodeGraphemeClusters
 * This tests KoCssTextUtils::textToUnicodeGraphemeClusters,
 * which is a prerequisite to doing robust font-charmap-matching.
 *
 * We'll be testing a number of texts and see if they break up
 * correctly.
 */
void TestSvgText::testUnicodeGraphemeClusters()
{
    QString langCode = "";
    QString test;
    QStringList expectedResult;
    QStringList result;

    // Simple test.

    test = "123ABC";
    expectedResult.clear();
    expectedResult << "1" << "2" << "3" << "A" << "B" << "C";

    result = KoCssTextUtils::textToUnicodeGraphemeClusters(test, langCode);

    QVERIFY2(result == expectedResult, QString("Text to unicode clusters for %1 is incorrect.\n Result:\t %2\n Expected:\t %3")
             .arg(test).arg(result.join(", ")).arg(expectedResult.join(", ")).toLatin1());

    // Testing text + combining marks.

    test = "K\u0304r\u0330i\u1dd1\u1ab2ta\u20d4";
    expectedResult.clear();
    expectedResult << "K\u0304" << "r\u0330" << "i\u1dd1\u1ab2" << "t" << "a\u20d4";

    result = KoCssTextUtils::textToUnicodeGraphemeClusters(test, langCode);

    QVERIFY2(result == expectedResult, QString("Text to unicode clusters for %1 is incorrect.\n Result:\t %2\n Expected:\t %3")
             .arg(test).arg(result.join(", ")).arg(expectedResult.join(", ")).toLatin1());

    // Testing text + emoji sequence
    // This tests the fitzpatrick modifiers (woman+black), a zero-width joiner (black woman+fire engine)
    // as well as the regional indicators which is how flags are handled.

    test = "Fire:\U0001F469\U0001F3FF\u200D\U0001F692 US:\U0001F1FA\U0001F1F8";
    expectedResult.clear();
    expectedResult << "F" << "i" << "r" << "e" << ":" << "\U0001F469\U0001F3FF\u200D\U0001F692"
                   << " " << "U" << "S" << ":" << "\U0001F1FA\U0001F1F8";

    result = KoCssTextUtils::textToUnicodeGraphemeClusters(test, langCode);

    QVERIFY2(result == expectedResult, QString("Text to unicode clusters for %1 is incorrect.\n Result:\t %2\n Expected:\t %3")
             .arg(test).arg(result.join(", ")).arg(expectedResult.join(", ")).toLatin1());

    // Testing variation selector.
    // These represent alternate forms of a glyph which may need to be selected for certain purposes.
    // For example a person's name and a place name may use the same character,
    // but will need different versions of that character.

    test = "Ashi:\u82A6\uFE03 or \u82A6";
    expectedResult.clear();
    expectedResult << "A" << "s" << "h" << "i" << ":" << "\u82A6\uFE03" << " " << "o" << "r" << " " << "\u82A6";

    result = KoCssTextUtils::textToUnicodeGraphemeClusters(test, langCode);

    QVERIFY2(result == expectedResult, QString("Text to unicode clusters for %1 is incorrect.\n Result:\t %2\n Expected:\t %3")
             .arg(test).arg(result.join(", ")).arg(expectedResult.join(", ")).toLatin1());
}

/**
 * @brief TestSvgText::testFontSelectionForText
 *
 * This tests whether we are selecting appropriate fonts for a given text.
 * Things we want to test amongst others are: mixed script, emoji selection,
 * unicode variation selection, combination marks and support for the unicode
 * supplementary (and above) planes.
 */
void TestSvgText::testFontSelectionForText()
{
    // Test the letter a.

    QString test = "a";
    QString fileName = TestUtil::fetchDataFileLazy("fonts/CSSTest");


    // First we verify that we can find the test fonts.

    bool res = KoFontRegistery::instance()->addFontFileDirectoryToRegistery(fileName);
    QVERIFY2(res, QString("KoFontRegistery could not add the directory of test fonts %1")
             .arg(fileName).toLatin1());


    QVector<int> lengths;
    const std::vector<FT_FaceUP> faces =
            KoFontRegistery::instance()->facesForCSSValues({"CSSTest Verify"}, lengths, test);

    QVERIFY2(lengths.size() == 1,
             QString("KoFontRegistery selected the wrong amount of fonts for the following text: %1")
             .arg(test).toLatin1());

    // Test combination marks. We should prefer combination marks to be using the same glyphs as the font.

    test = "K\u0304r\u0330ita";
    QStringList fontFamilies;
    QStringList foundFonts;
    QStringList expectedFonts;
    fontFamilies << "CSSTest Verify" << "DejaVu Sans";
    expectedFonts << "DejaVu Sans" << "CSSTest Verify";
    QVector<int> expectedLengths;
    expectedLengths << 4 << 3;
    fileName = TestUtil::fetchDataFileLazy("fonts/DejaVuSans.ttf");
    res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1")
             .arg("DejaVu Sans").toLatin1());

    const std::vector<FT_FaceUP> faces2 =
            KoFontRegistery::instance()->facesForCSSValues(fontFamilies, lengths, test);
    QVERIFY2(lengths == expectedLengths,
             QString("KoFontRegistery returns the wrong lengths for string %1")
             .arg(test).toLatin1());
    for (const FT_FaceUP &face : faces2) {
        //qDebug() << face->family_name;
        foundFonts.append(face->family_name);
    }
    QVERIFY2(foundFonts == expectedFonts, QString("KoFontRegistery returns the wrong fonts for string %1"
                                                 "\nResult:\t%2\nExpected:\t%3")
             .arg(test).arg(foundFonts.join(", ")).arg(expectedFonts.join(", ")).toLatin1());

    // Test emoji

    test = "Hand:\u270d\U0001F3FF etc.";

    const std::vector<FT_FaceUP> faces3 =
            KoFontRegistery::instance()->facesForCSSValues(fontFamilies, lengths, test);
    expectedLengths.clear();
    expectedLengths << 5 << 3 << 5;
    // we can only test the lengths here because dejavu sans doesn't
    // have the fitzpatrick emoji selectors, so on a regular
    // desktop the families would pick a proper emoji font for this.
    QVERIFY2(lengths == expectedLengths,
             QString("KoFontRegistery returns the wrong lengths for string %1")
             .arg(test).toLatin1());


    // Test variation selector (with and without graceful fallback).
    // What we want to do here is check whether if we have a font with a character
    // but not the variation selector, it will treat this as the fallback and select
    // when there's no better font. May not work on non-testing systems?

    test = "Ashi:\u82A6\uFE03 or \u82A6";
    fontFamilies << "Krita_Test_Unicode_Variation_A";
    expectedLengths.clear();
    expectedLengths << 5 << 2 << 4 << 1;
    fileName = TestUtil::fetchDataFileLazy("fonts/Krita_Test_Unicode_Variation_A.ttf");
    KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    foundFonts.clear();
    expectedFonts.clear();
    expectedFonts << "CSSTest Verify" << "Krita_Test_Unicode_Variation_A"
                  << "CSSTest Verify" << "Krita_Test_Unicode_Variation_A";
    const std::vector<FT_FaceUP> faces4 =
            KoFontRegistery::instance()->facesForCSSValues(fontFamilies, lengths, test);
    QVERIFY2(lengths == expectedLengths,
             QString("KoFontRegistery returns the wrong lengths for string %1")
             .arg(test).toLatin1());
    for (const FT_FaceUP &face : faces4) {
        foundFonts.append(face->family_name);
    }
    QVERIFY2(foundFonts == expectedFonts,
             QString("KoFontRegistery returns the wrong fonts for string %1"
                                                 "\nResult:\t%2\nExpected:\t%3")
             .arg(test).arg(foundFonts.join(", ")).arg(expectedFonts.join(", ")).toLatin1());


    // What we want to do here is check whether if we have a font with a character and a selector,
    // it will select that font over others that may only have the base character.

    fileName = TestUtil::fetchDataFileLazy("fonts/Krita_Test_Unicode_Variation_B.ttf");
    KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);
    foundFonts.clear();
    expectedFonts.clear();
    expectedFonts << "CSSTest Verify" << "Krita_Test_Unicode_Variation_B"
                  << "CSSTest Verify" << "Krita_Test_Unicode_Variation_B";
    fontFamilies.clear();
    fontFamilies << "CSSTest Verify" << "Krita_Test_Unicode_Variation_B"
                 << "Krita_Test_Unicode_Variation_A";

    const std::vector<FT_FaceUP> faces5 =
            KoFontRegistery::instance()->facesForCSSValues(fontFamilies, lengths, test);
    QVERIFY2(lengths == expectedLengths,
             QString("KoFontRegistery returns the wrong lengths for string %1")
             .arg(test).toLatin1());
    for (const FT_FaceUP &face : faces5) {
        //qDebug() << face->family_name;
        foundFonts.append(face->family_name);
    }
    QVERIFY2(foundFonts == expectedFonts,
             QString("KoFontRegistery returns the wrong fonts for string %1"
                                                 "\nResult:\t%2\nExpected:\t%3")
             .arg(test).arg(foundFonts.join(", ")).arg(expectedFonts.join(", ")).toLatin1());

    // Test Arabic + English + CJK
    // This is just a generic test to see if we can have mixed script without things blowing up.

    test = "Lo rem اللغة العربية المعيارية الحديثة ip あああ sum";
    fontFamilies << "DejaVu Sans";
    foundFonts.clear();
    expectedLengths.clear();
    expectedLengths << 7 << 5 << 1 << 7 << 1 << 9 << 1 << 7 << 4 << 3 << 4;
    expectedFonts.clear();
    expectedFonts << "CSSTest Verify"
                  << "DejaVu Sans"
                  << "CSSTest Verify"
                  << "DejaVu Sans"
                  << "CSSTest Verify"
                  << "DejaVu Sans"
                  << "CSSTest Verify"
                  << "DejaVu Sans"
                  << "CSSTest Verify"
                  << "Krita_Test_Unicode_Variation_B"
                  << "CSSTest Verify";
    const std::vector<FT_FaceUP> faces6 =
            KoFontRegistery::instance()->facesForCSSValues(fontFamilies, lengths, test);
    QVERIFY2(lengths == expectedLengths,
             QString("KoFontRegistery returns the wrong lengths for string %1").arg(test).toLatin1());
    for (const FT_FaceUP &face : faces6) {
        //qDebug() << face->family_name;
        foundFonts.append(face->family_name);
    }
    QVERIFY2(foundFonts == expectedFonts, QString("KoFontRegistery returns the wrong fonts for string %1"
                                                 "\nResult:\t%2\nExpected:\t%3")
             .arg(test).arg(foundFonts.join(", ")).arg(expectedFonts.join(", ")).toLatin1());

    // Test supplementary plane code points.

    // Jack of diamonds is U+1f0cb and is part of DejaVu Sans
    test = "Jack:🃋";
    const std::vector<FT_FaceUP> faces7 = KoFontRegistery::instance()->facesForCSSValues(fontFamilies, lengths, test);
    foundFonts.clear();
    expectedLengths.clear();
    expectedLengths << 5 << 2;
    expectedFonts.clear();
    expectedFonts << "CSSTest Verify"
                  << "DejaVu Sans";
    QVERIFY2(lengths == expectedLengths, QString("KoFontRegistery returns the wrong lengths for string %1")
             .arg(test).toLatin1());
    for (const FT_FaceUP &face : faces7) {
        //qDebug() << face->family_name;
        foundFonts.append(face->family_name);
    }
    QVERIFY2(foundFonts == expectedFonts, QString("KoFontRegistery returns the wrong fonts for string %1"
                                                 "\nResult:\t%2\nExpected:\t%3")
             .arg(test).arg(foundFonts.join(", ")).arg(expectedFonts.join(", ")).toLatin1());
}

/**
 * @brief TestSvgText::testFontStyleSelection
 *
 * This tests whether the font registery is selecting things like bold or italics correctly.
 * TODO: Test font-style selection (italics).
 */
void TestSvgText::testFontStyleSelection()
{
    QString verifyCSSTest = "CSSTest Verify";
    QString fileName = TestUtil::fetchDataFileLazy("fonts/CSSTest");
    QString test = "A";

    // First we verify that we can find the test fonts.

    bool res = KoFontRegistery::instance()->addFontFileDirectoryToRegistery(fileName);
    QVERIFY2(res, QString("KoFontRegistery could not add the directory of test fonts %1").arg(fileName).toLatin1());


    QVector<int> lengths;
    const std::vector<FT_FaceUP> faces = KoFontRegistery::instance()->facesForCSSValues({verifyCSSTest}, lengths, test);

    res = false;
    for (const FT_FaceUP &face : faces) {
        //qDebug() << face->family_name;
        if (face->family_name == verifyCSSTest) {
            res = true;
            break;
        }
    }
    QVERIFY2(res, QString("KoFontRegistery did not return the expected test font %1").arg(verifyCSSTest).toLatin1());

    // Now we go through a table of font-weights for the given test fonts.
    // This test is an adaptation of web-platform-test font-weight-bolder-001.xht
    // Note: when comparing to
    // https://github.com/web-platform-tests/wpt/blob/master/css/css-fonts/support/font-weight-bolder-001-ref.png
    // our implementation leaves things to be desired, but at the least it's using the correct fonts.

    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/font-weight-bolder-001.svg"));
    res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QRect renderRect(0, 0, 300, 150);

    SvgRenderTester t (data.data());
    t.setFuzzyThreshold(5);
    t.test_standard("font-weight-bolder-001", renderRect.size(), 72.0);

}
/**
 * @brief TestSvgText::testFontSizeConfiguration
 *
 * This tests setting the font size.
 *
 * TODO: Add pixelfont test.
 */
void TestSvgText::testFontSizeConfiguration()
{
    QString fontName = "Ahem";
    QString fontFileName = "fonts/Ahem/ahem.ttf";
    QString fileName = TestUtil::fetchDataFileLazy(fontFileName);

    bool res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1").arg(fontName).toLatin1());


    QVector<int> lengths;
    const std::vector<FT_FaceUP> faces = KoFontRegistery::instance()->facesForCSSValues({fontName}, lengths);
    KoFontRegistery::instance()->configureFaces(faces, 15.0, 1.0, 72, 72, QMap<QString, qreal>());

    int size = faces.front()->size->metrics.height;
    QVERIFY2(size == 960,
             QString("Configured value for Ahem at 15 pt is not returning as 960, instead %1")
             .arg(QString::number(size)).toLatin1());

}

/**
 * @brief TestSvgText::testFontOpenTypeVariationsConfiguration
 */
void TestSvgText::testFontOpenTypeVariationsConfiguration()
{
    QString fontName = "Variable Test Axis Matching";
    QString fontFileName = "fonts/variabletest_matching.ttf";
    QString fileName = TestUtil::fetchDataFileLazy(fontFileName);

    bool res = KoFontRegistery::instance()->addFontFilePathToRegistery(fileName);

    QVERIFY2(res, QString("KoFontRegistery could not add the test font %1").arg(fontName).toLatin1());


    // Testing rendering.

    QFile file(TestUtil::fetchDataFileLazy("fonts/textTestSvgs/font-opentype-variations.svg"));
    res = file.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY2(res, QString("Cannot open test svg file.").toLatin1());

    QXmlInputSource data;
    data.setData(file.readAll());

    QRect renderRect(0, 0, 300, 150);

    SvgRenderTester t (data.data());
    t.setFuzzyThreshold(5);
    t.test_standard("font-opentype-variations", renderRect.size(), 72.0);
}

#include "kistest.h"


KISTEST_MAIN(TestSvgText)
