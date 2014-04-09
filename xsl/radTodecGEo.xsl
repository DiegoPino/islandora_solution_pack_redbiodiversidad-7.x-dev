{\rtf1\ansi\ansicpg1252\cocoartf1187\cocoasubrtf400
{\fonttbl\f0\fnil\fcharset0 Consolas;}
{\colortbl;\red255\green255\blue255;\red107\green0\blue3;\red234\green234\blue234;\red251\green0\blue7;
\red0\green0\blue255;}
\paperw11900\paperh16840\margl1440\margr1440\vieww10800\viewh8400\viewkind0
\deftab720
\pard\pardeftab720

\f0\fs28 \cf2 \cb3 <xsl:stylesheet\cf0  \cf4 version\cf0 =\cf5 "1.0"\cf0 \
 \cf4 xmlns:xsl\cf0 =\cf5 "http://www.w3.org/1999/XSL/Transform"\cf2 >\cf0 \
 \cf2 <xsl:output\cf0  \cf4 omit-xml-declaration\cf0 =\cf5 "yes"\cf0  \cf4 indent\cf0 =\cf5 "yes"\cf2 />\cf0 \
 \cf2 <xsl:strip-space\cf0  \cf4 elements\cf0 =\cf5 "*"\cf2 />\cf0 \
\
 \cf2 <xsl:template\cf0  \cf4 match\cf0 =\cf5 "node()|@*"\cf2 >\cf0 \
  \cf2 <xsl:copy>\cf0 \
   \cf2 <xsl:apply-templates\cf0  \cf4 select\cf0 =\cf5 "node()|@*"\cf2 />\cf0 \
  \cf2 </xsl:copy>\cf0 \
 \cf2 </xsl:template>\cf0 \
\
 \cf2 <xsl:template\cf0  \cf4 match\cf0 =\cf5 "lat/text() | long/text()"\cf2 >\cf0 \
  \cf2 <xsl:call-template\cf0  \cf4 name\cf0 =\cf5 "DegreesToDecimal"\cf2 />\cf0 \
 \cf2 </xsl:template>\cf0 \
\
 \cf2 <xsl:template\cf0  \cf4 name\cf0 =\cf5 "DegreesToDecimal"\cf2 >\cf0 \
  \cf2 <xsl:param\cf0  \cf4 name\cf0 =\cf5 "pDegrees"\cf0  \cf4 select\cf0 =\cf5 "."\cf2 />\cf0 \
\
  \cf2 <xsl:variable\cf0  \cf4 name\cf0 =\cf5 "vDegrees"\cf0  \cf4 select\cf0 =\
   \cf5 "number(substring-before($pDegrees, ':'))"\cf2 />\cf0 \
\
  \cf2 <xsl:variable\cf0  \cf4 name\cf0 =\cf5 "vMinutes"\cf0  \cf4 select\cf0 =\
   \cf5 "number(\
       substring-before(\
           substring-after($pDegrees, ':'),\
           ':'\
                       )\
           )"\cf2 />\cf0 \
\
  \cf2 <xsl:variable\cf0  \cf4 name\cf0 =\cf5 "vSeconds"\cf0  \cf4 select\cf0 =\
   \cf5 "number(\
       substring-after(\
           substring-after($pDegrees, ':'),\
           ':'\
                       )\
           )"\cf2 />\cf0 \
\
  \cf2 <xsl:value-of\cf0  \cf4 select\cf0 =\
   \cf5 "$vDegrees\
  +\
    $vMinutes div 60\
  +\
    $vSeconds div 3600\
  "\cf2 />\cf0 \
 \cf2 </xsl:template>\cf0 \
\cf2 </xsl:stylesheet>}