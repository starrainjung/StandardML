/*---------------------------------------------------
	this file implements the Lexer


---------------------------------------------------*/

#include "Lexer.h"



//ʶ�����
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

int Lexer::readNum(string NumStr,int* LastChar) {
	
	bool flag1 = 0;
	bool flag2 = 0;
	while (isdigit(*LastChar) || *LastChar == '.' || *LastChar == 'E') {
		if (*LastChar == '.') {
			if (flag1 == 0)
				flag1 = 1;
			else
				break; // show exeption!! ����һ��С����
		}
		if (*LastChar == 'E') {
			if (flag2 == 0)
				flag2 = 1;
			else
				break; // show exeption!! ����һ��E
		}
		NumStr += *LastChar;
		*LastChar = getchar();
	} 

	NumVal = strtod(NumStr.c_str(), nullptr);
	if (flag1 == 0)
		return tok_int; // int
	else
		return tok_real; // real
}

// lex string
int Lexer::readStr(bool isStr,int* LastChar) {
	*LastChar = getchar();
	if (isStr) {
		if (*LastChar == '\"') {
			StrVal = "";
			return tok_string;
		}
		StrVal = *LastChar;
		while (*LastChar != '\"')
			StrVal += getchar();
		return tok_string;
	}
	else {
		if (*LastChar == '\"')//�����ַ�����Ϊ��
			return tok_error;	
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
			else
				return tok_error;//���Ϸ����ַ�
		}
		CharVal = *LastChar;
		*LastChar = getchar();
		if (*LastChar == '\"') {
			*LastChar = getchar();
			return tok_char;
		}
		else
			return tok_error;//�ַ����ȳ���1
	}
	
}

/// gettok - Return the next token from standard input.
int Lexer::gettok() {

	static int LastChar = ' ';

	/*----------�ɼ��̼����Tab���ո�ͻ���-----------*/
	// �����ո�
	while (isspace(LastChar))
		LastChar = getchar();
	//����Tab
	while (LastChar == '\t')
		LastChar = getchar();
	//��������---Ӧ�ò���Ҫ�жϣ�getchar()�����س���ʼ��ȡ
	while (LastChar == '\n' || LastChar == '\r') 
		LastChar = getchar();

	//��ĸ���֡��ؼ���
	if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9|_|']*
		IdentifierStr = LastChar;
		while (isalnum((LastChar = getchar())) || LastChar == '_' ||
			LastChar == '\'')
			IdentifierStr += LastChar;

		//if (IdentifierStr == "def")
		//	return tok_def;
		//if (IdentifierStr == "extern")
		//	return tok_extern;

		//  if (IdentifierStr == "it")
		//     return tok_it;

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
			if (isdigit(LastChar)) {
				string NumStr = "-";
				return readNum(NumStr,&LastChar);
			}
			else return tok_identifier;
				//~���ź��棺������------ʶ��Ϊ���ţ�����------ʶ��Ϊ��ʶ����
		}

		return tok_identifier;
	}

	//����
	//�ַ���
	if (LastChar == '"') {
		readStr(true,&LastChar);
	}
	//�ַ�


	//����
	if (isdigit(LastChar)) { // Number: [0-9.]+
		string NumStr;
		return  readNum(NumStr,&LastChar);
	}

	//ע�� ���﷨��������
	//ע�⣡��ע����Ҫʶ�������ַ� �ʷ�getchar()û�취���� ֻ�ܰ�(��*�ֱ�ʶ�𲢷���ascii��
	//(**)
	//(aBC+b)



	// Otherwise, just return the character as its ascii value.
	int ThisChar = LastChar;
	LastChar = getchar();
	return ThisChar;

	//��������
	if (LastChar == '\x03')
		return tok_exit;
}


