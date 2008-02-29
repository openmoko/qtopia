<!DOCTYPE xsl:stylesheet [
     <!ENTITY endl "&#10;">
]>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xs="http://www.w3.org/2001/XMLSchema">

<!-- Convenience templates -->

    <xsl:template name="cap-first-char">
        <xsl:param name="text"/>
        <xsl:value-of select="concat(translate(substring($text, 1, 1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring($text, 2))" />
    </xsl:template>

    <xsl:template name="lower-text">
        <xsl:param name="text"/>

        <xsl:if test="boolean($text)">
            <xsl:variable name="head" select="substring($text, 1, 1)"/>
            <xsl:variable name="tail" select="substring($text, 2)"/>
            <xsl:variable name="lower-head" select="translate($text, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')"/>
            <xsl:variable name="lower-tail">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="tail"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:value-of select='concat($lower-head, $lower-tail)'/>
        </xsl:if>
    </xsl:template>

    <xsl:template name="powers-of-two">
        <xsl:param name="num"/>

        <xsl:choose>
            <xsl:when test="$num=0">1</xsl:when>
            <xsl:otherwise>
                <xsl:variable name="x">
                    <xsl:call-template name="powers-of-two">
                        <xsl:with-param name="num" select="$num - 1"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:value-of select="2*$x"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- Convenience templates: xs-types to c++ types conversions -->

    <xsl:template name="xs-type-from-qstring-func">
        <xsl:param name="xs-type"/>
        <xsl:param name="val"/>
        <xsl:choose>
            <xsl:when test="$xs-type='xs:string'">
                <xsl:value-of select="$val"/>
            </xsl:when>
            <xsl:when test="$xs-type='xs:integer'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toInt()</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:float'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toFloat()</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:double'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toDouble()</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:boolean'">
                <xsl:text>(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text> == QLatin1String("true") ? true : false)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:longlong'">
                <xsl:value-of select="$val"/>
                <xsl:text>.toLongLong()</xsl:text>
            </xsl:when>
            <xsl:otherwise>### BZZZZT! ###</xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-to-qstring-func">
        <xsl:param name="xs-type"/>
        <xsl:param name="val"/>
        <xsl:choose>
            <xsl:when test="$xs-type='xs:string'">
                <xsl:value-of select="$val"/>
            </xsl:when>
            <xsl:when test="$xs-type='xs:integer'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:longlong'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:float'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:double'">
                <xsl:text>QString::number(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text>)</xsl:text>
            </xsl:when>
            <xsl:when test="$xs-type='xs:boolean'">
                <xsl:text>(</xsl:text>
                <xsl:value-of select="$val"/>
                <xsl:text> ? QLatin1String("true") : QLatin1String("false"))</xsl:text>
            </xsl:when>
            <xsl:otherwise>### BZZZZT! ###</xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-category">
        <xsl:param name="xs-type"/>
        <xsl:param name="array" select="false"/>
        <xsl:choose>
            <xsl:when test="$array">value</xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">value</xsl:when>
                    <xsl:when test="$xs-type='xs:longlong'">value</xsl:when>
                    <xsl:otherwise>pointer</xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-to-cpp-type">
        <xsl:param name="xs-type"/>
        <xsl:param name="array" select="false"/>
        <xsl:choose>
            <xsl:when test="$array">
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">QStringList</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">QList&lt;int&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">QList&lt;float&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">QList&lt;double&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">QList&lt;bool&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:longlong'">QList&lt;qlonglong&gt;</xsl:when>
                    <xsl:otherwise>QList&lt;Dom<xsl:value-of select="$xs-type"/>*&gt;</xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">QString</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">int</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">float</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">double</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">bool</xsl:when>
                    <xsl:when test="$xs-type='xs:longlong'">qlonglong</xsl:when>
                    <xsl:otherwise>Dom<xsl:value-of select="$xs-type"/></xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-to-cpp-return-type">
        <xsl:param name="xs-type"/>
        <xsl:param name="array" select="false"/>
        <xsl:choose>
            <xsl:when test="$array">
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">QStringList</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">QList&lt;int&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">QList&lt;float&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">QList&lt;double&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">QList&lt;bool&gt;</xsl:when>
                    <xsl:when test="$xs-type='xs:longlong'">QList&lt;qlonglong&gt;</xsl:when>
                    <xsl:otherwise>QList&lt;Dom<xsl:value-of select="$xs-type"/>*&gt;</xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">QString</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">int</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">float</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">double</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">bool</xsl:when>
                    <xsl:when test="$xs-type='xs:longlong'">qlonglong</xsl:when>
                    <xsl:otherwise>Dom<xsl:value-of select="$xs-type"/>*</xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="xs-type-to-cpp-argument-type">
        <xsl:param name="xs-type"/>
        <xsl:param name="array" select="false"/>
        <xsl:choose>
            <xsl:when test="$array">
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">const QStringList&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">const QList&lt;int&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">const QList&lt;float&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">const QList&lt;double&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">const QList&lt;bool&gt;&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:longlong'">const QList&lt;qlonglong&gt;&amp;</xsl:when>
                    <xsl:otherwise>const QList&lt;Dom<xsl:value-of select="$xs-type"/>*&gt;&amp;</xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:choose>
                    <xsl:when test="$xs-type='xs:string'">const QString&amp;</xsl:when>
                    <xsl:when test="$xs-type='xs:integer'">int</xsl:when>
                    <xsl:when test="$xs-type='xs:float'">float</xsl:when>
                    <xsl:when test="$xs-type='xs:double'">double</xsl:when>
                    <xsl:when test="$xs-type='xs:boolean'">bool</xsl:when>
                    <xsl:when test="$xs-type='xs:longlong'">qlonglong</xsl:when>
                    <xsl:otherwise>Dom<xsl:value-of select="$xs-type"/>*</xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>


</xsl:stylesheet>
