/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCSSPARSER_P_H
#define QCSSPARSER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QStringList>
#include <QVector>
#include <QVariant>
#include <QPair>
#include <QSize>
#include <QFont>

namespace QCss
{

enum Property {
    UnknownProperty,
    BackgroundColor,
    Color,
    Float,
    Font,
    FontFamily,
    FontSize,
    FontStyle,
    FontWeight,
    Margin,
    MarginBottom,
    MarginLeft,
    MarginRight,
    MarginTop,
    QtBlockIndent,
    QtListIndent,
    QtParagraphType,
    QtTableType,
    TextDecoration,
    TextIndent,
    VerticalAlignment,
    Whitespace,
    QtSelectionForeground,
    QtSelectionBackground,
    Border,
    BorderLeft,
    BorderRight,
    BorderTop,
    BorderBottom,
    Padding,
    PaddingLeft,
    PaddingRight,
    PaddingTop,
    PaddingBottom,
    PageBreakBefore,
    PageBreakAfter,
    QtAlternateBackground,
    BorderLeftStyle,
    BorderRightStyle,
    BorderTopStyle,
    BorderBottomStyle,
    BorderStyles,
    BorderLeftColor,
    BorderRightColor,
    BorderTopColor,
    BorderBottomColor,
    BorderColor,
    BorderLeftWidth,
    BorderRightWidth,
    BorderTopWidth,
    BorderBottomWidth,
    BorderWidth,
    BorderTopLeftRadius,
    BorderTopRightRadius,
    BorderBottomLeftRadius,
    BorderBottomRightRadius,
    BorderRadius,
    Background,
    BackgroundOrigin,
    BackgroundRepeat,
    BackgroundPosition,
    BackgroundImage,
    BorderImage,
    QtSpacing,
    Width,
    Height,
    MinimumWidth,
    MinimumHeight,
    MaximumWidth,
    MaximumHeight,
    QtImage,
    Left,
    Right,
    Top,
    Bottom,
    QtOrigin,
    QtPosition,
    Position,
    NumProperties
};

enum KnownValue {
    UnknownValue,
    Value_Normal,
    Value_Pre,
    Value_NoWrap,
    Value_PreWrap,
    Value_Small,
    Value_Medium,
    Value_Large,
    Value_XLarge,
    Value_XXLarge,
    Value_Italic,
    Value_Oblique,
    Value_Bold,
    Value_Underline,
    Value_Overline,
    Value_LineThrough,
    Value_Sub,
    Value_Super,
    Value_Left,
    Value_Right,
    Value_Top,
    Value_Bottom,
    Value_Center,
    Value_Native,
    Value_Auto,
    Value_Always,
    Value_None,
    NumKnownValues
};

enum BorderStyle {
    BorderStyle_Unknown,
    BorderStyle_None,
    BorderStyle_Dotted,
    BorderStyle_Dashed,
    BorderStyle_Solid,
    BorderStyle_Double,
    BorderStyle_DotDash,
    BorderStyle_DotDotDash,
    BorderStyle_Groove,
    BorderStyle_Ridge,
    BorderStyle_Inset,
    BorderStyle_Outset,
    NumKnownBorderStyles
};

enum Edge {
    TopEdge,
    RightEdge,
    BottomEdge,
    LeftEdge,
    NumEdges
};

enum Corner {
    TopLeftCorner,
    TopRightCorner,
    BottomLeftCorner,
    BottomRightCorner
};

enum TileMode {
    TileMode_Unknown,
    TileMode_Round,
    TileMode_Stretch,
    TileMode_Repeat,
    NumKnownTileModes
};

enum Repeat {
    Repeat_Unknown,
    Repeat_None,
    Repeat_X,
    Repeat_Y,
    Repeat_XY,
    NumKnownRepeats
};

enum Origin {
    Origin_Unknown,
    Origin_Padding,
    Origin_Border,
    Origin_Content,
    Origin_Margin,
    NumKnownOrigins
};

enum PositionMode {
    PositionMode_Unknown,
    PositionMode_Static,
    PositionMode_Relative,
    PositionMode_Absolute,
    PositionMode_Fixed,
    NumKnownPositionModes
};

struct Q_GUI_EXPORT Value
{
    enum Type {
        Unknown,
        Number,
        Percentage,
        Length,
        String,
        Identifier,
        KnownIdentifier,
        Uri,
        Color,
        Function,
        TermOperatorSlash,
        TermOperatorComma
    };
    inline Value() : type(Unknown) { }
    Type type;
    QVariant variant;
};

// 1. StyleRule - x:hover, y:clicked > z:checked { prop1: value1; prop2: value2; }
// 2. QVector<Selector> - x:hover, y:clicked z:checked
// 3. QVector<BasicSelector> - y:clicked z:checked
// 4. QVector<Declaration> - { prop1: value1; prop2: value2; }
// 5. Declaration - prop1: value1;

struct Q_GUI_EXPORT Declaration
{
    inline Declaration() : propertyId(UnknownProperty), important(false) {}
    QString property;
    Property propertyId;
    inline bool isEmpty() const { return property.isEmpty() && propertyId == UnknownProperty; }
    QVector<Value> values;
    bool important;

    // helper functions
    QColor colorValue() const;
    void colorValues(QColor *c) const;
    QBrush brushValue() const;

    BorderStyle styleValue() const;
    void styleValues(BorderStyle *s) const;

    Origin originValue() const;
    Repeat repeatValue() const;
    Qt::Alignment alignmentValue() const;
    PositionMode positionValue() const;

    bool intValue(int *i, const char *unit = 0) const;
    bool realValue(qreal *r, const char *unit = 0) const;

    QRect rectValue() const;
    QString uriValue() const;

    void borderImageValue(QString *image, int *cuts, TileMode *h, TileMode *v) const;
};

enum PseudoState
{
    PseudoState_Unknown         = 0x00000000,
    PseudoState_Enabled         = 0x00000001,
    PseudoState_Disabled        = 0x00000002,
    PseudoState_Pressed         = 0x00000004,
    PseudoState_Focus           = 0x00000008,
    PseudoState_Hover           = 0x00000010,
    PseudoState_Checked         = 0x00000020,
    PseudoState_On              = PseudoState_Checked,
    PseudoState_Unchecked       = 0x00000040,
    PseudoState_Off             = PseudoState_Unchecked,
    PseudoState_Indeterminate   = 0x00000080,
    PseudoState_Unspecified     = 0x00000100,
    NumPseudos = 11
};

struct Q_GUI_EXPORT Pseudo
{
    PseudoState type;
    QString name;
    QString function;
};

struct Q_GUI_EXPORT AttributeSelector
{
    enum ValueMatchType {
        NoMatch,
        MatchEqual,
        MatchContains,
        MatchBeginsWith
    };
    inline AttributeSelector() : valueMatchCriterium(NoMatch) {}

    QString name;
    QString value;
    ValueMatchType valueMatchCriterium;
};

struct Q_GUI_EXPORT BasicSelector
{
    inline BasicSelector() : relationToNext(NoRelation) {}

    enum Relation {
        NoRelation,
        MatchNextSelectorIfAncestor,
        MatchNextSelectorIfParent,
        MatchNextSelectorIfPreceeds
    };

    QString elementName;

    QStringList ids;
    QVector<Pseudo> pseudos;
    QVector<AttributeSelector> attributeSelectors;

    Relation relationToNext;
};

struct Q_GUI_EXPORT Selector
{
    QVector<BasicSelector> basicSelectors;
    int specificity() const;
    int pseudoState() const;
    QString pseudoElement() const;
};

struct StyleRule;
struct MediaRule;
struct PageRule;
struct ImportRule;

struct Q_GUI_EXPORT ValueExtractor
{
    ValueExtractor(const QVector<Declaration> &declarations);

    void extractFont(QFont *font, int *fontSizeAdjustment);
    bool extractBackground(QBrush *, QString *, Repeat *, Qt::Alignment *, QCss::Origin *);
    bool extractGeometry(int *w, int *h, int *mw, int *mh);
    bool extractPosition(int *l, int *t, int *r, int *b, QCss::Origin *, Qt::Alignment *, 
                         QCss::PositionMode *);
    bool extractBox(int *margins, int *paddings, int *spacing = 0);
    bool extractBorder(int *borders, QColor *colors, BorderStyle *Styles, QSize *radii);
    bool extractPalette(QColor *fg, QColor *sfg, QBrush *sbg, QBrush *abg);

private:
    void extractFont();
    void borderValue(const Declaration &decl, int *width, QCss::BorderStyle *style, QColor *color);
    int lengthValue(const Declaration &decl);
    int lengthValue(const Value& v);
    void lengthValues(const Declaration &decl, int *m);
    QSize sizeValue(const Declaration &decl);
    void sizeValues(const Declaration &decl, QSize *radii);

    QVector<Declaration> declarations;
    QFont f;
    int adjustment;
    bool fontExtracted;
};

struct Q_GUI_EXPORT StyleRule
{
    QVector<Selector> selectors;
    QVector<Declaration> declarations;
};

struct Q_GUI_EXPORT MediaRule
{
    QStringList media;
    QVector<StyleRule> styleRules;
};

struct Q_GUI_EXPORT PageRule
{
    QString selector;
    QVector<Declaration> declarations;
};

struct Q_GUI_EXPORT ImportRule
{
    QString href;
    QStringList media;
};

enum StyleSheetOrigin {
    StyleSheetOrigin_Unspecified,
    StyleSheetOrigin_UserAgent,
    StyleSheetOrigin_User,
    StyleSheetOrigin_Author,
    StyleSheetOrigin_Inline
};

struct Q_GUI_EXPORT StyleSheet
{
    StyleSheet() : origin(StyleSheetOrigin_Unspecified), depth(0) { }
    QVector<StyleRule> styleRules;
    QVector<MediaRule> mediaRules;
    QVector<PageRule> pageRules;
    QVector<ImportRule> importRules;
    StyleSheetOrigin origin;
    int depth; // applicable only for inline style sheets
};

class Q_GUI_EXPORT StyleSelector
{
public:
    virtual ~StyleSelector();

    union NodePtr {
        void *ptr;
        int id;
    };

    QVector<StyleRule> styleRulesForNode(NodePtr node);
    QVector<Declaration> declarationsForNode(NodePtr node);

    virtual bool nodeNameEquals(NodePtr node, const QString& nodeName) const = 0;
    virtual QString attribute(NodePtr node, const QString &name) const = 0;
    virtual bool hasAttribute(NodePtr node, const QString &name) const = 0;
    virtual bool hasAttributes(NodePtr node) const = 0;
    virtual QStringList nodeIds(NodePtr node) const;
    virtual bool isNullNode(NodePtr node) const = 0;
    virtual NodePtr parentNode(NodePtr node) = 0;
    virtual NodePtr previousSiblingNode(NodePtr node) = 0;
    virtual NodePtr duplicateNode(NodePtr node) = 0;
    virtual void freeNode(NodePtr node) = 0;

    QList<StyleSheet> styleSheets;
    QString medium;
private:
    void matchRules(NodePtr node, const QVector<StyleRule> &rules, StyleSheetOrigin origin,
                    int depth, QVector<QPair<int, StyleRule> > *weightedRules);
    bool selectorMatches(const Selector &rule, NodePtr node);
    bool basicSelectorMatches(const BasicSelector &rule, NodePtr node);
};

enum TokenType {
    NONE,

    S,

    CDO,
    CDC,
    INCLUDES,
    DASHMATCH,

    LBRACE,
    PLUS,
    GREATER,
    COMMA,

    STRING,
    INVALID,

    IDENT,

    HASH,

    ATKEYWORD_SYM,

    EXCLAMATION_SYM,

    LENGTH,

    PERCENTAGE,
    NUMBER,

    FUNCTION,

    COLON,
    SEMICOLON,
    RBRACE,
    SLASH,
    MINUS,
    DOT,
    STAR,
    LBRACKET,
    RBRACKET,
    EQUAL,
    LPAREN,
    RPAREN,
    OR
};

struct Q_GUI_EXPORT Symbol
{
    inline Symbol() : start(0), len(-1) {}
    TokenType token;
    QString text;
    int start, len;
    QString lexem() const;
};

class Q_AUTOTEST_EXPORT Scanner
{
public:
    static QString preprocess(const QString &input);
    static QVector<Symbol> scan(const QString &preprocessedInput);
    static const char *tokenName(TokenType t);
};

class Q_GUI_EXPORT Parser
{
public:
    explicit Parser(const QString &css);

    bool parse(StyleSheet *styleSheet);
    Symbol errorSymbol();

    bool parseImport(ImportRule *importRule);
    bool parseMedia(MediaRule *mediaRule);
    bool parseMedium(QStringList *media);
    bool parsePage(PageRule *pageRule);
    bool parsePseudoPage(QString *selector);
    bool parseNextOperator(Value *value);
    bool parseCombinator(BasicSelector::Relation *relation);
    bool parseProperty(Declaration *decl);
    bool parseRuleset(StyleRule *styleRule);
    bool parseSelector(Selector *sel);
    bool parseSimpleSelector(BasicSelector *basicSel);
    bool parseClass(QString *name);
    bool parseElementName(QString *name);
    bool parseAttrib(AttributeSelector *attr);
    bool parsePseudo(Pseudo *pseudo);
    bool parseNextDeclaration(Declaration *declaration);
    bool parsePrio(Declaration *declaration);
    bool parseExpr(QVector<Value> *values);
    bool parseTerm(Value *value);
    bool parseFunction(QString *name, QString *args);
    bool parseHexColor(QColor *col);
    bool testAndParseUri(QString *uri);

    inline bool testRuleset() { return testSelector(); }
    inline bool testSelector() { return testSimpleSelector(); }
    inline bool parseNextSelector(Selector *sel) { if (!testSelector()) return recordError(); return parseSelector(sel); }
    bool testSimpleSelector();
    inline bool parseNextSimpleSelector(BasicSelector *basicSel) { if (!testSimpleSelector()) return recordError(); return parseSimpleSelector(basicSel); }
    inline bool testElementName() { return test(IDENT) || test(STAR); }
    inline bool testClass() { return test(DOT); }
    inline bool testAttrib() { return test(LBRACKET); }
    inline bool testPseudo() { return test(COLON); }
    inline bool testMedium() { return test(IDENT); }
    inline bool parseNextMedium(QStringList *media) { if (!testMedium()) return recordError(); return parseMedium(media); }
    inline bool testPseudoPage() { return test(COLON); }
    inline bool testImport() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("import")); }
    inline bool testMedia() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("media")); }
    inline bool testPage() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("page")); }
    inline bool testCombinator() { return test(PLUS) || test(GREATER) || test(S); }
    inline bool testProperty() { return test(IDENT); }
    bool testTerm();
    inline bool testExpr() { return testTerm(); }
    inline bool parseNextExpr(QVector<Value> *values) { if (!testExpr()) return recordError(); return parseExpr(values); }
    bool testPrio();
    inline bool testHexColor() { return test(HASH); }
    inline bool testFunction() { return test(FUNCTION); }
    inline bool parseNextFunction(QString *name, QString *args) { if (!testFunction()) return recordError(); return parseFunction(name, args); }

    inline bool lookupElementName() const { return lookup() == IDENT || lookup() == STAR; }

    inline void skipSpace() { while (test(S)); }

    inline bool hasNext() const { return index < symbols.count(); }
    inline TokenType next() { return symbols.at(index++).token; }
    bool next(TokenType t);
    bool test(TokenType t);
    inline void prev() { index--; }
    inline const Symbol &symbol() const { return symbols.at(index - 1); }
    inline QString lexem() const { return symbol().lexem(); }
    QString unquotedLexem() const;
    QString lexemUntil(TokenType t);
    bool until(TokenType target, TokenType target2 = NONE);
    inline TokenType lookup() const {
        return (index - 1) < symbols.count() ? symbols.at(index - 1).token : NONE;
    }

    bool testTokenAndEndsWith(TokenType t, const QLatin1String &str);

    inline bool recordError() { errorIndex = index; return false; }

    QVector<Symbol> symbols;
    int index;
    int errorIndex;
};

}

#endif
