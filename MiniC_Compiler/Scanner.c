#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "Scanner.h"

char*keyword[] = { "const", "else", "if",
 "int", "return", "void", "while"
};
enum tsymbol tnum[] = {
tconst, telse, tif, tint,treturn, tvoid, twhile
};

struct tokentype scanner() {
	struct tokentype token;
	int i, j, k, num;
	char ch, id[ID_LENGTH];

	token.number = tnull;
	do {
		while (isspace(ch = getchar()));	//state 1 : skip blanks
		if (isalpha(ch)) {					//state 2 : identifier or keyword
			i = 0;
			do {
				if (i < ID_LENGTH)id[i++] = ch;
				ch = getchar();
			} while (isalnum(ch));
			id[i] = '\0';
			ungetc(ch, stdin);		//retract
			/* find the identifier in the keyword table */
			i = 0; j = NO_KEYWORDS - 1;
			do {					//binary search
				k = (i + j) / 2;
				if (strcmp(id, keyword[k]) >= 0) i = k + 1;		//비교. id가 크거나 같으면
				if (strcmp(id, keyword[k]) <= 0) j = k - 1;
			} while (i <= j);

			if ((i - 1) > j) {			//found, key word exit		//state 4 : keyword
				token.number = tnum[k];
				strcpy(token.value.id, id);
			}
			else {						//not found, identifier exit	//state 3 : identifier
				token.number = tident;
				strcpy(token.value.id, id);
			}
		}
		else if (isdigit(ch)) {			//state 5 : number
			num = 0;
			do {
				num = 10 * num + (int)(ch - '0');
				ch = getchar();
			} while (isdigit(ch));
			ungetc(ch, stdin);
			token.number = tnumber;
			token.value.num = num;
		}//number

		else switch (ch) {							// special characters
		case '/': ch = getchar();				// state 10 
			if (ch == '*') {					// text comment            
				do {
					while (ch != '*') ch = getchar();
					ch = getchar();
				} while (ch != '/');
			}
			else {
				if (ch == '/') {			// line comment    
					while (getchar() != '\n');
				}
				else {
					if (ch == '=') {
						strcpy(token.value.id, "/=");
						token.number = tdivAssign;
					}
					else {
						strcpy(token.value.id, "/");
						token.number = tdiv;
						ungetc(ch, stdin);			// retract
					}
				}
			}break;
		case '!': ch = getchar();				//state 17
			if (ch == '=') {
				strcpy(token.value.id, "!=");
				token.number = tnotequ;
			}
			else {
				strcpy(token.value.id, "!");
				token.number = tnot;
				ungetc(ch, stdin);
			} break;
		case '%': ch = getchar();				//state 20
			if (ch == '=') {
				strcpy(token.value.id, "%=");
				token.number = tmodAssign;
			}
			else {
				strcpy(token.value.id, "%");
				token.number = tmod;
				ungetc(ch, stdin);
			} break;
		case '&': ch = getchar();				//state 23
			if (ch == '&') {
				strcpy(token.value.id, "&&");
				token.number = tand;
			}
			else {
				strcpy(token.value.id, "&");
				ungetc(ch, stdin);
			}
			break;
		case '*': ch = getchar();				// state 25
			if (ch == '=') {
				strcpy(token.value.id, "*=");
				token.number = tmulAssign;
			}
			else {
				strcpy(token.value.id, "*");
				token.number = tmul;
				ungetc(ch, stdin);
			} break;
		case '+': ch = getchar();				//state 28
			if (ch == '+') {
				strcpy(token.value.id, "++");
				token.number = tinc;
			}
			else if (ch == '=') {
				strcpy(token.value.id, "+=");
				token.number = taddAssign;
			}
			else {
				strcpy(token.value.id, "+");
				token.number = tplus;
				ungetc(ch, stdin);
			} break;
		case '-': ch = getchar();				//state 32
			if (ch == '-') {
				strcpy(token.value.id, "--");
				token.number = tdec;
			}
			else if (ch == '=') {
				strcpy(token.value.id, "-=");
				token.number = tsubAssign;
			}
			else {
				strcpy(token.value.id, "-");
				token.number = tminus;
				ungetc(ch, stdin);
			} break;
		case '<': ch = getchar();				//state 36
			if (ch == '=') {
				strcpy(token.value.id, "<=");
				token.number = tlesse;
			}
			else {
				strcpy(token.value.id, "<");
				token.number = tless;
				ungetc(ch, stdin);
			} break;
		case '=': ch = getchar();				//state 39
			if (ch == '=') {
				strcpy(token.value.id, "==");
				token.number = tequal;
			}
			else {
				strcpy(token.value.id, "=");
				token.number = tassign;
				ungetc(ch, stdin);
			} break;
		case '>': ch = getchar();				//state 42
			if (ch == '=') {
				strcpy(token.value.id, ">=");
				token.number = tgreate;
			}
			else {
				strcpy(token.value.id, ">");
				token.number = tgreat;
				ungetc(ch, stdin);
			} break;
		case '|': ch = getchar();				//state 45
			if (ch == '|') {
				strcpy(token.value.id, "||");
				token.number = tor;
			}
			else {
				strcpy(token.value.id, "|");
				ungetc(ch, stdin);
			} break;
		case '(': strcpy(token.value.id, "(");
			token.number = tlparen;		//state 49
			break;
		case ')': strcpy(token.value.id, ")");
			token.number = trparen;		//state 50
			break;
		case ',': strcpy(token.value.id, ",");
			token.number = tcomma;		//state 51
			break;
		case ';': strcpy(token.value.id, ";");
			token.number = tsemicolon;	//state 52
			break;
		case '[': strcpy(token.value.id, "[");
			token.number = tlbracket;		//state 53
			break;
		case ']': strcpy(token.value.id, "]");
			token.number = trbracket;		//state 54
			break;
		case '{': strcpy(token.value.id, "{");
			token.number = tlbrace;		//state 55
			break;
		case '}': strcpy(token.value.id, "}");
			token.number = trbrace;		//state 56
			break;
		case EOF: token.number = teof;
			break;
		}// switch end 
	} while (token.number == tnull);
	return token;
}// end of scanner
/*void main()
{
	struct tokentype token;
	token = scanner();
	while (token.number != tnull) {
		if (token.number == tnumber)
			printf("Token -----> %-12d : ( %2d, %7d ) \n",
				token.value.num, token.number, token.value.num);
		else if (token.number == teof) exit(0);
		else if(token.number == tident)
			printf("Token -----> %-12s : ( %2d, %7s ) \n", token.value.id, token.number, token.value.id);
		else printf("Token -----> %-12s : ( %2d, %7d ) \n", token.value.id, token.number, 0);
		token = scanner();
	}
}*/