//#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
	tok_eof = -1,

	// commands
	tok_def = -2, 
	tok_extern = -3,

	// primary
	tok_identifier = -4,
	tok_number = -5, //0-9之一

	// keywords
	tok_val = -11,
	tok_real = -12,
	tok_int = -13,
	tok_char = -14,
	tok_string = -15,

	tok_fn = -16,
	tok_fun = -17,
	tok_functor = -55,
	tok_type = -18,
	tok_datatype = -19,
	tok_abstype = -20,
	tok_eqtype = -21,
	tok_withtype = -22,
	tok_handle = -23,
	tok_infix = -24,
	tok_nonfix = -25,
	tok_infixr = -26,
	tok_local = -27,
	tok_sharing = -28,
	tok_and = -29,
	tok_as = -30,
	tok_with = -31,
	tok_then = -32,
	tok_where = -33,
	tok_include = -34,
	tok_of = -35,
	tok_open = -36,
	tok_op = -37,
	tok_raise = -38,
	tok_rec = -39,
	tok_exception = -40,
	tok_andalso = -41,
	tok_orelse = -42,
	tok_do = -43,
	tok_while = -44,
	tok_if = -45,
	tok_else = -46,
	tok_case = -47,
	tok_let = -48,
	tok_in = -49,
	tok_end = -50,
	tok_sig = -51,
	tok_signature = -52,
	tok_struct = -53,
	tok_structure = -54,
};

static std::string IdentifierStr; // Filled in if tok_identifier
static char CharVal;
static std::string StringVal;
static double NumVal; // Filled in if tok_int or double
static int LastChar;
//识别符号
static bool issymble(int LastChar) {
	bool ans = LastChar == '!' || LastChar == '%' || LastChar == '&' ||
		LastChar == '$' || LastChar == '#' || LastChar == '+' ||
		LastChar == '-' || LastChar == '*' || LastChar == '/' ||
		LastChar == ':' || LastChar == '<' || LastChar == '=' ||
		LastChar == '>' || LastChar == '?' || LastChar == '@' ||
		LastChar == '\\' || LastChar == '~' || LastChar == '\'' ||
		LastChar == '^' || LastChar == '|';
	return ans;
}

static int readNul(std::string NumStr) {
	bool flag1 = 0;
	bool flag2 = 0;
	do {
		if (LastChar == '.') {
			if (flag1 == 0)
				flag1 = 1;
			else
				break; // show exeption!! 超过一个小数点
		}
		if (LastChar == 'E') {
			if (flag2 == 0)
				flag2 = 1;
			else
				break; // show exeption!! 超过一个E
		}
		NumStr += LastChar;
		LastChar = getchar();
	} while (isdigit(LastChar) || LastChar == '.' || LastChar == 'E');

	NumVal = strtod(NumStr.c_str(), nullptr);
	if (flag1 == 0)
		return tok_int; // int
	else
		return tok_real; // real
}
static int readStr() {
	LastChar = getchar();
	CharVal = LastChar;
	StringVal = LastChar;
	while (LastChar != '"')
		StringVal += getchar();
	if (StringVal.length() < 2) {
		return tok_char;
	}
	else {
		return tok_string;
	}
}

/// gettok - Return the next token from standard input.
static int gettok() {

	static int LastChar = ' ';
	// 跳过空格
	while (isspace(LastChar))
		LastChar = getchar();

	//跳过换行
	while (LastChar == '\\') {
		LastChar = getchar();
		if (LastChar == 'n' || LastChar == 't') {
			LastChar = getchar();
		}
		else {
			break;
		}
	}

	//字母名字、关键字
	if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9|_|']*
		IdentifierStr = LastChar;
		while (isalnum((LastChar = getchar())) || LastChar == '_' ||
			LastChar == '\'')
			IdentifierStr += LastChar;

		if (IdentifierStr == "def")
			return tok_def;
		if (IdentifierStr == "extern")
			return tok_extern;

		//  if (IdentifierStr == "it")
		//     return tok_it;

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
		if (IdentifierStr == "val")
			return tok_val;

		if (IdentifierStr == "where")
			return tok_where;
		if (IdentifierStr == "while")
			return tok_while;
		if (IdentifierStr == "with")
			return tok_with;
		if (IdentifierStr == "withtype")
			return tok_withtype;
		return tok_identifier;
	}

	//符号名字、保留符号
	if (issymble(
		LastChar)) { // identifier: [!|%|&|$|#|+|-|*|/|:|<|=|>|?|@|\|~|'|^||]*
					 // - [:|||=|=>|->|#|:>]
		IdentifierStr = LastChar;
		while (issymble((LastChar = getchar())))
			IdentifierStr += LastChar;

		if (IdentifierStr == ":")
			return ':'; ////
		if (IdentifierStr == "|")
			return '|'; ////
		if (IdentifierStr == "=")
			return '='; ////
		if (IdentifierStr == "=>")
			return '=>'; ////
		if (IdentifierStr == "->")
			return '->'; ////
		if (IdentifierStr == ":>")
			return ':>'; ////

		//区分字符串及 # 标识符
		if (IdentifierStr == "#") {
			if (LastChar == '"')
				return readStr();
			else return '#';
			//~符号后面：跟“------识别为字符串，其他------识别为特殊符号。
		}

		//区分负数及 ~ 标识符
		if (IdentifierStr == "~") {
			if (isdigit(LastChar)) {
				std::string NumStr = "-";
				return readNul(NumStr);
			}
			else return tok_identifier;
				//~符号后面：跟数字------识别为负号，其他------识别为标识符。
		}

		return tok_identifier;
	}

	//！！
	//字符串
	if (LastChar == '"') {
		readStr();
	}
	//字符


	//数字
	if (isdigit(LastChar)) { // Number: [0-9.]+
		std::string NumStr;
		return  readNul(NumStr);
	}

	//注释 到语法分析处理
	//注意！！注释需要识别两个字符 词法getchar()没办法回退 只能把(、*分别识别并返回ascii码
	//(**)
	//(aBC+b)


	// Otherwise, just return the character as its ascii value.
	int ThisChar = LastChar;
	LastChar = getchar();
	return ThisChar;

}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main() {

	return 0;
}
