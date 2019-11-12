/*---------------------------------------------------
	this file implements the Lexer
	TODO：分号结尾处理(;)
---------------------------------------------------*/

#include "Lexer.h"


//识别符号
bool Lexer::issymble(int LastChar) {
	return LastChar == '!' || LastChar == '%' || LastChar == '&' ||
		LastChar == '$' || LastChar == '#' || LastChar == '+' ||
		LastChar == '-' || LastChar == '*' || LastChar == '/' ||
		LastChar == ':' || LastChar == '<' || LastChar == '=' ||
		LastChar == '>' || LastChar == '?' || LastChar == '@' ||
		LastChar == '\\' || LastChar == '~' || LastChar == '`' ||
		LastChar == '^' || LastChar == '|' || LastChar == '.';
}

//lex number when meet 'E'
int Lexer::readE(int* LastChar, string* NumStr) {
	*NumStr += "E";
	*LastChar = getchar();
	if (isdigit(*LastChar)) {
		while (isdigit(*LastChar)) {
			*NumStr += *LastChar;
			*LastChar = getchar();
		}
		NumVal = strtod((*NumStr).c_str(), nullptr);
		return tok_real;
	}
	else if (*LastChar == '~') {
		*NumStr += "-";
		*LastChar = getchar();
		if (isdigit(*LastChar)) {
			while (isdigit(*LastChar)) {
				*NumStr += *LastChar;
				*LastChar = getchar();
			}
			NumVal = strtod((*NumStr).c_str(), nullptr);
			return tok_real;
		}
		else
			return LexerError("illegal expression of negative '~'");// 报错，E后只有~没有数字
	}
	else		
		return LexerError("illegal expression of scientific notation 'E'");// 报错，E后不为数
}

//lex number when meet '.'
int Lexer::readPoint(int* LastChar, string* NumStr) {
	*NumStr += ".";
	*LastChar = getchar();
	while (isdigit(*LastChar)) {
		NumStr += *LastChar;
		*LastChar = getchar();
	}
	if (*LastChar == '.')
		return LexerError("illegal expression of decimal '.'");// 报错，出现多余的小数点
	else if (*LastChar == 'E')
		return readE(LastChar, NumStr);
	else {
		NumVal = strtod((*NumStr).c_str(), nullptr);
		return tok_real;
	}	
}

// lex number(int and real)
int Lexer::readNum(bool isNegative,int* LastChar) {
	string NumStr = "";
	if (isNegative)
		NumStr += "-";
	while (isdigit(*LastChar)) {
		NumStr += *LastChar;
		*LastChar = getchar();	
	}
	if (*LastChar == '.')
		return readPoint(LastChar, &NumStr);
	else if (*LastChar == 'E')
		return readE(LastChar, &NumStr);
	else {
		NumVal = strtod(NumStr.c_str(), nullptr);
		return tok_int;
	}
}

// lex escape sequence of character and string
// NOTE: 转义序列包括: '\n' '\t' '\v' '\b' '\r' '\f' '\a' 
//                    '\\' '\"' '\ 空格，制表符 换行 换页 \'
int Lexer::readEscapeSequence(bool isStr, int* LastChar) {
	*LastChar = getchar();
	if (*LastChar == '\"' || *LastChar == '\\')
		if (isStr)
			StrVal += *LastChar;
		else
			CharVal = *LastChar;
	else if(*LastChar == 'n')
		if (isStr)
			StrVal += '\n';
		else
			CharVal = '\n';
	else if (*LastChar == 't')
		if (isStr)
			StrVal += '\t';
		else
			CharVal = '\t';
	else if (*LastChar == 'v')
		if (isStr)
			StrVal += '\v';
		else
			CharVal = '\v';
	else if (*LastChar == 'b')
		if (isStr)
			StrVal += '\b';
		else
			CharVal = '\b';
	else if (*LastChar == 'r')
		if (isStr)
			StrVal += '\r';
		else
			CharVal = '\r';
	else if (*LastChar == 'f')
		if (isStr)
			StrVal += '\f';
		else
			CharVal = '\f';
	else if (*LastChar == 'a')
		if (isStr)
			StrVal += '\a';
		else
			CharVal = '\a';
	else if (*LastChar == '\n' || *LastChar == '\t' || *LastChar == ' ' || *LastChar == '\f') {
		*LastChar = getchar();
		while (*LastChar == '\n' || *LastChar == '\t' || *LastChar == ' ' || *LastChar == '\f')
			*LastChar = getchar();
		if (*LastChar != '\\')			
			return LexerError("illegal expression of character");
		return 0;
	}
	else		
		return LexerError("illegal expression of character");
	return 1;
}

// lex string or char
int Lexer::readStr(bool isStr, int* LastChar) {
	*LastChar = getchar();
	if (isStr) {
		StrVal = "";
		while (*LastChar != '\"') {
			if (*LastChar == '\n' || *LastChar == '\t')
				return LexerError("illegal non-printing character in string");
			else if (*LastChar == '\\') {
				if (readEscapeSequence(1, LastChar) == tok_error)
					return tok_error;
				*LastChar = getchar();
			}
			StrVal += *LastChar;
			*LastChar = getchar();
		}
		// 跳过 " 
		*LastChar = getchar();
		return tok_string;
	}
	else {
		bool isSetChar = false;
		if (*LastChar == '\"')		
			return LexerError("character cannot be empty"); //报错，字·符不能为空
		while (*LastChar == '\\') {
			int i = readEscapeSequence(0, LastChar);
			if (i == tok_error)
				return tok_error;
			else if (i == 1 && !isSetChar)
				isSetChar = true;
			else if (i == 1 && isSetChar)			
				return LexerError("length of character not 1");
			*LastChar = getchar();
		}
		if (!isSetChar) {
			if (*LastChar == '\n' || *LastChar == '\t')				
				return LexerError("illegal non-printing character");
			CharVal = *LastChar;
			*LastChar = getchar();
		}
		else
			while (*LastChar != '\\') {
				int i = readEscapeSequence(0, LastChar);
				if (i == tok_error)
					return tok_error;
				else if (i == 1)				
					return LexerError("length of character not 1");
				*LastChar = getchar();
			}
		if (*LastChar == '\"') {
			*LastChar = getchar();
			return tok_char;
		}
		else
			
			return LexerError("length of character not 1"); //字符长度超过1 or 字符未闭合
	}
}

/// gettok - Return the next token from standard input.
int Lexer::gettok() {

	static int LastChar = ' ';

	// 跳过空格,tab键，换行
	while (isspace(LastChar))
		LastChar = getchar();

	//字母名字、关键字
	if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9|_|']*
		IdentifierStr = LastChar;
		while (isalnum((LastChar = getchar())) || LastChar == '_' || 
			LastChar == '\'')
			IdentifierStr += LastChar;
		//排除保留字
		if (IdentifierStr == "val")
			return tok_val;
		if (IdentifierStr == "abstype")
			return tok_abstype;
		if (IdentifierStr == "and")
			return tok_and;
		if (IdentifierStr == "andalso")
			return tok_andalso;
		if (IdentifierStr == "as")
			return tok_as;
		if (IdentifierStr == "case")
			return tok_case;
		if (IdentifierStr == "datatype")
			return tok_datatype;
		if (IdentifierStr == "do")
			return tok_do;
		if (IdentifierStr == "else")
			return tok_else;
		if (IdentifierStr == "end")
			return tok_end;
		if (IdentifierStr == "eqtype")
			return tok_eqtype;
		if (IdentifierStr == "exception")
			return tok_exception;
		if (IdentifierStr == "fn")
			return tok_fn;
		if (IdentifierStr == "fun")
			return tok_fun;
		if (IdentifierStr == "functor")
			return tok_functor;
		if (IdentifierStr == "handle")
			return tok_handle;
		if (IdentifierStr == "if")
			return tok_if;
		if (IdentifierStr == "in")
			return tok_in;
		if (IdentifierStr == "include")
			return tok_include;
		if (IdentifierStr == "infix")
			return tok_infix;
		if (IdentifierStr == "infixer")
			return tok_infixr;
		if (IdentifierStr == "let")
			return tok_let;
		if (IdentifierStr == "local")
			return tok_local;
		if (IdentifierStr == "nonfix")
			return tok_nonfix;
		if (IdentifierStr == "of")
			return tok_of;
		if (IdentifierStr == "op")
			return tok_op;
		if (IdentifierStr == "open")
			return tok_open;
		if (IdentifierStr == "orelse")
			return tok_orelse;
		if (IdentifierStr == "raise")
			return tok_raise;
		if (IdentifierStr == "rec")
			return tok_rec;
		if (IdentifierStr == "sharing")
			return tok_sharing;
		if (IdentifierStr == "sig")
			return tok_sig;
		if (IdentifierStr == "signature")
			return tok_signature;
		if (IdentifierStr == "struct")
			return tok_struct;
		if (IdentifierStr == "structure")
			return tok_structure;
		if (IdentifierStr == "then")
			return tok_then;
		if (IdentifierStr == "type")
			return tok_type;
		if (IdentifierStr == "where")
			return tok_where;
		if (IdentifierStr == "while")
			return tok_while;
		if (IdentifierStr == "with")
			return tok_with;
		if (IdentifierStr == "withtype")
			return tok_withtype;

		//识别保留字true、false，返回tok_bool
		if (IdentifierStr == "false") {
			BoolVal = false;
			return tok_bool;
		}
		if (IdentifierStr == "true") {
			BoolVal = true;
			return tok_bool;
		}
		return tok_identifier;
	}

	//符号名字、保留符号
	if (issymble(LastChar)) { // identifier: [!|%|&|$|#|+|-|*|/|:|<|=|>|?|@|\|~|`|^||]*
							   // - [:|||=|=>|->|#|:>|+|-|*|/|>|<|<=|>=|<>|^]
		IdentifierStr = LastChar;
		while (issymble((LastChar = getchar())))
			IdentifierStr += LastChar;

		if (IdentifierStr == ":")
			return ':'; 
		if (IdentifierStr == "|")
			return '|'; 
		if (IdentifierStr == "=")
			return '='; 
		if (IdentifierStr == "=>")
			return '=>'; 
		if (IdentifierStr == "->")
			return '->'; 
		if (IdentifierStr == ":>")
			return ':>'; 
		if (IdentifierStr == "+")
			return '+';
		if (IdentifierStr == "-")
			return '-'; 
		if (IdentifierStr == "*")
			return '*';
		if (IdentifierStr == "/")
			return '/';
		if (IdentifierStr == ">")
			return '>';
		if (IdentifierStr == "<")
			return '<';
		if (IdentifierStr == "<=")
			return '<=';
		if (IdentifierStr == ">=")
			return '>=';
		if (IdentifierStr == "<>")
			return '<>';
		if (IdentifierStr == "^")
			return '^';

		//区分字符串及 # 标识符
		if (IdentifierStr == "#") {
			if (LastChar == '"')
				return readStr(false,&LastChar);
			else 
				return '#';
			//~符号后面：跟"------识别为字符串，其他------识别为特殊符号。
		}

		//区分负数及 ~ 标识符
		if (IdentifierStr == "~") {
			if (isdigit(LastChar)) 
				return readNum(true,&LastChar);
			else return tok_identifier;
				//~符号后面：跟数字------识别为负号，其他------识别为标识符。
		}

		return tok_identifier;
	}

	//字符串
	if (LastChar == '"')
		readStr(true,&LastChar);

	//数字
	if (isdigit(LastChar)) // Number: [0-9.]+
		return  readNum(false,&LastChar);

	//注释 到语法分析处理
	//注意！！注释需要识别两个字符 词法getchar()没办法回退 只能把(、*分别识别并返回ascii码
	//(**)、(aBC+b)
	if (LastChar == '(') {
		if ((LastChar = getchar()) != '*')
			return '(';
		else
			while (1) {
				LastChar = getchar();
				if (LastChar == '*')
					if ((LastChar = getchar()) == ')')//对于注释直接跳过
						return gettok();
					else
						continue;
				else if (LastChar == '\n')				
					return LexerError("unclosed annotation"); //报错，注释未闭合
			}
	}

	//TODO：解决分号结尾处理(;)


	// Otherwise, just return the character as its ascii value.
	int ThisChar = LastChar;
	LastChar = getchar();
	return ThisChar;

	//结束程序
	if (LastChar == '\x03')
		return tok_exit;
}


