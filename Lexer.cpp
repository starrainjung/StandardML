/*---------------------------------------------------
	this file implements the Lexer
	TODO���ֺŽ�β����(;)
---------------------------------------------------*/

#include "Lexer.h"


//ʶ�����
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
			return LexerError("illegal expression of negative '~'");// ����E��ֻ��~û������
	}
	else		
		return LexerError("illegal expression of scientific notation 'E'");// ����E��Ϊ��
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
		return LexerError("illegal expression of decimal '.'");// �������ֶ����С����
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
// NOTE: ת�����а���: '\n' '\t' '\v' '\b' '\r' '\f' '\a' 
//                    '\\' '\"' '\ �ո��Ʊ�� ���� ��ҳ \'
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
		// ���� " 
		*LastChar = getchar();
		return tok_string;
	}
	else {
		bool isSetChar = false;
		if (*LastChar == '\"')		
			return LexerError("character cannot be empty"); //�����֡�������Ϊ��
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
			
			return LexerError("length of character not 1"); //�ַ����ȳ���1 or �ַ�δ�պ�
	}
}

/// gettok - Return the next token from standard input.
int Lexer::gettok() {

	static int LastChar = ' ';

	// �����ո�,tab��������
	while (isspace(LastChar))
		LastChar = getchar();

	//��ĸ���֡��ؼ���
	if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9|_|']*
		IdentifierStr = LastChar;
		while (isalnum((LastChar = getchar())) || LastChar == '_' || 
			LastChar == '\'')
			IdentifierStr += LastChar;
		//�ų�������
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

		//ʶ������true��false������tok_bool
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

	//�������֡���������
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

		//�����ַ����� # ��ʶ��
		if (IdentifierStr == "#") {
			if (LastChar == '"')
				return readStr(false,&LastChar);
			else 
				return '#';
			//~���ź��棺��"------ʶ��Ϊ�ַ���������------ʶ��Ϊ������š�
		}

		//���ָ����� ~ ��ʶ��
		if (IdentifierStr == "~") {
			if (isdigit(LastChar)) 
				return readNum(true,&LastChar);
			else return tok_identifier;
				//~���ź��棺������------ʶ��Ϊ���ţ�����------ʶ��Ϊ��ʶ����
		}

		return tok_identifier;
	}

	//�ַ���
	if (LastChar == '"')
		readStr(true,&LastChar);

	//����
	if (isdigit(LastChar)) // Number: [0-9.]+
		return  readNum(false,&LastChar);

	//ע�� ���﷨��������
	//ע�⣡��ע����Ҫʶ�������ַ� �ʷ�getchar()û�취���� ֻ�ܰ�(��*�ֱ�ʶ�𲢷���ascii��
	//(**)��(aBC+b)
	if (LastChar == '(') {
		if ((LastChar = getchar()) != '*')
			return '(';
		else
			while (1) {
				LastChar = getchar();
				if (LastChar == '*')
					if ((LastChar = getchar()) == ')')//����ע��ֱ������
						return gettok();
					else
						continue;
				else if (LastChar == '\n')				
					return LexerError("unclosed annotation"); //����ע��δ�պ�
			}
	}

	//TODO������ֺŽ�β����(;)


	// Otherwise, just return the character as its ascii value.
	int ThisChar = LastChar;
	LastChar = getchar();
	return ThisChar;

	//��������
	if (LastChar == '\x03')
		return tok_exit;
}


