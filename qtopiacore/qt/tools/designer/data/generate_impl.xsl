<!DOCTYPE xsl:stylesheet [
     <!ENTITY endl "&#10;">
]>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xs="http://www.w3.org/2001/XMLSchema">

    <xsl:output method="text"/>

    <xsl:include href="generate_shared.xsl"/>

<!-- Implementation: constructor -->

    <xsl:template name="ctor-init-attributes">
        <xsl:param name="node"/>
        <xsl:for-each select="$node/xs:attribute">
            <xsl:text>    m_has_attr_</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text> = false;&endl;</xsl:text>
            <xsl:choose>
                <xsl:when test="@type = 'xs:integer'">
                    <xsl:text>    m_attr_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text> = 0;&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@type = 'xs:double'">
                    <xsl:text>    m_attr_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text> = 0.0;&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@type = 'xs:float'">
                    <xsl:text>    m_attr_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text> = 0.0;&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="@type = 'xs:boolean'">
                    <xsl:text>    m_attr_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text> = false;&endl;</xsl:text>
                </xsl:when>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="ctor-init-child-elements">
        <xsl:param name="node"/>
        <xsl:variable name="array" select="$node/@maxOccurs = 'unbounded'"/>
        <xsl:variable name="cpp-type">
            <xsl:call-template name="xs-type-to-cpp-type">
                <xsl:with-param name="xs-type" select="@type"/>
                <xsl:with-param name="array" select="$array"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:if test="not($array)">
            <xsl:for-each select="$node/xs:element">
                <xsl:choose>
                    <xsl:when test="@type = 'xs:integer'">
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="@name"/>
                        <xsl:text> = 0;&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:float'">
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="@name"/>
                        <xsl:text> = 0.0;&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:boolean'">
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="@name"/>
                        <xsl:text> = false;&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="@type = 'xs:string'"></xsl:when>
                    <xsl:otherwise>
                        <xsl:text>    m_</xsl:text>
                        <xsl:value-of select="@name"/>
                        <xsl:text> = 0;&endl;</xsl:text>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <xsl:template name="ctor-init-members">
        <xsl:param name="node"/>

        <xsl:if test="boolean($node/xs:choice)">
            <xsl:text>    m_kind = Unknown;&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:if test="boolean($node/xs:sequence/xs:element) and not($node/xs:sequence/@maxOccurs = 'unbounded')">
            <xsl:text>    m_children = 0;&endl;</xsl:text>
        </xsl:if>

        <xsl:call-template name="ctor-init-attributes">
            <xsl:with-param name="node" select="."/>
        </xsl:call-template>

        <xsl:for-each select="$node/xs:choice">
            <xsl:call-template name="ctor-init-child-elements">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:for-each select="$node/xs:sequence">
            <xsl:call-template name="ctor-init-child-elements">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="ctor-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:value-of select="$name"/>
        <xsl:text>::</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>()&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>
        <xsl:call-template name="ctor-init-members">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>
        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: destructor -->

    <xsl:template name="dtor-delete-members">
        <xsl:param name="node"/>

        <xsl:variable name="array" select="$node/@maxOccurs = 'unbounded'"/>

        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:choose>
                <xsl:when test="$array">
                    <xsl:if test="$xs-type-cat = 'pointer'">
                        <xsl:text>    for (int i = 0; i &lt; m_</xsl:text>
                        <xsl:value-of select="@name"/>
                        <xsl:text>.size(); ++i)&endl;</xsl:text>
                        <xsl:text>        delete m_</xsl:text>
                        <xsl:value-of select="@name"/>
                        <xsl:text>[i];&endl;</xsl:text>
                    </xsl:if>
                    <xsl:text>    m_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>.clear();&endl;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:if test="$xs-type-cat = 'pointer'">
                        <xsl:text>    delete m_</xsl:text>
                        <xsl:value-of select="@name"/>
                        <xsl:text>;&endl;</xsl:text>
                    </xsl:if>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="dtor-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:value-of select="$name"/>
        <xsl:text>::~</xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>()&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>

        <xsl:for-each select="$node/xs:choice">
            <xsl:call-template name="dtor-delete-members">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:for-each select="$node/xs:sequence">
            <xsl:call-template name="dtor-delete-members">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: clear() -->

    <xsl:template name="clear-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:text>void </xsl:text><xsl:value-of select="$name"/>
        <xsl:text>::clear(bool clear_all)&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>

        <xsl:for-each select="$node/xs:choice">
            <xsl:call-template name="dtor-delete-members">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:for-each select="$node/xs:sequence">
            <xsl:call-template name="dtor-delete-members">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>&endl;    if (clear_all) {&endl;</xsl:text>
        <xsl:text>    m_text = QString();&endl;</xsl:text>
        <xsl:call-template name="ctor-init-attributes">
            <xsl:with-param name="node" select="."/>
        </xsl:call-template>
        <xsl:text>    }&endl;&endl;</xsl:text>

        <xsl:if test="boolean($node/xs:choice)">
            <xsl:text>    m_kind = Unknown;&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:if test="boolean($node/xs:sequence/xs:element) and not($node/xs:sequence/@maxOccurs = 'unbounded')">
            <xsl:text>    m_children = 0;&endl;</xsl:text>
        </xsl:if>

        <xsl:for-each select="$node/xs:choice">
            <xsl:call-template name="ctor-init-child-elements">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:for-each select="$node/xs:sequence">
            <xsl:call-template name="ctor-init-child-elements">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>}&endl;&endl;</xsl:text>

    </xsl:template>

<!-- Implementation: read() -->

    <xsl:template name="read-impl-load-attributes">
        <xsl:param name="node"/>

        <xsl:for-each select="$node/xs:attribute">
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="qstring-func">
                <xsl:call-template name="xs-type-from-qstring-func">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="val">
                        <xsl:text>node.attribute(QLatin1String("</xsl:text>
                        <xsl:value-of select="$lower-name"/>
                        <xsl:text>"))</xsl:text>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    if (node.hasAttribute(QLatin1String("</xsl:text>
            <xsl:value-of select="$lower-name"/>
            <xsl:text>")))&endl;</xsl:text>
            <xsl:text>        setAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$qstring-func"/>
            <xsl:text>);&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="read-impl-load-child-element">
        <xsl:param name="node"/>

        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="array" select="$node/@maxOccurs = 'unbounded'"/>

            <xsl:text>        if (tag == QLatin1String("</xsl:text>
            <xsl:value-of select="$lower-name"/>
            <xsl:text>")) {&endl;</xsl:text>

            <xsl:choose>
                <xsl:when test="not($array) and $xs-type-cat = 'value'">
                    <xsl:variable name="qstring-func">
                        <xsl:call-template name="xs-type-from-qstring-func">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="val" select="'e.text()'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>            setElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>(</xsl:text>
                    <xsl:value-of select="$qstring-func"/>
                    <xsl:text>);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="$array and $xs-type-cat = 'value'">
                    <xsl:variable name="qstring-func">
                        <xsl:call-template name="xs-type-from-qstring-func">
                            <xsl:with-param name="xs-type" select="@type"/>
                            <xsl:with-param name="val" select="'e.text()'"/>
                        </xsl:call-template>
                    </xsl:variable>

                    <xsl:text>            m_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>.append(</xsl:text>
                    <xsl:value-of select="$qstring-func"/>
                    <xsl:text>);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="not($array) and $xs-type-cat = 'pointer'">
                    <xsl:text>            Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text> *v = new Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text>();&endl;</xsl:text>
                    <xsl:text>            v->read(e);&endl;</xsl:text>
                    <xsl:text>            setElement</xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>(v);&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="$array and $xs-type-cat = 'pointer'">
                    <xsl:text>            Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text> *v = new Dom</xsl:text>
                    <xsl:value-of select="@type"/>
                    <xsl:text>();&endl;</xsl:text>
                    <xsl:text>            v->read(e);&endl;</xsl:text>
                    <xsl:text>            m_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>.append(v);&endl;</xsl:text>
                </xsl:when>
            </xsl:choose>
            <xsl:text>            continue;&endl;</xsl:text>
            <xsl:text>        }&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="read-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:text>void </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>::read(const QDomElement &amp;node)&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>

        <xsl:call-template name="read-impl-load-attributes">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:text>&endl;</xsl:text>

        <xsl:if test="boolean($node/xs:choice) or boolean($node/xs:sequence)">

            <xsl:text>    for (QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling()) {&endl;</xsl:text>
            <xsl:text>        if (!n.isElement())&endl;</xsl:text>
            <xsl:text>            continue;&endl;</xsl:text>
            <xsl:text>        QDomElement e = n.toElement();&endl;</xsl:text>
            <xsl:text>        QString tag = e.tagName().toLower();&endl;</xsl:text>

            <xsl:for-each select="$node/xs:choice">
                <xsl:call-template name="read-impl-load-child-element">
                    <xsl:with-param name="node" select="."/>
                </xsl:call-template>
            </xsl:for-each>

            <xsl:for-each select="$node/xs:sequence">
                <xsl:call-template name="read-impl-load-child-element">
                    <xsl:with-param name="node" select="."/>
                </xsl:call-template>
            </xsl:for-each>

            <xsl:text>    }&endl;&endl;</xsl:text>
        </xsl:if>

        <xsl:text>    m_text.clear();&endl;</xsl:text>
        <xsl:text>    for (QDomNode child = node.firstChild(); !child.isNull(); child = child.nextSibling()) {&endl;</xsl:text>
        <xsl:text>        if (child.isText())&endl;</xsl:text>
        <xsl:text>            m_text.append(child.nodeValue());&endl;</xsl:text>
        <xsl:text>    }&endl;</xsl:text>
        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: write() -->

    <xsl:template name="write-impl-save-attributes">
        <xsl:param name="node"/>

        <xsl:for-each select="$node/xs:attribute">
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>    if (hasAttribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>())&endl;</xsl:text>
            <xsl:text>        e.setAttribute(QLatin1String("</xsl:text>
            <xsl:value-of select="$lower-name"/>
            <xsl:text>"), attribute</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>());&endl;&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="write-impl-save-choice-child-element">
        <xsl:param name="node"/>
        <xsl:variable name="have-kind" select="name($node) = 'xs:choice'"/>
        <xsl:variable name="array" select="$node/@maxOccurs = 'unbounded'"/>

        <xsl:text>    switch(kind()) {&endl;</xsl:text>

        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>        case </xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>: {&endl;</xsl:text>
                <xsl:choose>
                    <xsl:when test="$xs-type-cat = 'value'">
                        <xsl:variable name="qstring-func">
                            <xsl:call-template name="xs-type-to-qstring-func">
                                <xsl:with-param name="xs-type" select="@type"/>
                                <xsl:with-param name="val" select="concat('element', $cap-name, '()')"/>
                            </xsl:call-template>
                        </xsl:variable>

                        <xsl:text>            QDomElement child = doc.createElement(QLatin1String("</xsl:text>
                        <xsl:value-of select="@name"/>
                        <xsl:text>"));&endl;</xsl:text>
                        <xsl:text>            QDomText text = doc.createTextNode(</xsl:text>
                        <xsl:value-of select="$qstring-func"/>
                        <xsl:text>);&endl;</xsl:text>
                        <xsl:text>            child.appendChild(text);&endl;</xsl:text>
                        <xsl:text>            e.appendChild(child);&endl;</xsl:text>
                    </xsl:when>
                    <xsl:when test="$xs-type-cat = 'pointer'">
                        <xsl:variable name="cpp-return-type">
                            <xsl:call-template name="xs-type-to-cpp-return-type">
                                <xsl:with-param name="xs-type" select="@type"/>
                            </xsl:call-template>
                        </xsl:variable>

                        <xsl:text>            </xsl:text>
                        <xsl:value-of select="$cpp-return-type"/>
                        <xsl:text> v = element</xsl:text>
                        <xsl:value-of select="$cap-name"/>
                        <xsl:text>();&endl;</xsl:text>
                        <xsl:text>            if (v != 0) {&endl;</xsl:text>
                        <xsl:text>                QDomElement child = v->write(doc, QLatin1String("</xsl:text>
                        <xsl:value-of select="$lower-name"/>
                        <xsl:text>"));&endl;</xsl:text>
                        <xsl:text>                e.appendChild(child);&endl;</xsl:text>
                        <xsl:text>            }&endl;</xsl:text>
                    </xsl:when>
                </xsl:choose>
            <xsl:text>            break;&endl;</xsl:text>
            <xsl:text>        }&endl;</xsl:text>
        </xsl:for-each>

        <xsl:text>        default:&endl;</xsl:text>
        <xsl:text>            break;&endl;</xsl:text>
        <xsl:text>    }&endl;</xsl:text>
    </xsl:template>

    <xsl:template name="write-impl-save-sequence-child-element">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>
        <xsl:variable name="array" select="$node/@maxOccurs = 'unbounded'"/>

        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="lower-name">
                <xsl:call-template name="lower-text">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="cpp-return-type">
                <xsl:call-template name="xs-type-to-cpp-return-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:choose>
                <xsl:when test="$array">
                    <xsl:text>    for (int i = 0; i &lt; m_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>.size(); ++i) {&endl;</xsl:text>
                    <xsl:text>        </xsl:text>
                    <xsl:value-of select="$cpp-return-type"/>
                    <xsl:text> v = m_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>[i];&endl;</xsl:text>
                    <xsl:choose>
                        <xsl:when test="$xs-type-cat = 'pointer'">
                            <xsl:text>        QDomNode child = v->write(doc, QLatin1String("</xsl:text>
                            <xsl:value-of select="$lower-name"/>
                            <xsl:text>"));&endl;</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:variable name="qstring-func">
                                <xsl:call-template name="xs-type-to-qstring-func">
                                    <xsl:with-param name="xs-type" select="@type"/>
                                    <xsl:with-param name="val" select="'v'"/>
                                </xsl:call-template>
                            </xsl:variable>

                            <xsl:text>        QDomNode child = doc.createElement(QLatin1String("</xsl:text>
                            <xsl:value-of select="$lower-name"/>
                            <xsl:text>"));&endl;</xsl:text>
                            <xsl:text>        child.appendChild(doc.createTextNode(</xsl:text>
                            <xsl:value-of select="$qstring-func"/>
                            <xsl:text>));&endl;</xsl:text>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:text>        e.appendChild(child);&endl;</xsl:text>
                    <xsl:text>    }&endl;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>    if (m_children &amp; </xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>) {&endl;</xsl:text>
                    <xsl:choose>
                        <xsl:when test="$xs-type-cat = 'pointer'">
                            <xsl:text>        e.appendChild(m_</xsl:text>
                            <xsl:value-of select="@name"/>
                            <xsl:text>->write(doc, QLatin1String("</xsl:text>
                            <xsl:value-of select="$lower-name"/>
                            <xsl:text>")));&endl;</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:variable name="qstring-func">
                                <xsl:call-template name="xs-type-to-qstring-func">
                                    <xsl:with-param name="xs-type" select="@type"/>
                                    <xsl:with-param name="val" select="concat('m_', @name)"/>
                                </xsl:call-template>
                            </xsl:variable>
                            <xsl:text>        child = doc.createElement(QLatin1String("</xsl:text>
                            <xsl:value-of select="$lower-name"/>
                            <xsl:text>"));&endl;</xsl:text>
                            <xsl:text>        child.appendChild(doc.createTextNode(</xsl:text>
                            <xsl:value-of select="$qstring-func"/>
                            <xsl:text>));&endl;</xsl:text>
                            <xsl:text>        e.appendChild(child);&endl;</xsl:text>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:text>    }&endl;&endl;</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="write-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>
        <xsl:variable name="lower-name">
            <xsl:call-template name="lower-text">
                <xsl:with-param name="text" select="@name"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:text>QDomElement </xsl:text>
        <xsl:value-of select="$name"/>
        <xsl:text>::write(QDomDocument &amp;doc, const QString &amp;tagName)&endl;</xsl:text>
        <xsl:text>{&endl;</xsl:text>

        <xsl:text>    QDomElement e = doc.createElement(tagName.isEmpty() ? QString::fromUtf8("</xsl:text>
        <xsl:value-of select="$lower-name"/>
        <xsl:text>") : tagName.toLower());&endl;&endl;</xsl:text>
        <xsl:text>    QDomElement child;&endl;&endl;</xsl:text>

        <xsl:call-template name="write-impl-save-attributes">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:for-each select="$node/xs:choice">
            <xsl:call-template name="write-impl-save-choice-child-element">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:for-each select="$node/xs:sequence">
            <xsl:call-template name="write-impl-save-sequence-child-element">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>    if (!m_text.isEmpty())&endl;</xsl:text>
        <xsl:text>        e.appendChild(doc.createTextNode(m_text));&endl;&endl;</xsl:text>

        <xsl:text>    return e;&endl;</xsl:text>
        <xsl:text>}&endl;&endl;</xsl:text>
    </xsl:template>

<!-- Implementation: child element setters -->

    <xsl:template name="child-setter-impl-helper">
        <xsl:param name="node"/>
        <xsl:param name="name"/>
        <xsl:variable name="array" select="$node/@maxOccurs = 'unbounded'"/>
        <xsl:variable name="make-kind-enum" select="name($node) = 'xs:choice'"/>
        <xsl:variable name="make-child-enum" select="name($node) = 'xs:sequence' and not($node/@maxOccurs = 'unbounded')"/>

        <xsl:for-each select="$node/xs:element">
            <xsl:variable name="cap-name">
                <xsl:call-template name="cap-first-char">
                    <xsl:with-param name="text" select="@name"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="return-cpp-type">
                <xsl:call-template name="xs-type-to-cpp-return-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="$array"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="argument-cpp-type">
                <xsl:call-template name="xs-type-to-cpp-argument-type">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="$array"/>
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="xs-type-cat">
                <xsl:call-template name="xs-type-category">
                    <xsl:with-param name="xs-type" select="@type"/>
                    <xsl:with-param name="array" select="$array"/>
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>void </xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:text>::setElement</xsl:text>
            <xsl:value-of select="$cap-name"/>
            <xsl:text>(</xsl:text>
            <xsl:value-of select="$argument-cpp-type"/>
            <xsl:text> a)&endl;</xsl:text>
            <xsl:text>{&endl;</xsl:text>
            <xsl:choose>
                <xsl:when test="$make-kind-enum">
                    <xsl:text>    clear(false);&endl;</xsl:text>
                    <xsl:text>    m_kind = </xsl:text>
                    <xsl:value-of select="$cap-name"/>
                    <xsl:text>;&endl;</xsl:text>
                </xsl:when>
                <xsl:when test="$xs-type-cat = 'pointer'">
                    <xsl:text>    delete </xsl:text>
                    <xsl:text>m_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>;&endl;</xsl:text>
                </xsl:when>
            </xsl:choose>
            <xsl:if test="$make-child-enum">
                <xsl:text>    m_children |= </xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>;&endl;</xsl:text>
            </xsl:if>
            <xsl:text>    m_</xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:text> = a;&endl;</xsl:text>
            <xsl:text>}&endl;&endl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="child-setter-impl">
        <xsl:param name="node"/>
        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>

        <xsl:for-each select="$node/xs:choice">
            <xsl:call-template name="child-setter-impl-helper">
                <xsl:with-param name="node" select="."/>
                <xsl:with-param name="name" select="$name"/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:for-each select="$node/xs:sequence">
            <xsl:call-template name="child-setter-impl-helper">
                <xsl:with-param name="node" select="."/>
                <xsl:with-param name="name" select="$name"/>
            </xsl:call-template>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="child-clear-impl">
        <xsl:param name="node"/>

        <xsl:variable name="name" select="concat('Dom', $node/@name)"/>
        <xsl:variable name="array" select="$node/xs:sequence/@maxOccurs = 'unbounded'"/>
        <xsl:variable name="make-child-enum" select="boolean($node/xs:sequence) and not($array)"/>

        <xsl:if test="$make-child-enum">
            <xsl:for-each select="$node/xs:sequence/xs:element">
                <xsl:variable name="cap-name">
                    <xsl:call-template name="cap-first-char">
                        <xsl:with-param name="text" select="@name"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:variable name="xs-type-cat">
                    <xsl:call-template name="xs-type-category">
                        <xsl:with-param name="xs-type" select="@type"/>
                        <xsl:with-param name="array" select="$array"/>
                    </xsl:call-template>
                </xsl:variable>

                <xsl:text>void </xsl:text>
                <xsl:value-of select="$name"/>
                <xsl:text>::clearElement</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>()&endl;</xsl:text>
                <xsl:text>{&endl;</xsl:text>
                <xsl:if test="$xs-type-cat = 'pointer'">
                    <xsl:text>    delete m_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text>;&endl;</xsl:text>
                    <xsl:text>    m_</xsl:text>
                    <xsl:value-of select="@name"/>
                    <xsl:text> = 0;&endl;</xsl:text>
                </xsl:if>
                <xsl:text>    m_children &amp;= ~</xsl:text>
                <xsl:value-of select="$cap-name"/>
                <xsl:text>;&endl;</xsl:text>
                <xsl:text>}&endl;&endl;</xsl:text>
            </xsl:for-each>
        </xsl:if>

    </xsl:template>


<!-- Implementation -->

    <xsl:template name="class-implementation">
        <xsl:param name="node"/>

        <xsl:call-template name="clear-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="ctor-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="dtor-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="read-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="write-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="child-setter-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

        <xsl:call-template name="child-clear-impl">
            <xsl:with-param name="node" select="$node"/>
        </xsl:call-template>

    </xsl:template>

<!-- Root -->

    <xsl:template match="xs:schema">

<xsl:text>/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
</xsl:text>
        <xsl:text>#include "ui4_p.h"&endl;</xsl:text>
        <xsl:text>#include &lt;QDomDocument&gt;&endl;</xsl:text>
        <xsl:text>&endl;</xsl:text>

        <xsl:text>#ifdef QFORMINTERNAL_NAMESPACE&endl;</xsl:text>
        <xsl:text>using namespace QFormInternal;&endl;</xsl:text>
        <xsl:text>#endif&endl;</xsl:text>
        <xsl:text>&endl;</xsl:text>

        <xsl:text>/*******************************************************************************&endl;</xsl:text>
        <xsl:text>** Implementations&endl;</xsl:text>
        <xsl:text>*/&endl;&endl;</xsl:text>

        <xsl:for-each select="xs:complexType">
            <xsl:call-template name="class-implementation">
                <xsl:with-param name="node" select="."/>
            </xsl:call-template>
        </xsl:for-each>

        <xsl:text>&endl;</xsl:text>
    </xsl:template>

</xsl:stylesheet>
