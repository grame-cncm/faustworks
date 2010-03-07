/*
 * FaustHighlighter.cpp
 *
 * Created by Christophe Daudin on 12/05/09.
 * Copyright 2009 Grame. All rights reserved.
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 2.1 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU Lesser General Public License version 2.1 requirements
 * will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "FaustHighlighter.h"
//#include <execinfo.h> only on unix
#include <iostream>
using namespace std;

#define WHITE				Qt::white
#define TRANSPARENT			Qt::transparent

#define VIOLET				QColor(72,66,139)
#define LIGHT_YELLOW		QColor(250,250,210)
#define LIGHT_GREEN			QColor(240,255,240)
#define FLORAL_GREEN		QColor(255,250,240)
#define DARK_OLIVE_GREEN	QColor(85,107,47)
#define DODGER_BLUE_4		QColor(16,78,139)
#define INDIAN_RED_1		QColor(255,106,106)
#define INDIAN_RED_2		QColor(190,84,83)
#define DARK_SEA_GREEN_1	QColor(193,255,193)
#define DARK_SEA_BLUE_1		QColor(193,193,255)
#define LAVENDER_BLUSH		QColor(255,240,245)
#define GREEN_4				QColor(0,139,0)
#define ORANGE_1			QColor(240,110,36)
#define ORANGE_2			QColor(209,110,36)
#define DEEP_BLUE			QColor(31,68,110)
#define DEEP_TURQUOISE		QColor(31,109,110)
#define DEEP_PURPLE			QColor( 70 , 0 , 83 )

/*
#define SQUARE_BRACKETS_FONT	DEEP_PURPLE
#define SQUARE_BRACKETS_BACK	TRANSPARENT
#define SQUARE_BRACKETS_WEIGHT	QFont::Black

#define CURLY_BRACKETS_FONT		DEEP_PURPLE
#define CURLY_BRACKETS_BACK		TRANSPARENT
#define CURLY_BRACKETS_WEIGHT	QFont::Black

#define NOTE_NAMES_FONT		DEEP_BLUE
#define NOTE_NAMES_BACK		TRANSPARENT
#define NOTE_NAMES_WEIGHT	QFont::Bold

#define ACCIDENTALS_FONT	DEEP_BLUE
#define ACCIDENTALS_BACK	TRANSPARENT
#define ACCIDENTALS_WEIGHT	QFont::Bold

#define DURATIONS_FONT		INDIAN_RED_2
#define DURATIONS_BACK		TRANSPARENT
#define DURATIONS_WEIGHT	QFont::Bold

#define TAGS_FONT			ORANGE_2
#define TAGS_BACK			TRANSPARENT
#define TAGS_WEIGHT			QFont::Bold

#define FULL_TAGS_FONT		Qt::black
#define FULL_TAGS_BACK		TRANSPARENT
#define FULL_TAGS_WEIGHT	QFont::Bold
*/

#define COMMENTS_FONT       	GREEN_4
#define COMMENTS_BACK       	TRANSPARENT
#define COMMENTS_WEIGHT     	QFont::Normal

#define MDOC_FONT               Qt::blue
#define MDOC_BACK               TRANSPARENT
#define MDOC_WEIGHT             QFont::Normal

#define QUOTED_STRING_FONT		Qt::red
#define QUOTED_STRING_BACK		TRANSPARENT
#define QUOTED_STRING_WEIGHT	QFont::Normal

#define NUMBER_FONT				Qt::blue
#define NUMBER_BACK				TRANSPARENT
#define NUMBER_WEIGHT			QFont::Normal

#define FAUST_COMBINATOR_FONT	Qt::red
#define FAUST_COMBINATOR_BACK	TRANSPARENT
#define FAUST_COMBINATOR_WEIGHT	QFont::Bold

#define OPERATORS1_FONT			ORANGE_2
#define OPERATORS1_BACK			TRANSPARENT
#define OPERATORS1_WEIGHT		QFont::Bold

#define OPERATORS2_FONT			DEEP_BLUE
#define OPERATORS2_BACK			TRANSPARENT
#define OPERATORS2_WEIGHT		QFont::Bold

#define VARIABLES_FONT			Qt::black
#define VARIABLES_BACK			TRANSPARENT
#define VARIABLES_WEIGHT		QFont::Normal

#define OPERATORS1 "\\b(component|library|environment|mem|prefix|int|float|rdtable|rwtable|select2|select3|ffunction|fconstant|fvariable|button|checkbox|vslider|hslider|nentry|vbargraph|hbargraph|attach|acos|asin|atan|atan2|cos|sin|tan|exp|log|log10|pow|sqrt|abs|min|max|fmod|remainder|floor|ceil|rint)\\b"
#define OPERATORS2 "\\b(process|with|case|seq|par|sum|prod|import|vgroup|hgroup|tgroup|declare)\\b"
#define FAUST_COMBINATORS "(<:|:>|,|~|:)"
//-------------------------------------------------------------------------
FaustHighlighter::FaustHighlighter(QTextDocument *parent)
	:MusicNotationHighlighter(parent)
{	
/*
	//					SPECIAL CHARS
	QStringList squareBracketPattern;
	squareBracketPattern
	<<   "\\["
	<<   "\\]"
	;
	appendLineRule(squareBracketPattern, buildTextCharFormat( SQUARE_BRACKETS_FONT,SQUARE_BRACKETS_BACK,SQUARE_BRACKETS_WEIGHT ),false);

	QStringList curlyBracketPattern;
	curlyBracketPattern
	<<   "\\{"
	<<   "\\}"
	;
	appendLineRule(curlyBracketPattern, buildTextCharFormat( CURLY_BRACKETS_FONT,CURLY_BRACKETS_BACK,CURLY_BRACKETS_WEIGHT ),false);
*/

	//Variables
	
	{
		QStringList pattern;
		pattern << "\\b([a-z]|[A-Z])(\\w*)\\b" ;
		appendLineRule(pattern, buildTextCharFormat( VARIABLES_FONT, VARIABLES_BACK , VARIABLES_WEIGHT ),false);
	}

	//Numbers
	appendLineRule(QStringList() << "\\b((\\d*\\.?\\d+)|(\\d+\\.?\\d*))\\b", buildTextCharFormat( NUMBER_FONT, NUMBER_BACK , NUMBER_WEIGHT ),false);
	
	appendLineRule(QStringList() << FAUST_COMBINATORS, buildTextCharFormat( FAUST_COMBINATOR_FONT, FAUST_COMBINATOR_BACK , FAUST_COMBINATOR_WEIGHT ),false);

	//							TAGS
	//the tag itself
	{
		QStringList pattern;
		pattern << OPERATORS1 ;
		appendLineRule(pattern, buildTextCharFormat( OPERATORS1_FONT,OPERATORS1_BACK,OPERATORS1_WEIGHT ),false);
	}
	{
		QStringList pattern;
		pattern << OPERATORS2 ;
		appendLineRule(pattern, buildTextCharFormat( OPERATORS2_FONT,OPERATORS2_BACK,OPERATORS2_WEIGHT ),false);
	}
	{
		QStringList pattern;
		pattern << "(\".*\")";
		appendLineRule(pattern, buildTextCharFormat( QUOTED_STRING_FONT,QUOTED_STRING_BACK,QUOTED_STRING_WEIGHT ),true);
	}

	QStringList commentsPatterns;
	commentsPatterns <<"//.*";
	appendLineRule(commentsPatterns, buildTextCharFormat( COMMENTS_FONT,COMMENTS_BACK,COMMENTS_WEIGHT ),false);

    QString startCommentsPatterns("/\\*");
    QString endCommentsPatterns("\\*/");
    appendMultilineRule(startCommentsPatterns,endCommentsPatterns,
            buildTextCharFormat( COMMENTS_FONT,COMMENTS_BACK,COMMENTS_WEIGHT ),true);

    QString startMDocPatterns("<mdoc>");
    QString endMDocPatterns("</mdoc>");
    appendMultilineRule(startMDocPatterns,endMDocPatterns,
            buildTextCharFormat( MDOC_FONT,MDOC_BACK,MDOC_WEIGHT ),true);


}
