/*---------------------------------------------------
	this file implements the Lexer
	TODO：分号结尾处理(;)
---------------------------------------------------*/

#include "Lexer.h"

//识别符号
bool Lexer::issymble(int LastChar) {
	bool ans = LastChar == '!' || LastChar == '%' || LastChar == '&' ||
		LastChar == '$' || LastChar == '#' || LastChar == '+' ||
		LastChar == '-' || LastChar == '*' || LastChar == '/' ||
		LastChar == ':' || LastChar == '<' || LastChar == '=' ||
		LastChar == '>' || LastChar == '?' || LastChar == '@' ||
		LastChar == '\\' || LastChar == '~' || LastChar == '`' ||
		LastChar == '^' || LastChar == '|' || LastChar == '.';
	return ans;
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
		else {
			LexerError("illegal expression of negative '~'");
			return tok_error;// 报错，E后只有~没有数字
		}
	}
	else {
		LexerError("illegal expression of scientific notation 'E'");
		return tok_error;// 报错，E后不为数
	}
}

//lex number when meet '.'
int Lexer::readPoint(int* LastChar, string* NumStr) {
	*NumStr += ".";
	*LastChar = getchar();
	while (isdigit(*LastChar)) {
		NumStr += *LastChar;
		*LastChar = getchar();
	}
	if (*LastChar == '.') {
		LexerError("illegal expression of decimal '.'");
		return tok_error;// 报错，出现多余的小数点
	}
	else if (*LastChar == 'E')
		return readE(LastChar, NumStr);
	else {
		NumVal = strtod((*NumStr).c_str(), nullptr);
		return tok_real;
	}	
}

// lex number
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

// lex escape sequence of character
int Lexer::readEscapeSequence(bool isStr, int* LastChar) {
	*LastChar = getchar();
	if (*LastChar == '\"')
		if (isStr)
			StrVal += '\"';
		else
			CharVal = '\"';
	else if (*LastChar == '\\')
		if (isStr)
			StrVal += '\\';
		else
			CharVal = '\\';
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
	else if (*LastChar == '\n' || *LastChar == '\t' || *LastChar == ' ' || *LastChar == '\n') {
		*LastChar = getchar();
		while (*LastChar != '\n' && *LastChar == '\t' && *LastChar == ' ' && *LastChar == '\n')
			*LastChar = getchar();
		if (*LastChar == '\\')
			return 0;
		else {
			LexerError("illegal expression of character");
			return tok_error;
		}
	}
	else {
		LexerError("illegal expression of character");
	}
		
}

// lex string or char
// NOTE: 转义序列包括 \n \t \\ \"
// TODO: 字符串时的转义序列识别
int Lexer::readStr(bool isStr,int* LastChar) {
	*LastChar = getchar();
	if (isStr) {
		StrVal = "";
		if (*LastChar == '\"') { //识别空字符串
			return tok_string;
		}
		while (*LastChar != '\"') {
			StrVal += *LastChar;
			*LastChar = getchar();
			if (*LastChar == '\"')
				if (StrVal[StrVal.length() - 1] != '\\')// what if "\\" 
					break;
				else
					continue;
			if (*LastChar == '\n') { //error，字符串未闭合
				LexerError("unclosed string");
				return tok_error;
			}
		}
		// 跳过 " 
		*LastChar = getchar();
		return tok_string;
	}
	else {	
		if (*LastChar == '\"') { //报错，字符不能为空
			LexerError("character cannot be empty");
			return tok_error;
		}
		if (*LastChar == '\n') { //报错，字符未闭合
			LexerError("unclosed character");
			return tok_error;
		}
		if (*LastChar == '\\') {
			*LastChar = getchar();
			if (*LastChar == '\\')
				CharVal = '\\';
			else if (*LastChar == 'n')
				CharVal = '\n';
			else if (*LastChar == 't')
				CharVal = '\t';
			else if (*LastChar == '\"')
				CharVal = '\"';
			else {
				LexerError("length of character not 1");
				return tok_error;//不合法的字符
			}
		}
		else {
			CharVal = *LastChar;		
		}
		*LastChar = getchar();
		if (*LastChar == '\"') {
			*LastChar = getchar();
			return tok_char;
		}
		else {
			LexerError("length of character not 1");
			return tok_error;//字符长度超过1 or 字符未闭合
		}
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
				else if (LastChar == '\n') {
					LexerError("unclosed annotation");
					return tok_error; //报错，注释未闭合
				}
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


